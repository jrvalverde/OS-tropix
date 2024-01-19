/*
 ****************************************************************
 *								*
 *			ata.c					*
 *								*
 *	Driver para dispositivos ATA				*
 *								*
 *	Versão	4.0.0, de 17.10.00				*
 *		4.6.0, de 10.08.04				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2004 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/scb.h"
#include "../h/region.h"
#include "../h/map.h"

#include "../h/pci.h"
#include "../h/frame.h"
#include "../h/intr.h"
#include "../h/disktb.h"
#include "../h/kfile.h"
#include "../h/inode.h"
#include "../h/devhead.h"
#include "../h/bhead.h"
#include "../h/ioctl.h"
#include "../h/signal.h"
#include "../h/uproc.h"
#include "../h/uerror.h"

#include "../h/scsi.h"
#include "../h/ata.h"
#include "../h/cdio.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Variáveis e Definições globais				*
 ****************************************************************
 */
#define PL		3	/* Prioridade de interrupção */
#define splata		spl3	/* Função de prioridade de interrupção */

#define	NUMTRY		10	/* Número de tentativas */

/*
 ****** Variáveis globais ***************************************
 */
entry ATA	atadata[NATAC * NATAT];
entry ATAUNIT	ataunit[NATAC];

entry DEVHEAD	atatab;		/* Cabeca da lista de dp's e do major */
entry BHEAD	ratabuf;	/* Para as operações "raw" */

entry int	ata_busy;	/* Para verificar o uso */

entry const char err_bits[] = 	"\x08" "BAD_BLOCK"
				"\x07" "ECC_ERROR"
				"\x06" "ID_CRC"
				"\x05" "NO_ID"
				/* Indicador desconhecido */
				"\x03" "ABORT"
				"\x02" "TRK_0"
				"\x01" "BAD_MARK";

/*
 ****************************************************************
 *	Protótipos de Funções					*
 ****************************************************************
 */
void		get_transfer_mode (ATA *);
int		atastrategy (BHEAD *);
void		atastart (int, ATASCB *);
void		atapi_start (ATA *);
int		atapi_internal_cmd (BHEAD *);
void		ataint (struct intr_frame);

void		ata_unit_attach (ATA *);
int		ata_command (ATA *, int, daddr_t, int, int, int);
int		ata_wait (ATA *, int);
int		atapi_command (ATA *, const ATASCB *);
int		atapi_next_phase (ATA *, ATASCB *, int);

void		ata_init_scb_list (void);
#if (0)	/*******************************************************/
ATASCB		*get_ata_scb (ATAUNIT *);
void		put_ata_scb (ATASCB *, ATAUNIT *);
#endif	/*******************************************************/

void		ata_dma_init (ATA *, int, PCIDATA *, ulong);

int		acer_dma_init (ATA *, PCIDATA *);
int		via_dma_init (ATA *, PCIDATA *);
int		intel_dma_init (ATA *, PCIDATA *, ulong);
int		intel_dma_init (ATA *, PCIDATA *, ulong);

/*
 ****************************************************************
 *	Identifica controladores ATA PCI			*
 ****************************************************************
 */
char *
pci_ata_probe (PCIDATA *tag, ulong type)
{
	ulong	class = pci_read (tag, PCI_CLASS_REG, 4);

	if ((class & PCI_CLASS_MASK) != PCI_CLASS_MASS_STORAGE)
		return (NOSTR);

	switch (type)
	{
	    case 0x05711106:
		return ("VIA 82C571 ATA controller");

	    case 0x522910B9:
		return ("Acer Aladdin IV/V (M5229) ATA controller");

	    case 0x12308086:
		return ("Intel PIIX ATA controller");

	    case 0x70108086:
		return ("Intel PIIX3 ATA controller");

	    case 0x71118086:
	    case 0x71998086:
	    case 0x84CA8086:
		return ("Intel PIIX4 ATA33 controller");

	    case 0x24218086:
		return ("Intel ICH0 ATA33 controller");

	    case 0x24118086:
	    case 0x76018086:
		return ("Intel ICH ATA66 controller");

	    case 0x244A8086:
	    case 0x244B8086:
		return ("Intel ICH2 ATA100 controller");

	    case 0x248A8086:
	    case 0x248B8086:
		return ("Intel ICH3 ATA100 controller");

	    case 0x24CA8086:
	    case 0x24CB8086:
		return ("Intel ICH4 ATA100 controller");

	    case 0x24DB8086:
	    case 0x24D18086:
		return ("Intel ICH5 ATA133 controller");

#if (0)	/*******************************************************/
	    case 0x01021078:
		return ("Cyrix 5530 ATA controller");

            case 0x55131039:
		return ("SiS 5591 ATA Controller");

	    case 0x4D33105A:
    		return ("Promise Ultra/33 ATA controller");

	    case 0x4D38105A:
    		return ("Promise Ultra/66 ATA controller");

	    case 0x74091022:
		return ("AMD 756 ATA controller");

	    case 0xc6931080:
		return ("Cypress 82c693 ATA controller");

	    case 0x00041103:
		return ("HighPoint HPT366 ATA controller");
#endif	/*******************************************************/

	} 	/* end switch (type) */

	return (NOSTR);

}	/* end pci_ata_probe */

/*
 ****************************************************************
 *	Anexa controladores ATA PCI				*
 ****************************************************************
 */
void
pci_ata_attach (PCIDATA *tag, int unit)
{
	int		bmport;
	ATA		*ap;
	ulong		type;

	if (scb.y_dma_enable == 0)
		return;

	if ((bmport = pci_read (tag, 0x20, 4) & ~3) == 0)
	{
		printf ("pci_ata_attach: NÃO obtive o PORT\n");
		return;
	}

	type  = pci_read (tag, PCI_ID_REG, 4);

	for (ap = &atadata[0]; ap < &atadata[NATAC * NATAT]; ap++)
	{
		if (ap->at_is_present && ap->at_param->support_dma)
			ata_dma_init (ap, bmport, tag, type);
	}

}	/* end pci_ata_attach */

