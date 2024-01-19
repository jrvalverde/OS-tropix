/*
 ****************************************************************
 *								*
 *			ata.c					*
 *								*
 *	Driver para os dispositivos ATA				*
 *								*
 *	Versão	3.0.0, de 20.07.94				*
 *		4.6.0, de 27.09.04				*
 *								*
 *	Módulo: Boot2						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2004 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include <common.h>

#include <disktb.h>
#include <devmajor.h>
#include <bhead.h>
#include <scsi.h>
#include <ata.h>
#include <uerror.h>

#include "../h/common.h"
#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Variáveis e Definições globais				*
 ****************************************************************
 */
#undef	DEBUG

const char	err_bits[] = 	"\x08" "BAD_BLOCK"
				"\x07" "ECC_ERROR"
				"\x06" "ID_CRC"
				"\x05" "NO_ID"
				/* Indicador desconhecido */
				"\x03" "ABORT"
				"\x02" "TRK_0"
				"\x01" "BAD_MARK";

/*
 ******	Informações acerca de um disco **************************
 */
entry ATA	atadata[NATAC * NATAT];

/*
 ******	Protótipos de funções ***********************************
 */
void		ata_unit_attach (ATA *);
int		atapi_process_phases (ATA *, ATASCB *);

/*
 ****************************************************************
 *	Verifica a existencia dos controladores e discos	*
 ****************************************************************
 */
void
ataattach (void)
{
	ATA		*ap;
	DISKTB		*pp;
	int		target;
	char		drive_letter;
	char		revision[12];
	static DISKTB	d = { "hd?", 0, 0, 0, 0, 0, BLSHIFT, MAKEDEV (IDE_MAJOR, 0), 0, 0 };

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
	 *	Preenche a tabela de partições
	 */
	drive_letter = 'a';

	for (ap = atadata, target = 0; target < NATAC * NATAT; target++, ap++)
	{
		if (ap->at_is_present == 0)
			continue;

		/* Atribui o nome definitivo */

		ap->at_dev_nm[0]  = 'h';
		ap->at_dev_nm[1]  = 'd';
		ap->at_dev_nm[2]  = drive_letter;
		ap->at_dev_nm[3]  = '\0';

		/* Cria uma entrada na tabela de partições */

		if ((pp = malloc_byte (sizeof (DISKTB))) == NODISKTB)
			{ printf ("Memória esgotada\n"); break; }

		memmove (pp, &d, sizeof (DISKTB));

	   /***	pp->p_name[0] = 'h';	***/
	   /***	pp->p_name[1] = 'd';	***/
		pp->p_name[2] = drive_letter++;
	   /***	pp->p_name[3] = '\0';	***/

	   /***	pp->p_dev     = MAKEDEV (IDE_MAJOR, 0); ***/
		pp->p_target  = target;

		ap->at_disktb = pp;

		ide_ptr[ide_index++] = ap;

		if (ap->at_flags & ATAPI_DEV)
		{
			SCSI		*sp = ap->at_scsi;

			/* Se for ATAPI, processa a inicialização do SCSI */

			sp->scsi_target = target;
			sp->scsi_disktb = pp;
			sp->scsi_dev_nm = pp->p_name;
			sp->scsi_cmd	= atastrategy;

			scsi_attach (sp);

			pp->p_blshift = sp->scsi_blshift;
		}
		else
		{
			const DISK_INFO	*dp = disk_info_table_ptr;

			/* Se NÃO for ATAPI, coleta as informações da BIOS */

			if (dp->info_present)
			{
				daddr_t		CYLSZ;

				ap->at_bios_head = dp->info_nhead;
				ap->at_bios_sect = dp->info_nsect;

				CYLSZ = ap->at_bios_head * ap->at_bios_sect;
			   	ap->at_bios_cyl  = (ap->at_disksz + CYLSZ - 1) / CYLSZ;

				if (dp->info_int13ext)
					ap->at_flags |= HD_INT13_EXT;

				disk_info_table_ptr++;
			}

			memmove (revision, ap->at_param->revision, 8); revision[8] = '\0';

			printf
			(	"%s: <%s %s, %d MB, %s bits, geo = (%d, %d, %d, %s)>\n",
				ap->at_dev_nm, ap->at_param->model, revision,
				BLtoMB (ap->at_disksz),
				ap->at_flags & HD_32_BITS ? "32" : "16",
				ap->at_bios_cyl, ap->at_bios_head, ap->at_bios_sect,
				ap->at_flags & HD_INT13_EXT ? "L" : "G"
			);
		}
	}

}	/* end ataattach */

/*
 ****************************************************************
 *	Inicializa a operação					*
 ****************************************************************
 */
int
ataopen (int target)
{
	ATA		*ap;

	if (target < 0 || target > NATAC * NATAT)
		return (-1);

	ap = &atadata[target];

	if (ap->at_flags & ATAPI_DEV)
	{
		/* Chama a camada SCSI */

		if (scsi_open (ap->at_scsi) < 0)
			return (-1);
	}

	return (0);

}	/* end ataopen */

