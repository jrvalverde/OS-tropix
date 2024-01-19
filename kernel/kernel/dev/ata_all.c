/*
 ****************************************************************
 *								*
 *			ata_all.c				*
 *								*
 *	Rotinas auxiliares para ATA/ATAPI (kernel e boot2)	*
 *								*
 *	Versão	3.2.3, de 22.02.00				*
 *		4.8.0, de 09.03.05				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2005 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */
#undef	DEBUG

#ifdef	BOOT

#include <common.h>
#include <bhead.h>

#include <scsi.h>
#include <ata.h>

#include "../../boot/boot2/h/proto.h"

#else

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/devhead.h"
#include "../h/bhead.h"

#include "../h/scsi.h"
#include "../h/ata.h"

#include "../h/extern.h"
#include "../h/proto.h"

#endif

/*
 ******	Protótipos de funções ***********************************
 */
void		ata_unit_attach (ATA *);
int		ata_getparam (ATA *, int);
void		ata_disk_attach (ATA *);
void		ata_atapi_attach (ATA *);

int		atapi_process_phases (ATA *, ATASCB *);
int		atapi_command (ATA *, ATASCB *);
int		atapi_next_phase (ATA *, ATASCB *, int);
void		atapi_read (ATA *, ATASCB *);
void		atapi_write (ATA *, ATASCB *);

int		ata_command (ATA *, int, daddr_t, int, int, int);
int		ata_wait (ATA *, int);

void		ata_bswap (char *, int);
void		ata_btrim (char *, int);
void		ata_bpack (char *, char *, int);
int		ata_disk_version (int);

/*
 ****************************************************************
 *	Verifica a existência de um controlador			*
 ****************************************************************
 */
void
ata_unit_attach (ATA *ap)
{
	char		lsb, msb, ostat0, ostat1;
	char		stat0 = 0, stat1 = 0;
	int		port = ap->at_port, mask = 0, timeout;
	int		devices = 0;

	/*
	 *	Procura o alvo mestre
	 */
	write_port (ATA_D_IBM|ATA_MASTER, port + ATA_DRIVE);
	DELAY(10);
	ostat0 = read_port (port + ATA_STATUS);

	if ((ostat0 & 0xF8) != 0xF8 && ostat0 != 0xA5)
		{ stat0 = ATA_S_BUSY; mask |= 0x01; }

	/*
	 *	Procura o alvo escravo
	 */
	write_port (ATA_D_IBM|ATA_SLAVE, port + ATA_DRIVE);
	DELAY(10);
	ostat1 = read_port (port + ATA_STATUS);

	if ((ostat1 & 0xF8) != 0xF8 && ostat1 != 0xA5)
		{ stat1 = ATA_S_BUSY; mask |= 0x02; }

	if (mask == 0)		/* Nada achou */
		return;

	/*
	 *	Inicializa a controladora
	 */
	write_port (ATA_D_IBM|ATA_MASTER, port + ATA_DRIVE);
	DELAY (10);
	write_port (ATA_A_IDS|ATA_A_RESET, port + ATA_ALTOFFSET + ATA_ALTSTAT);
	DELAY (10000); 
	write_port (ATA_A_IDS, port + ATA_ALTOFFSET + ATA_ALTSTAT);
	DELAY (100000);
	read_port (port + ATA_ERROR);

	/*
	 *	Espera ficar novamente desocupada
	 */
	for (timeout = 0; timeout < 310000; timeout++)
	{
		if (stat0 & ATA_S_BUSY)
		{
			write_port (ATA_D_IBM|ATA_MASTER, port + ATA_DRIVE);
			DELAY (10);
			stat0 = read_port (port + ATA_STATUS);

			if (!(stat0 & ATA_S_BUSY))
			{
				/* Procura a assinatura ATAPI */

				lsb = read_port (port + ATA_CYL_LSB);
				msb = read_port (port + ATA_CYL_MSB);

				if (lsb == ATAPI_MAGIC_LSB && msb == ATAPI_MAGIC_MSB)
					devices |= ATA_ATAPI_MASTER;
			}
		}

		if (stat1 & ATA_S_BUSY)
		{
			write_port (ATA_D_IBM|ATA_SLAVE, port + ATA_DRIVE);
			DELAY (10);
			stat1 = read_port (port + ATA_STATUS);

			if (!(stat1 & ATA_S_BUSY))
			{
				/* Procura a assinatura ATAPI */

				lsb = read_port (port + ATA_CYL_LSB);
				msb = read_port (port + ATA_CYL_MSB);

				if (lsb == ATAPI_MAGIC_LSB && msb == ATAPI_MAGIC_MSB)
					devices |= ATA_ATAPI_SLAVE;
			}
		}

		if (mask == 1 && !(stat0 & ATA_S_BUSY))	/* Esperando apenas pelo mestre */
			break;

		if (mask == 2 && !(stat1 & ATA_S_BUSY))	/* Esperando apenas pelo escravo */
			break;

		if (mask == 3 && !(stat0 & ATA_S_BUSY) && !(stat1 & ATA_S_BUSY)) /* Ambos */
			break;

		DELAY (100);
	}

	DELAY(10);
	write_port (ATA_A_4BIT, port + ATA_ALTOFFSET + ATA_ALTSTAT);

	if (stat0 & ATA_S_BUSY)
		mask &= ~0x01;

	if (stat1 & ATA_S_BUSY)
		mask &= ~0x02;

	if (!mask)
		return;

	/*
	 *	Encontra (ou não) os dispositivos ATA
	 */
	if (mask & 0x01 && ostat0 != 0x00 && !(devices & ATA_ATAPI_MASTER))
	{
		write_port (ATA_D_IBM|ATA_MASTER, port + ATA_DRIVE);
		DELAY (10);
		write_port (0x58, port + ATA_ERROR);
		write_port (0xA5, port + ATA_CYL_LSB);
		lsb = read_port (port + ATA_ERROR);
		msb = read_port (port + ATA_CYL_LSB);

		if (lsb != 0x58 && msb == 0xA5)
			devices |= ATA_ATA_MASTER;
	}

	if (mask & 0x02 && ostat1 != 0x00 && !(devices & ATA_ATAPI_SLAVE))
	{
		write_port (ATA_D_IBM|ATA_SLAVE, port + ATA_DRIVE);
		DELAY (10);
		write_port (0x58, port + ATA_ERROR);
		write_port (0xA5, port + ATA_CYL_LSB);
		lsb = read_port (port + ATA_ERROR);
		msb = read_port (port + ATA_CYL_LSB);

		if (lsb != 0x58 && msb == 0xA5)
			devices |= ATA_ATA_SLAVE;
	}

	/*
	 *	Anexa o alvo escravo, se encontrado
	 */
	if (devices & ATA_ATA_SLAVE)
	{
		if (ata_getparam (ap + 1, ATA_C_ATA_IDENTIFY))
			devices &= ~ATA_ATA_SLAVE;
		else
			ata_disk_attach (ap + 1);
	}

	if (devices & ATA_ATAPI_SLAVE)
	{
		if (ata_getparam (ap + 1, ATA_C_ATAPI_IDENTIFY))
			devices &= ~ATA_ATAPI_SLAVE;
		else
			ata_atapi_attach (ap + 1);
	}

	/*
	 *	Anexa o alvo mestre, se encontrado
	 */
	if (devices & ATA_ATA_MASTER)
	{
		if (ata_getparam (ap, ATA_C_ATA_IDENTIFY))
			devices &= ~ATA_ATA_MASTER;
		else
			ata_disk_attach (ap);
	}

	if (devices & ATA_ATAPI_MASTER)
	{
		if (ata_getparam (ap, ATA_C_ATAPI_IDENTIFY))
			devices &= ~ATA_ATAPI_MASTER;
		else
			ata_atapi_attach (ap);
	}

}	/* end ata_unit_attach */