/*
 ****************************************************************
 *	Anexa os dispositivos ATA/ATAPI				*
 ****************************************************************
 */
int
ataattach (int major)
{
	ATA		*ap;
	int		irq, unit;
	int		drive_letter = 'a';
	char		revision[12];

	/*
	 *	Primeiro controlador
	 */
	ap = &atadata[0];

	ap->at_unit		= 0;
	ap->at_port		= ATA_PRIMARY;
	ap->at_target		= 0;

	(ap+1)->at_unit		= 0;
	(ap+1)->at_port		= ATA_PRIMARY;
	(ap+1)->at_target	= 1;

	ata_unit_attach (ap);

	/*
	 *	Segundo controlador
	 */
	ap += NATAT;

	ap->at_unit		= 1;
	ap->at_port		= ATA_SECONDARY;
	ap->at_target		= 0;

	(ap+1)->at_unit		= 1;
	(ap+1)->at_port		= ATA_SECONDARY;
	(ap+1)->at_target	= 1;

	ata_unit_attach (ap);

	/*
	 *	Completa as informações que faltam
	 */
	for (ap = &atadata[0]; ap < &atadata[NATAC * NATAT]; ap++)
	{
		if (ap->at_is_present == 0)
			continue;

		/* Atribui o nome definitivo */

		ap->at_dev_nm[0]  = 'h';
		ap->at_dev_nm[1]  = 'd';
		ap->at_dev_nm[2]  = drive_letter++;
		ap->at_dev_nm[3]  = '\0';

		/* Obtém os modos de transferência */

		get_transfer_mode (ap);

		/* Imprime as características do dispositivo */

		memmove (revision, ap->at_param->revision, 8); revision[8] = '\0';

		printf
		(	"%s: <%s %s, %d MB, %s",
			ap->at_dev_nm,
			ap->at_param->model, revision,
			ap->at_disksz >> 11,
			ap->at_flags & HD_CHS_USED ? "GEO"
						   : ap->at_param->support.address48 ? "LBA48" : "LBA32"
		);

		if (ap->at_piomode >= 0)
		{
			printf
			(	", PIO%d (%d bits)",
				ap->at_piomode,
				ap->at_flags & HD_32_BITS ? 32 : 16
			);
		}
		else
		{
			printf (", PIO %d bits", ap->at_flags & HD_32_BITS ? 32 : 16);
		}

		if (ap->at_param->support_dma)
		{
			printf
			(	", DMA = %d/%d",
				ap->at_wdmamode, ap->at_udmamode
			);
		}

		if (ap->at_flags & ATAPI_DEV)
			printf (", DRQ = %d", ap->at_param->drq_type);

		printf (">\n");

	}	/* end for (percorrendo alvos) */

	/*
	 *	Prepara a interrupção
	 */
	for (ap = atadata, irq = 14, unit = 0; unit < NATAC; unit++, irq++, ap += NATAT)
	{
		if (!ap[0].at_is_present && !ap[1].at_is_present)
			continue;

		if (set_dev_irq (irq, PL, unit, ataint) < 0)
			return (-1);

		ataunit[unit].unit_irq = irq;

	} 	/* end for (percorrendo controladoras) */

   /***	atatab.v_flags &= ~V_DMA_24; ***/

	ata_init_scb_list ();

	return (0);

}	/* end ataattach */

/*
 ****************************************************************
 *	Função de "open"					*
 ****************************************************************
 */
int
ataopen (dev_t dev, int oflag)
{
	DISKTB		*up;
	ATA		*ap;

	/*
	 *	Prólogo
	 */
	if ((up = disktb_get_entry (dev)) == NODISKTB)
		return (-1);

	ap = &atadata[up->p_target];

	if ((unsigned)up->p_target >= NATAC * NATAT || !ap->at_is_present)
		{ u.u_error = ENXIO; return (-1); }

	/*
	 *	Verifica o "O_LOCK"
	 */
	if (up->p_lock || (oflag & O_LOCK) && up->p_nopen)
		{ u.u_error = EBUSY; return (-1); }

	/*
	 *	Verifica se é ATAPI
	 */
	if (ap->at_flags & ATAPI_DEV)
	{
		SCSI		*sp = ap->at_scsi;

		/* Prepara a função interna */

		sp->scsi_cmd = atapi_internal_cmd;

		/* Realiza o "open" do SCSI */

		if (scsi_open (sp, dev, oflag) < 0)
			return (-1);

		/* Trava o meio no primeiro "open", se possível */

		if (sp->scsi_nopen == 0 && sp->scsi_removable)
			scsi_ctl (sp, dev, ZIP_LOCK_UNLOCK, 1);

		sp->scsi_nopen++;

		/* Usa o tamanho de bloco devolvido pelo SCSI */

		if (ap->at_scsi->scsi_blsz == CD_RAW_BLSZ)
		{
			ap->at_scsi->scsi_blsz    = 4 * BLSZ;
			ap->at_scsi->scsi_blshift = 2 + BLSHIFT;
		}

		ap->at_blsz	= ap->at_scsi->scsi_blsz;
		ap->at_blshift	= ap->at_scsi->scsi_blshift;

	} 	/* end if (ap->at_flags & ATAPI_DEV) */

	/*
	 *	Sucesso
	 */
	up->p_nopen++;

	if (oflag & O_LOCK)
		up->p_lock = 1;

	return (0);

}	/* end ataopen */

/*
 ****************************************************************
 *	Função de close						*
 ****************************************************************
 */
int
ataclose (dev_t dev, int flag)
{
	DISKTB		*up;
	ATA		*ap;

	/*
	 *	Prólogo
	 */
	up = &disktb[MINOR (dev)]; 	ap = &atadata[up->p_target];

	/*
	 *	Atualiza os contadores
	 */
	if (--up->p_nopen <= 0)
		up->p_lock = 0;

	/*
	 *	Verifica se é ATAPI
	 */
	if (ap->at_flags & ATAPI_DEV)
	{
		SCSI		*sp = ap->at_scsi;

		if (--sp->scsi_nopen <= 0)
		{
			if (sp->scsi_removable)
				scsi_ctl (sp, dev, ZIP_LOCK_UNLOCK, 0 /* unlock */);

			scsi_close (sp, dev);
		}
	}

	return (0);

}	/* end ataclose */

