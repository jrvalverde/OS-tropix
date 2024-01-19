/*
 ****************************************************************
 *								*
 *			ata.h					*
 *								*
 *	Definições acerca de dispositivos ATA			*
 *								*
 *	Versão	4.0.0, de 18.07.01				*
 *		4.6.0, de 09.08.04				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2004 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#define	ATA_H

/*
 ****************************************************************
 *	Registradores e bits das controladoras			*
 ****************************************************************
 */
#define NATAC				2	/* Temos 2 controladores (unidades) */
#define NATAT				2	/* Cada controladora tem 2 alvos */

#define ATA_PRIMARY			0x1F0	/* Port da controladora primária */
#define ATA_SECONDARY			0x170	/* Port da controladora secundária */
#define	ATA_MAX_BL			255	/* Nr. máx. de blocos por comando */

/*
 ******	Registrador "DATA" **************************************
 */
#define ATA_DATA			0x00

/*
 ******	Registrador "ERROR" *************************************
 */
#define ATA_ERROR			0x01
#define		ATA_E_NM		0x02	/* no media */
#define		ATA_E_ABORT		0x04	/* command aborted */
#define		ATA_E_MCR		0x08	/* media change request */
#define		ATA_E_IDNF		0x10	/* ID not found */
#define		ATA_E_MC		0x20	/* media changed */
#define		ATA_E_UNC		0x40	/* uncorrectable data */
#define		ATA_E_ICRC		0x80	/* UDMA crc error */

/*
 ******	Registrador "FEATURES" **********************************
 */
#define ATA_FEATURE			0x01
#define		ATA_F_DMA		0x01	/* enable DMA */
#define		ATA_F_OVL		0x02	/* enable overlap */

/*
 ******	Registrador "SECTOR COUNT" ******************************
 */
#define ATA_COUNT			0x02

/*
 ******	Registrador "INTERRUPT REASON" **************************
 */
#define ATA_IREASON			0x02
#define		ATA_I_CMD		0x01	/* cmd (1) | data (0) */
#define		ATA_I_IN		0x02	/* read (1) | write (0) */
#define		ATA_I_RELEASE		0x04	/* released bus (1) */
#define		ATA_I_TAGMASK		0xF8	/* tag mask */

/*
 ******	Registrador "SECTOR NUMBER" *****************************
 */
#define ATA_SECTOR			0x03

/*
 ******	Registradores "CYLINDER NUMBER LOW e HIGH ***************
 */
#define ATA_CYL_LSB			0x04
#define ATA_CYL_MSB			0x05

/*
 ******	Registrador "DRIVE" *************************************
 */
#define ATA_DRIVE			0x06
#define		ATA_D_LBA		0x40	/* use LBA addressing */
#define		ATA_D_IBM		0xA0	/* 512 byte sectors, ECC */

/*
 ******	Registrador "COMMAND" ***********************************
 */