/*
 ****************************************************************
 *	Envia o comando IDENTIFY para um alvo			*
 ****************************************************************
 */
int
ata_getparam (ATA *ap, int command)
{
#if (0)	/*******************************************************/
	int		retry = 0, wanted;
	int		bits;
#endif	/*******************************************************/
	int		retry, wanted, bits;
	ATAPARAM	tst_ataparam, *atp;

	if ((ap->at_param = atp = malloc_byte (sizeof (ATAPARAM))) == NOATAPARAM)
		{ printf ("hd[%d,%d]: memória esgotada\n", ATA_DEVNO); return (-1); }

	/*
	 *	Prepara os bits esperados
	 */
	if (command == ATA_C_ATAPI_IDENTIFY)
		wanted = ATA_S_DRQ;
	else
		wanted = ATA_S_READY|ATA_S_DSC|ATA_S_DRQ;

	/*
	 *	Determina se pode usar PIO de 32 bits
	 */
	for (bits = 16; bits <= 32; bits += 16)
	{
		/* Repete para algumas unidades */

		for (retry = 0; /* abaixo */; retry++)
		{
			if (retry >= 8)
			{
				printf
				(	"hd[%d,%d]: Excedido o número de tentativas do comando IDENTIFY\n",
					ATA_DEVNO
				);

				return (-1);
			}

			if (ata_command (ap, command, 0, 0, 0, ATA_IMMEDIATE) < 0)
				printf ("hd[%d,%d]: O comando IDENTIFY falhou\n", ATA_DEVNO);

			if (ata_wait (ap, wanted) == 0)
				break;
		}

#if (0)	/*******************************************************/
		do
		{
			if (ata_command (ap, command, 0, 0, 0, 0) < 0)
			{
				printf ("hd[%d,%d]: O comando IDENTIFY falhou\n", ATA_DEVNO);
				return (-1);
			}

			if (retry++ > 4)
			{
				printf
				(	"hd[%d,%d]: Excedido o número de tentativas do comando IDENTIFY\n",
					ATA_DEVNO
				);
				return (-1);
			}

		}
		while (ata_wait (ap, wanted));
#endif	/*******************************************************/

		/* Lê a estrutura */

		if (bits == 16)
		{
			read_port_string_short
			(
				ap->at_port + ATA_DATA,
				atp,
				sizeof (ATAPARAM) / sizeof (short)
			);
		}
		else
		{
			read_port_string_long
			(
				ap->at_port + ATA_DATA,
				&tst_ataparam,
				sizeof (ATAPARAM) / sizeof (long)
			);
		}
	}

#ifndef BOOT
	if (memeq (&tst_ataparam, atp, sizeof (ATAPARAM)))
	{
		ap->at_flags	 |= HD_32_BITS;
		ap->at_read_port  = read_port_string_long;
		ap->at_write_port = write_port_string_long;
		ap->at_pio_shift  = 2;
	}
	else
#endif BOOT
	{
		ap->at_read_port  = read_port_string_short;
		ap->at_write_port = write_port_string_short;
		ap->at_pio_shift  = 1;
	}

	/*
	 *	Ajeita alguns campos ...
	 */
	if
	(	command == ATA_C_ATA_IDENTIFY ||
		!((atp->model[0] == 'N' && atp->model[1] == 'E') ||
		(atp->model[0] == 'F' && atp->model[1] == 'X') ||
		(atp->model[0] == 'P' && atp->model[1] == 'i'))
	)
		ata_bswap (atp->model, sizeof (atp->model));

	ata_btrim (atp->model,    sizeof (atp->model));
	ata_bpack (atp->model,    atp->model, sizeof (atp->model));

	ata_bswap (atp->revision, sizeof (atp->revision));
	ata_btrim (atp->revision, sizeof (atp->revision));
	ata_bpack (atp->revision, atp->revision, sizeof (atp->revision));

#ifdef	DEBUG
	printf ("hd[%d,%d]: ata_get_param = %s %s\n", ATA_DEVNO, atp->model, atp->revision);
#endif	DEBUG

	return (0);

}	/* end ata_getparam */