/*
 ****************************************************************
 *	Executa uma operação de entrada/saida			*
 ****************************************************************
 */
int
atastrategy (BHEAD *bp)
{
	ATAUNIT		*unitp;
	int		unit;
	ATA		*ap;
	const DISKTB	*up;
	daddr_t		bn;

	up = &disktb[MINOR (bp->b_phdev)];

	ap = &atadata[up->p_target];

	unit = ap->at_unit; unitp = &ataunit[unit];

	/*
	 *	Verifica a validade do pedido
	 */
	if ((bn = bp->b_phblkno) < 0 || bn + BYTOBL (bp->b_base_count) > up->p_size)
	{
		if ((bp->b_flags & B_STAT) == 0)
		{
			bp->b_error  = ENXIO;
			bp->b_flags |= B_ERROR;

			EVENTDONE (&bp->b_done);

			return (-1);
		}
	}

	/*
	 *	Coloca o pedido na fila e inicia a operação
	 */
	bp->b_cylin	= bn + up->p_offset;
	bp->b_dev_nm	= up->p_name;
	bp->b_retry_cnt = NUMTRY;
	bp->b_ptr	= NOATASCB;

#if (0)	/*******************************************************/
	splata (); 
#endif	/*******************************************************/
	splx (irq_pl_vec[unitp->unit_irq]);

	insdsort (&ap->at_utab, bp); ata_busy++;

	if (TAS (&unitp->unit_busy) >= 0)
	{
		/* A controladora estava livre: incia a operação */

		atastart (unit, unitp->unit_scb);
#if (0)	/*******************************************************/
		atastart (unit, get_ata_scb (unitp));
#endif	/*******************************************************/
	}

	spl0 ();

	return (0);

}	/* end atastrategy */

/*
 ****************************************************************
 *	Inicia uma operação em uma unidade			*
 ****************************************************************
 */
void
atastart (int unit, ATASCB *scbp)
{
	ATAUNIT		*unitp;
	ATA		*ap;
	DEVHEAD		*dp;
	BHEAD		*bp;
	daddr_t		block;
	int		count, cmd, rw;

	unitp = &ataunit[unit];

	if ((ap = unitp->unit_active) == NOATA)
	{
		int		i;
		const DISKTB	*up;

		/*
		 *	O pedido anterior foi concluído.
		 *	Obtém o próximo pedido, circulando pelas filas
		 *	das unidades.
		 */
		for (i = 0; /* abaixo */; i++)
		{
			if (i >= NATAT)
			{
				/*
				 *	Nada a fazer...
				 */
			   /***	put_ata_scb (scbp, unitp); ***/
				CLEAR (&unitp->unit_busy);
				return;
			}

			if (++unitp->unit_target >= NATAT)
				unitp->unit_target = 0;
		
			ap = &atadata[(unit * NATAT) | unitp->unit_target];

			dp = &ap->at_utab; 

			SPINLOCK (&dp->v_lock);

			if ((bp = dp->v_first) != NOBHEAD)
				break;

			SPINFREE (&dp->v_lock);
		}

		/*
		 *	Achou algo para fazer
		 */
		SPINFREE (&dp->v_lock);

		ap->at_bp = bp;
		bp->b_ptr = scbp;

		if ((bp->b_flags & B_STAT) == 0)
		{
			/*
			 *	É um comando regular de leitura ou escrita.
			 *	Guarda os argumentos da operação total.
			 */
			up = &disktb[MINOR (bp->b_phdev)];

			scbp->scb_limit  = ap->at_multi;
			scbp->scb_blkno  = (ulong)(bp->b_phblkno + up->p_offset) >> (ap->at_blshift - BLSHIFT);
			scbp->scb_area   = bp->b_base_addr;
			scbp->scb_count  = bp->b_base_count >> ap->at_blshift;

			/*
			 *	Guarda os argumentos do fragmento corrente
			 */
			scbp->scb_frag_blkno = scbp->scb_blkno;
			scbp->scb_frag_area  = scbp->scb_area;
			scbp->scb_frag_count = MIN (scbp->scb_count, scbp->scb_limit);	/* Multibloco */
			scbp->scb_frag_incr  = scbp->scb_frag_count;

		   /***	scbp->scb_cmd	     = ...;	***/
		   /***	scbp->scb_addr	     = ...;	***/
		   /***	scbp->scb_cnt	     = ...;	***/
		   /***	scbp->scb_result     = ...;	***/

		} 	/* end if (é uma operação regular de RW) */

		unitp->unit_active = ap;	/* unidade ativa corrente */
	}
	else
	{
		/* Continua a operação anterior */

		bp   = ap->at_bp;

		if (scbp != bp->b_ptr)
			printf ("ATENÇÃO: scbp != bp->b_ptr\n");

	} 	/* end if (há operação pendente) */

	/*
	 *	Trata separadamente os dispositivos ATAPI
	 */
	if (ap->at_flags & ATAPI_DEV)
		{ atapi_start (ap); return; }

	block   = scbp->scb_frag_blkno;
	count	= scbp->scb_frag_count;

	/*
	 *	Determina o modo de operação: bloco a bloco ou DMA
	 */
	rw	= bp->b_flags & (B_READ|B_WRITE);
	cmd	= (rw == B_WRITE) ? ATA_C_WRITE : ATA_C_READ;

	bp->b_flags &= ~B_DMA;

	if (ap->at_bmport != 0)
	{
		if (ata_dma_setup (ap, scbp, rw) == 0)
		{
			bp->b_flags |= B_DMA;
			cmd = (rw == B_WRITE) ? ATA_C_WRITE_DMA : ATA_C_READ_DMA;
		}
		else
		{
			printf
			(	"%s: não foi possível usar DMA: addr = %P, nblks = %d\n",
				ap->at_dev_nm, scbp->scb_frag_area, count
			);
		}
	}

#ifdef	MSG
	if (CSWT (10))
	{
		printf
		(	"%s: %s block %d (%s), area = %P, count = %d, %s\n",
			ap->at_dev_nm,
			rw == B_WRITE ? "WRITE" : "READ",
			scbp->scb_frag_blkno,
			ap->at_flags & HD_CHS_USED ? "GEO" : "LBA",
			scbp->scb_frag_area, count,
			bp->b_flags & B_DMA ? "DMA" : (ap->at_flags & HD_32_BITS) ? "PIO 32" : "PIO 16"
		);
	}
#endif	MSG

	/*
	 *	Envia o comando para o dispositivo
	 */
	ata_command (ap, cmd, block, count, 0, ATA_IMMEDIATE);

	/*
	 *	Se estiver usando DMA, inicia a operação e retorna
	 */
	if (bp->b_flags & B_DMA)
		{ ata_dma_start (ap); return; }

	/*
	 *	Do contrário, haverá uma interrupção a cada bloco
	 */
	scbp->scb_frag_incr = 1;

	/*
	 *	Operação de leitura: aguarda a interrupção
	 */
	if (rw == B_READ)
		return;

	/*
	 *	Operação de escrita: fornece os dados
	 */
	ata_wait (ap, ATA_S_READY|ATA_S_DSC|ATA_S_DRQ);

	(*ap->at_write_port)
	(	ap->at_port + ATA_DATA,
		scbp->scb_frag_area,
		ap->at_blsz >> ap->at_pio_shift
	);

}	/* end atastart */