#define ATA_CMD				0x07
#define		ATA_C_NOP		0x00	/* NOP command */
#define		    ATA_C_F_FLUSHQUEUE	0x00	/* flush queued cmd's */
#define		    ATA_C_F_AUTOPOLL	0x01	/* start autopoll function */
#define		ATA_C_ATAPI_RESET	0x08	/* reset ATAPI device */
#define		ATA_C_READ		0x20	/* read command */
#define		ATA_C_READ48		0x24	/* read command */
#define		ATA_C_READ_DMA48	0x25	/* read w/DMA command */
#define		ATA_C_READ_DMA_QUEUED48 0x26	/* read w/DMA QUEUED command */
#define		ATA_C_READ_MUL48	0x29	/* read multi command */
#define		ATA_C_WRITE		0x30	/* write command */
#define		ATA_C_WRITE48		0x34	/* write command */
#define		ATA_C_WRITE_DMA48	0x35	/* write w/DMA command */
#define		ATA_C_WRITE_DMA_QUEUED48 0x36	/* write w/DMA QUEUED command */
#define		ATA_C_WRITE_MUL48	0x39	/* write multi command */
#define		ATA_C_PACKET_CMD	0xA0	/* packet command */
#define		ATA_C_ATAPI_IDENTIFY	0xA1	/* get ATAPI params*/
#define		ATA_C_SERVICE		0xA2	/* service command */
#define		ATA_C_READ_MUL		0xC4	/* read multi command */
#define		ATA_C_WRITE_MUL		0xC5	/* write multi command */
#define		ATA_C_SET_MULTI		0xC6	/* set multi size command */
#define		ATA_C_READ_DMA_QUEUED	0xC7	/* read w/DMA QUEUED command */
#define		ATA_C_READ_DMA		0xC8	/* read w/DMA command */
#define		ATA_C_WRITE_DMA		0xCA	/* write w/DMA command */
#define		ATA_C_WRITE_DMA_QUEUED	0xCC	/* write w/DMA QUEUED command */
#define		ATA_C_SLEEP		0xE6	/* sleep command */
#define		ATA_C_FLUSHCACHE	0xE7	/* flush cache to disk */
#define		ATA_C_FLUSHCACHE48	0xEA	/* flush cache to disk */
#define		ATA_C_ATA_IDENTIFY	0xEC	/* get ATA params */
#define		ATA_C_SETFEATURES	0xEF	/* features command */
#define		    ATA_C_F_SETXFER	0x03	/* set transfer mode */
#define		    ATA_C_F_ENAB_WCACHE 0x02	/* enable write cache */
#define		    ATA_C_F_DIS_WCACHE	0x82	/* disable write cache */
#define		    ATA_C_F_ENAB_RCACHE 0xAA	/* enable readahead cache */
#define		    ATA_C_F_DIS_RCACHE	0x55	/* disable readahead cache */
#define		    ATA_C_F_ENAB_RELIRQ 0x5D	/* enable release interrupt */
#define		    ATA_C_F_DIS_RELIRQ	0xDD	/* disable release interrupt */
#define		    ATA_C_F_ENAB_SRVIRQ 0x5E	/* enable service interrupt */
#define		    ATA_C_F_DIS_SRVIRQ	0xDE	/* disable service interrupt */

/*
 ******	Registrador "STATUS" ************************************
 */
#define ATA_STATUS			0x07	/* status register */
#define		ATA_S_ERROR		0x01	/* error */
#define		ATA_S_INDEX		0x02	/* index */
#define		ATA_S_CORR		0x04	/* data corrected */
#define		ATA_S_DRQ		0x08	/* data request */
#define		ATA_S_DSC		0x10	/* drive seek completed */
#define		ATA_S_SERVICE		0x10	/* drive needs service */
#define		ATA_S_DWF		0x20	/* drive write fault */
#define		ATA_S_DMA		0x20	/* DMA ready */
#define		ATA_S_READY		0x40	/* drive ready */
#define		ATA_S_BUSY		0x80	/* busy */

/*
 ******	Registrador "ALTERNATE STATUS" **************************
 */
#define ATA_ALTOFFSET			0x206
#define ATA_ALTSTAT			0x00	/* alternate status register */
#define		ATA_A_IDS		0x02	/* disable interrupts */
#define		ATA_A_RESET		0x04	/* RESET controller */
#define		ATA_A_4BIT		0x08	/* 4 head bits */

/*
 ******	Definições relativas ao DMA *****************************
 */
#define	BM_SEC_OFFSET			8	/* para a segunda controladora */

#define ATA_BMCMD_PORT			0x00	/* comandos para o DMA */
#define		ATA_BMCMD_START_STOP	0x01
#define		ATA_BMCMD_WRITE_READ	0x08

#define ATA_BMDEVSPEC_0			0x01

#define ATA_BMSTAT_PORT			0x02	/* estado do DMA */
#define		ATA_BMSTAT_ACTIVE	0x01
#define		ATA_BMSTAT_ERROR	0x02
#define		ATA_BMSTAT_INTERRUPT	0x04
#define		ATA_BMSTAT_MASK		0x07
#define		ATA_BMSTAT_DMA_MASTER	0x20
#define		ATA_BMSTAT_DMA_SLAVE	0x40
#define		ATA_BMSTAT_DMA_SIMPLEX	0x80

#define ATA_BMDEVSPEC_1			0x03
#define ATA_BMDTP_PORT			0x04

#define DMATB_MAX_SZ			256
#define	NODMATB				(DMATB *)0

#define	DMA_EOT_BIT			0x80000000

