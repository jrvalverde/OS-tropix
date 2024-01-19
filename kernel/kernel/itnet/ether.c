/*
 ****************************************************************
 *								*
 *			ether.c					*
 *								*
 *	Fun��es relacionadas com dispositivos ETHERNET		*
 *								*
 *	Vers�o	3.0.0, de 01.12.95				*
 *		3.0.0, de 09.08.96				*
 *								*
 *	Fun��es:						*
 *		ether_init,		ether_get_ether_addr,	*
 *		ether_put_ether_addr,	ether_receive_arp_frame,*
 *		ether_send_arp_request,	ether_pr_ether_addr	*
 *								*
 *	M�dulo: N�cleo						*
 *		N�CLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 1999 NCE/UFRJ - tecle "man licen�a"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/scb.h"
#include "../h/sync.h"

#include "../h/itnet.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Vari�veis e Defini��es globais				*
 ****************************************************************
 */

/*
 ******	Estrutura do protocolo ARP ******************************
 */
typedef struct
{
	ushort	ar_header;		/* Tipo do circuito (radio, ethernet) */
	ushort	ar_proto;		/* Protocolo usado */
	char	ar_hardware_sz;		/* Tamanho do endere�o do circuito */
	char	ar_protocol_sz;		/* Tamanho do endere�o do protocolo */
	ushort	ar_op;			/* Opera��o pedida */

	ETHADDR	ar_ether_src;		/* Endere�o ethernet fonte */
	char	ar_proto_src[4];	/* Endere�o do protocolo fonte */
	ETHADDR	ar_ether_dst;		/* Endere�o ethernet destino */
	char	ar_proto_dst[4];	/* Endere�o do protocolo destino */

}	ARP;

#define	ARPHRD_ETHER	1		/* Tipo do circuito == ETHERNET */

#define	ARPOP_REQUEST	1		/* Pergunta ARP */
#define	ARPOP_REPLY	2		/* Resposta ARP */

/*
 ******	Vari�veis globais ***************************************
 */
entry ETHER	*ether_avail;		/* Lista de entradas vazias */
entry LOCK	ether_lock;		/* Sem�foro da fila de entradas */

const ETHADDR	ether_broadcast_addr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const ETHADDR	ether_null_addr      = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/*
 ****************************************************************
 *	Inicializa a tabela ETHER				*
 ****************************************************************
 */
void
ether_init (void)
{
	register ETHER		*ep;
	register const SCB	*sp = &scb;

	/*
	 *	Constr�i a lista LINKada de tr�s para frente
	 */
   /***	SPINLOCK (&ether_lock); ***/

	ep = &sp->y_ether[sp->y_n_ether - 1]; ep->e_next = NOETHER;

	for (ep--; ep >= &sp->y_ether[0]; ep--)
		ep->e_next = ep + 1;

	ether_avail = ep + 1;

   /***	SPINFREE (&ether_lock); ***/

}	/* end ether_init */

/*
 ****************************************************************
 *	Procura uma entrada dado o endere�o IP			*
 ****************************************************************
 */
int
ether_get_ether_addr (IPADDR ip_addr, ETHADDR *ether_p)
{
	register ETHER		*ap, *ep;
	register SCB		*sp = &scb;

	/*
	 *	Procura na lista
	 */
	SPINLOCK (&ether_lock);

	for
	(	ap  = NOETHER, ep = sp->y_ether_first;
		ep != NOETHER;
		ap  = ep, ep = ep->e_next
	)
	{
		if (ep->e_ip_addr == ip_addr)
		{
			/*
			 *	Coloca sempre no in�cio da lista
			 */
			if (ep != sp->y_ether_first)
			{
				ap->e_next = ep->e_next;

				ep->e_next = sp->y_ether_first;
				sp->y_ether_first = ep;
			}

			*ether_p = ep->e_ether_addr;

			SPINFREE (&ether_lock);
			return (0);
		}
	}

	/*
	 *	N�o achou
	 */
	SPINFREE (&ether_lock);
	return (-1);

}	/* end ether_get_ether_addr */

/*
 ****************************************************************
 *	Insere uma entrada (endere�o IP, endere�o ETHERNET)	*
 ****************************************************************
 */