/*
 ****************************************************************
 *	Inicia uma operação					*
 ****************************************************************
 */
void
atapi_start (ATA *ap)
{
	int		nblk, count;
	daddr_t		blkno;
	BHEAD		*bp;
	ATASCB		*scbp;

	bp   = ap->at_bp;
	scbp = bp->b_ptr;

	if ((bp->b_flags & B_STAT) == 0)
	{
		/* É um comando regular de leitura ou escrita */

		blkno = scbp->scb_frag_blkno;
		nblk  = scbp->scb_frag_count;
		count = nblk << ap->at_blshift;

		if ((bp->b_flags & (B_READ|B_WRITE)) == B_READ)
			bp->b_cmd_txt[0] = SCSI_CMD_READ_BIG;
		else
			bp->b_cmd_txt[0] = SCSI_CMD_WRITE_BIG;

		bp->b_cmd_txt[1] = 0;
		*(long *)&bp->b_cmd_txt[2] = long_endian_cv (blkno);
		bp->b_cmd_txt[6] = 0;
		*(short *)&bp->b_cmd_txt[7] = short_endian_cv (nblk);
		bp->b_cmd_txt[9] = 0;

		scbp->scb_addr	= scbp->scb_frag_area;
		scbp->scb_cnt	= count;
	}
	else
	{
		/* É um comando de controle */

		scbp->scb_addr	= bp->b_base_addr;
		scbp->scb_cnt	= bp->b_base_count;

	} 	/* end if ((bp->b_flags & B_STAT) == 0) */

	scbp->scb_cmd	= bp->b_cmd_txt;
	scbp->scb_flags = bp->b_flags;

	/*
	 *	Inicia o comando
	 */
	if (atapi_command (ap, scbp) < 0)
	{
		printf
		(	"%s: ERRO ao tentar enviar o comando %s\n",
			ap->at_dev_nm, scsi_cmd_name (scbp->scb_cmd[0])
		);
	}

}	/* end atapi_start */

/*
 ****************************************************************
 *	Coloca na fila um comando ATAPI interno			*
 ****************************************************************
 */
int
atapi_internal_cmd (BHEAD *bp)
{
	/*
	 *	Prepara o BHEAD
	 */
	bp->b_flags |= B_STAT;
	bp->b_blkno  = bp->b_phblkno = 0;

	/*
	 *	Coloca na fila do "driver" e espera concluir
	 */
	atastrategy (bp);

	EVENTWAIT (&bp->b_done, PBLKIO);	/* Espera concluir */

	return (geterror (bp));

}	/* end atapi_internal_cmd */

/*
 ****************************************************************
 *	Interrupção						*
 ****************************************************************
 */