#ifndef	BOOT
/*
 ****************************************************************
 *	Obtém o modo de transferência				*
 ****************************************************************
 */
void
get_transfer_mode (ATA *ap)
{
	ATAPARAM	*atp = ap->at_param;

	ap->at_piomode  = -1;
	ap->at_wdmamode = -1;
	ap->at_udmamode = -1;

	if (atp->atavalid & ATA_FLAG_64_70)
	{
		if   (atp->apiomodes & 2)
			ap->at_piomode = 4;
		elif (atp->apiomodes & 1)
			ap->at_piomode = 3;
	}
	else
	{
		ap->at_piomode = atp->retired_piomode;
	}

	if   (atp->mwdmamodes & 4)
		ap->at_wdmamode = 2;
	elif (atp->mwdmamodes & 2)
		ap->at_wdmamode = 1;
	elif (atp->mwdmamodes & 1)
		ap->at_wdmamode = 0;

	if (atp->atavalid & ATA_FLAG_88)
	{
		if   (atp->udmamodes & 0x40)
			ap->at_udmamode = 6;
		elif (atp->udmamodes & 0x20)
			ap->at_udmamode = 5;
		elif (atp->udmamodes & 0x10)
			ap->at_udmamode = 4;
		elif (atp->udmamodes & 8)
			ap->at_udmamode = 3;
		elif (atp->udmamodes & 4)
			ap->at_udmamode = 2;
		elif (atp->udmamodes & 2)
			ap->at_udmamode = 1;
		elif (atp->udmamodes & 1)
			ap->at_udmamode = 0;
	}

}	/* end get_transfer_mode */
#endif

/*
 ****************************************************************
 *	Anexa um disco rígido ATA				*
 ****************************************************************
 */
void
ata_disk_attach (ATA *ap)
{
	ulong		lbasize;
	ATAPARAM	*atp = ap->at_param;

	ap->at_type	  = ATA_HD;
	ap->at_is_present = 1;

	ap->at_cyl	  = atp->cylinders;
	ap->at_head	  = atp->heads;
	ap->at_sect	  = atp->sectors;
	ap->at_disksz	  = ap->at_cyl * ap->at_head * ap->at_sect;

	ap->at_blsz	  = BLSZ;
	ap->at_blshift	  = BLSHIFT;
	ap->at_multi	  = ATA_MAX_BL;

	lbasize		  = atp->lba_size;
	
	/*
	 *	Verifica se este disco deve usar o endereçamento geométrico
	 */
	if
	(	lbasize == 0 || ata_disk_version (atp->version_major) == 0 || 
		(atp->atavalid & ATA_FLAG_54_58) == 0
	)
		ap->at_flags |= HD_CHS_USED;

	if (atp->hwres_cblid)
		ap->at_flags |= HD_CBLID;

	if (atp->support_queueing)
		ap->at_flags |= HD_TAG_CAP;

	/*
	 *	Usa o LBA de 28 bits, se válido
	 */
	if (atp->cylinders == 16383 && ap->at_disksz < lbasize)
	{
		ap->at_disksz = lbasize;
		ap->at_cyl    = ap->at_disksz / (ap->at_head * ap->at_sect);
	}

	/*
	 *	Liga o CACHE, se não for o "default"
	 */
	if (ata_command (ap, ATA_C_SETFEATURES, 0, 0, ATA_C_F_ENAB_RCACHE, 0))
		printf ("hd[%d,%d]: Não consegui ligar o CACHE do disco\n", ATA_DEVNO);

}	/* end ata_disk_attach */

/*
 ****************************************************************
 *	Anexa um alvo ATAPI					*
 ****************************************************************
 */