typedef	struct				/* A tabela de pedidos */
{
	ulong	dma_base;
	ulong	dma_count;

}	DMATB;

/*
 ******	Definições diversas *************************************
 */
#define ATA_OP_FINISHED			0x00	/* A operação terminou */
#define ATA_OP_CONTINUES		0x01	/* A operação continua */
#define	ATA_IMMEDIATE			0x01	/* Não deve aguardar ATA_S_READY */

#define ATA_MASTER			0x00
#define ATA_SLAVE			0x10
#define	ATA_ATA_MASTER			0x01
#define	ATA_ATA_SLAVE			0x02
#define	ATA_ATAPI_MASTER		0x04
#define	ATA_ATAPI_SLAVE			0x08

#define	ATA_PIO				0x00	/* flow-controlled PIO modes */
#define ATA_PIO0			0x08
#define ATA_PIO1			0x09
#define ATA_PIO2			0x0A
#define ATA_PIO3			0x0B
#define ATA_PIO4			0x0C

#define	ATA_MDMA			0x20	/* multi-word DMA timing modes */
#define	ATA_MDMA0			0x20
#define	ATA_MDMA1			0x21
#define	ATA_MDMA2			0x22

#define	ATA_UDMA			0x40	/* Ultra DMA timing modes */
#define	ATA_UDMA0			0x40
#define	ATA_UDMA1			0x41
#define	ATA_UDMA2			0x42
#define	ATA_UDMA4			0x44
#define	ATA_UDMA5			0x45
#define	ATA_UDMA6			0x46
#define ATA_SA150			0x47

/*
 ******	ATAPI  **************************************************
 */
/* ATAPI misc defines */
#define ATAPI_MAGIC_LSB			0x14
#define ATAPI_MAGIC_MSB			0xEB
#define ATAPI_P_READ			(ATA_S_DRQ | ATA_I_IN)
#define ATAPI_P_WRITE			(ATA_S_DRQ)
#define ATAPI_P_CMDOUT			(ATA_S_DRQ | ATA_I_CMD)
#define ATAPI_P_DONEDRQ			(ATA_S_DRQ | ATA_I_CMD | ATA_I_IN)
#define ATAPI_P_DONE			(ATA_I_CMD | ATA_I_IN)
#define ATAPI_P_ABORT			0

/* error register bits */
#define ATAPI_E_MASK			0x0F	/* error mask */
#define ATAPI_E_ILI			0x01	/* illegal length indication */
#define ATAPI_E_EOM			0x02	/* end of media detected */
#define ATAPI_E_ABRT			0x04	/* command aborted */
#define ATAPI_E_MCR			0x08	/* media change requested */
#define ATAPI_SK_MASK			0xF0	/* sense key mask */
#define ATAPI_SK_NO_SENSE		0x00	/* no specific sense key info */
#define ATAPI_SK_RECOVERED_ERROR	0x10	/* command OK, data recovered */
#define ATAPI_SK_NOT_READY		0x20	/* no access to drive */
#define ATAPI_SK_MEDIUM_ERROR		0x30	/* non-recovered data error */
#define ATAPI_SK_HARDWARE_ERROR		0x40	/* non-recoverable HW failure */
#define ATAPI_SK_ILLEGAL_REQUEST	0x50	/* invalid command param(s) */
#define ATAPI_SK_UNIT_ATTENTION		0x60	/* media changed */
#define ATAPI_SK_DATA_PROTECT		0x70	/* write protect */
#define ATAPI_SK_BLANK_CHECK		0x80	/* blank check */
#define ATAPI_SK_VENDOR_SPECIFIC	0x90	/* vendor specific skey */
#define ATAPI_SK_COPY_ABORTED		0xA0	/* copy aborted */
#define ATAPI_SK_ABORTED_COMMAND	0xB0	/* command aborted, try again */
#define ATAPI_SK_EQUAL			0xC0	/* equal */
#define ATAPI_SK_VOLUME_OVERFLOW	0xD0	/* volume overflow */
#define ATAPI_SK_MISCOMPARE		0xE0	/* data dont match the medium */
#define ATAPI_SK_RESERVED		0xF0