void
ataint (struct intr_frame frame)
{
	ATAUNIT		*unitp;
	int		unit, dmastat, rw;
	ATA		*ap;
	BHEAD		*bp;
	DEVHEAD		*dp;
	ATASCB		*scbp;

	/*
	 *	Obtém a operação em andamento
	 */
	unitp = &ataunit[unit = frame.if_unit];

	if ((ap = unitp->unit_active) == NOATA)
	{
		ap = &atadata[unit * NATAT];

		read_port (ap->at_port + ATA_ALTOFFSET + ATA_ALTSTAT);

		if (read_port (ap->at_port + ATA_STATUS) & ATA_S_ERROR)
			read_port (ap->at_port + ATA_ERROR);

		return;
	}

	bp   = ap->at_bp;
	scbp = bp->b_ptr;
	rw   = bp->b_flags & (B_READ|B_WRITE);

	if (ap->at_flags & ATAPI_DEV)
	{
		/* Processa a próxima etapa do comando ATAPI */

		if (atapi_next_phase (ap, scbp, 1) == ATA_OP_CONTINUES)
			return;

		if (bp->b_flags & B_STAT)
			goto done;

		if (scbp->scb_result != 0)
			goto io_error;
	}
	elif (bp->b_flags & B_DMA)
	{
		/* Utilizou DMA na transferência */

		if ((ata_dma_status (ap) & ATA_BMSTAT_INTERRUPT) == 0)
			return;

		dmastat = ata_dma_done (ap);

		if (dmastat & ATA_BMSTAT_ERROR)
		{
			printf ("%s: erro na transferência com DMA\n", ap->at_dev_nm);
			ap->at_bmport = 0;	/* Reverte para PIO */
			goto io_error;
		}
	}
	else
	{
		/* Não utilizou DMA */

		if (ata_wait (ap, 0) != 0)
			goto io_error;

		/* Se foi leitura, obtém o conteúdo do bloco */

		if (rw == B_READ)
		{
			if (ata_wait (ap, ATA_S_READY|ATA_S_DSC|ATA_S_DRQ) != 0)
			{
				printf
				(	"%s: TIMEOUT esperando pela leitura\n",
					ap->at_dev_nm
				);
			}

			(*ap->at_read_port)
			(	ap->at_port + ATA_DATA,
				scbp->scb_frag_area,
				ap->at_blsz >> ap->at_pio_shift
			);
		}
	}

	/*
	 *	Prepara a continuação da operação
	 */
	scbp->scb_frag_blkno += scbp->scb_frag_incr;
	scbp->scb_frag_area  += scbp->scb_frag_incr << ap->at_blshift;
	scbp->scb_frag_count -= scbp->scb_frag_incr;

	if (scbp->scb_frag_count <= 0)
	{
		/* Terminou mais um fragmento de operação */

		scbp->scb_blkno += scbp->scb_limit;
		scbp->scb_area  += scbp->scb_limit << ap->at_blshift;
		scbp->scb_count -= scbp->scb_limit;

		if (scbp->scb_count <= 0)
			goto done;	/* A operação terminou com sucesso */

		/*
		 *	Ainda há blocos a transferir: comanda o próximo
		 *	fragmento
		 */
		scbp->scb_frag_count = MIN (scbp->scb_count, scbp->scb_limit);
		atastart (unit, scbp);
		return;
	}

	/*
	 *	É um comando de vários blocos: se for uma escrita,
	 *	fornece os dados
	 */
	if (rw == B_READ)
		return;

	if (ata_wait (ap, ATA_S_READY|ATA_S_DSC|ATA_S_DRQ) != 0)
		printf ("%s: TIMEOUT esperando pela escrita\n", ap->at_dev_nm);

	(*ap->at_write_port)
	(	ap->at_port + ATA_DATA,
		scbp->scb_frag_area,
		ap->at_blsz >> ap->at_pio_shift
	);

	return;

	/*
	 *	Houve erro na operação
	 */
    io_error:
	printf
	(	"%s: ERRO de leitura/escrita: bloco = %d, erro = %b\n",
		ap->at_dev_nm, scbp->scb_frag_blkno,
		ap->at_error, err_bits
	);

	if (bp->b_retry_cnt == (NUMTRY >> 1))
	{
		printf ("%s: Passando para bloco a bloco\n", ap->at_dev_nm);
		scbp->scb_limit = 1;
	}

	if (--bp->b_retry_cnt > 0)
	{
		/* Repete a operação */

		scbp->scb_frag_blkno = scbp->scb_blkno;
		scbp->scb_frag_area  = scbp->scb_area;
		scbp->scb_frag_count = MIN (scbp->scb_count, scbp->scb_limit);

		atastart (unit, scbp);
		return;
	}

	bp->b_error  = EIO;		/* Erro irrecuperável */
	bp->b_flags |= B_ERROR;

	/*
	 *	Terminou esta operação
	 */
    done:
	dp = &ap->at_utab; 

	SPINLOCK (&dp->v_lock);

	bp = remdsort (dp); ata_busy--;

	SPINFREE (&dp->v_lock);

	bp->b_residual = 0;
	bp->b_flags    &= ~B_DMA;

	EVENTDONE (&bp->b_done);

	unitp->unit_active = NOATA; 	/* Operação finalizada */

	atastart (unit, scbp);		/* Próxima operação */

}	/* end ataint */

/*
 ****************************************************************
 *	Leitura em modo "raw"					*
 ****************************************************************
 */
int
ataread (IOREQ *iop)
{
	if (iop->io_offset_low & BLMASK || iop->io_count & BLMASK)
		u.u_error = EINVAL;
	else
		physio (iop, atastrategy, &ratabuf, B_READ, 0 /* dma */);

	return (UNDEF);

}	/* end ataread */

/*
 ****************************************************************
 *	Escrita em modo "raw"					*
 ****************************************************************
 */
int
atawrite (IOREQ *iop)
{
	if (iop->io_offset_low & BLMASK || iop->io_count & BLMASK)
		u.u_error = EINVAL;
	else
		physio (iop, atastrategy, &ratabuf, B_WRITE, 0 /* dma */);

	return (UNDEF);

}	/* end atawrite */

/*
 ****************************************************************
 *	Rotina de IOCTL						*
 ****************************************************************
 */
int
atactl (dev_t dev, int cmd, int arg, int flag)
{
	ATA		*ap;

	ap = &atadata[disktb[MINOR (dev)].p_target];

	switch (ap->at_type)
	{
	    case ATA_HD:
		if (cmd == DKISADISK)
			return (0);
		break;

	    case ATAPI_ZIP:
	    case ATAPI_CDROM:
		return (scsi_ctl (ap->at_scsi, dev, cmd, arg));

	    case ATA_NONE:
		u.u_error = ENXIO;
		return (-1);
	}

	u.u_error = EINVAL;
	return (-1);

}	/* end atactl */

/*
 ****************************************************************
 *	Inicializa a lista de SCBs				*
 ****************************************************************
 */
void
ata_init_scb_list (void)
{
	ATAUNIT		*unitp;
	ATASCB		*scbp;

	/*
	 *	Por enquanto, apenas um SCB por controlador
	 */
	for (unitp = &ataunit[0]; unitp < &ataunit[NATAC]; unitp++)
	{
		if ((scbp = malloc_byte (sizeof (ATASCB))) == NOATASCB)
			panic ("ata_init_scb_list: Não consegui alocar os SCBs");

		unitp->unit_scb = scbp;

		memclr (scbp, sizeof (ATASCB));
	}

}	/* end ata_init_scb_list */

#if (0)	/*******************************************************/
/*
 ****************************************************************
 *	Obtém um SCB						*
 ****************************************************************
 */