void
ata_atapi_attach (ATA *ap)
{
	SCSI		  *sp;
	ATAPARAM	  *atp = ap->at_param;
	static const char zip_id[] = "IOMEGA ZIP 100";

	/*
	 *	Obtém o tamanho (global) do comando SCSI
	 */
	switch (atp->packet_size)
	{
	    case ATAPI_PSIZE_12:
		ap->at_cmdsz = 12 / sizeof (short);
		break;

	    case ATAPI_PSIZE_16:
		ap->at_cmdsz = 16 / sizeof (short);
		break;

	    default:
		printf
		(	"hd[%d,%d]: Tamanho %d inválido do comando SCSI\n",
			ATA_DEVNO, atp->packet_size
		);
		return;

	}	/* end switch */

	/*
	 *	Examina o tipo
	 */
	switch (atp->type)
	{
	    case ATAPI_TYPE_CDROM:             /* CD-ROM */
		ap->at_type	= ATAPI_CDROM;
	   	ap->at_cyl	= ap->at_head = ap->at_sect = 1;
		ap->at_multi	= ATA_MAX_BL;

		ap->at_blsz	= 4 * BLSZ;
		ap->at_blshift	= 2 + BLSHIFT;
		break;

	    case ATAPI_TYPE_DIRECT:            /* ZIP ? */
		if
		(	atp->removable &&
			memeq (atp->model, zip_id, sizeof (zip_id) - 1)
		)
		{
			ap->at_type	= ATAPI_ZIP;
#ifdef	BOOT
		   	ap->at_cyl	= ap->at_bios_cyl  = 96;
			ap->at_head	= ap->at_bios_head = 64;
			ap->at_sect	= ap->at_bios_sect = 32;
#else
		   	ap->at_cyl	= 96;
			ap->at_head	= 64;
			ap->at_sect	= 32;
#endif
			ap->at_multi	= 64;

			ap->at_blsz	= BLSZ;
			ap->at_blshift	= BLSHIFT;

			break;
		}

		/* continua no caso abaixo */

	    default:
		printf ("hd[%d,%d]: Dispositivo ATAPI não suportado: %02X\n", ATA_DEVNO, atp->type);
		return;

	}	/* end switch (atp->type) */

	/*
	 *	Aloca a estrutura SCSI
	 */
	if ((ap->at_scsi = sp = malloc_byte (sizeof (SCSI))) == NOSCSI)
		return;

	memclr (sp, sizeof (SCSI));

	/*
	 *	Preenche o restante das estruturas
	 */
	ap->at_is_present = 1;
	ap->at_flags	 |= ATAPI_DEV;

	ap->at_disksz	  = ap->at_cyl * ap->at_head * ap->at_sect;

	sp->scsi_is_atapi = 1;

}	/* end ata_atapi_attach */

#ifdef	BOOT
/*
 ****************************************************************
 *	Envia o comando ATAPI, processando todas as suas fases	*
 ****************************************************************
 */
int 
atapi_process_phases (ATA *ap, ATASCB *scbp)
{
	int		phase, count;
	int		port = ap->at_port;
	int		alt_stat;

	/*
	 *	Envia o comando ATAPI
	 */
	if (atapi_command (ap, scbp) < 0)
		return (-1);

	/*
	 *	Processa as diversas fases
	 */
	for (phase = 1; phase <= 100; phase++)
	{
#ifdef	DEBUG
		alt_stat = read_port (port + ATA_ALTOFFSET + ATA_ALTSTAT);
		printf ("hd[%d,%d]: Entrada: alt_stat = %P\n", ATA_DEVNO, alt_stat);
#endif	DEBUG
		/* Espera a controladora desocupar */

		for (count = 20000; /* abaixo */; count--)
		{
			if (count <= 0)
				{ printf ("hd[%d,%d]: Tempo esgotado esperando desocupar\n", ATA_DEVNO); return (-1); }

			alt_stat = read_port (port + ATA_ALTOFFSET + ATA_ALTSTAT);

			if ((alt_stat & ATA_S_BUSY) == 0)
				break;

			DELAY (100);
		}
#ifdef	DEBUG
		printf ("hd[%d,%d]: Saída: alt_stat = %P\n", ATA_DEVNO, alt_stat);
#endif	DEBUG
		if (atapi_next_phase (ap, scbp, phase) == ATA_OP_FINISHED)
			return (scbp->scb_result);
	}

	printf
	(
		"hd[%d,%d]: cmd = %s, número excessivo de fases: %d\n",
		ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]),
		phase
	);

	return (-1);

}	/* end atapi_process_phases */
#endif	BOOT

/*
 ****************************************************************
 *	Envia o comando ATAPI para a controladora		*
 ****************************************************************
 */
int
atapi_command (ATA *ap, ATASCB *scbp)
{
	int		timeout, reason, status = 0;
	int		port = ap->at_port;

#ifndef	BOOT
	/*
	 *	Se possível, utiliza o DMA na transferência.
	 */
	scbp->scb_flags &= ~B_DMA;

	if (ap->at_bmport != 0 && (scbp->scb_flags & B_STAT) == 0)
	{
		if (ata_dma_setup (ap, scbp, scbp->scb_flags & (B_READ|B_WRITE)) == 0)
		{
#ifdef	DEBUG
			printf
			(	"%s: Usando o DMA, cmd = %s, addr = %P, count = %d\n",
				ap->at_dev_nm,
				scsi_cmd_name (scbp->scb_cmd[0]),
				scbp->scb_addr,
				scbp->scb_count
			);
#endif	DEBUG
			scbp->scb_flags |= B_DMA;
		}

	} 	/* end if (pode usar DMA) */
#endif

	/*
	 *	Inicia a operação no dispositivo ATAPI.
	 */
	if
	(	ata_command
		(	ap, ATA_C_PACKET_CMD, scbp->scb_cnt << 8, 0,
			scbp->scb_flags & B_DMA ? ATA_F_DMA : 0, ATA_IMMEDIATE
		) < 0
	)
	{
		printf ("hd[%d,%d]: NÃO consegui iniciar o comando ATAPI\n", ATA_DEVNO);
		return (-1);
	}

#ifndef	BOOT
	if (scbp->scb_flags & B_DMA)
		ata_dma_start (ap);

	if (ap->at_param->drq_type == ATAPI_DRQT_INTR)
		return (0);
#endif

	/*
	 *	Pronto para enviar o comando ATAPI
	 */
	for (timeout = 5000; /* abaixo */; timeout--)
	{
		if (timeout <= 0)
		{
			printf ("hd[%d,%d]: NÃO consegui executar o comando ATAPI\n", ATA_DEVNO);

			ap->at_status = status;
			return (-1);
		}

		reason = read_port (port + ATA_IREASON);
		status = read_port (port + ATA_STATUS);

		if (((reason & (ATA_I_CMD|ATA_I_IN)) | (status & (ATA_S_DRQ|ATA_S_BUSY))) == ATAPI_P_CMDOUT)
			break;

		DELAY (20);
	}

	ap->at_status = status;

	DELAY (10);	/* Necessário para certos dispositivos lentos */

	/*
	 *	Envia propriamente o comando ATAPI
	 */
	write_port_string_short (port + ATA_DATA, scbp->scb_cmd, ap->at_cmdsz);

	return (0);

}	/* end atapi_command */