void
ether_put_ether_addr (IPADDR ip_addr, const ETHADDR *ether_p, int insert)
{
	register ETHER		*ap, *ep;
	register SCB		*sp = &scb;

	/*
	 *	Verifica se por acaso o endere�o IP � 0.0.0.0
	 */
	if (ip_addr == 0)
	{
#ifdef	MSG
		if (itscb.it_list_info)
			printf ("%g: Tentando inserir endere�o 0.0.0.0\n");
#endif	MSG
		return;
	}

	/*
	 *	Procura na lista
	 */
	SPINLOCK (&ether_lock);

	for
	(	ap  = NOETHER, ep = sp->y_ether_first;
		ep != NOETHER;
		ap  = ep, ep = ep->e_next
	)
	{
		/* Se j� estava, atualiza o endere�o ethernet */

		if (ep->e_ip_addr == ip_addr)
		{
			ep->e_ether_addr = *ether_p;
			SPINFREE (&ether_lock);
			return;
		}
	}

	/*
	 *	Se "insert" for NULO, apenas atualiza o endere�o ethernet
	 */
	if (!insert)
		{ SPINFREE (&ether_lock); return; }

	/*
	 *	N�o achou, mas tem entrada livre
	 *	(Insere no in�cio da lista)
	 */
	if ((ep = ether_avail) != NOETHER)
	{
		ether_avail = ep->e_next;

		ep->e_ip_addr	 = ip_addr;
		ep->e_ether_addr = *ether_p;

		ep->e_next = sp->y_ether_first;
		sp->y_ether_first = ep;

		SPINFREE (&ether_lock);
		return;
	}

	/*
	 *	N�o achou, mas N�O mais tem entrada livre
	 *	(Altera a �ltima entrada da lista)
	 */
	if (ap == NOETHER)
		{ printf ("%g: Lista vazia (?)\n"); SPINFREE (&ether_lock); return; }

	ap->e_ip_addr	 = ip_addr;
	ap->e_ether_addr = *ether_p;

	SPINFREE (&ether_lock);

}	/* end ether_put_ether_addr */

/*
 ****************************************************************
 *	Processa pacotes ARP recebidos				*
 ****************************************************************
 */