ATASCB *
get_ata_scb (ATAUNIT *unitp)
{
	ATASCB		*scbp;

	/*
	 *	Recebe e devolve "unitp->unit_busy" travado
	 */
	if ((scbp = unitp->unit_scb) != NOATASCB)
		unitp->unit_scb = scbp->scb_next;
	else
		panic ("get_ata_scb: lista de SCBs vazia");

	return (scbp);

}	/* end get_ata_scb */

/*
 ****************************************************************
 *	Devolve um SCB						*
 ****************************************************************
 */
void
put_ata_scb (ATASCB *scbp, ATAUNIT *unitp)
{
	/*
	 *	Recebe e devolve "unitp->unit_busy" travado
	 */
	scbp->scb_next  = unitp->unit_scb;
	unitp->unit_scb = scbp;

}	/* end put_ata_scb */
#endif	/*******************************************************/

/*
 ****************************************************************
 *	Inicializa o DMA					*
 ****************************************************************
 */
void
ata_dma_init (ATA *ap, int bmport, PCIDATA *tag, ulong type)
{
	int		status_port, ret;

	/*
	 *	Não sei bem o motivo...
	 */
	if ((ap->at_flags & ATAPI_DEV) && ap->at_param->drq_type == ATAPI_DRQT_INTR)
		return;

	if (ap->at_unit == 1)
		bmport += BM_SEC_OFFSET;	/* É o segundo controlador */

	status_port = bmport + ATA_BMSTAT_PORT;

	if (ap->at_unit == 1)
	{
		/* Verifica se pode utilizar o DMA na transferência */

		write_port
		(	read_port (status_port) & (ATA_BMSTAT_DMA_MASTER | ATA_BMSTAT_DMA_SLAVE),
			status_port
		);

		if (read_port (status_port) & ATA_BMSTAT_DMA_SIMPLEX)
		{
			printf
			(	"%s: não é possível usar DMA no controlador secundário\n",
				ap->at_dev_nm
			);
			return;
		}
	}

	ap->at_bmport = bmport;

	/*
	 *	A inicialização é específica para cada "chipset"
	 */
	switch (type & 0xFFFF)
	{
	    case 0x10B9:
		ret = acer_dma_init (ap, tag);
		break;

	    case 0x1106:
		ret = via_dma_init (ap, tag);
		break;

	    case 0x8086:
		ret = intel_dma_init (ap, tag, type);
		break;

	    default:
		ap->at_bmport = 0;
		return;

	} 	/* end switch (type) */

	/*
	 *	Imprime o modo que será utilizado
	 */
	switch (ret)
	{
	    case ATA_MDMA2:
		printf ("%s: Usando Multi-Word DMA\n", ap->at_dev_nm);
		break;

	    case ATA_UDMA2:
		printf ("%s: Usando Ultra-DMA33\n", ap->at_dev_nm);
		break;

	    case ATA_UDMA4:
		printf ("%s: Usando Ultra-DMA66\n", ap->at_dev_nm);
		break;

	    case ATA_UDMA5:
		printf ("%s: Usando Ultra-DMA100\n", ap->at_dev_nm);
		break;

	    case ATA_UDMA6:
		printf ("%s: Usando Ultra-DMA133\n", ap->at_dev_nm);
		break;

	    case 0:
		printf ("%s: DMA não configurado\n", ap->at_dev_nm);

	    default:
		ap->at_bmport = 0;
		break;

	} 	/* end switch (ret) */

}	/* end ata_dma_init */

/*
 ****************************************************************
 *	Inicia a operação por DMA				*
 ****************************************************************
 */
void
ata_dma_start (ATA *ap)
{
	write_port
	(	read_port (ap->at_bmport + ATA_BMCMD_PORT) | ATA_BMCMD_START_STOP,
		ap->at_bmport + ATA_BMCMD_PORT
	);

}	/* end ata_dma_start */

/*
 ****************************************************************
 *	Encerra uma operação por DMA				*
 ****************************************************************
 */
int
ata_dma_done (ATA *ap)
{
	int	status, bmport;

	bmport = ap->at_bmport;
	status = ata_dma_status (ap);

	write_port
	(
		read_port (bmport + ATA_BMCMD_PORT) & ~ATA_BMCMD_START_STOP,
		bmport + ATA_BMCMD_PORT
	);

	return (status);

}	/* end ata_dma_done */

/*
 ****************************************************************
 *	Obtém o estado de uma operação por DMA			*
 ****************************************************************
 */
int
ata_dma_status (ATA *ap)
{
	return (read_port (ap->at_bmport + ATA_BMSTAT_PORT));

}	/* end ata_dma_status */

/*
 ****************************************************************
 *	Programa uma transferência por DMA			*
 ****************************************************************
 */