#if (0)	/*******************************************************/
/* ATAPI commands */
#define ATAPI_TEST_UNIT_READY		0x00	/* check if device is ready */
#define ATAPI_REZERO			0x01	/* rewind */
#define ATAPI_REQUEST_SENSE		0x03	/* get sense data */
#define ATAPI_FORMAT			0x04	/* format unit */
#define ATAPI_READ			0x08	/* read data */
#define ATAPI_WRITE			0x0A	/* write data */
#define ATAPI_WEOF			0x10	/* write filemark */
#define	    WF_WRITE				0x01
#define ATAPI_SPACE			0x11	/* space command */
#define	    SP_FM				0x01
#define	    SP_EOD				0x03
#define ATAPI_MODE_SELECT		0x15	/* mode select */
#define ATAPI_ERASE			0x19	/* erase */
#define ATAPI_MODE_SENSE		0x1A	/* mode sense */
#define ATAPI_START_STOP		0x1B	/* start/stop unit */
#define	    SS_LOAD				0x01
#define	    SS_RETENSION			0x02
#define	    SS_EJECT				0x04
#define ATAPI_PREVENT_ALLOW		0x1E	/* media removal */
#define ATAPI_READ_CAPACITY		0x25	/* get volume capacity */
#define ATAPI_READ_BIG			0x28	/* read data */
#define ATAPI_WRITE_BIG			0x2A	/* write data */
#define ATAPI_LOCATE			0x2B	/* locate to position */
#define ATAPI_READ_POSITION		0x34	/* read position */
#define ATAPI_SYNCHRONIZE_CACHE		0x35	/* flush buf, close channel */
#define ATAPI_WRITE_BUFFER		0x3B	/* write device buffer */
#define ATAPI_READ_BUFFER		0x3C	/* read device buffer */
#define ATAPI_READ_SUBCHANNEL		0x42	/* get subchannel info */
#define ATAPI_READ_TOC			0x43	/* get table of contents */
#define ATAPI_PLAY_10			0x45	/* play by lba */
#define ATAPI_PLAY_MSF			0x47	/* play by MSF address */
#define ATAPI_PLAY_TRACK		0x48	/* play by track number */
#define ATAPI_PAUSE			0x4B	/* pause audio operation */
#define ATAPI_READ_DISK_INFO		0x51	/* get disk info structure */
#define ATAPI_READ_TRACK_INFO		0x52	/* get track info structure */
#define ATAPI_RESERVE_TRACK		0x53	/* reserve track */
#define ATAPI_SEND_OPC_INFO		0x54	/* send OPC structurek */
#define ATAPI_MODE_SELECT_BIG		0x55	/* set device parameters */
#define ATAPI_REPAIR_TRACK		0x58	/* repair track */
#define ATAPI_READ_MASTER_CUE		0x59	/* read master CUE info */
#define ATAPI_MODE_SENSE_BIG		0x5A	/* get device parameters */
#define ATAPI_CLOSE_TRACK		0x5B	/* close track/session */
#define ATAPI_READ_BUFFER_CAPACITY	0x5C	/* get buffer capicity */
#define ATAPI_SEND_CUE_SHEET		0x5D	/* send CUE sheet */
#define ATAPI_BLANK			0xa1	/* blank the media */
#define ATAPI_SEND_KEY			0xa3	/* send DVD key structure */
#define ATAPI_REPORT_KEY		0xa4	/* get DVD key structure */
#define ATAPI_PLAY_12			0xa5	/* play by lba */
#define ATAPI_LOAD_UNLOAD		0xa6	/* changer control command */
#define ATAPI_READ_STRUCTURE		0xaD	/* get DVD structure */
#define ATAPI_PLAY_CD			0xb4	/* universal play command */
#define ATAPI_SET_SPEED			0xBB	/* set drive speed */
#define ATAPI_MECH_STATUS		0xBD	/* get changer status */
#define ATAPI_READ_CD			0xBE	/* read data */
#define ATAPI_POLL_DSC			0xFF	/* poll DSC status bit */
#endif	/*******************************************************/

/*
 ****************************************************************
 *	Parâmetros das Unidades ATA/ATAPI			*
 ****************************************************************
 */
/*
 ******	Valores para "packet_size" ******************************
 */
#define ATAPI_PSIZE_12          0       /* 12 bytes */
#define ATAPI_PSIZE_16          1       /* 16 bytes */