/*
 ****************************************************************
 *	Processa a próxima fase de um comando ATAPI		*
 ****************************************************************
 */
int
atapi_next_phase (ATA *ap, ATASCB *scbp, int phase)
{
	int		reason, sense, dma_status = 0;
	int		port = ap->at_port;
	static char	sense_cmd[16] = { SCSI_CMD_REQUEST_SENSE, 0, 0, 0,
					  sizeof (struct scsi_sense_data) };

	/*
	 *	Lê o estado e determina a causa da interrupção
	 */
	ap->at_status = read_port (port + ATA_STATUS);

	reason = (read_port (port + ATA_IREASON) & (ATA_I_CMD|ATA_I_IN)) |
		 (ap->at_status & ATA_S_DRQ);

	if (reason == ATAPI_P_CMDOUT)
	{
		/* Controlador aguardando comando */

		if ((ap->at_status & ATA_S_DRQ) == 0)
		{
			printf ("hd[%d,%d]: Interrupção espúria\n", ATA_DEVNO);
			scbp->scb_result = -1;
			return (ATA_OP_FINISHED);
		}

		write_port_string_short (port + ATA_DATA, scbp->scb_cmd, ap->at_cmdsz);

		return (ATA_OP_CONTINUES);
	}

#ifndef	BOOT
	if (scbp->scb_flags & B_DMA)
	{
		/* Usou DMA na transferência */

		dma_status	= ata_dma_done (ap);
		ap->at_status	= read_port (port + ATA_STATUS);

	   /***	scbp->scb_addr += scbp->scb_count;	***/
	   /***	scbp->scb_count = 0;			***/

		if (reason != ATAPI_P_DONE)
		{
			printf ("%s: DMA ATAPI reason != ATAPI_P_DONE\n", ap->at_dev_nm);
			reason = ATAPI_P_DONE;
		}

	}	/* end if (usou DMA) */
#endif	BOOT

	/*
	 *	Analisa a causa da interrupção
	 */
	switch (reason)
	{
	    case ATAPI_P_WRITE:
#ifdef	DEBUG
		printf
		(
			"hd[%d,%d]: cmd = %s, Fase %d => ATAPI_P_WRITE\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]), phase
		);
#endif	DEBUG
		if (scbp->scb_flags & B_READ)
		{
			ap->at_error = read_port (port + ATA_ERROR);
			printf ("hd[%d,%d]: Escrevendo uma área de entrada\n", ATA_DEVNO);
			break;
		}

		atapi_write (ap, scbp);
		return (ATA_OP_CONTINUES);

	    case ATAPI_P_READ:
#ifdef	DEBUG
		printf
		(
			"hd[%d,%d]: cmd = %s, Fase %d => ATAPI_P_READ\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]), phase
		);
#endif	DEBUG
		if ((scbp->scb_flags & B_READ) == 0)
		{
			ap->at_error = read_port (port + ATA_ERROR);
			printf ("hd[%d,%d]: Lendo uma área de escrita\n", ATA_DEVNO);
			break;
		}

		atapi_read (ap, scbp);
		return (ATA_OP_CONTINUES);

	    case ATAPI_P_DONEDRQ:
#ifdef	DEBUG
		printf
		(
			"hd[%d,%d]: cmd = %s, Fase %d => ATAPI_P_DONEDRQ\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]), phase
		);
#endif	DEBUG
		if (scbp->scb_flags & B_READ)
			atapi_read (ap, scbp);
		else
			atapi_write (ap, scbp);

		/* continua no caso abaixo */

	    case ATAPI_P_ABORT:
	    case ATAPI_P_DONE:
#ifdef	DEBUG
		printf
		(
			"hd[%d,%d]: cmd = %s, Fase %d => ATAPI_P_%s\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]),
			phase, reason == ATAPI_P_ABORT ? "ABORT" : "DONE"
		);
#endif	DEBUG
		break;

	    default:
		printf
		(
			"hd[%d,%d]: cmd = %s, Fase %d => Desconhecida, reason = %P\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]),
			phase, reason
		);
		break;

	}	/* end switch (reason) */

	/*
	 *	Verifica se o comando terminou com sucesso
	 */
	if
	(	(dma_status & ATA_BMSTAT_ERROR) == 0 &&
		(ap->at_status & (ATA_S_ERROR | ATA_S_DWF)) == 0
	)
	{
		if (scbp->scb_cmd[0] != SCSI_CMD_REQUEST_SENSE)
			{ scbp->scb_result = 0; return (ATA_OP_FINISHED); }

		/* Analisa o resultado do comando REQUEST SENSE */

		sense = scbp->scb_sense.flags & 0x0F;

		if (sense == 2 /* Unit not ready */)
			{ scbp->scb_result = 1; return (ATA_OP_FINISHED); }

#ifdef	DEBUG
		printf
		(	"hd[%d,%d]: cmd = %s, sense = %02X\n",
			ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]), sense
		);