int
ata_dma_setup (ATA *ap, ATASCB *scbp, int rw)
{
	ATAUNIT		*unitp;
	DMATB		*dma_tb_ptr;
	ulong		vaddr;
	int		count, ncount, bmport;

	/*
	 *	Consistência inicial
	 */
	vaddr  = (ulong)scbp->scb_frag_area;
	count  = scbp->scb_frag_count << ap->at_blshift;

	if (count <= 0 || (vaddr & 0x03) != 0 || (count & 0x03) != 0)
	{
		printf
		(	"%s: impossível usar DMA: vaddr = %P, count = %d\n",
			ap->at_dev_nm, vaddr, count
		);

		return (-1);
	}

	/*
	 *	Verifica se é preciso alocar a tabela de pedidos
	 */
	unitp = &ataunit[ap->at_unit];

	if ((dma_tb_ptr = unitp->unit_dma_tb) == NODMATB)
	{
		dma_tb_ptr = (DMATB *)PGTOBY (malloce (M_CORE, 1 /* página */));

		if (dma_tb_ptr == NODMATB)
		{
			printf ("%s: não foi possível alocar a tabela DMATB\n", ap->at_dev_nm);
			return (-1);
		}

		unitp->unit_dma_tb = dma_tb_ptr;

	}	/* end if (não há DMATB alocada) */

	/*
	 *	Gera a primeira entrada da tabela
	 */
	dma_tb_ptr->dma_base  = VIRT_TO_PHYS_ADDR (vaddr);
	dma_tb_ptr->dma_count = ncount = MIN (count, PGSZ - (vaddr & PGMASK));

	vaddr += ncount;
	count -= ncount;

	/*
	 *	Gera as demais entradas
	 */
	while (count > 0)
	{
		if (++dma_tb_ptr >= &unitp->unit_dma_tb[DMATB_MAX_SZ])
		{
			printf ("%s: estouro na tabela DMATB\n", ap->at_dev_nm);
			return (-1);
		}

		dma_tb_ptr->dma_base  = VIRT_TO_PHYS_ADDR (vaddr);
		dma_tb_ptr->dma_count = ncount = MIN (count, PGSZ);

		vaddr += ncount;
		count -= ncount;
	}

	dma_tb_ptr->dma_count |= DMA_EOT_BIT;	/* Última entrada */

	bmport = ap->at_bmport;

	/*
	 *	Programa o DMA
	 */
	write_port_long (VIRT_TO_PHYS_ADDR (unitp->unit_dma_tb), bmport + ATA_BMDTP_PORT);

	write_port
	(	rw == B_READ ? ATA_BMCMD_WRITE_READ : 0,
		bmport + ATA_BMCMD_PORT
	);

	write_port
	(	read_port (bmport + ATA_BMSTAT_PORT) | ATA_BMSTAT_INTERRUPT | ATA_BMSTAT_ERROR,
		bmport + ATA_BMSTAT_PORT
	);

	return (0);

}	/* end ata_dma_setup */

/*
 ****************************************************************
 *	Inicialização para o Chipset "Acer Alladin"		*
 ****************************************************************
 */
int
acer_dma_init (ATA *ap, PCIDATA *tag)
{
	ulong		word54;
	uchar		reg53;

	/*
	 *	Verifica se o cabo é apropriado para Ultra-DMA-66 ou superior
	 */
	if (ap->at_udmamode > 2 && (ap->at_flags & HD_CBLID) == 0)
	{
		printf ("%s: DMA limitado a UDMA33\n", ap->at_dev_nm);
		ap->at_udmamode = 2;
	}

	/*
	 *	Tenta primeiro Ultra-DMA-33
	 */
	if (ap->at_udmamode >= 2)
	{
		if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA2, ATA_C_F_SETXFER, 0) == 0)
		{
			word54  = pci_read (tag, 0x54, 4);
			word54 |= 0x5555;
			word54 |= (0x0A << (16 + (ap->at_unit << 3) + (ap->at_target << 2)));

			pci_write (tag, 0x54, word54, 4);
			reg53 = pci_read (tag, 0x53, 1);
			pci_write (tag, 0x53, reg53 | 0x03, 1);

			return (ATA_UDMA2);
		}
	}

	/*
	 *	Tenta o DMA regular
	 */
	if (ap->at_wdmamode >= 2 && ap->at_piomode >= 4)
	{
		if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_MDMA2, ATA_C_F_SETXFER, 0) == 0)
		{
			reg53 = pci_read (tag, 0x53, 1);
			pci_write (tag, 0x53, reg53 | 0x03, 1);

			return (ATA_MDMA2);
		}
	}

	/*
	 *	Vai mesmo usar PIO
	 */
	reg53 = pci_read (tag, 0x53, 1);
	pci_write (tag, 0x53, (reg53 & ~0x01) | 0x02, 1);

	return (0);

}	/* end acer_dma_init */

/*
 ****************************************************************
 *	Inicialização para o Chipset "VIA"			*
 ****************************************************************
 */
int
via_dma_init (ATA *ap, PCIDATA *tag)
{
	int		devno;
	const PCIDATA	*parent_tag = tag->pci_parent;

	/* set prefetch, postwrite */
	pci_write (tag, 0x41, pci_read (tag, 0x41, 1) | 0xF0, 1);

	/* set fifo configuration half'n'half */
	pci_write (tag, 0x43, (pci_read (tag, 0x43, 1) & 0x90) | 0x2A, 1);

	/* set status register read retry */
	pci_write (tag, 0x44, pci_read (tag, 0x44, 1) | 0x08, 1);

	/* set DMA read & end-of-sector fifo flush */
	pci_write (tag, 0x46, (pci_read (tag, 0x46, 1) & 0x0C) | 0xF0, 1);

	/* set sector size */
	pci_write (tag, 0x60, BLSZ, 2);
	pci_write (tag, 0x68, BLSZ, 2);

	/*
	 *	Verifica se o cabo é apropriado para Ultra-DMA-66 ou superior
	 */
	if (ap->at_udmamode > 2 && (ap->at_flags & HD_CBLID) == 0)
	{
		printf ("%s: DMA limitado a UDMA33\n", ap->at_dev_nm);
		ap->at_udmamode = 2;
	}

	/*
	 *	Prepara os DMAs
	 */
	devno = (ap->at_unit << 1) + ap->at_target;

	if
	(	parent_tag->pci_device == 0x3147 && parent_tag->pci_revid >= 0x00 ||	/* VIA 82C571 */
		parent_tag->pci_device == 0x3177 && parent_tag->pci_revid >= 0x00
	)
	{
		/* Tenta primeiro Ultra-DMA-133 */

		if (ap->at_udmamode >= 6)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA6, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xF0, 1);
				return (ATA_UDMA6);
			}
		}

		/* Tenta Ultra-DMA-100 */

		if (ap->at_udmamode >= 5)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA5, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xF1, 1);
				return (ATA_UDMA5);
			}
		}

		/* Tenta Ultra-DMA-66 */

		if (ap->at_udmamode >= 4)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA4, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xE8, 1);
				return (ATA_UDMA4);
			}
		}
	
		/* Tenta Ultra-DMA-33 */

		if (ap->at_udmamode >= 2)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA2, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xEA, 1);
				return (ATA_UDMA2);
			}
		}
	}
	elif
	(	parent_tag->pci_device == 0x0686 && parent_tag->pci_revid >= 0x00 ||	/* VIA 82C686 A */
		parent_tag->pci_device == 0x0596 && parent_tag->pci_revid >= 0x12	/* VIA 82C596 B */
	)
	{
		pci_write (tag, 0x50, pci_read (tag, 0x50, 4) | 0x070F070F, 4);   

		/* Tenta primeiro Ultra-DMA-66 */

		if (ap->at_udmamode >= 4)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA4, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xE8, 1);
				return (ATA_UDMA4);
			}
		}
	
		/* Tenta Ultra-DMA-33 */

		if (ap->at_udmamode >= 2)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA2, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xEA, 1);
				return (ATA_UDMA2);
			}
		}
	}
	elif
	(	parent_tag->pci_device == 0x0596 && parent_tag->pci_revid >= 0x00 ||	/* VIA 82C596 a */
		parent_tag->pci_device == 0x0586 && parent_tag->pci_revid >= 0x02	/* VIA 82C586 b */
	)
	{
		/* Tenta Ultra-DMA-33 */

		if (ap->at_udmamode >= 2)
		{
			if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_UDMA2, ATA_C_F_SETXFER, 0) == 0)
			{
			        pci_write (tag, 0x53 - devno, 0xC0, 1);
				return (ATA_UDMA2);
			}
		}
	}

	/*
	 *	Tenta o DMA regular
	 */
	if (ap->at_wdmamode >= 2 && ap->at_piomode >= 4)
	{
		if (ata_command (ap, ATA_C_SETFEATURES, 0, ATA_MDMA2, ATA_C_F_SETXFER, 0) == 0)
		{
		        pci_write (tag, 0x53 - devno, 0x82, 1);
		        pci_write (tag, 0x4B - devno, 0x31, 1);

			return (ATA_MDMA2);
		}
	}

	return (0);	/* Vai usar PIO */

}	/* end via_dma_init */