/*
 ******	Valores para "drq_type" *********************************
 */
#define ATAPI_DRQT_MPROC        0       /* cpu    3 ms delay */
#define ATAPI_DRQT_INTR         1       /* intr  10 ms delay */
#define ATAPI_DRQT_ACCEL        2       /* accel 50 us delay */

/*
 ******	Valores para "type" *************************************
 */
#define ATAPI_TYPE_DIRECT       0       /* disk/floppy */
#define ATAPI_TYPE_TAPE         1       /* streaming tape */
#define ATAPI_TYPE_CDROM        5       /* CD-ROM device */
#define ATAPI_TYPE_OPTICAL      7       /* optical disk */

/*
 ******	Valores para "atavalid" *********************************
 */
#define	ATA_FLAG_54_58		1		/* words 54-58 valid */
#define	ATA_FLAG_64_70		2		/* words 64-70 valid */
#define	ATA_FLAG_88		4		/* word 88 valid */

/*
 ****** Estrutura retornada pelo comando IDENTIFY ***************
 */
typedef struct
{
/*000*/	ushort		packet_size	:2;	/* packet command size */

    	ushort		incomplete	:1;
    	ushort		:2;
    	ushort		drq_type	:2;	/* DRQ type */

    	ushort		removable	:1;	/* device is removable */
    	ushort		type		:5;	/* device type */

    	ushort		:2;
    	ushort		cmd_protocol	:1;	/* command protocol */

/*001*/	ushort		cylinders;		/* # of cylinders */
	ushort		reserved2;

/*003*/	ushort		heads;			/* # heads */
	ushort		obsolete4;
	ushort		obsolete5;

/*006*/	ushort		sectors;		/* # sectors/track */
/*007*/	ushort		vendor7[3];
/*010*/	char		serial[20];		/* serial number */
	ushort		retired20;
	ushort		retired21;
	ushort		obsolete22;

/*023*/	char		revision[8];		/* firmware revision */
/*027*/	char		model[40];		/* model name */

/*047*/	ushort		sectors_intr:8;		/* sectors per interrupt */
	ushort		:8;

/*048*/	ushort		usedmovsd;		/* double word read/write? */

/*049*/	ushort		retired49:8;
	ushort		support_dma	:1;	/* DMA supported */
	ushort		support_lba	:1;	/* LBA supported */
	ushort		disable_iordy	:1;	/* IORDY may be disabled */
	ushort		support_iordy	:1;	/* IORDY supported */
	ushort		softreset	:1;	/* needs softreset when busy */
	ushort		stdby_ovlap	:1;	/* standby/overlap supported */
	ushort		support_queueing:1;	/* supports queuing overlap */
	ushort		support_idma	:1;	/* interleaved DMA supported */

/*050*/	ushort		device_stdby_min:1;
	ushort		:13;
	ushort		capability_one:1;
	ushort		capability_zero:1;

/*051*/	ushort		vendor51:8;
	ushort		retired_piomode:8;	/* PIO modes 0-2 */
/*052*/	ushort		vendor52:8;
	ushort		retired_dmamode:8;	/* DMA modes, not ATA-3 */

/*053*/	ushort		atavalid;		/* fields valid */

	ushort		obsolete54[5];

/*059*/	ushort		multi_count:8;
	ushort		multi_valid:1;
	ushort		:7;

/*060*/	ulong		lba_size;
	ushort		obsolete62;

/*063*/	ushort		mwdmamodes;		/* multiword DMA modes */ 
/*064*/	ushort		apiomodes;		/* advanced PIO modes */ 

/*065*/	ushort		mwdmamin;		/* min. M/W DMA time/word ns */
/*066*/	ushort		mwdmarec;		/* rec. M/W DMA time ns */
/*067*/	ushort		pioblind;		/* min. PIO cycle w/o flow */
/*068*/	ushort		pioiordy;		/* min. PIO cycle IORDY flow */
	ushort		reserved69;
	ushort		reserved70;
/*071*/	ushort		rlsovlap;		/* rel time (us) for overlap */
/*072*/	ushort		rlsservice;		/* rel time (us) for service */
	ushort		reserved73;
	ushort		reserved74;

/*075*/	ushort		queuelen:5;
	ushort		:11;

	ushort		reserved76;
	ushort		reserved77;
	ushort		reserved78;
	ushort		reserved79;
/*080*/	ushort		version_major;
/*081*/	ushort		version_minor;

	struct
	{
/*082/085*/	ushort	smart:1;
		ushort	security:1;
		ushort	removable:1;
		ushort	power_mngt:1;
		ushort	packet:1;
		ushort	write_cache:1;
		ushort	look_ahead:1;
		ushort	release_irq:1;
		ushort	service_irq:1;
		ushort	reset:1;
		ushort	protected:1;
		ushort	:1;
		ushort	write_buffer:1;
		ushort	read_buffer:1;
		ushort	nop:1;
		ushort	:1;

/*083/086*/	ushort	microcode:1;
		ushort	queued:1;
		ushort	cfa:1;
		ushort	apm:1;
		ushort	notify:1;
		ushort	standby:1;
		ushort	spinup:1;
		ushort	:1;
		ushort	max_security:1;
		ushort	auto_acoustic:1;
		ushort	address48:1;
		ushort	config_overlay:1;
		ushort	flush_cache:1;
		ushort	flush_cache48:1;
		ushort	support_one:1;
		ushort	support_zero:1;

/*084/087*/	ushort	smart_error_log:1;
		ushort	smart_self_test:1;
		ushort	media_serial_no:1;
		ushort	media_card_pass:1;
		ushort	streaming:1;
		ushort	logging:1;
		ushort	:8;
		ushort	extended_one:1;
		ushort	extended_zero:1;

	}	support, enabled;

/*088*/	ushort		udmamodes;		/* UltraDMA modes */
/*089*/	ushort		erase_time;
/*090*/	ushort		enhanced_erase_time;
/*091*/	ushort		apm_value;
/*092*/	ushort		master_passwd_revision;

/*093*/	ushort		hwres_master	:8;
	ushort		hwres_slave	:5;
	ushort		hwres_cblid	:1;
	ushort		hwres_valid:2;

/*094*/	ushort		current_acoustic:8;
	ushort		vendor_acoustic:8;

/*095*/	ushort		stream_min_req_size;
/*096*/	ushort		stream_transfer_time;
/*097*/	ushort		stream_access_latency;
/*098*/	ulong		stream_granularity;
/*100*/	ushort		lba_size48_1;
	ushort		lba_size48_2;
	ushort		lba_size48_3;
	ushort		lba_size48_4;
	ushort		reserved104[23];
/*127*/	ushort		removable_status;
/*128*/	ushort		security_status;
	ushort		reserved129[31];
/*160*/	ushort		cfa_powermode1;
	ushort		reserved161[14];
/*176*/	ushort		media_serial[30];
	ushort		reserved206[49];
/*255*/	ushort		integrity;

}	ATAPARAM;