/*
 ****************************************************************
 *	Realiza uma operação					*
 ****************************************************************
 */
int
atastrategy (BHEAD *bp)
{
	const DISKTB	*pp = bp->b_disktb;
	ATA		*ap;
	int		port, cmd, rw;
	int		count, error_count = 0;
	void		*area;
	daddr_t		block;

	/*
	 *	Prólogo
	 */
	ap = &atadata[pp->p_target];

	port = ap->at_port;

	if (ap->at_is_present == 0)
		{ printf ("hd[%d,%d]: Unidade NÃO presente\n", ATA_DEVNO); return (-1); }

	if (pp->p_size != 0 && (unsigned)bp->b_blkno >= pp->p_size)
		{ printf ("hd[%d,%d]: Bloco inválido: %d\n", ATA_DEVNO, bp->b_blkno); return (-1); }

	block = (bp->b_blkno + pp->p_offset) >> (ap->at_blshift - BLSHIFT);

	/*
	 *	Trata os dispositivos ATAPI
	 */
	if (ap->at_flags & ATAPI_DEV)
	{
		ATASCB		req, *scbp = &req;
		int		ret;

		if ((bp->b_flags & B_STAT) == 0)
		{
			/* É um comando regular de leitura ou escrita */

			if ((bp->b_flags & (B_READ|B_WRITE)) == B_READ)
				bp->b_cmd_txt[0] = SCSI_CMD_READ_BIG;
			else
				bp->b_cmd_txt[0] = SCSI_CMD_WRITE_BIG;

			bp->b_cmd_txt[1] = 0;
			*(long *)&bp->b_cmd_txt[2] = long_endian_cv (block);
			bp->b_cmd_txt[6] = 0;
			*(short *)&bp->b_cmd_txt[7] = short_endian_cv (bp->b_base_count >> ap->at_blshift);
			bp->b_cmd_txt[9] = 0;
		}

		/*
		 *	Tenta algumas vezes...
		 */
		for (error_count = 10; error_count > 0; error_count--)
		{
			scbp->scb_cmd	= bp->b_cmd_txt;
			scbp->scb_addr	= bp->b_addr;
			scbp->scb_cnt	= bp->b_base_count;
			scbp->scb_flags = bp->b_flags;

			if ((ret = atapi_process_phases (ap, scbp)) == 0)
				return (0);

			if (ret > 0)
				return (-1);
		}

		return (-1);
	}

	area = bp->b_addr;
	rw   = bp->b_flags & (B_READ|B_WRITE);
	cmd  = (rw == B_WRITE) ? ATA_C_WRITE : ATA_C_READ;

#ifdef	DEBUG
	printf
	(	"hd[%d,%d]: %s blkno = %d (%s, %d), addr = %P, count = 1\n", 
		ATA_DEVNO, rw == B_WRITE ? "WRITE" : "READ",
		block, ap->at_flags & HD_CHS_USED ? "GEO" : "LBA",
		ap->at_pio_shift << 4, area
	);
#endif	DEBUG

	/*
	 *	Envia o comando
	 */
    again:
	for (count = bp->b_base_count; count > 0; count -= BLSZ)
	{
		ata_command (ap, cmd, block, 1, 0, ATA_IMMEDIATE);

		if (ata_wait (ap, ATA_S_READY|ATA_S_DSC|ATA_S_DRQ) != 0)
			goto error;

		/* Lê/escreve a informação */

		if (rw == B_WRITE)
			(*ap->at_write_port) (port + ATA_DATA, area, BLSZ >> ap->at_pio_shift);
		else
			(*ap->at_read_port)  (port + ATA_DATA, area, BLSZ >> ap->at_pio_shift);

		if (read_port (port + ATA_STATUS) & ATA_S_DRQ)
			{ printf ("hd[%d,%d]: Dados demais\n", ATA_DEVNO); goto error; }

		if (rw == B_WRITE)
		{
			while (read_port (port + ATA_STATUS) & ATA_S_BUSY)
				/* vazio */;
		}

		if (read_port (port + ATA_STATUS) & ATA_S_ERROR)
			{ printf ("hd[%d,%d]: Erro póstumo\n", ATA_DEVNO); goto error; }

		area  += BLSZ;
		block += 1;
	}

	return (0);

	/*
	 *	Caso de erro
	 */
    error:
	printf
	(	"hd[%d,%d]: Erro de %s: bloco = %d, erro = %b\n",
		ATA_DEVNO, (rw == B_WRITE) ? "escrita" : "leitura",
		block, read_port (port + ATA_ERROR), err_bits
	);

	if (++error_count < 10)
		goto again;

	return (-1);

}	/* end atastrategy */

/*
 ****************************************************************
 *	Rotina de IOCTL						*
 ****************************************************************
 */
int
ata_ctl (const DISKTB *up, int cmd, int arg)
{
	ATA		*ap;

	ap = &atadata[up->p_target];

	return (scsi_ctl (ap->at_scsi, up, cmd, arg));

}	/* end ata_ctl */