#endif	DEBUG

		/*
		 *	Prepara o retorno
		 */
		if ((sense = scsi_sense (ap->at_scsi, sense)) > 0)
			scbp->scb_result = 0;	/* recuperou */
		elif (sense == 0)
			scbp->scb_result = -1;	/* pode repetir */
		else
			scbp->scb_result = 1;	/* irrecuperável */

		return (ATA_OP_FINISHED);
	}

	/*
	 *	Houve erro na operação 
	 */
	ap->at_error = read_port (port + ATA_ERROR);

#ifdef	DEBUG
	printf
	(
		"hd[%d,%d]: cmd = %s, Fase %d => Erro %P\n",
		ATA_DEVNO, scsi_cmd_name (scbp->scb_cmd[0]),
		phase, ap->at_error
	);
#endif	DEBUG

	if ((ap->at_error & ATAPI_SK_MASK) && scbp->scb_cmd[0] != SCSI_CMD_REQUEST_SENSE)
	{
		/* Prepara e tenta enviar o comando REQUEST SENSE */

		scbp->scb_cmd	 = sense_cmd;
		scbp->scb_addr	 = &scbp->scb_sense;
		scbp->scb_cnt	 = sizeof (struct scsi_sense_data);
		scbp->scb_flags  = B_READ|B_STAT;

		if (atapi_command (ap, scbp) >= 0)
			return (ATA_OP_CONTINUES);
	}

	scbp->scb_result = -1;

	return (ATA_OP_FINISHED);

}	/* end atapi_next_phase */


/*
 ****************************************************************
 *	Leitura ATAPI						*
 ****************************************************************
 */
void
atapi_read (ATA *ap, ATASCB *scbp)
{
	int		count, resid, length;
	int		port = ap->at_port;

	length = read_port (port + ATA_CYL_LSB) | (read_port (port + ATA_CYL_MSB) << 8);

	if ((count = scbp->scb_cnt) > length)
		count = length;

#ifdef	DEBUG
	printf ("hd[%d,%d]: atapi_read: length = %d, count = %d\n", ATA_DEVNO, length, count);
#endif	DEBUG

	if ((count & 3) == 0)
		(*ap->at_read_port)    (port + ATA_DATA, scbp->scb_addr, count >> ap->at_pio_shift);
	else
		read_port_string_short (port + ATA_DATA, scbp->scb_addr, count / sizeof (short));

	if (length > scbp->scb_cnt)
	{
		printf ("hd[%d,%d]: Há mais dados para serem lidos %d/%d\n", ATA_DEVNO, length, scbp->scb_cnt);

		for (resid = scbp->scb_cnt; resid < length; resid += sizeof (short))
			 read_port_short (port + ATA_DATA);
	}

	scbp->scb_addr += count;
	scbp->scb_cnt  -= count;

}	/* end atapi_read */

/*
 ****************************************************************
 *	Escrita ATAPI						*
 ****************************************************************
 */
void
atapi_write (ATA *ap, ATASCB *scbp)
{
	int		count, resid, length;
	int		port = ap->at_port;

	length = read_port (port + ATA_CYL_LSB) | (read_port (port + ATA_CYL_MSB) << 8);

	if ((count = scbp->scb_cnt) > length)
		count = length;

#ifdef	DEBUG
	printf ("hd[%d,%d]: atapi_write: length = %d, count = %d\n", ATA_DEVNO, length, count);
#endif	DEBUG

	if ((count & 3) == 0)
		(*ap->at_write_port)    (port + ATA_DATA, scbp->scb_addr, count >> ap->at_pio_shift);
	else
		write_port_string_short (port + ATA_DATA, scbp->scb_addr, count / sizeof (short));

	if (length > scbp->scb_cnt)
	{
		printf ("hd[%d,%d]: Há mais dados para serem escritos %d/%d\n", ATA_DEVNO, length, scbp->scb_cnt);

		for (resid = scbp->scb_cnt; resid < length; resid += sizeof (short))
			 write_port_short (0, port + ATA_DATA);
	}

	scbp->scb_addr += count;
	scbp->scb_cnt  -= count;

}	/* end atapi_write */

/*
 ****************************************************************
 *	Envia um comando qualquer para um alvo			*
 ****************************************************************
 */