#define	NOATAPARAM	(ATAPARAM *)0

/*
 ****************************************************************
 *	Complemento aos dados de um pedido			*
 ****************************************************************
 */
#define	NOATASCB	(ATASCB *)0

typedef struct atascb	ATASCB;

struct atascb
{
	/*
	 *	Pedido total
	 */
	daddr_t		scb_blkno;	/* No. de bloco */
	void		*scb_area;	/* Endereço da área */
	int		scb_count;	/* No. de setores total */
	int		scb_limit;	/* Tamanho máximo de cada fragmento */

	/*
	 *	Fragmento corrente
	 */
	daddr_t		scb_frag_blkno;	/* No. de bloco da cópia */
	void		*scb_frag_area;	/* Endereço da área da cópia */
	int		scb_frag_count;	/* No. de setores faltando da cópia */
	int		scb_frag_incr;	/* No. de setores transferidos a cada intr. */

	/*
	 *	Para os dispositivos ATAPI apenas
	 */
	char		*scb_cmd;	/* Comando SCSI corrente */
	void		*scb_addr;	/* Endereço da área */
	int		scb_cnt;	/* No. de bytes */
	int		scb_flags;	/* Indicadores */
	int		scb_result;	/* Resultado da operação */

	struct scsi_sense_data	scb_sense;	/* Resultado do REQUEST SENSE */

	/*
	 *	Campos comuns
	 */
	ATASCB		*scb_next;	/* Próximo SCB livre */
};