void
ether_receive_arp_frame (register ITBLOCK *bp)
{
	register ARP		*ap;
	register const ITSCB	*ip = &itscb;
	register ROUTE		*rp;
	register ETH_H		*ep;
	IPADDR			ip_src_addr, ip_dst_addr;
	int			ar_header, ar_proto, ar_op;
	int			insert = 0, we_are_target = 0;

	/*
	 *	Converte o formato, se for o caso
	 */
	ap = (ARP *)(bp->it_u_area + sizeof (ETH_H));

	ar_header = ENDIAN_SHORT (ap->ar_header);
	ar_proto  = ENDIAN_SHORT (ap->ar_proto);
	ar_op	  = ENDIAN_SHORT (ap->ar_op);

	ip_src_addr = ENDIAN_LONG (*(IPADDR *)ap->ar_proto_src);
	ip_dst_addr = ENDIAN_LONG (*(IPADDR *)ap->ar_proto_dst);

	/*
	 *	Verifica se � arp ETHERNET (ou outros)
	 */
	if (ar_header != ARPHRD_ETHER)
	{
#ifdef	MSG
		if (ip->it_report_error)
			printf ("%g: Recebendo ARP de cabe�alho %04X desconhecido\n", ar_header);
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Verifica se o ARP se refere ao protocolo IP
	 */
	if (ar_proto != ETHERTYPE_IP)
	{
#ifdef	MSG
		if (ip->it_report_error)
			printf ("%g: Recebendo ARP de protocolo %04X desconhecido\n", ar_proto);
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Verifica se o tamanho do pacote � suficiente
	 */
	if (bp->it_u_count < sizeof (ETH_H) + sizeof (ARP))
	{
#ifdef	MSG
		if (ip->it_report_error)
			printf ("%g: Recebendo ARP com tamanho %d insuficiente\n", bp->it_u_count);
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Verifica se o tamanho do endere�o de HARDWARE confere
	 */
	if (ap->ar_hardware_sz != sizeof (ETHADDR))
	{
#ifdef	MSG
		if (ip->it_report_error)
		{
			printf
			(	"%g: Recebendo ARP com tamanho de endere�o de HARDWARE %d inv�lido\n",
				ap->ar_hardware_sz
			);
		}
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Verifica se o tamanho do endere�o de SOFTWARE confere
	 */
	if (ap->ar_protocol_sz != sizeof (IPADDR))
	{
#ifdef	MSG
		if (ip->it_report_error)
		{
			printf
			(	"%g: Recebendo ARP com tamanho de endere�o de SOFTWARE %d inv�lido\n",
				ap->ar_protocol_sz
			);
		}
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Verifica se somos o "Target Protocol Address"
	 */
	for (rp = scb.y_route; /* abaixo */; rp++)
	{
		if (rp->r_dev_nm[0] == '\0')
			break;

		if (rp->r_ether_dev == 0)
			continue;

		if (rp->r_gateway_addr == ip_src_addr)
			insert++;

		if (rp->r_my_addr == ip_dst_addr)
			{ we_are_target++; insert++; }
	}

	/*
	 *	Insere o endere�o ethernet obtido na tabela
	 */
	ether_put_ether_addr (ip_src_addr, &ap->ar_ether_src, insert);

	/*
	 *	Se n�o formos o destino, n�o h� mais o que fazer
	 */
	if (!we_are_target)
		{ put_it_block (bp); return; }

	/*
	 *	Analisa se � um pedido ou resposta
	 */
	switch (ar_op)
	{
	    case ARPOP_REQUEST:		/* Responde abaixo */
#ifdef	MSG
		if (ip->it_list_info)
		{
			printf ("%g: Recebi PEDIDO ARP para ");
			pr_itn_addr (ip_dst_addr);
			printf (" de ");
			pr_itn_addr (ip_src_addr);
			printf ("\n");
		}
#endif	MSG
		break;

	    case ARPOP_REPLY:		/* J� fez tudo */
#ifdef	MSG
		if (ip->it_list_info)
		{
			printf ("%g: Recebi RESPOSTA ARP (");
			pr_itn_addr (ip_src_addr);
			printf (" => ");
			ether_pr_ether_addr (&ap->ar_ether_src);
			printf (")\n");
		}
#endif	MSG
		put_it_block (bp); return;

	    default:			/* Erro */
#ifdef	MSG
		if (ip->it_report_error)
			printf ("%g: Recebendo ARP opera��o %d inv�lida\n", ar_op);
#endif	MSG
		put_it_block (bp); return;
	}

	/*
	 *	Prepara a resposta (Aproveita o mesmo bloco)
	 */
	if ((rp = get_route_entry (ip_src_addr)) == NOROUTE)
	{
#ifdef	MSG
		if (ip->it_report_error)
		{
			printf ("%g: N�o sei rotear a resposta ARP para o endere�o IP ");
			pr_itn_addr (ip_src_addr);
			printf ("\n");
		}
#endif	MSG
		put_it_block (bp); return;
	}

	bp->it_route = rp;

	/*
	 *	Pequena consist�ncia
	 */
	if (ip_dst_addr != rp->r_my_addr)
	{
#ifdef	MSG
		if (ip->it_report_error)
		{
			printf ("%g: Meu endere�o IP da resposta ARP n�o confere: ");
			pr_itn_addr (ip_dst_addr);
			printf (" :: ");
			pr_itn_addr (rp->r_my_addr);
			printf ("\n");
		}
#endif	MSG
	}

	/*
	 *	Completa os diversos campos
	 */
	ep = (ETH_H *)bp->it_u_area;

	ap->ar_ether_dst = ap->ar_ether_src;	/* Para facilitar */

	ep->it_ether_dst = ap->ar_ether_src;
	ep->it_ether_src = rp->r_my_ether_addr;
   /***	ep->it_type = ENDIAN_SHORT (ETHERTYPE_ARP) ***/

   /***	ap->ar_header = ENDIAN_SHORT (ARPHRD_ETHER); ***/
   /***	ap->ar_proto  = ENDIAN_SHORT (ETHERTYPE_IP); ***/
   /***	ap->ar_hardware_sz = sizeof (ETHADDR); ***/
   /***	ap->ar_protocol_sz = sizeof (IPADDR); ***/
	ap->ar_op     = ENDIAN_SHORT (ARPOP_REPLY);

	ap->ar_ether_src = rp->r_my_ether_addr;
	*(IPADDR *)ap->ar_proto_src = ENDIAN_LONG (rp->r_my_addr);
   /***	ap->ar_ether_dst = ... /* Acima ***/
	*(IPADDR *)ap->ar_proto_dst = ENDIAN_LONG (ip_src_addr);

	/*
	 *	Envia o pacote
	 */
	bp->it_free_after_IO = 1;
	bp->it_ether_header_ready = 1;	/* Cabe�alho ETHERNET j� pronto */

   /***	bp->it_u_area   = ep; ***/
	bp->it_u_count  = sizeof (ETH_H) + sizeof (ARP);

	route_frame (bp);

#ifdef	MSG
	if (ip->it_list_info)
	{
		printf ("%g: Respondendo Pedido ARP de ");
		pr_itn_addr (ip_src_addr);
		printf ("\n");
	}
#endif	MSG

}	/* end ether_receive_arp_frame */

/*
 ****************************************************************
 *	Envia um pedido ARP					*
 ****************************************************************
 */
int
ether_send_arp_request (ITBLOCK *data_bp, IPADDR ip_addr)
{
	register ITBLOCK	*bp;
	register const ROUTE	*rp;
	register ETH_H		*ep;
	register ARP		*ap;

	/*
	 *	Usa um novo ITBLOCK
	 */
	if ((bp = get_it_block (IT_OUT_CTL)) == NOITBLOCK)
	{
#ifdef	MSG
		printf ("%g: N�O obtive bloco\n");
#endif	MSG
		return (-1);
	}

#if (0)	/*******************************************************/
	int			delay;

	delay = data_bp->it_wait_for_arp ? IT_DELAY : IT_NODELAY;

	if ((bp = get_it_block (delay)) == NOITBLOCK)
		return (-1);
#endif	/*******************************************************/

	rp = bp->it_route = data_bp->it_route;

	/*
	 *	Completa o cabe�alho ETHERNET
	 */
	ep = (ETH_H *)bp->it_frame;

	ep->it_ether_dst = ether_broadcast_addr;
	ep->it_ether_src = rp->r_my_ether_addr;
	ep->it_type = ENDIAN_SHORT (ETHERTYPE_ARP);

	/*
	 *	Completa o pedido ARP
	 */
	ap = (ARP *)((int)ep + sizeof (ETH_H));

	ap->ar_header = ENDIAN_SHORT (ARPHRD_ETHER);
	ap->ar_proto  = ENDIAN_SHORT (ETHERTYPE_IP);
	ap->ar_hardware_sz = sizeof (ETHADDR);
	ap->ar_protocol_sz = sizeof (IPADDR);
	ap->ar_op     = ENDIAN_SHORT (ARPOP_REQUEST);

	ap->ar_ether_src = rp->r_my_ether_addr;
	*(IPADDR *)ap->ar_proto_src = ENDIAN_LONG (rp->r_my_addr);
	ap->ar_ether_dst = ether_null_addr;	/* A inc�gnita */
	*(IPADDR *)ap->ar_proto_dst = ENDIAN_LONG (ip_addr);

	/*
	 *	Envia o pacote
	 */
	bp->it_free_after_IO = 1;
   /***	bp->it_ether_header_ready = 1; ***/

	bp->it_u_area   = ep;
	bp->it_u_count  = sizeof (ETH_H) + sizeof (ARP);

#ifdef	MSG
	if (itscb.it_list_info)
		{ printf ("%g: Enviando ARP para obter endere�o de "); pr_itn_addr (ip_addr); printf ("\n"); }
#endif	MSG

	route_frame (bp);

	return (0);

}	/* end ether_send_arp_request */

/*
 ****************************************************************
 *	Imprime um endere�o ETHERNET				*
 ****************************************************************
 */
void
ether_pr_ether_addr (const ETHADDR *ep)
{
	register const char	*cp = (char *)ep;

	printf
	(	"%02X%02X %02X%02X %02X%02X",
		cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]
	);

}	/* end pr_itn_addr */