int
ata_command (ATA *ap, int command, daddr_t lba_low, int count, int feature, int flags)
{
	int		port = ap->at_port;

	/*
	 *	Seleciona o dispositivo
	 */
	write_port (ATA_D_IBM | (ap->at_target << 4), port + ATA_DRIVE);

	/*
	 *	Está pronta para receber um comando?
	 */
	if (ata_wait (ap, 0) < 0)
	{ 
		printf
		(	"hd[%d,%d]: Tempo expirado tentando enviar comando %02X\n",
			ATA_DEVNO, command
		);
		return (-1);
	}

	/*
	 *	Só utiliza o endereçamento estendido se necessário
	 *	(por enquanto, apenas 32 bits => 2 Tb)
	 */
	if
	(	(/* lba_high != 0 || */ lba_low > 0x0FFFFFFF || count > 256) &&
		ap->at_param->support.address48
	)
	{
		/* Traduz para um comando apropriado */

		switch (command)
		{
		    case ATA_C_READ:
			command = ATA_C_READ48;
			break;

		    case ATA_C_READ_MUL:
			command = ATA_C_READ_MUL48;
			break;

		    case ATA_C_READ_DMA:
			command = ATA_C_READ_DMA48;
			break;

		    case ATA_C_READ_DMA_QUEUED:
			command = ATA_C_READ_DMA_QUEUED48;
			break;

		    case ATA_C_WRITE:
			command = ATA_C_WRITE48;
			break;

		    case ATA_C_WRITE_MUL:
			command = ATA_C_WRITE_MUL48;
			break;

		    case ATA_C_WRITE_DMA:
			command = ATA_C_WRITE_DMA48;
			break;

		    case ATA_C_WRITE_DMA_QUEUED:
			command = ATA_C_WRITE_DMA_QUEUED48;
			break;

		    case ATA_C_FLUSHCACHE:
			command = ATA_C_FLUSHCACHE48;
			break;

		    default:
			printf
			(	"hd[%,%d]: o comando \"%X\" NÃO possui um "
				"equivalente em 48 bits\n",
				ATA_DEVNO, command
			);
			return (-1);
		}

		write_port (feature >> 8,	port + ATA_FEATURE);
		write_port (feature,		port + ATA_FEATURE);
		write_port (count >> 8,		port + ATA_COUNT);
		write_port (count,		port + ATA_COUNT);
		write_port (lba_low >> 24,	port + ATA_SECTOR);
		write_port (lba_low,		port + ATA_SECTOR);
		write_port (0,			port + ATA_CYL_LSB);
	   /***	write_port (lba_high,		port + ATA_CYL_LSB); ***/
		write_port (lba_low >> 8,	port + ATA_CYL_LSB);
		write_port (0,			port + ATA_CYL_MSB);
	   /***	write_port (lba_high >> 8,	port + ATA_CYL_MSB); ***/
		write_port (lba_low >> 16,	port + ATA_CYL_MSB);

		write_port (ATA_D_LBA | (ap->at_target << 4), port + ATA_DRIVE);
	}
	else
	{
		ulong		cylin, head, sector, cmd;

		if (ap->at_flags & HD_CHS_USED)
		{
			/* Endereçamento geométrico */

			sector	= (lba_low % ap->at_sect) + 1;
			head	= (lba_low / ap->at_sect) % ap->at_head;
			cylin	= (lba_low / ap->at_sect) / ap->at_head;
			cmd	= ATA_D_IBM;
		}
		else
		{
			/* Endereçamento LBA */

			sector  = lba_low & 0xFF;
			head	= lba_low >> 24;
			cylin	= (lba_low >> 8) & 0xFFFF;
			cmd	= ATA_D_IBM | ATA_D_LBA;
		}

		cmd |= (ap->at_target << 4) | (head & 0x0F);

		write_port (feature,	port + ATA_FEATURE);
		write_port (count,	port + ATA_COUNT);
		write_port (sector,	port + ATA_SECTOR);
		write_port (cylin,	port + ATA_CYL_LSB);
		write_port (cylin >> 8, port + ATA_CYL_MSB);
		write_port (cmd,	port + ATA_DRIVE);
	}

	/*
	 *	Envia o comando
	 */
	write_port (command, port + ATA_CMD);

	if (flags == ATA_IMMEDIATE)		/* Retorna logo */
		return (0);

	/*
	 *	Aguarda ficar pronto
	 */
	if (ata_wait (ap, ATA_S_READY) < 0)
	{ 
		printf
		(	"hd[%d,%d]: Espera esgotada para o comando = %02X\n",
			ATA_DEVNO, command
		);

		return (-1);
	}

	return (0);

}	/* end ata_command */

/*
 ****************************************************************
 *	Espera a controladora desocupar				*
 ****************************************************************
 */
int
ata_wait (ATA *ap, int bits_wanted)
{
	int		timeout, stport, errport, incr, status = 0;

	stport  = ap->at_port + ATA_STATUS;
	errport = ap->at_port + ATA_ERROR;

	DELAY (1);

	/*
	 *	Espera desocupar
	 */
	for (incr = 10, timeout = 0; timeout < 5000000; timeout += incr)
	{
		status = read_port (stport);

		if (status == 0xFF)
		{
			write_port
			(	ATA_D_IBM | (ap->at_target << 4),
				ap->at_port + ATA_DRIVE
			);

			DELAY (10);

			if ((status = read_port (stport)) == 0xFF)
				return (-1);
		}

		if ((status & ATA_S_BUSY) == 0)
			break;

		if (timeout == 1000)
			incr = 1000;

		DELAY (incr);
	}

	if (status & ATA_S_ERROR)
		ap->at_error = read_port (errport);

	if (timeout >= 5000000)
		{ ap->at_status = status; return (-1); }

	if (bits_wanted == 0)
		return ((ap->at_status = status) & ATA_S_ERROR);

	/*
	 *	Espera o padrão de bits desejado
	 */
	for (timeout = 5000; timeout > 0; timeout--)
	{
		status = read_port (stport);

		if ((status & bits_wanted) == bits_wanted)
		{
			if (status & ATA_S_ERROR)
				ap->at_error = read_port (stport);

			return ((ap->at_status = status) & ATA_S_ERROR);
		}

		DELAY (10);
	}

	ap->at_status = status;
	return (-1);

}	/* end ata_wait */

/*
 ****************************************************************
 *	Troca a ordem dos bytes					*
 ****************************************************************
 */