/*
 ****************************************************************
 *	Inicialização para os Chipsets Intel			*
 ****************************************************************
 */
int
intel_dma_init (ATA *ap, PCIDATA *tag, ulong type)
{
	int		devno, mode, command, index;
	ulong		reg40;
	uchar		reg44, reg48;
	ushort		reg4a, reg54;
	ulong		mask40 = 0, new40 = 0;
	uchar		mask44 = 0, new44 = 0;
	static uchar	timings[] =
	{
		0x00, 0x00, 0x10, 0x21,	0x23,
		0x10, 0x21, 0x23,
		0x23, 0x23, 0x23, 0x23, 0x23, 0x23
	};

	devno = (ap->at_unit << 1) + ap->at_target;

	reg40 = pci_read (tag, 0x40, 4);
	reg44 = pci_read (tag, 0x44, 1);
	reg48 = pci_read (tag, 0x48, 1);
	reg4a = pci_read (tag, 0x4a, 2);
	reg54 = pci_read (tag, 0x54, 2);

	if (ap->at_udmamode > ATA_UDMA5)
		ap->at_udmamode = ATA_UDMA5;

	if (ap->at_udmamode > 2 && !(reg54 & (0x10 << devno)))
	{
		printf ("%s: DMA limitado a UDMA33\n", ap->at_dev_nm);
		ap->at_udmamode = 2;
	}

	/*
	 *	Tenta primeiro Ultra DMA
	 */
	for (mode = ap->at_udmamode; mode > 0; mode--)
	{
		command = mode + ATA_UDMA0;

		if (ata_command (ap, ATA_C_SETFEATURES, 0, command, ATA_C_F_SETXFER, 0) == 0)
			goto got_dma;
	}

	/*
	 *	Tenta Multi-Word DMA
	 */
	for (mode = ap->at_wdmamode; mode > 0; mode--)
	{
		command = mode + ATA_MDMA0;

		if (ata_command (ap, ATA_C_SETFEATURES, 0, command, ATA_C_F_SETXFER, 0) == 0)
			goto got_dma;
	}

	return (0);	/* Vai usar PIO */

	/*
	 *	Será possível usar o DMA; finaliza a configuração
	 */
    got_dma:
	if (command >= ATA_UDMA0)
	{
		pci_write (tag, 0x48, reg48 | (0x0001 << devno), 2);
		pci_write (tag, 0x4A, (reg4a & ~(0x3 << (devno<<2))) | (0x01 + !(mode & 0x01)), 2);

		index = command - ATA_UDMA0 + 8;
	}
	else
	{
		pci_write (tag, 0x48, reg48 & ~(0x0001 << devno), 2);
		pci_write (tag, 0x4A, (reg4a & ~(0x3 << (devno << 2))), 2);

		index = command - ATA_MDMA0 + 5;
	}

	if (command >= ATA_UDMA2)
		pci_write (tag, 0x54, reg54 |  (0x00001 << devno), 2);
	else
		pci_write (tag, 0x54, reg54 & ~(0x00001 << devno), 2);

	if (command >= ATA_UDMA5)
		pci_write (tag, 0x54, reg54 |  (0x10000 << devno), 2);
	else 
		pci_write (tag, 0x54, reg54 & ~(0x10000 << devno), 2);

	reg40 &= ~0x00FF00FF; 	reg40 |= 0x40774077;

	if (ap->at_target == 0)
	{
		mask40 = 0x3300;
		new40  = timings[index] << 8;
	}
	else
	{
		mask44 = 0x0F;
		new44  = ((timings[index] & 0x30) >> 2) | (timings[index] & 0x03);
	}

	if (ap->at_unit == 1)
		{ mask40 <<= 16; new40 <<= 16; mask44 <<= 4; new44 <<= 4; }

	pci_write (tag, 0x40, (reg40 & ~mask40) | new40, 4);
	pci_write (tag, 0x44, (reg44 & ~mask44) | new44, 1);

	return (command);

}	/* end intel_dma_init */