/*
 ****************************************************************
 *	Dados de um Dispositivo					*
 ****************************************************************
 */
#define	NOATA		(ATA *)NULL
#define	ATA_DEVNO	ap->at_unit, ap->at_target

/*
 ******	Tipos de dispositivos ***********************************
 */
enum
{
	ATA_NONE,		/* Nenhum dispositivo encontrado */
	ATA_HD,			/* Disco Rígido */
	ATAPI_CDROM,		/* CDROM ATAPI */
	ATAPI_ZIP		/* ZIP 100 ATAPI */
};

/*
 ******	Valores para "at_flags" *********************************
 */
#define	HD_32_BITS	0x01	/* O dispositivo aceita PIO de 32 bits */
#define	ATAPI_DEV	0x02	/* O dispositivo é ATAPI */
#define	HD_INT13_EXT	0x04	/* O disco aceita as extensões da INT 13 */
#define	HD_CBLID	0x08	/* O cabo permite Ultra-DMA-66 */
#define	HD_TAG_CAP	0x10	/* O disco aceita tag queuing */
#define	HD_CHS_USED	0x20	/* O disco deve usar o método geométrico */

/*
 ******	Estrutura de um dispositivo ******************************
 */
typedef struct
{
	int		at_type;	/* Tipo de periférico (ver "enum" acima) */

	int		at_unit;	/* No. do controlador */
	int		at_port;	/* Porta inicial do controlador */
	int		at_bmport;	/* "Bus Master Port" */
	int		at_target;	/* No. do alvo neste controlador */

	char		at_dev_nm[4];	/* Nome do dispositivo: "hda\0", ... */

	char		at_is_present;	/* Está presente */
	char		at_error;	/* Último erro */
	char		at_status;	/* Último estado */

	int		at_flags;	/* Indicadores (ver acima) */

	int		at_head;	/* No. de cabeças   (REAL) */
	int		at_sect;	/* No. de setores   (REAL) */
	int		at_cyl;		/* No. de cilindros (REAL) */
	int		at_multi;	/* No. máximo de blocos em uma operação */

	int		at_blshift;	/* Log (2) do tamanho acima */
	int		at_blsz;	/* Tamanho de um setor (em bytes) */

	daddr_t		at_disksz;	/* No. total de blocos */

	ATAPARAM	*at_param;	/* Parâmetros da controladora */

	/*
	 *	PIO de 16 ou 32 bits
	 */
	void		(*at_read_port)  (int, void *, int);
	void		(*at_write_port) (int, const void *, int);
	int		at_pio_shift;

	/*
	 *	Somente para dispositivos ATAPI
	 */
	int		at_cmdsz;	/* Tamanho do comando SCSI (em shorts) */
	SCSI		*at_scsi;	/* Parâmetros do SCSI */

#ifdef	BOOT
	/*
	 *	Parte utilizada apenas pelo BOOT2
	 */
	int		at_bios_head;	/* No. de cabeças   (BIOS) */
	int		at_bios_sect;	/* No. de setores   (BIOS) */
	int		at_bios_cyl;	/* No. de cilindros (BIOS) */

	const void	*at_disktb;	/* Ponteiro para DISKTB */
#else
	/*
	 *	Parte utilizada apenas pelo KERNEL
	 */
	schar		at_piomode;	/* PIO mode */
	schar		at_wdmamode;	/* Multi-word DMA mode */
	schar		at_udmamode;	/* Ultra-DMA mode */
	schar		at_filler0;

	DEVHEAD		at_utab;	/* Cabeça da lista de pedidos */
	BHEAD		*at_bp;		/* Pedido corrente */
#endif

}	ATA;

#ifndef	BOOT
/*
 ******	Informações por controlador *****************************
 */
typedef struct
{
	LOCK	unit_busy;		/* Controlador Unidade ocupada */

	int	unit_target;		/* O "target" ativo no momento */
	int	unit_irq;		/* No. do IRQ */
	ATA	*unit_active;		/* A unidade  ativa no momento */

	DMATB	*unit_dma_tb;		/* Tabela para programação do DMA */

	ATASCB	*unit_scb;		/* Lista de SCBs disponíveis */

}	ATAUNIT;
#endif