void
ata_bswap (char *buf, int len)
{
	char	aux, *cp;

	for (cp = buf; cp < buf + len; cp += 2)
	{
		aux   = cp[0];
		cp[0] = cp[1];
		cp[1] = aux;
	}
	
}	/* end ata_bswap */

/*
 ****************************************************************
 *	Elimina brancos						*
 ****************************************************************
 */
void
ata_btrim (char *buf, int len)
{
	char *p;

	for (p = buf; p < buf + len; ++p)
	{
		if (!*p)
			*p = ' ';
	}

	for (p = buf + len - 1; p >= buf && *p == ' '; --p)
		*p = 0;

}	/* end ata_btrim */

/*
 ****************************************************************
 *	Compacta						*
 ****************************************************************
 */
void
ata_bpack (char *src, char *dst, int len)
{
	int	i, j, blank;

	for (i = j = blank = 0 ; i < len; i++)
	{
		if (blank && src[i] == ' ')
			continue;

		if (blank && src[i] != ' ')
			{ dst[j++] = src[i]; blank = 0; continue; }

		if (src[i] == ' ')
		{
			blank = 1;

			if (i == 0)
				continue;
		}

		dst[j++] = src[i];
	}

	if (j < len) 
		dst[j] = 0x00;

}	/* end ata_bpack */

/*
 ****************************************************************
 *	Procura o primeiro bit mais significativo ligado	*
 ****************************************************************
 */
int
ata_disk_version (int version)
{
	int		Bit;

	if (version == 0xFFFF)
		return (0);

	for (Bit = 15; Bit >= 0; Bit--)
	{
		if (version & (1 << Bit))
			return (Bit);
	}

	return (0);

}	/* end ata_disk_version */

#if (0)	/*******************************************************/
/*
 ****************************************************************
 *	Envia o comando MODE_SENSE para CDROMs ATAPI		*
 ****************************************************************
 */
int 
ata_cdrom_sense (ATA *ap)
{
	int		i, ret;
	ACD_CAPPAGE	cap;
	ATASCB		req, *scbp = &req;
	char		cmd[16];

	/*
	 *	Prepara o comando MODE SENSE
	 */
	memclr (cmd, sizeof (cmd));

	cmd[0] = SCSI_CMD_MODE_SENSE;

	cmd[2] = ATAPI_CDROM_CAP_PAGE;
	cmd[7] = sizeof (ACD_CAPPAGE) >> 8;
	cmd[8] = sizeof (ACD_CAPPAGE);

	/*
	 *	Tenta algumas vezes...
	 */
	for (i = 0; /* abaixo */; i++)
	{
		if (i >= 10)
		{
			printf
			(	"hd[%d,%d]: Não consegui enviar o comando MODE SENSE\n",
				ATA_DEVNO 
			);

			return (-1);
		}

		scbp->scb_cmd   = cmd;
		scbp->scb_cnt   = sizeof (ACD_CAPPAGE);
		scbp->scb_addr  = &cap;
		scbp->scb_flags = B_READ;

		if ((ret = atapi_process_phases (ap, scbp)) == 0)
			break;

		if (ret > 0)
			return (-1);
	}

	if (cap.page_code != ATAPI_CDROM_CAP_PAGE)
		return (-1);

#ifdef	DEBUG
	printf
	(
		"hd[%d,%d]: ACD_CAPPAGE: max_read_speed = %d\n",
		ATA_DEVNO, short_endian_cv (cap.max_read_speed)
	);
#endif	DEBUG

	return (0);

}	/* end ata_cdrom_sense */

/*
 ****************************************************************
 *	Envia o comando MODE_SENSE para dispositivos DIRECT	*
 ****************************************************************
 */
int 
ata_direct_sense (ATA *ap)
{
	int		i, ret;
	AFD_CAPPAGE	cap;
	ATASCB		req, *scbp = &req;
	char		cmd[16];

	/*
	 *	Prepara o comando MODE SENSE
	 */
	memclr (cmd, sizeof (cmd));

	cmd[0] = SCSI_CMD_MODE_SENSE;

	cmd[2] = ATAPI_REWRITEABLE_CAP_PAGE;
	cmd[7] = sizeof (AFD_CAPPAGE) >> 8;
	cmd[8] = sizeof (AFD_CAPPAGE);

	/*
	 *	Tenta algumas vezes...
	 */
	for (i = 0; /* abaixo */; i++)
	{
		if (i >= 10)
		{
			printf
			(	"hd[%d,%d]: Não consegui enviar o comando MODE SENSE\n",
				ATA_DEVNO 
			);

			return (-1);
		}

		scbp->scb_cmd   = cmd;
		scbp->scb_cnt   = sizeof (AFD_CAPPAGE);
		scbp->scb_addr  = &cap;
		scbp->scb_flags = B_READ;

		if ((ret = atapi_process_phases (ap, scbp)) == 0)
			break;

		if (ret > 0)
			return (-1);
	}

	if (cap.page_code != ATAPI_REWRITEABLE_CAP_PAGE)
		return (-1);

#ifdef	DEBUG
	printf
	(	"hd[%d,%d]: AFD_CAPPAGE: geo = <%d, %d, %d>, blsz = %d\n",
		ATA_DEVNO,
		short_endian_cv (cap.cylinders), cap.heads, cap.sectors,
		short_endian_cv (cap.sector_size)
	);
#endif	DEBUG

	return (0);

}	/* end ata_direct_sense */
#endif	/*******************************************************/
