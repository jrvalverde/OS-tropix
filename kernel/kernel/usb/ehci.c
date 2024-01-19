/*
 ****************************************************************
 *								*
 *			ehci.c					*
 *								*
 *	"Driver" para controladoras USB EHCI			*
 *								*
 *	Vers�o	4.3.0, de 07.10.02				*
 *		4.6.0, de 06.10.04				*
 *								*
 *	M�dulo: N�cleo						*
 *		N�CLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 2004 NCE/UFRJ - tecle "man licen�a"	*
 *		Baseado no FreeBSD 5.2				*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/region.h"
#include "../h/map.h"
#include "../h/scb.h"

#include "../h/pci.h"
#include "../h/frame.h"
#include "../h/intr.h"
#include "../h/usb.h"

#include "../h/timeout.h"

#include "../h/uerror.h"
#include "../h/signal.h"
#include "../h/uproc.h"

#include "../h/extern.h"
#include "../h/proto.h"

#include "../h/queue.h"

#if (0)	/*******************************************************/
/*
 ******	Tabelas de m�todos **************************************
 */
int			ehci_open (struct usbd_pipe *pipe_ptr);
void			ehci_softintr (void *v);
void			ehci_poll (struct usbd_bus *bus);
struct usbd_xfer	*ehci_allocx (struct usbd_bus *bus);
void			ehci_freex (struct usbd_bus *bus, struct usbd_xfer *xfer);

const struct usbd_bus_methods ehci_bus_methods =
{
	ehci_open,
	ehci_softintr,
	ehci_poll,
	NULL,		/* ehci_allocm (N�O PRECISA) */
	NULL,		/* ehci_freem  (N�O PRECISA) */
	ehci_allocx,
	ehci_freex
};

int			ehci_root_ctrl_transfer (struct usbd_xfer *xfer);
int			ehci_root_ctrl_start (struct usbd_xfer *xfer);
void			ehci_root_ctrl_abort (struct usbd_xfer *xfer);
void			ehci_root_ctrl_close (struct usbd_pipe *pipe_ptr);
void			ehci_noop (struct usbd_pipe *pipe_ptr);
void			ehci_root_ctrl_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_root_ctrl_methods =
{
	ehci_root_ctrl_transfer,
	ehci_root_ctrl_start,
	ehci_root_ctrl_abort,
	ehci_root_ctrl_close,
	ehci_noop,
	ehci_root_ctrl_done,
};

int			ehci_root_intr_transfer (struct usbd_xfer *xfer);
int			ehci_root_intr_start (struct usbd_xfer *xfer);
void			ehci_root_intr_abort (struct usbd_xfer *xfer);
void			ehci_root_intr_close (struct usbd_pipe *pipe_ptr);
void			ehci_root_intr_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_root_intr_methods =
{
	ehci_root_intr_transfer,
	ehci_root_intr_start,
	ehci_root_intr_abort,
	ehci_root_intr_close,
	ehci_noop,
	ehci_root_intr_done,
};

int			ehci_device_ctrl_transfer (struct usbd_xfer *xfer);
int			ehci_device_ctrl_start (struct usbd_xfer *xfer);
void			ehci_device_ctrl_abort (struct usbd_xfer *xfer);
void			ehci_device_ctrl_close (struct usbd_pipe *pipe_ptr);
void			ehci_device_ctrl_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_device_ctrl_methods =
{
	ehci_device_ctrl_transfer,
	ehci_device_ctrl_start,
	ehci_device_ctrl_abort,
	ehci_device_ctrl_close,
	ehci_noop,
	ehci_device_ctrl_done,
};

int			ehci_device_intr_transfer (struct usbd_xfer *xfer);
int			ehci_device_intr_start (struct usbd_xfer *xfer);
void			ehci_device_intr_abort (struct usbd_xfer *xfer);
void			ehci_device_intr_close (struct usbd_pipe *pipe_ptr);
void			ehci_device_clear_toggle (struct usbd_pipe *pipe_ptr);
void			ehci_device_intr_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_device_intr_methods =
{
	ehci_device_intr_transfer,
	ehci_device_intr_start,
	ehci_device_intr_abort,
	ehci_device_intr_close,
	ehci_device_clear_toggle,
	ehci_device_intr_done,
};

int			ehci_device_bulk_transfer (struct usbd_xfer *xfer);
int			ehci_device_bulk_start (struct usbd_xfer *xfer);
void			ehci_device_bulk_abort (struct usbd_xfer *xfer);
void			ehci_device_bulk_close (struct usbd_pipe *pipe_ptr);
void			ehci_device_bulk_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_device_bulk_methods =
{
	ehci_device_bulk_transfer,
	ehci_device_bulk_start,
	ehci_device_bulk_abort,
	ehci_device_bulk_close,
	ehci_device_clear_toggle,
	ehci_device_bulk_done,
};

int			ehci_device_isoc_transfer (struct usbd_xfer *xfer);
int			ehci_device_isoc_start (struct usbd_xfer *xfer);
void			ehci_device_isoc_abort (struct usbd_xfer *xfer);
void			ehci_device_isoc_close (struct usbd_pipe *pipe_ptr);
void			ehci_device_isoc_done (struct usbd_xfer *xfer);

const struct usbd_pipe_methods ehci_device_isoc_methods =
{
	ehci_device_isoc_transfer,
	ehci_device_isoc_start,
	ehci_device_isoc_abort,
	ehci_device_isoc_close,
	ehci_noop,
	ehci_device_isoc_done,
};

/*
 * Data structures and routines to emulate the root hub.
 */
const struct usb_device_descriptor ehci_devd =
{
	USB_DEVICE_DESCRIPTOR_SIZE,
	UDESC_DEVICE,		/* type */
	{0x00, 0x02},		/* USB version */
	UDCLASS_HUB,		/* class */
	UDSUBCLASS_HUB,		/* subclass */
	UDPROTO_HSHUBSTT,	/* protocol */
	64,			/* max packet */
	{0},{0},{0x00,0x01},	/* device id */
	1,2,0,			/* string indicies */
	1			/* # of configurations */
};

const struct usb_device_qualifier ehci_odevd =
{
	USB_DEVICE_DESCRIPTOR_SIZE,
	UDESC_DEVICE_QUALIFIER,	/* type */
	{0x00, 0x02},		/* USB version */
	UDCLASS_HUB,		/* class */
	UDSUBCLASS_HUB,		/* subclass */
	UDPROTO_FSHUB,		/* protocol */
	64,			/* max packet */
	1,			/* # of configurations */
	0
};

const struct usb_config_descriptor ehci_confd =
{
	USB_CONFIG_DESCRIPTOR_SIZE,
	UDESC_CONFIG,
	{USB_CONFIG_DESCRIPTOR_SIZE +
	 USB_INTERFACE_DESCRIPTOR_SIZE +
	 USB_ENDPOINT_DESCRIPTOR_SIZE},
	1,
	1,
	0,
	UC_SELF_POWERED,
	0			/* max power */
};

const struct usb_interface_descriptor ehci_ifcd =
{
	USB_INTERFACE_DESCRIPTOR_SIZE,
	UDESC_INTERFACE,
	0,
	0,
	1,
	UICLASS_HUB,
	UISUBCLASS_HUB,
	UIPROTO_HSHUBSTT,
	0
};

#define EHCI_INTR_ENDPT 1

const struct usb_endpoint_descriptor ehci_endpd =
{
	USB_ENDPOINT_DESCRIPTOR_SIZE,
	UDESC_ENDPOINT,
	UE_DIR_IN | EHCI_INTR_ENDPT,
	UE_INTERRUPT,
	{8, 0},			/* max packet */
	255
};

const struct usb_hub_descriptor ehci_hubd =
{
	USB_HUB_DESCRIPTOR_SIZE,
	UDESC_HUB,
	0,
	{0,0},
	0,
	0,
	{0},
};

/*
 ****** Registros ***********************************************
 */
#define PCI_EHCI_FLADJ		0x61	/*RW Frame len adj, SOF=59488+6*fladj */

#define PCI_EHCI_PORTWAKECAP	0x62	/* RW Port wake caps (opt)  */

/* Regs ar EECP + offset */
#define PCI_EHCI_USBLEGSUP	0x00
#define PCI_EHCI_USBLEGCTLSTS	0x04

/*** EHCI capability registers ***/

#define EHCI_CAPLENGTH		0x00	/*RO Capability register length field */
/* reserved			0x01 */
#define EHCI_HCIVERSION		0x02	/* RO Interface version number */

#define EHCI_HCSPARAMS		0x04	/* RO Structural parameters */
#define  EHCI_HCS_DEBUGPORT(x)	(((x) >> 20) & 0xf)
#define  EHCI_HCS_P_INCICATOR(x) ((x) & 0x10000)
#define  EHCI_HCS_N_CC(x)	(((x) >> 12) & 0xf) /* # of companion ctlrs */
#define  EHCI_HCS_N_PCC(x)	(((x) >> 8) & 0xf) /* # of ports per comp. */
#define  EHCI_HCS_PPC(x)	((x) & 0x10) /* port power control */
#define  EHCI_HCS_N_PORTS(x)	((x) & 0xf) /* # of ports */

#define EHCI_HCCPARAMS		0x08	/* RO Capability parameters */
#define  EHCI_HCC_EECP(x)	(((x) >> 8) & 0xff) /* extended ports caps */
#define  EHCI_HCC_IST(x)	(((x) >> 4) & 0xf) /* isoc sched threshold */
#define  EHCI_HCC_ASPC(x)	((x) & 0x4) /* async sched park cap */
#define  EHCI_HCC_PFLF(x)	((x) & 0x2) /* prog frame list flag */
#define  EHCI_HCC_64BIT(x)	((x) & 0x1) /* 64 bit address cap */

#define EHCI_HCSP_PORTROUTE	0x0c	/*RO Companion port route description */

/* EHCI operational registers.  Offset given by EHCI_CAPLENGTH register */
#define EHCI_USBCMD		0x00	/* RO, RW, WO Command register */
#define  EHCI_CMD_ITC_M		0x00ff0000 /* RW interrupt threshold ctrl */
#define   EHCI_CMD_ITC_1	0x00010000
#define   EHCI_CMD_ITC_2	0x00020000
#define   EHCI_CMD_ITC_4	0x00040000
#define   EHCI_CMD_ITC_8	0x00080000
#define   EHCI_CMD_ITC_16	0x00100000
#define   EHCI_CMD_ITC_32	0x00200000
#define   EHCI_CMD_ITC_64	0x00400000
#define  EHCI_CMD_ASPME		0x00000800 /* RW/RO async park enable */
#define  EHCI_CMD_ASPMC		0x00000300 /* RW/RO async park count */
#define  EHCI_CMD_LHCR		0x00000080 /* RW light host ctrl reset */
#define  EHCI_CMD_IAAD		0x00000040 /* RW intr on async adv door bell */
#define  EHCI_CMD_ASE		0x00000020 /* RW async sched enable */
#define  EHCI_CMD_PSE		0x00000010 /* RW periodic sched enable */
#define  EHCI_CMD_FLS_M		0x0000000c /* RW/RO frame list size */
#define  EHCI_CMD_FLS(x)	(((x) >> 2) & 3) /* RW/RO frame list size */
#define  EHCI_CMD_HCRESET	0x00000002 /* RW reset */
#define  EHCI_CMD_RS		0x00000001 /* RW run/stop */

#define EHCI_USBSTS		0x04	/* RO, RW, RWC Status register */
#define  EHCI_STS_ASS		0x00008000 /* RO async sched status */
#define  EHCI_STS_PSS		0x00004000 /* RO periodic sched status */
#define  EHCI_STS_REC		0x00002000 /* RO reclamation */
#define  EHCI_STS_HCH		0x00001000 /* RO host controller halted */
#define  EHCI_STS_IAA		0x00000020 /* RWC interrupt on async adv */
#define  EHCI_STS_HSE		0x00000010 /* RWC host system error */
#define  EHCI_STS_FLR		0x00000008 /* RWC frame list rollover */
#define  EHCI_STS_PCD		0x00000004 /* RWC port change detect */
#define  EHCI_STS_ERRINT	0x00000002 /* RWC error interrupt */
#define  EHCI_STS_INT		0x00000001 /* RWC interrupt */
#define  EHCI_STS_INTRS(x)	((x) & 0x3f)

#define EHCI_NORMAL_INTRS (EHCI_STS_IAA | EHCI_STS_HSE | EHCI_STS_PCD | EHCI_STS_ERRINT | EHCI_STS_INT)

#define EHCI_USBINTR		0x08	/* RW Interrupt register */
#define EHCI_INTR_IAAE		0x00000020 /* interrupt on async advance ena */
#define EHCI_INTR_HSEE		0x00000010 /* host system error ena */
#define EHCI_INTR_FLRE		0x00000008 /* frame list rollover ena */
#define EHCI_INTR_PCIE		0x00000004 /* port change ena */
#define EHCI_INTR_UEIE		0x00000002 /* USB error intr ena */
#define EHCI_INTR_UIE		0x00000001 /* USB intr ena */

#define EHCI_FRINDEX		0x0c	/* RW Frame Index register */

#define EHCI_CTRLDSSEGMENT	0x10	/* RW Control Data Structure Segment */

#define EHCI_PERIODICLISTBASE	0x14	/* RW Periodic List Base */
#define EHCI_ASYNCLISTADDR	0x18	/* RW Async List Base */

#define EHCI_CONFIGFLAG		0x40	/* RW Configure Flag register */
#define  EHCI_CONF_CF		0x00000001 /* RW configure flag */

#define EHCI_PORTSC(n)		(0x40+4*(n)) /* RO, RW, RWC Port Status reg */
#define  EHCI_PS_WKOC_E		0x00400000 /* RW wake on over current ena */
#define  EHCI_PS_WKDSCNNT_E	0x00200000 /* RW wake on disconnect ena */
#define  EHCI_PS_WKCNNT_E	0x00100000 /* RW wake on connect ena */
#define  EHCI_PS_PTC		0x000f0000 /* RW port test control */
#define  EHCI_PS_PIC		0x0000c000 /* RW port indicator control */
#define  EHCI_PS_PO		0x00002000 /* RW port owner */
#define  EHCI_PS_PP		0x00001000 /* RW,RO port power */
#define  EHCI_PS_LS		0x00000c00 /* RO line status */
#define  EHCI_PS_IS_LOWSPEED(x)	(((x) & EHCI_PS_LS) == 0x00000400)
#define  EHCI_PS_PR		0x00000100 /* RW port reset */
#define  EHCI_PS_SUSP		0x00000080 /* RW suspend */
#define  EHCI_PS_FPR		0x00000040 /* RW force port resume */
#define  EHCI_PS_OCC		0x00000020 /* RWC over current change */
#define  EHCI_PS_OCA		0x00000010 /* RO over current active */
#define  EHCI_PS_PEC		0x00000008 /* RWC port enable change */
#define  EHCI_PS_PE		0x00000004 /* RW port enable */
#define  EHCI_PS_CSC		0x00000002 /* RWC connect status change */
#define  EHCI_PS_CS		0x00000001 /* RO connect status */
#define  EHCI_PS_CLEAR		(EHCI_PS_OCC|EHCI_PS_PEC|EHCI_PS_CSC)

#define EHCI_PORT_RESET_COMPLETE 2 /* ms */

#define EHCI_FLALIGN_ALIGN	0x1000

/* No data structure may cross a page boundary. */
#define EHCI_PAGE_SIZE		0x1000
#define EHCI_PAGE(x)		((x) & ~0x0FFF)
#define EHCI_PAGE_OFFSET(x)	((x) &  0x0FFF)
#define EHCI_PAGE_MASK(x)	((x) &  0x0FFF)

typedef ulong ehci_link_t;
#define EHCI_LINK_TERMINATE	0x00000001
#define EHCI_LINK_TYPE(x)	((x) & 0x00000006)
#define  EHCI_LINK_ITD		0x0
#define  EHCI_LINK_QH		0x2
#define  EHCI_LINK_SITD		0x4
#define  EHCI_LINK_FSTN		0x6
#define EHCI_LINK_ADDR(x)	((x) &~ 0x1f)

/*
 ****** Descritor de transfer�ncias is�cronas *******************
 */
#define EHCI_ITD_ALIGN 32

typedef struct
{
	ehci_link_t	itd_next;
	/* XXX many more */

}	ehci_itd_t;

/*
 ****** Split Transaction Isochronous Transfer Descriptor *******
 */
#define EHCI_SITD_ALIGN 32

typedef struct
{
	ehci_link_t	sitd_next;
	/* XXX many more */

}	ehci_sitd_t;

/*
 ****** Queue Element Transfer Descriptor ***********************
 */
#define EHCI_QTD_NBUFFERS 5
#define EHCI_QTD_ALIGN 32

#define EHCI_QTD_GET_STATUS(x)	(((x) >>  0) & 0xff)
#define  EHCI_QTD_ACTIVE	0x80
#define  EHCI_QTD_HALTED	0x40
#define  EHCI_QTD_BUFERR	0x20
#define  EHCI_QTD_BABBLE	0x10
#define  EHCI_QTD_XACTERR	0x08
#define  EHCI_QTD_MISSEDMICRO	0x04
#define  EHCI_QTD_SPLITXSTATE	0x02
#define  EHCI_QTD_PINGSTATE	0x01
#define  EHCI_QTD_STATERRS	0x7c
#define EHCI_QTD_GET_PID(x)	(((x) >>  8) & 0x3)
#define EHCI_QTD_SET_PID(x)	((x) <<  8)
#define  EHCI_QTD_PID_OUT	0x0
#define  EHCI_QTD_PID_IN	0x1
#define  EHCI_QTD_PID_SETUP	0x2
#define EHCI_QTD_GET_CERR(x)	(((x) >> 10) &  0x3)
#define EHCI_QTD_SET_CERR(x)	((x) << 10)
#define EHCI_QTD_GET_C_PAGE(x)	(((x) >> 12) &  0x7)
#define EHCI_QTD_SET_C_PAGE(x)	((x) << 12)
#define EHCI_QTD_GET_IOC(x)	(((x) >> 15) &  0x1)
#define EHCI_QTD_IOC		0x00008000
#define EHCI_QTD_GET_BYTES(x)	(((x) >> 16) &  0x7fff)
#define EHCI_QTD_SET_BYTES(x)	((x) << 16)
#define EHCI_QTD_GET_TOGGLE(x)	(((x) >> 31) &  0x1)
#define EHCI_QTD_TOGGLE		0x80000000

typedef struct
{
	ehci_link_t	qtd_next;
	ehci_link_t	qtd_altnext;
	ulong		qtd_status;
	ulong		qtd_buffer[EHCI_QTD_NBUFFERS];

}	ehci_qtd_t;

/*
 ****** Queue Head **********************************************
 */
#define EHCI_QH_ALIGN		32

#define EHCI_QH_GET_ADDR(x)	(((x) >>  0) & 0x7f) /* endpoint addr */
#define EHCI_QH_SET_ADDR(x)	(x)
#define EHCI_QH_ADDRMASK	0x0000007f
#define EHCI_QH_GET_INACT(x)	(((x) >>  7) & 0x01) /* inactivate on next */
#define EHCI_QH_INACT		0x00000080
#define EHCI_QH_GET_ENDPT(x)	(((x) >>  8) & 0x0f) /* endpoint no */
#define EHCI_QH_SET_ENDPT(x)	((x) <<  8)
#define EHCI_QH_GET_EPS(x)	(((x) >> 12) & 0x03) /* endpoint speed */
#define EHCI_QH_SET_EPS(x)	((x) << 12)
#define  EHCI_QH_SPEED_FULL	0x0
#define  EHCI_QH_SPEED_LOW	0x1
#define  EHCI_QH_SPEED_HIGH	0x2
#define EHCI_QH_GET_DTC(x)	(((x) >> 14) & 0x01) /* data toggle control */
#define EHCI_QH_DTC		0x00004000
#define EHCI_QH_GET_HRECL(x)	(((x) >> 15) & 0x01) /* head of reclamation */
#define EHCI_QH_HRECL		0x00008000
#define EHCI_QH_GET_MPL(x)	(((x) >> 16) & 0x7ff) /* max packet len */
#define EHCI_QH_SET_MPL(x)	((x) << 16)
#define EHCI_QG_MPLMASK		0x07ff0000
#define EHCI_QH_GET_CTL(x)	(((x) >> 26) & 0x01) /* control endpoint */
#define EHCI_QH_CTL		0x08000000
#define EHCI_QH_GET_NRL(x)	(((x) >> 28) & 0x0f) /* NAK reload */
#define EHCI_QH_SET_NRL(x)	((x) << 28)
#define EHCI_QH_GET_SMASK(x)	(((x) >>  0) & 0xff) /* intr sched mask */
#define EHCI_QH_SET_SMASK(x)	((x) <<  0)
#define EHCI_QH_GET_CMASK(x)	(((x) >>  8) & 0xff) /* split completion mask */
#define EHCI_QH_SET_CMASK(x)	((x) <<  8)
#define EHCI_QH_GET_HUBA(x)	(((x) >> 16) & 0x7f) /* hub address */
#define EHCI_QH_SET_HUBA(x)	((x) << 16)
#define EHCI_QH_GET_PORT(x)	(((x) >> 23) & 0x7f) /* hub port */
#define EHCI_QH_SET_PORT(x)	((x) << 23)
#define EHCI_QH_GET_MULT(x)	(((x) >> 30) & 0x03) /* pipe multiplier */
#define EHCI_QH_SET_MULT(x)	((x) << 30)

typedef struct
{
	ehci_link_t	qh_link;
	ulong		qh_endp;
	ulong		qh_endphub;
	ehci_link_t	qh_curqtd;
	ehci_qtd_t	qh_qtd;

}	ehci_qh_t;

/*
 ****** Periodic Frame Span Traversal Node **********************
 */
#define EHCI_FSTN_ALIGN 32

typedef struct
{
	ehci_link_t	fstn_link;
	ehci_link_t	fstn_back;

}	ehci_fstn_t;

#define EHCI_SQTD_SIZE ((sizeof (struct ehci_soft_qtd) + EHCI_QTD_ALIGN - 1) / EHCI_QTD_ALIGN * EHCI_QTD_ALIGN)
#define EHCI_SQTD_CHUNK (EHCI_PAGE_SIZE / EHCI_SQTD_SIZE)

typedef struct ehci_soft_qtd
{
	ehci_qtd_t		qtd;
	struct ehci_soft_qtd	*nextqtd; /* mirrors nextqtd in TD */
	ulong			physaddr;
	struct usbd_xfer	*xfer;

	LIST_ENTRY(ehci_soft_qtd) hnext;

	ushort			len;

}	ehci_soft_qtd_t;


#define EHCI_SQH_SIZE ((sizeof (struct ehci_soft_qh) + EHCI_QH_ALIGN - 1) / EHCI_QH_ALIGN * EHCI_QH_ALIGN)
#define EHCI_SQH_CHUNK (EHCI_PAGE_SIZE / EHCI_SQH_SIZE)

typedef struct ehci_soft_qh
{
	ehci_qh_t		qh;
	struct ehci_soft_qh	*next;
	struct ehci_soft_qtd	*sqtd;
	ulong			physaddr;

}	ehci_soft_qh_t;

struct ehci_xfer
{
	struct usbd_xfer	xfer;
	struct usb_task		abort_task;

	LIST_ENTRY(ehci_xfer) inext; /* list of active xfers */

	ehci_soft_qtd_t		*sqtdstart;
	ehci_soft_qtd_t		*sqtdend;
};

/*
 ****** Estrutura "ehci_softc" **********************************
 */
#define EHCI_HASH_SIZE		128
#define EHCI_COMPANION_MAX	8

struct ehci_softc
{
	struct usbd_bus		sc_bus;			/* base device */

	ulong			sc_offs;		/* offset to operational regs */

	char			sc_vendor[16];		/* vendor string for root hub */
	int			sc_id_vendor;		/* vendor ID for root hub */

	ulong			sc_ncomp;
	ulong			sc_npcomp;
	struct usbd_bus		*sc_comps[EHCI_COMPANION_MAX];

	ulong			*sc_frame_list;
	ulong			sc_frame_list_sz;

	LIST_HEAD(, ehci_xfer)	sc_intrhead;

	ehci_soft_qh_t		*sc_freeqhs;
	ehci_soft_qtd_t		*sc_freeqtds;

	int			sc_noport;

	uchar			sc_addr;		/* device address */
	uchar			sc_conf;		/* device configuration */

	struct usbd_xfer	*sc_intrxfer;
	char			sc_isreset;

	ulong			sc_eintrs;
	ehci_soft_qh_t		*sc_async_head;

	EVENT			sc_async_head_done;

	struct usbd_xfer	*sc_free_xfers_first,	/* Lista de descritores livres */
				**sc_free_xfers_last;

	LOCK			sc_doorbell_lock;

	TIMEOUT			*sc_tmo_pcd;

#if (0)	/*******************************************************/
	struct device		*sc_child;		/* /dev/usb# device */
#endif	/*******************************************************/

	char			sc_dying;
};

/*
 ****** Estrutura "ehci_pipe" ***********************************
 */
struct ehci_pipe
{
	struct usbd_pipe		pipe;

	ehci_soft_qh_t			*sqh;

	union
	{
		ehci_soft_qtd_t		*qtd;
#if (0)	/*******************************************************/
		ehci_soft_itd_t		*itd;
#endif	/*******************************************************/

	}	tail;

	union
	{
		struct					/* Control pipe */
		{
			void		*reqdma;
			ulong		length;

#if (0)	/*******************************************************/
			ehci_soft_qtd_t *setup, *data, *stat;
#endif	/*******************************************************/
		}	ctl;

		struct					/* Bulk pipe */
		{
			ulong		length;

		}	bulk;

		/* Interrupt pipe */
		/* Iso pipe */
	} u;
};

#define EHCI_NULL	(EHCI_LINK_TERMINATE)

#define ehci_add_intr_list(sc, ex) \
	LIST_INSERT_HEAD(&(sc)->sc_intrhead, (ex), inext);

#define ehci_del_intr_list(ex) \
	do { \
		LIST_REMOVE((ex), inext); \
		(ex)->inext.le_prev = NULL; \
	} while (0)

#define ehci_active_intr_list(ex) ((ex)->inext.le_prev != NULL)

/*
 ******	Prot�tipos de fun��es ***********************************
 */
int			ehci_init (struct ehci_softc *sc);
void			ehci_intr (struct intr_frame frame);
void			ehci_intr1 (struct ehci_softc *sc);
void			ehci_pcd_able (struct ehci_softc *sc, int on);
void			ehci_pcd_enable (struct ehci_softc *sc);
void			ehci_pcd (struct ehci_softc *sc, struct usbd_xfer *xfer);
void			ehci_check_intr (struct ehci_softc *sc, struct ehci_xfer *ex);
void			ehci_idone (struct ehci_xfer *ex);
int			ehci_device_request (struct usbd_xfer *xfer);
void			ehci_waitintr (struct ehci_softc *sc, struct usbd_xfer *xfer);
void			ehci_disown (struct ehci_softc *sc, int index, int lowspeed);
void			ehci_free_sqtd_chain (struct ehci_softc *sc, ehci_soft_qtd_t *sqtd, ehci_soft_qtd_t *sqtdend);
int			ehci_alloc_sqtd_chain
			(	struct ehci_pipe *epipe, struct ehci_softc *sc,
				int alen, int rd, struct usbd_xfer *xfer,
				ehci_soft_qtd_t **sp, ehci_soft_qtd_t **ep
			);
void			ehci_abort_xfer (struct usbd_xfer *xfer, int status);
void			ehci_close_pipe (struct usbd_pipe *pipe_ptr, ehci_soft_qh_t *head);
void			ehci_set_qh_qtd (ehci_soft_qh_t *sqh, ehci_soft_qtd_t *sqtd);
void			ehci_sync_hc (struct ehci_softc *sc);
ehci_soft_qh_t		*ehci_alloc_sqh (struct ehci_softc *sc);
void			ehci_free_sqh (struct ehci_softc *sc, ehci_soft_qh_t *sqh);
void			ehci_free_sqtd (struct ehci_softc *sc, ehci_soft_qtd_t *sqtd);
ehci_soft_qtd_t		*ehci_alloc_sqtd (struct ehci_softc *sc);
void			ehci_add_qh (ehci_soft_qh_t *sqh, ehci_soft_qh_t *head);
void			ehci_rem_qh (struct ehci_softc *sc, ehci_soft_qh_t *sqh, ehci_soft_qh_t *head);

int			ehci_read_port_or_mem_char (const struct ehci_softc *up, int reg);
void			ehci_write_port_or_mem_char (const struct ehci_softc *up, int value, int reg);
int			ehci_read_port_or_mem_short (const struct ehci_softc *up, int reg);
void			ehci_write_port_or_mem_short (const struct ehci_softc *up, int value, int reg);
int			ehci_read_port_or_mem_long (const struct ehci_softc *up, int reg);
void			ehci_write_port_or_mem_long (const struct ehci_softc *up, int value, int reg);

void			ehci_dump_regs (struct ehci_softc *sc);
#endif	/*******************************************************/

/*
 ****************************************************************
 *	Fun��o de "probe" dos controladores USB EHCI		*
 ****************************************************************
 */
char *
ehci_probe (PCIDATA *pci, ulong dev_vendor)
{
	if
	(	pci->pci_baseclass != PCIC_SERIALBUS     ||
		pci->pci_subclass  != PCIS_SERIALBUS_USB ||
		pci->pci_progif    != PCI_INTERFACE_EHCI
	)
		return (NOSTR);

	switch (dev_vendor)
	{
	    case 0x00E01033:
		return ("NEC uPD 720100 USB 2.0 controller");

	    default:
		return ("EHCI (generic) USB controller");
	}

}	/* ehci_probe */

/*
 ****************************************************************
 *	Fun��o de "attach" dos controladores USB UHCI		*
 ****************************************************************
 */
void
ehci_attach (PCIDATA *pci, ulong dev_vendor)
{
#if (0)	/*******************************************************/
	ulong			base_addr;
	struct ehci_softc	*sc;
	struct device		*ehci, *usb;
#endif	/*******************************************************/

	/*
	 *	Verifica se este controlador deve ser habilitado
	 */
	if (++usb_unit >= NUSB)
		{ printf ("ehci_attach: N�O h� mais espa�o para controladores USB\n"); return; }

	if (scb.y_usb_enable[usb_unit] == 0)
		return;

	printf ("ehci_attach: Controladora USB EHCI ainda N�O suportada\n");

	return;

#if (0)	/*******************************************************/
	/*
	 *	Aloca e zera a estrutura "ehci_softc"
	 */
	if ((sc = malloc_byte (sizeof (struct ehci_softc))) == NULL)
		{ printf ("ehci_attach: mem�ria esgotada\n"); return; }

	memclr (sc, sizeof (struct ehci_softc));

	usb_data[usb_unit] = sc;

	/*
	 *	Obt�m o endere�o base (sempre mapeado em mem�ria)
	 */
	base_addr = pci_read (pci, PCI_CBMEM, 4) & ~3;

	if ((sc->sc_bus.base_addr = mmu_map_phys_addr (base_addr, 1024)) == NULL)
	{
		printf ("ehci_attach: N�o consegui mapear os registros de usb%d\n", usb_unit);
		goto bad;
	}

   	sc->sc_bus.port_mapped	= 0;
	sc->sc_bus.unit		= usb_unit;
	sc->sc_bus.irq		= pci->pci_intline;

	/*
	 *	Imprime o que encontrou
	 */
	printf ("usb: [%d: 0x%X, %d, EHCI rev = ", usb_unit, base_addr, sc->sc_bus.irq);

	switch (pci_read (pci, PCI_USBREV, 1) & PCI_USBREV_MASK)
	{
	    case PCI_USBREV_2_0:
		sc->sc_bus.usbrev = USBREV_2_0;
		printf ("2.0]\n");
		break;

	    default:
		sc->sc_bus.usbrev = USBREV_UNKNOWN;
		printf ("pre-2.0]\n");
		goto bad;
	}

	/*
	 *	Prepara a interrup��o
	 */
	if (set_dev_irq (sc->sc_bus.irq, USB_PL, usb_unit, ehci_intr) < 0)
		goto bad;

	/*
	 *	Habilita o "BUS mastering"
	 */
	pci_write (pci, PCIR_COMMAND, pci_read (pci, PCIR_COMMAND, 2) | PCIM_CMD_BUSMASTEREN, 2);

	/*
	 *	Cria o dispositivo pai na m�o
	 */
	if (usb_class_list == NULL && usb_create_classes () < 0)
		goto bad;

	ehci = make_device (NULL, "usb_controller", -1);

	if ((usb = device_add_child (ehci, "usb", -1)) == NULL)
		{ printf ("ehci_attach: mem�ria esgotada\n"); goto bad; }

	usb->ivars = sc; sc->sc_bus.bdev = usb;

#if (0)	/*******************************************************/
	/*
	 * Find companion controllers.  According to the spec they always
	 * have lower function numbers so they should be enumerated already.
	 */
	parent = device_get_parent(self);
	res = device_get_children(parent, &neighbors, &count);
	if (res != 0) {
		device_printf(self, "Error finding companion busses\n");
		ehci_pci_detach(self);
		return ENXIO;
	}

	ncomp = 0;

	slot = pci_get_slot(self);
	function = pci_get_function(self);

	for (i = 0; i < count; i++)
	{
		if (pci_get_slot(neighbors[i]) == slot && pci_get_function(neighbors[i]) < function)
		{
			res = device_get_children(neighbors[i], &nbus, &buscount);

			if (res != 0 || buscount != 1)
				continue;

			bsc = device_get_softc(nbus[0]);

			printf("ehci_pci_attach: companion %s\n", USBDEVNAME(bsc->bdev));

			sc->sc_comps[ncomp++] = bsc;

			if (ncomp >= EHCI_COMPANION_MAX)
				break;
		}
	}
	sc->sc_ncomp = ncomp;
#endif	/*******************************************************/

	sc->sc_ncomp = 0;

	/*
	 *	Inicializa��o
	 */
	if (ehci_init (sc) != USBD_NORMAL_COMPLETION)
		{ printf ("ehci_attach: Erro na inicializa��o de usb%d\n", usb_unit); goto bad; }

	/*
	 *	Somente agora deixa interromper
	 */
	sc->sc_eintrs = EHCI_NORMAL_INTRS;

	ehci_write_port_or_mem_long (sc, sc->sc_eintrs, sc->sc_offs + EHCI_USBINTR);

	/*
	 *	Procura dispsitivos USB
	 */
	device_probe_and_attach (usb);

	return;

    bad:
	free_byte (sc); 	usb_data[usb_unit] = NULL;
#endif	/*******************************************************/

}	/* end ehci_attach */

#if (0)	/*******************************************************/
/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_init (struct ehci_softc *sc)
{
	ulong		version, sparams, cparams, hcr, i;
	int		err;
	ehci_soft_qh_t	*sqh;

	/*
	 *	Obt�m o deslocamento para os registros operacionais e a vers�o
	 */
	sc->sc_offs = ehci_read_port_or_mem_char (sc, EHCI_CAPLENGTH);

	version = ehci_read_port_or_mem_short (sc, EHCI_HCIVERSION);

	printf ("%s: EHCI version %02X.%02X\n", sc->sc_bus.bdev->nameunit, version >> 8, version & 0xff);

	sparams = ehci_read_port_or_mem_long (sc, EHCI_HCSPARAMS);

#ifdef	USB_MSG
	printf ("ehci_init: sparams=%P\n", sparams);
#endif	USB_MSG

	sc->sc_npcomp = EHCI_HCS_N_PCC (sparams);

	if (EHCI_HCS_N_CC (sparams) != sc->sc_ncomp)
	{
		printf
		(	"%s: n�mero incorreto de controladores colaterais (%d != %d)\n",
			sc->sc_bus.bdev->nameunit, EHCI_HCS_N_CC (sparams), sc->sc_ncomp
		);

#if (0)	/*******************************************************/
		return (USBD_IOERROR);
#endif	/*******************************************************/
	}

	if (sc->sc_ncomp > 0)
	{
		printf
		(	"%s: controlador colatera%s, %d porta%s cada:",
			sc->sc_bus.bdev->nameunit, sc->sc_ncomp!=1 ? "is" : "l",
			EHCI_HCS_N_PCC (sparams), EHCI_HCS_N_PCC (sparams) != 1 ? "s" : ""
		);

		for (i = 0; i < sc->sc_ncomp; i++)
			printf (" %s", sc->sc_comps[i]->bdev->nameunit);

		printf ("\n");
	}

	sc->sc_noport = EHCI_HCS_N_PORTS (sparams);

	cparams = ehci_read_port_or_mem_long (sc, EHCI_HCCPARAMS);

#ifdef	USB_MSG
	printf ("ehci_init: cparams=%P\n", cparams);
#endif	USB_MSG

	if (EHCI_HCC_64BIT (cparams))
	{
		/* Limpa a parte alta do endere�o-base da Frame List */

		ehci_write_port_or_mem_long (sc, 0, sc->sc_offs + EHCI_CTRLDSSEGMENT);
	}

	sc->sc_bus.usbrev = USBREV_2_0;

	/*
	 *	D� o "reset" no controlador
	 */
	printf ("%s: resetting\n", sc->sc_bus.bdev->nameunit);

	ehci_write_port_or_mem_long (sc, 0, sc->sc_offs + EHCI_USBINTR);
	ehci_write_port_or_mem_long (sc, 0, sc->sc_offs + EHCI_USBCMD);

	usb_delay_ms (&sc->sc_bus, 1);

	ehci_write_port_or_mem_long (sc, EHCI_CMD_HCRESET, sc->sc_offs + EHCI_USBCMD);

	for (i = 0; /* abaixo */; i++)
	{
		if (i >= 100)
		{
			printf ("%s: tempo expirado para \"reset\"\n", sc->sc_bus.bdev->nameunit);
			return (USBD_IOERROR);
		}

		usb_delay_ms (&sc->sc_bus, 1);

		if ((hcr = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD) & EHCI_CMD_HCRESET) == 0)
			break;
	}

	/*
	 *	Obt�m o tamanho da Frame List
	 */
	switch (EHCI_CMD_FLS (ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD)))
	{
	    case 0:
		sc->sc_frame_list_sz = 1024;
		break;

	    case 1:
		sc->sc_frame_list_sz = 512;
		break;

	    case 2:
		sc->sc_frame_list_sz = 256;
		break;

	    case 3:
		return (USBD_IOERROR);
	}

	/*
	 *	Aloca a Frame List e inicializa todos os seus descritores como inv�lidos
	 *	O endere�o base deve ser m�ltiplo de 4k
	 */
	i = sc->sc_frame_list_sz * sizeof (ulong);

	if ((sc->sc_frame_list = (ulong *)PGTOBY (malloce (M_CORE, BYUPPG (i)))) == NULL)
		return (USBD_NOMEM);

	for (i = 0; i < sc->sc_frame_list_sz; i++)
		sc->sc_frame_list[i] = 1;

	/*
	 *	Aloca o QH fict�cio que inicia a lista ass�ncrona
	 */
	if ((sqh = ehci_alloc_sqh (sc)) == NULL)
		{ err = USBD_NOMEM; goto bad; }

	/* Fill the QH */

	sqh->qh.qh_endp		   = EHCI_QH_SET_EPS (EHCI_QH_SPEED_HIGH) | EHCI_QH_HRECL;
	sqh->qh.qh_link		   = (sqh->physaddr | EHCI_LINK_QH);
	sqh->qh.qh_curqtd	   = EHCI_NULL;
	sqh->next		   = NULL;

	/* Fill the overlay qTD */

	sqh->qh.qh_qtd.qtd_next	   = EHCI_NULL;
	sqh->qh.qh_qtd.qtd_altnext = EHCI_NULL;
	sqh->qh.qh_qtd.qtd_status  = EHCI_QTD_HALTED;

	sqh->sqtd		   = NULL;

#ifdef USB_DEBUG
	if (ehcidebug) {
		ehci_dump_sqh(sqh);
	}
#endif

	/* Inicializa a lista ass�ncrona */

	sc->sc_async_head = sqh;

	ehci_write_port_or_mem_long (sc, sqh->physaddr | EHCI_LINK_QH, sc->sc_offs + EHCI_ASYNCLISTADDR);

	/* Inicializa a lista peri�dica */

	ehci_write_port_or_mem_long (sc, 0, sc->sc_offs + EHCI_FRINDEX);
	ehci_write_port_or_mem_long (sc, VIRT_TO_PHYS_ADDR (sc->sc_frame_list), sc->sc_offs + EHCI_PERIODICLISTBASE);

#if (0)	/*******************************************************/
	usb_callout_init(sc->sc_tmo_pcd);
#endif	/*******************************************************/

   /***	EVENTCLEAR (&sc->sc_async_head_done);	***/
   /***	SLEEPCLEAR (&sc->sc_doorbell_lock);	***/

	/*
	 *	Inicia o processamento no controlador
	 */
	i = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD) & EHCI_CMD_FLS_M;

	ehci_write_port_or_mem_long
	(	sc, 
		EHCI_CMD_ITC_8 |			/* 8 microframes */
		i |					/* Tamanho da FRAMELIST */
		EHCI_CMD_ASE |				/* Lista ass�ncrona habilitada */
#if (0)	/*******************************************************/
		EHCI_CMD_PSE |				/* Lista peri�dica  habilitada */
#endif	/*******************************************************/
		EHCI_CMD_RS,				/* Run */
		sc->sc_offs + EHCI_USBCMD
	);

	/*
	 *	Roteia todas as portas para este controlador
	 */
	ehci_write_port_or_mem_long (sc, EHCI_CONF_CF, sc->sc_offs + EHCI_CONFIGFLAG);

	for (i = 0; /* abaixo */; i++)
	{
		if (i >= 100)
		{
			printf ("%s: tempo expirado\n", sc->sc_bus.bdev->nameunit);
			return (USBD_IOERROR);
		}

		usb_delay_ms (&sc->sc_bus, 1);

		hcr = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS) & EHCI_STS_HCH;

		if (hcr == 0)
			break;
	}

	/*
	 *	Inicializa o restante das estruturas de dados
	 */
	sc->sc_bus.methods	= &ehci_bus_methods;
	sc->sc_bus.pipe_size	= sizeof (struct ehci_pipe);

	return (USBD_NORMAL_COMPLETION);

	/*
	 *	Em caso de erro, ...
	 */
    bad:
	mrelease (M_CORE, BYUPPG (sc->sc_frame_list_sz), BYUPPG (sc->sc_frame_list));
	return (err);

}	/* end ehci_init */

/*
 ****************************************************************
 *	Rotina de Interrup��o					*
 ****************************************************************
 */
void
ehci_intr (struct intr_frame frame)
{
	struct ehci_softc	*sc = usb_data[frame.if_unit];

	if (sc == NULL || sc->sc_dying)
		return;

#ifdef	USB_MSG
	printf ("ehci_intr: real interrupt\n");
#endif	USB_MSG

#ifdef	USB_POLLING
	if (sc->sc_bus.use_polling)
	{
		printf ("ehci_intr: ignored interrupt while polling\n");
		return;
	}
#endif	USB_POLLING

	ehci_intr1 (sc);

}	/* end ehci_intr */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_intr1 (struct ehci_softc *sc)
{
	ulong		intrs, eintrs, status;

	/* In case the interrupt occurs before initialization has completed. */

	if (sc == NULL)
		{ printf ("ehci_intr: sc == NULL\n"); return; }

	status = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS);
	intrs  = EHCI_STS_INTRS (status);

#if (0)	/*******************************************************/
printf ("ehci_intr1: status = %P, intrs = %P, sc->sc_eintrs = %P\n", status, intrs, sc->sc_eintrs);
#endif	/*******************************************************/

	if (intrs == 0)
		return;

	ehci_write_port_or_mem_long (sc, intrs, sc->sc_offs + EHCI_USBSTS); /* Acknowledge */

	eintrs = intrs & sc->sc_eintrs;

#ifdef	USB_MSG
	printf
	(	"ehci_intr: sc=%P intrs=%P(%P) eintrs=%P\n",
		sc, intrs, ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS), eintrs
	);
#endif	USB_MSG

	if (eintrs == 0)
		return;

	sc->sc_bus.intr_context++; 	sc->sc_bus.no_intrs++;

	if (eintrs & EHCI_STS_IAA)
	{
		printf ("ehci_intr1: door bell\n");

		EVENTDONE (&sc->sc_async_head_done);

		eintrs &= ~EHCI_STS_IAA;
	}

	if (eintrs & (EHCI_STS_INT | EHCI_STS_ERRINT))
	{
#ifdef	USB_MSG
		printf
		(	"ehci_intr1: %s %s\n",
			eintrs & EHCI_STS_INT ? "INT" : "",
			eintrs & EHCI_STS_ERRINT ? "ERRINT" : ""
		);
#endif	USB_MSG

		usb_schedsoftintr (&sc->sc_bus);

		eintrs &= ~(EHCI_STS_INT | EHCI_STS_ERRINT);
	}

	if (eintrs & EHCI_STS_HSE)
	{
		printf ("%s: unrecoverable error, controller halted\n", sc->sc_bus.bdev->nameunit);
		/* XXX what else */
	}

	if (eintrs & EHCI_STS_PCD)
	{
		ehci_pcd (sc, sc->sc_intrxfer);

		/* Disable PCD interrupt for now, because it will be on until the port has been reset */

		ehci_pcd_able (sc, 0);

		/* Do not allow RHSC interrupts > 1 per second */

		sc->sc_tmo_pcd = toutset (scb.y_hz, ehci_pcd_enable, (int)sc);

#if (0)	/*******************************************************/
                usb_callout(sc->sc_tmo_pcd, hz, ehci_pcd_enable, sc);
#endif	/*******************************************************/

		eintrs &= ~EHCI_STS_PCD;
	}

	sc->sc_bus.intr_context--;

	if (eintrs != 0)
	{
		/* Block unprocessed interrupts */

		sc->sc_eintrs &= ~eintrs;

		ehci_write_port_or_mem_long (sc, sc->sc_eintrs, sc->sc_offs + EHCI_USBINTR);

		printf ("%s: blocking intrs %P\n", sc->sc_bus.bdev->nameunit, eintrs);
	}

}	/* end ehci_intr1 */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_pcd_able (struct ehci_softc *sc, int on)
{
	printf ("ehci_pcd_able: on=%d\n", on);

	if (on)
		sc->sc_eintrs |= EHCI_STS_PCD;
	else
		sc->sc_eintrs &= ~EHCI_STS_PCD;

	ehci_write_port_or_mem_long (sc, sc->sc_eintrs, sc->sc_offs + EHCI_USBINTR);

}	/* end ehci_pcd_able */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_pcd_enable (struct ehci_softc *sc)
{
	ehci_pcd_able (sc, 1);

}	/* end ehci_pcd_enable */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_pcd (struct ehci_softc *sc, struct usbd_xfer *xfer)
{
#if (0)	/*******************************************************/
	struct usbd_pipe	*pipe_ptr;
#endif	/*******************************************************/
	uchar			*p;
	int			i, m;

	if (xfer == NULL)
		return;

#if (0)	/*******************************************************/
	pipe_ptr = xfer->pipe;
#endif	/*******************************************************/

	p = xfer->dmabuf;

	m = MIN (sc->sc_noport, xfer->length * 8 - 1);

	memclr (p, xfer->length);

	for (i = 1; i <= m; i++)
	{
		/* Pick out CHANGE bits from the status reg */

		if (ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_PORTSC (i)) & EHCI_PS_CLEAR)
			p[i / 8] |= 1 << (i % 8);
	}

	printf ("ehci_pcd: change=0x%02X\n", *p);

	xfer->actlen = xfer->length;
	xfer->status = USBD_NORMAL_COMPLETION;

	usb_transfer_complete (xfer);

}	/* end ehci_pcd */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_open (struct usbd_pipe *pipe_ptr)
{
	struct usbd_device		*udev	 = pipe_ptr->device;
	struct ehci_softc		*sc 	 = (struct ehci_softc *)udev->bus;
	struct usb_endpoint_descriptor	*ed	 = pipe_ptr->endpoint->edesc;
	uchar				addr	 = udev->address;
	uchar				xfertype = ed->bmAttributes & UE_XFERTYPE;
	struct ehci_pipe		*epipe	 = (struct ehci_pipe *)pipe_ptr;
	ehci_soft_qh_t			*sqh;
	int				s, speed, naks;

#ifdef	USB_MSG
	printf
	(	"ehci_open: pipe=%P, addr=%d, endpt=%d (%d)\n",
		pipe_ptr, addr, ed->bEndpointAddress, sc->sc_addr
	);
#endif	USB_MSG

	if (sc->sc_dying)
		return (USBD_IOERROR);

	if (addr == sc->sc_addr)
	{
		switch (ed->bEndpointAddress)
		{
		    case USB_CONTROL_ENDPOINT:
			pipe_ptr->methods = &ehci_root_ctrl_methods;
			break;

		    case UE_DIR_IN | EHCI_INTR_ENDPT:
			pipe_ptr->methods = &ehci_root_intr_methods;
			break;

		    default:
			return (USBD_INVAL);
		}

		return (USBD_NORMAL_COMPLETION);
	}

	/* XXX All this stuff is only valid for async */

	switch (udev->speed)
	{
	    case USB_SPEED_LOW:
		speed = EHCI_QH_SPEED_LOW;
		break;

	    case USB_SPEED_FULL:
		speed = EHCI_QH_SPEED_FULL;
		break;

	    case USB_SPEED_HIGH:
		speed = EHCI_QH_SPEED_HIGH;
		break;

	    default:
		printf ("ehci_open: bad device speed %d", udev->speed);
		return (USBD_INVAL);
	}

	naks = 8;		/* XXX */

	if ((sqh = ehci_alloc_sqh (sc)) == NULL)
		goto bad0;

	/* qh_link filled when the QH is added */

	sqh->qh.qh_endp = EHCI_QH_SET_ADDR(addr) |
				EHCI_QH_SET_ENDPT(ed->bEndpointAddress) |
				EHCI_QH_SET_EPS(speed) | /* XXX */
				/* XXX EHCI_QH_DTC ? */
				EHCI_QH_SET_MPL(UGETW(ed->wMaxPacketSize)) |
				(speed != EHCI_QH_SPEED_HIGH && xfertype == UE_CONTROL ?
				EHCI_QH_CTL : 0) |
				EHCI_QH_SET_NRL(naks);

	sqh->qh.qh_endphub = EHCI_QH_SET_MULT(1);	/* XXX TT stuff XXX interrupt mask */

	sqh->qh.qh_curqtd = EHCI_NULL;

	/* Fill the overlay qTD */

	sqh->qh.qh_qtd.qtd_next		= EHCI_NULL;
	sqh->qh.qh_qtd.qtd_altnext	= EHCI_NULL;
	sqh->qh.qh_qtd.qtd_status	= 0;

	epipe->sqh = sqh;

	switch (xfertype)
	{
	    case UE_CONTROL:
		if ((epipe->u.ctl.reqdma = malloc_byte (sizeof (struct usb_device_request))) == NULL)
			goto bad1;

		pipe_ptr->methods = &ehci_device_ctrl_methods;

		s = splx (irq_pl_vec[sc->sc_bus.irq]);

		ehci_add_qh (sqh, sc->sc_async_head);

		splx (s);
		break;

	    case UE_BULK:
		pipe_ptr->methods = &ehci_device_bulk_methods;

		s = splx (irq_pl_vec[sc->sc_bus.irq]);

		ehci_add_qh (sqh, sc->sc_async_head);

		splx(s);
		break;

	    case UE_INTERRUPT:
		pipe_ptr->methods = &ehci_device_intr_methods;
		return (USBD_INVAL);

	    case UE_ISOCHRONOUS:
		pipe_ptr->methods = &ehci_device_isoc_methods;
		return (USBD_INVAL);

	    default:
		return (USBD_INVAL);
	}

	return (USBD_NORMAL_COMPLETION);

	/*
	 *	Em caso de erro, ...
	 */
 bad1:
	ehci_free_sqh (sc, sqh);
 bad0:
	return (USBD_NOMEM);

}	/* end ehci_open */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_softintr (void *v)
{
	struct ehci_softc	*sc = v;
	struct ehci_xfer	*ex;

#ifdef	USB_MSG
	printf ("%s: ehci_softintr (%d)\n", sc->sc_bus.bdev->nameunit, sc->sc_bus.intr_context);
#endif	USB_MSG

	sc->sc_bus.intr_context++;

	/*
	 * The only explanation I can think of for why EHCI is as brain dead
	 * as UHCI interrupt-wise is that Intel was involved in both.
	 * An interrupt just tells us that something is done, we have no
	 * clue what, so we need to scan through all active transfers. :-(
	 */
	for (ex = LIST_FIRST (&sc->sc_intrhead); ex != NULL; ex = LIST_NEXT (ex, inext))
		ehci_check_intr (sc, ex);

	sc->sc_bus.intr_context--;

}	/* end ehci_softintr */

/*
 ****************************************************************
 *	Check for an interrupt					*
 ****************************************************************
 */
void
ehci_check_intr (struct ehci_softc *sc, struct ehci_xfer *ex)
{
	ehci_soft_qtd_t		*sqtd, *lsqtd;
	ulong			status;

#ifdef	USB_MSG
	printf ("ehci_check_intr: ex=%P\n", ex);
#endif	USB_MSG

	if (ex->sqtdstart == NULL)
		{ printf("ehci_check_intr: sqtdstart=NULL\n"); return; }

	lsqtd = ex->sqtdend;

	if (lsqtd == NULL)
		{ printf ("ehci_check_intr: sqtd==0\n"); return; }

	/*
	 * If the last TD is still active we need to check whether there
	 * is a an error somewhere in the middle, or whether there was a
	 * short packet (SPD and not ACTIVE).
	 */
	if (lsqtd->qtd.qtd_status & EHCI_QTD_ACTIVE)
	{
#ifdef	USB_MSG
		printf ("ehci_check_intr: active ex=%P, lsqtd = %P\n", ex, lsqtd);
#endif	USB_MSG

		for (sqtd = ex->sqtdstart; sqtd != lsqtd; sqtd = sqtd->nextqtd)
		{
			status = sqtd->qtd.qtd_status;

			/* If there's an active QTD the xfer isn't done. */

			if (status & EHCI_QTD_ACTIVE)
				break;

			/* Any kind of error makes the xfer done. */

			if (status & EHCI_QTD_HALTED)
				goto done;

			/* We want short packets, and it is short: it's done */

			if (EHCI_QTD_SET_BYTES (status) != 0)
				{ printf ("Haha: sqtd = %P\n", sqtd); goto done; }
		}

#ifdef	USB_MSG
		printf ("ehci_check_intr: ex=%P sqtd=%P still active\n", ex, sqtd);
#endif	USB_MSG
		return;
	}

    done:
#ifdef	USB_MSG
	printf ("ehci_check_intr: ex=%P done\n", ex);
#endif	USB_MSG

#if (0)	/*******************************************************/
	usb_uncallout(ex->xfer.timeout_handle, ehci_timeout, ex);
#endif	/*******************************************************/

	ehci_idone (ex);

}	/* end ehci_check_intr */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_idone (struct ehci_xfer *ex)
{
	struct usbd_xfer	*xfer = &ex->xfer;
#if (0)	/*******************************************************/
	struct ehci_pipe	*epipe = (struct ehci_pipe *)xfer->pipe;
#endif	/*******************************************************/
	ehci_soft_qtd_t		*sqtd;
	ulong			status = 0, nstatus;
	int			actlen;

#ifdef	USB_MSG
	printf ("ehci_idone: ex=%P\n", ex);
#endif	USB_MSG

	if (xfer->status == USBD_CANCELLED || xfer->status == USBD_TIMEOUT)
		{ printf ("ehci_idone: aborted xfer=%P\n", xfer); return; }

#ifdef USB_DEBUG
	DPRINTFN(/*10*/2, ("ehci_idone: xfer=%P, pipe=%P ready\n", xfer, epipe));
	if (ehcidebug > 10)
		ehci_dump_sqtds(ex->sqtdstart);
#endif

	/* The transfer is done, compute actual length and status. */

	actlen = 0;

	for (sqtd = ex->sqtdstart; sqtd != NULL; sqtd = sqtd->nextqtd)
	{ 
		if ((nstatus = sqtd->qtd.qtd_status) & EHCI_QTD_ACTIVE)
			break;

		status = nstatus;

		if (EHCI_QTD_GET_PID (status) != EHCI_QTD_PID_SETUP)
			actlen += sqtd->len - EHCI_QTD_GET_BYTES (status);
	}

	/* If there are left over TDs we need to update the toggle. */

	if (sqtd != NULL)
	{
#ifdef	USB_MSG
		printf ("ehci_idone: ex = %P sqtd=%P still active\n", ex, sqtd);
#endif	USB_MSG
		if ((xfer->rqflags & URQ_REQUEST) == 0)
			printf ("ehci_idone: need toggle update\n");

#if 0
		epipe->nexttoggle = EHCI_TD_GET_DT (std->td.td_token);
#endif
	}

	status &= EHCI_QTD_STATERRS;

#ifdef	USB_MSG
	printf ("ehci_idone: len=%d, actlen=%d, status=0x%x\n", xfer->length, actlen, status);
#endif	USB_MSG

	xfer->actlen = actlen;

	if (status != 0)
	{
		if (status == EHCI_QTD_HALTED)
			xfer->status = USBD_STALLED;
		else
			xfer->status = USBD_IOERROR; /* more info XXX */
	}
	else
	{
		xfer->status = USBD_NORMAL_COMPLETION;
	}

	usb_transfer_complete (xfer);

#ifdef	USB_MSG
	printf ("ehci_idone: ex=%P done\n", ex);
#endif	USB_MSG

}	/* end ehci_idone */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_poll (struct usbd_bus *bus)
{
	struct ehci_softc	*sc = (struct ehci_softc *)bus;

	if (ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS) & sc->sc_eintrs)
		ehci_intr1 (sc);

}	/* end ehci_poll */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
struct usbd_xfer *
ehci_allocx (struct usbd_bus *bus)
{
	struct ehci_softc	*sc = (struct ehci_softc *)bus;
	struct usbd_xfer	*xfer;

	/*
	 *	Verifica se h� descritor dispon�vel na lista livre
	 */
	if ((xfer = sc->sc_free_xfers_first) != NULL)
	{
		/* Retira o primeiro da lista de descritores livres */

		if ((sc->sc_free_xfers_first = sc->sc_free_xfers_first->next_xfer) == NULL)
			sc->sc_free_xfers_last = &sc->sc_free_xfers_first;
	}
	else
	{
		/* � necess�rio alocar */

		xfer = malloc_byte (sizeof (struct ehci_xfer));
	}

	if (xfer != NULL)
		memclr (xfer, sizeof (struct ehci_xfer));

	return (xfer);

}	/* end ehci_allocx */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_freex (struct usbd_bus *bus, struct usbd_xfer *xfer)
{
	struct ehci_softc	*sc = (struct ehci_softc *)bus;

	if ((xfer->next_xfer = sc->sc_free_xfers_first) == NULL)
		sc->sc_free_xfers_last = &xfer->next_xfer;

	sc->sc_free_xfers_first = xfer;

}	/* end ehci_freex */

/*
 ****************************************************************
 *	Simulate a hardware hub					*
 ****************************************************************
 */
int
ehci_root_ctrl_transfer (struct usbd_xfer *xfer)
{
	int	err;

	if (err = usb_insert_transfer (xfer))
		return (err);

	return (ehci_root_ctrl_start (xfer->pipe->first_xfer));

}	/* end ehci_root_ctrl_transfer */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_root_ctrl_start (struct usbd_xfer *xfer)
{
	struct ehci_softc		*sc = (struct ehci_softc *)xfer->pipe->device->bus;
	struct usb_device_request	*req;
	void				*buf = NULL;
	int				port, i;
	int				s, len, value, index, l, totlen = 0;
	struct usb_port_status		ps;
	struct usb_hub_descriptor	hubd;
	int				err;
	ulong				v;

	if (sc->sc_dying)
		return (USBD_IOERROR);

	req = &xfer->request;

#ifdef	USB_MSG
	printf ("ehci_root_ctrl_control type=0x%02x request=%02x\n", req->bmRequestType, req->bRequest);
#endif	USB_MSG

	len	= UGETW (req->wLength);
	value	= UGETW (req->wValue);
	index	= UGETW (req->wIndex);

	if (len != 0)
		buf = xfer->dmabuf;

#define C(x,y) ((x) | ((y) << 8))

	switch (C (req->bRequest, req->bmRequestType))
	{
	    case C(UR_CLEAR_FEATURE, UT_WRITE_DEVICE):
	    case C(UR_CLEAR_FEATURE, UT_WRITE_INTERFACE):
	    case C(UR_CLEAR_FEATURE, UT_WRITE_ENDPOINT):
		/*
		 * DEVICE_REMOTE_WAKEUP and ENDPOINT_HALT are no-ops
		 * for the integrated root hub.
		 */
		break;

	    case C(UR_GET_CONFIG, UT_READ_DEVICE):
		if (len > 0)
			{ *(uchar *)buf = sc->sc_conf; totlen = 1; }
		break;

	    case C(UR_GET_DESCRIPTOR, UT_READ_DEVICE):
#ifdef	USB_MSG
		printf ("ehci_root_ctrl_control wValue=0x%04x\n", value);
#endif	USB_MSG

		switch (value >> 8)
		{
		    case UDESC_DEVICE:
			if ((value & 0xff) != 0)
				{ err = USBD_IOERROR; goto ret; }

			totlen = l = MIN (len, USB_DEVICE_DESCRIPTOR_SIZE);

			USETW (ehci_devd.idVendor, sc->sc_id_vendor);

			memmove (buf, &ehci_devd, l);

			break;
		/*
		 * We can't really operate at another speed, but the spec says
		 * we need this descriptor.
		 */
		    case UDESC_DEVICE_QUALIFIER:
			if ((value & 0xff) != 0)
				{ err = USBD_IOERROR; goto ret;	}

			totlen = l = MIN (len, USB_DEVICE_DESCRIPTOR_SIZE);

			memmove (buf, &ehci_odevd, l);

			break;
		/*
		 * We can't really operate at another speed, but the spec says
		 * we need this descriptor.
		 */
		    case UDESC_OTHER_SPEED_CONFIGURATION:
		    case UDESC_CONFIG:
			if ((value & 0xff) != 0)
				{ err = USBD_IOERROR; goto ret;	}

			totlen = l = MIN (len, USB_CONFIG_DESCRIPTOR_SIZE);

			memmove (buf, &ehci_confd, l);

			((struct usb_config_descriptor *)buf)->bDescriptorType = value >> 8;

			buf = (char *)buf + l;
			len -= l;
			l = MIN (len, USB_INTERFACE_DESCRIPTOR_SIZE);
			totlen += l;
			memmove (buf, &ehci_ifcd, l);

			buf = (char *)buf + l;
			len -= l;
			l = MIN (len, USB_ENDPOINT_DESCRIPTOR_SIZE);
			totlen += l;
			memmove (buf, &ehci_endpd, l);

			break;

		    case UDESC_STRING:
			if (len == 0)
				break;

			*(uchar *)buf = 0; totlen = 1;

			switch (value & 0xff)
			{
			    case 1: /* Vendor */
				totlen = usb_str (buf, len, sc->sc_vendor);
				break;

			    case 2: /* Product */
				totlen = usb_str (buf, len, "EHCI root hub");
				break;
			}

			break;

		    default:
			err = USBD_IOERROR; goto ret;
		}
		break;

	    case C(UR_GET_INTERFACE, UT_READ_INTERFACE):
		if (len > 0)
			{ *(uchar *)buf = 0; totlen = 1; }
		break;

	    case C(UR_GET_STATUS, UT_READ_DEVICE):
		if (len > 1)
		{
			USETW (((struct usb_status *)buf)->wStatus, UDS_SELF_POWERED);
			totlen = 2;
		}
		break;

	    case C(UR_GET_STATUS, UT_READ_INTERFACE):
	    case C(UR_GET_STATUS, UT_READ_ENDPOINT):
		if (len > 1)
		{
			USETW (((struct usb_status *)buf)->wStatus, 0);
			totlen = 2;
		}
		break;

	    case C(UR_SET_ADDRESS, UT_WRITE_DEVICE):
		if (value >= USB_MAX_DEVICES)
			{ err = USBD_IOERROR; goto ret; }
		sc->sc_addr = value;
		break;

	    case C(UR_SET_CONFIG, UT_WRITE_DEVICE):
		if (value != 0 && value != 1)
			{ err = USBD_IOERROR; goto ret; }
		sc->sc_conf = value;
		break;

	    case C(UR_SET_DESCRIPTOR, UT_WRITE_DEVICE):
		break;

	    case C(UR_SET_FEATURE, UT_WRITE_DEVICE):
	    case C(UR_SET_FEATURE, UT_WRITE_INTERFACE):
	    case C(UR_SET_FEATURE, UT_WRITE_ENDPOINT):
		err = USBD_IOERROR;
		goto ret;

	    case C(UR_SET_INTERFACE, UT_WRITE_INTERFACE):
		break;

	    case C(UR_SYNCH_FRAME, UT_WRITE_ENDPOINT):
		break;

	/* Hub requests */
	    case C(UR_CLEAR_FEATURE, UT_WRITE_CLASS_DEVICE):
		break;

	    case C(UR_CLEAR_FEATURE, UT_WRITE_CLASS_OTHER):
#ifdef	USB_MSG
		printf
		(	"ehci_root_ctrl_control: UR_CLEAR_PORT_FEATURE "
			"port=%d feature=%d\n",
			index, value
		);
#endif	USB_MSG

		if (index < 1 || index > sc->sc_noport)
			{ err = USBD_IOERROR; goto ret;	}

		port = EHCI_PORTSC (index);

		v = ehci_read_port_or_mem_long (sc, sc->sc_offs + port) & ~EHCI_PS_CLEAR;

		switch(value)
		{
		    case UHF_PORT_ENABLE:
			ehci_write_port_or_mem_long (sc, v & ~EHCI_PS_PE, sc->sc_offs + port);
			break;

		    case UHF_PORT_SUSPEND:
			ehci_write_port_or_mem_long (sc, v & ~EHCI_PS_SUSP, sc->sc_offs + port);
			break;

		    case UHF_PORT_POWER:
			ehci_write_port_or_mem_long (sc, v & ~EHCI_PS_PP, sc->sc_offs + port);
			break;

		    case UHF_PORT_TEST:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: clear port test %d\n", index);
#endif	USB_MSG
			break;

		    case UHF_PORT_INDICATOR:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: clear port ind %d\n", index);
#endif	USB_MSG
			ehci_write_port_or_mem_long (sc, v & ~EHCI_PS_PIC, sc->sc_offs + port);
			break;

		    case UHF_C_PORT_CONNECTION:
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_CSC, sc->sc_offs + port);
			break;

		    case UHF_C_PORT_ENABLE:
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_PEC, sc->sc_offs + port);
			break;

		    case UHF_C_PORT_SUSPEND:
			/* how? */
			break;

		    case UHF_C_PORT_OVER_CURRENT:
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_OCC, sc->sc_offs + port);
			break;

		    case UHF_C_PORT_RESET:
			sc->sc_isreset = 0;
			break;

		    default:
			err = USBD_IOERROR;
			goto ret;
		}
#if (0)	/*******************************************************/
#if 0
		switch (value)
		{
		    case UHF_C_PORT_CONNECTION:
		    case UHF_C_PORT_ENABLE:
		    case UHF_C_PORT_SUSPEND:
		    case UHF_C_PORT_OVER_CURRENT:
		    case UHF_C_PORT_RESET:
			/* Enable RHSC interrupt if condition is cleared. */
			if ((OREAD4(sc, port) >> 16) == 0)
				ehci_pcd_able(sc, 1);
			break;
		default:
			break;
		}
#endif
#endif	/*******************************************************/
		break;

	    case C(UR_GET_DESCRIPTOR, UT_READ_CLASS_DEVICE):
		if (value != 0)
			{ err = USBD_IOERROR; goto ret; }

		hubd = ehci_hubd;
		hubd.bNbrPorts = sc->sc_noport;

		v = ehci_read_port_or_mem_long (sc, EHCI_HCSPARAMS);
#if (0)	/*******************************************************/
		v = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_HCSPARAMS);
#endif	/*******************************************************/

		USETW
		(	hubd.wHubCharacteristics,
			EHCI_HCS_PPC (v) ? UHD_PWR_INDIVIDUAL : UHD_PWR_NO_SWITCH |
			EHCI_HCS_P_INCICATOR (ehci_read_port_or_mem_long (sc, EHCI_HCSPARAMS))
			? UHD_PORT_IND : 0
		);

		hubd.bPwrOn2PwrGood = 200; /* XXX can't find out? */

		for (i = 0, l = sc->sc_noport; l > 0; i++, l -= 8, v >>= 8)
			hubd.DeviceRemovable[i++] = 0; /* XXX can't find out? */

		hubd.bDescLength = USB_HUB_DESCRIPTOR_SIZE + i;

		l = MIN (len, hubd.bDescLength);
		totlen = l;
		memmove (buf, &hubd, l);
		break;

	    case C(UR_GET_STATUS, UT_READ_CLASS_DEVICE):
		if (len != 4)
			{ err = USBD_IOERROR; goto ret;	}

		memclr (buf, len); /* ? XXX */

		totlen = len;
		break;

	    case C(UR_GET_STATUS, UT_READ_CLASS_OTHER):
#ifdef	USB_MSG
		printf ("ehci_root_ctrl_transfer: get port status i=%d\n", index);
#endif	USB_MSG

		if (index < 1 || index > sc->sc_noport || len != 4)
			{ err = USBD_IOERROR; goto ret;	}

		v = ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_PORTSC (index));

#ifdef	USB_MSG
		printf ("ehci_root_ctrl_transfer: port status=0x%04x\n", v);
#endif	USB_MSG

		i = UPS_HIGH_SPEED;
		if (v & EHCI_PS_CS)	i |= UPS_CURRENT_CONNECT_STATUS;
		if (v & EHCI_PS_PE)	i |= UPS_PORT_ENABLED;
		if (v & EHCI_PS_SUSP)	i |= UPS_SUSPEND;
		if (v & EHCI_PS_OCA)	i |= UPS_OVERCURRENT_INDICATOR;
		if (v & EHCI_PS_PR)	i |= UPS_RESET;
		if (v & EHCI_PS_PP)	i |= UPS_PORT_POWER;

		USETW (ps.wPortStatus, i);

		i = 0;
		if (v & EHCI_PS_CSC)	i |= UPS_C_CONNECT_STATUS;
		if (v & EHCI_PS_PEC)	i |= UPS_C_PORT_ENABLED;
		if (v & EHCI_PS_OCC)	i |= UPS_C_OVERCURRENT_INDICATOR;
		if (sc->sc_isreset)	i |= UPS_C_PORT_RESET;

		USETW (ps.wPortChange, i);

		l = MIN (len, sizeof (ps));
		memmove (buf, &ps, l);
		totlen = l;
		break;

	    case C(UR_SET_DESCRIPTOR, UT_WRITE_CLASS_DEVICE):
		err = USBD_IOERROR;
		goto ret;

	    case C(UR_SET_FEATURE, UT_WRITE_CLASS_DEVICE):
		break;

	    case C(UR_SET_FEATURE, UT_WRITE_CLASS_OTHER):
		if (index < 1 || index > sc->sc_noport)
			{ err = USBD_IOERROR; goto ret; }

		port = EHCI_PORTSC (index);

		v = ehci_read_port_or_mem_long (sc, sc->sc_offs + port) & ~EHCI_PS_CLEAR;

		switch (value)
		{
		    case UHF_PORT_ENABLE:
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_PE, sc->sc_offs + port);
			break;

		    case UHF_PORT_SUSPEND:
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_SUSP, sc->sc_offs + port);
			break;

		    case UHF_PORT_RESET:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: reset port %d\n", index);
#endif	USB_MSG

			if (EHCI_PS_IS_LOWSPEED (v))
			{
				/* Low speed device, give up ownership */

				ehci_disown (sc, index, 1);
				break;
			}

			/* Start reset sequence */

			v &= ~ (EHCI_PS_PE | EHCI_PS_PR);

			ehci_write_port_or_mem_long (sc, v | EHCI_PS_PR, sc->sc_offs + port);

			/* Wait for reset to complete */

			usb_delay_ms (&sc->sc_bus, USB_PORT_ROOT_RESET_DELAY);

			if (sc->sc_dying)
				{ err = USBD_IOERROR; goto ret;	}

			/* Terminate reset sequence */

			ehci_write_port_or_mem_long (sc, v, sc->sc_offs + port);

			/* Wait for HC to complete reset */

			usb_delay_ms (&sc->sc_bus, EHCI_PORT_RESET_COMPLETE);

			if (sc->sc_dying)
				{ err = USBD_IOERROR; goto ret;	}

			v = ehci_read_port_or_mem_long (sc, sc->sc_offs + port);

#ifdef	USB_MSG
			printf ("ehci after reset, status=%P\n", v);
#endif	USB_MSG

			if (v & EHCI_PS_PR)
			{
#ifdef	USB_MSG
				printf ("%s: port reset timeout\n", sc->sc_bus.bdev->nameunit);
#endif	USB_MSG
				return (USBD_TIMEOUT);
			}

			if ((v & EHCI_PS_PE) == 0)
			{
				/* Not a high speed device, give up ownership.*/
				ehci_disown (sc, index, 0);
				break;
			}

			sc->sc_isreset = 1;

#ifdef	USB_MSG
			printf ("ehci port %d reset, status = %P\n", index, v);
#endif	USB_MSG
			break;

		    case UHF_PORT_POWER:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: set port power %d\n", index);
#endif	USB_MSG
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_PP, sc->sc_offs + port);
			break;

		    case UHF_PORT_TEST:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: set port test %d\n", index);
#endif	USB_MSG
			break;

		    case UHF_PORT_INDICATOR:
#ifdef	USB_MSG
			printf ("ehci_root_ctrl_transfer: set port ind %d\n", index);
#endif	USB_MSG
			ehci_write_port_or_mem_long (sc, v | EHCI_PS_PIC, sc->sc_offs + port);
			break;

		    default:
			err = USBD_IOERROR;
			goto ret;
		}
		break;

	    case C(UR_CLEAR_TT_BUFFER, UT_WRITE_CLASS_OTHER):
	    case C(UR_RESET_TT, UT_WRITE_CLASS_OTHER):
	    case C(UR_GET_TT_STATE, UT_READ_CLASS_OTHER):
	    case C(UR_STOP_TT, UT_WRITE_CLASS_OTHER):
		break;

	    default:
		err = USBD_IOERROR;
		goto ret;
	}

	xfer->actlen = totlen; 	err = USBD_NORMAL_COMPLETION;

 ret:
	xfer->status = err;

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	usb_transfer_complete (xfer);

	splx (s);

	return (USBD_IN_PROGRESS);

}	/* end ehci_root_ctrl_start */

/*
 ****************************************************************
 *	Abort a root control request				*
 ****************************************************************
 */
void
ehci_root_ctrl_abort (struct usbd_xfer *xfer)
{

}	/* end ehci_root_ctrl_abort */

/*
 ****************************************************************
 *	 Close the root pipe					*
 ****************************************************************
 */
void
ehci_root_ctrl_close (struct usbd_pipe *pipe_ptr)
{
}	/* end ehci_root_ctrl_close */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_noop (struct usbd_pipe *pipe_ptr)
{
}	/* end ehci_noop */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_root_ctrl_done (struct usbd_xfer *xfer)
{
	xfer->hcpriv = NULL;

}	/* end ehci_root_ctrl_done */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_root_intr_transfer (struct usbd_xfer *xfer)
{
	int	err;

	if (err = usb_insert_transfer (xfer))
		return (err);

	return (ehci_root_intr_start (xfer->pipe->first_xfer));

}	/* end ehci_root_intr_transfer */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_root_intr_start (struct usbd_xfer *xfer)
{
	struct usbd_pipe		*pipe_ptr = xfer->pipe;
	struct ehci_softc		*sc = (struct ehci_softc *)pipe_ptr->device->bus;

	if (sc->sc_dying)
		return (USBD_IOERROR);

	sc->sc_intrxfer = xfer;

	return (USBD_IN_PROGRESS);

}	/* end ehci_root_intr_start */

/*
 ****************************************************************
 *	Abort a root interrupt request				*
 ****************************************************************
 */
void
ehci_root_intr_abort (struct usbd_xfer *xfer)
{
	struct usbd_pipe		*pipe_ptr = xfer->pipe;
	struct ehci_softc		*sc = (struct ehci_softc *)pipe_ptr->device->bus;
	int				s;

	if (pipe_ptr->intrxfer == xfer)
	{
		printf ("ehci_root_intr_abort: remove\n");
		xfer->pipe->intrxfer = NULL;
	}

	xfer->status = USBD_CANCELLED;

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	usb_transfer_complete (xfer);

	splx (s);

}	/* end ehci_root_intr_abort */

/*
 ****************************************************************
 *	Close the root pipe					*
 ****************************************************************
 */
void
ehci_root_intr_close (struct usbd_pipe *pipe_ptr)
{
	struct ehci_softc		*sc = (struct ehci_softc *)pipe_ptr->device->bus;

	printf ("ehci_root_intr_close\n");

	sc->sc_intrxfer = NULL;

}	/* end ehci_root_intr_close */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_root_intr_done (struct usbd_xfer *xfer)
{
	xfer->hcpriv = NULL;

}	/* end ehci_root_intr_done */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_device_ctrl_transfer (struct usbd_xfer *xfer)
{
	int	err;

	if (err = usb_insert_transfer (xfer))
		return (err);

	return (ehci_device_ctrl_start (xfer->pipe->first_xfer));

}	/* end ehci_device_ctrl_transfer */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_device_ctrl_start (struct usbd_xfer *xfer)
{
	struct ehci_softc	*sc = (struct ehci_softc *)xfer->pipe->device->bus;
	int			err;

	if (sc->sc_dying)
		return (USBD_IOERROR);

	if (err = ehci_device_request (xfer))
		return (err);

#ifdef	USB_POLLING
	if (sc->sc_bus.use_polling)
		ehci_waitintr (sc, xfer);
#endif	USB_POLLING

	return (USBD_IN_PROGRESS);

}	/* end ehci_device_ctrl_start */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_device_ctrl_done (struct usbd_xfer *xfer)
{
	struct ehci_xfer		*ex = (struct ehci_xfer *)xfer;
	struct ehci_softc		*sc = (struct ehci_softc *)xfer->pipe->device->bus;
#if (0)	/*******************************************************/
	struct ehci_pipe		*epipe = (struct ehci_pipe *)xfer->pipe;
#endif	/*******************************************************/

#ifdef	USB_MSG
	printf ("ehci_ctrl_done: xfer=%P\n", xfer);
#endif	USB_MSG

	if (xfer->status != USBD_NOMEM && ehci_active_intr_list (ex))
	{
		ehci_del_intr_list(ex);				/* remove from active list */
		ehci_free_sqtd_chain (sc, ex->sqtdstart, NULL);
	}

#ifdef	USB_MSG
	printf ("ehci_ctrl_done: length=%d\n", xfer->actlen);
#endif	USB_MSG

}	/* end ehci_device_ctrl_done */

/*
 ****************************************************************
 *	Abort a device control request				*
 ****************************************************************
 */
void
ehci_device_ctrl_abort (struct usbd_xfer *xfer)
{
	ehci_abort_xfer (xfer, USBD_CANCELLED);

}	/* end ehci_device_ctrl_abort */

/*
 ****************************************************************
 *	Close a device control pipe				*
 ****************************************************************
 */
void
ehci_device_ctrl_close (struct usbd_pipe *pipe_ptr)
{
	struct ehci_softc		*sc = (struct ehci_softc *)pipe_ptr->device->bus;
#if (0)	/*******************************************************/
	struct ehci_pipe		*epipe = (struct ehci_pipe *)pipe_ptr;
#endif	/*******************************************************/

	ehci_close_pipe (pipe_ptr, sc->sc_async_head);

}	/* end ehci_device_ctrl_close */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
#define exfer ((struct ehci_xfer *)xfer)

int
ehci_device_request (struct usbd_xfer *xfer)
{
	struct ehci_pipe		*epipe	= (struct ehci_pipe *)xfer->pipe;
	struct usb_device_request	*req	= &xfer->request;
	struct usbd_device		*udev	= epipe->pipe.device;
	struct ehci_softc		*sc	= (struct ehci_softc *)udev->bus;
	int				addr	= udev->address;
	ehci_soft_qtd_t			*setup, *stat, *next;
	ehci_soft_qh_t			*sqh;
	int				isread, len, err, s;

	isread = req->bmRequestType & UT_READ;
	len    = UGETW (req->wLength);

	printf
	(	"ehci_device_request: type=0x%02x, request=0x%02x, "
		"wValue=0x%04x, wIndex=0x%04x len=%d, addr=%d, endpt=%d\n",
		req->bmRequestType, req->bRequest, UGETW(req->wValue),
		UGETW(req->wIndex), len, addr,
		epipe->pipe.endpoint->edesc->bEndpointAddress
	);
#ifdef	USB_MSG
#endif	USB_MSG

	if ((setup = ehci_alloc_sqtd (sc)) == NULL)
		{ err = USBD_NOMEM; goto bad1; }

	if ((stat = ehci_alloc_sqtd (sc)) == NULL)
		{ err = USBD_NOMEM; goto bad2; }

	sqh = epipe->sqh;
	epipe->u.ctl.length = len;

	/* XXX
	 * Since we're messing with the QH we must know the HC is in sync.
	 * This needs to go away since it slows down control transfers.
	 * Removing it entails:
	 *  - fill the QH only once with addr & wMaxPacketSize
	 *  - put the correct data toggles in the qtds and set DTC
	 */
	/* ehci_sync_hc(sc); */
	/* Update device address and length since they may have changed. */
	/* XXX This only needs to be done once, but it's too early in open. */
	/* XXXX Should not touch ED here! */

	sqh->qh.qh_endp =
	    (sqh->qh.qh_endp & (~(EHCI_QH_ADDRMASK | EHCI_QG_MPLMASK))) |
	     EHCI_QH_SET_ADDR (addr) |
	     /* EHCI_QH_DTC | */
	     EHCI_QH_SET_MPL (UGETW (epipe->pipe.endpoint->edesc->wMaxPacketSize));

	/* Clear toggle */

	sqh->qh.qh_qtd.qtd_status &= ~EHCI_QTD_TOGGLE;

	/* Set up data transaction */

	if (len != 0)
	{
		ehci_soft_qtd_t		*End;

		if (err = ehci_alloc_sqtd_chain (epipe, sc, len, isread, xfer, &next, &End))
			goto bad3;

		End->nextqtd	  = stat;
		End->qtd.qtd_next = End->qtd.qtd_altnext = stat->physaddr;

		/* Start toggle at 1 */

		/*next->qtd.td_flags |= EHCI_QTD_TOGGLE;*/
	}
	else
	{
		next = stat;
	}

	memmove (epipe->u.ctl.reqdma, req, sizeof (*req));

	setup->qtd.qtd_status =  EHCI_QTD_ACTIVE |
					EHCI_QTD_SET_PID(EHCI_QTD_PID_SETUP) |
					EHCI_QTD_SET_CERR(3) |
					EHCI_QTD_SET_BYTES (sizeof (*req));

	setup->qtd.qtd_buffer[0]	= VIRT_TO_PHYS_ADDR (epipe->u.ctl.reqdma);
	setup->nextqtd			= next;
	setup->qtd.qtd_next		= setup->qtd.qtd_altnext = next->physaddr;
	setup->xfer			= xfer;
	setup->len			= sizeof (*req);

	stat->qtd.qtd_status = EHCI_QTD_ACTIVE |
				EHCI_QTD_SET_PID(isread ? EHCI_QTD_PID_OUT : EHCI_QTD_PID_IN) |
				EHCI_QTD_SET_CERR(3) |
				EHCI_QTD_IOC;

	stat->qtd.qtd_buffer[0]	= 0; /* XXX not needed? */
	stat->nextqtd		= NULL;
	stat->qtd.qtd_next	= stat->qtd.qtd_altnext = EHCI_NULL;
	stat->xfer		= xfer;
	stat->len		= 0;

#ifdef USB_DEBUG
	if (ehcidebug > 5) {
		DPRINTF(("ehci_device_request:\n"));
		ehci_dump_sqh(sqh);
		ehci_dump_sqtds(setup);
	}
#endif

	exfer->sqtdstart = setup;
	exfer->sqtdend   = stat;

	/* Insert qTD in QH list. */

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	ehci_set_qh_qtd (sqh, setup);

#if (0)	/*******************************************************/
	if (xfer->timeout && !sc->sc_bus.use_polling)
	{
                usb_callout(xfer->timeout_handle, MS_TO_TICKS(xfer->timeout),
			    ehci_timeout, xfer);
	}
#endif	/*******************************************************/

	ehci_add_intr_list (sc, exfer);

	xfer->status = USBD_IN_PROGRESS;

	splx (s);

#ifdef USB_DEBUG
	if (ehcidebug > 10) {
		DPRINTF(("ehci_device_request: status=%x\n",
			 EOREAD4(sc, EHCI_USBSTS)));
		DELAY (10000);
		ehci_dump_regs(sc);
		ehci_dump_sqh(sc->sc_async_head);
		ehci_dump_sqh(sqh);
		ehci_dump_sqtds(setup);
	}
#endif

	return (USBD_NORMAL_COMPLETION);

 bad3:
	ehci_free_sqtd (sc, stat);
 bad2:
	ehci_free_sqtd (sc, setup);
 bad1:
	printf ("ehci_device_request: no memory\n");
	xfer->status = err;

	usb_transfer_complete (xfer);

	return (err);

}	/* end ehci_device_request */
#undef exfer

#ifdef	USB_POLLING
/*
 ****************************************************************
 *	Aguarda uma interrup��o					*
 ****************************************************************
 */
void
ehci_waitintr (struct ehci_softc *sc, struct usbd_xfer *xfer)
{
	int		timo = xfer->timeout, usecs;
	ulong		intrs;

	xfer->status = USBD_IN_PROGRESS;

	for (usecs = timo * 1000000 / hz; usecs > 0; usecs -= 1000)
	{
		usb_delay_ms (&sc->sc_bus, 1);

		if (sc->sc_dying)
			break;

		intrs  = EHCI_STS_INTRS (ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS));
		intrs &= sc->sc_eintrs;

		printf ("ehci_waitintr: 0x%04x\n", intrs);

#ifdef USB_DEBUG
		if (ehcidebug > 15)
			ehci_dump_regs(sc);
#endif

		if (intrs)
		{
			ehci_intr1 (sc);

			if (xfer->status != USBD_IN_PROGRESS)
				return;
		}
	}

	/* Timeout */

	printf ("ehci_waitintr: timeout\n");

	xfer->status = USBD_TIMEOUT;

	usb_transfer_complete (xfer);

	/* XXX should free TD */

}	/* end ehci_waitintr */
#endif	USB_POLLING

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_disown (struct ehci_softc *sc, int index, int lowspeed)
{
	int		port;
	ulong		v;

	printf ("ehci_disown: index=%d lowspeed=%d\n", index, lowspeed);

	port = EHCI_PORTSC (index);

	v = ehci_read_port_or_mem_long (sc, sc->sc_offs + port) & ~EHCI_PS_CLEAR;

	ehci_write_port_or_mem_long (sc, v | EHCI_PS_PO, sc->sc_offs + port);

}	/* end ehci_disown */

/*
 ****************************************************************
 *	Add an ED to the schedule				*
 ****************************************************************
 */
void
ehci_add_qh (ehci_soft_qh_t *sqh, ehci_soft_qh_t *head)
{
	sqh->next	 = head->next;
	sqh->qh.qh_link  = head->qh.qh_link;
	head->next	 = sqh;
	head->qh.qh_link = sqh->physaddr | EHCI_LINK_QH;

#ifdef USB_DEBUG
	if (ehcidebug > 5) {
		printf("ehci_add_qh:\n");
		ehci_dump_sqh(sqh);
	}
#endif

}	/* end ehci_add_qh */

/*
 ****************************************************************
 *	Remove an ED from the schedule				*
 ****************************************************************
 */
void
ehci_rem_qh (struct ehci_softc *sc, ehci_soft_qh_t *sqh, ehci_soft_qh_t *head)
{
	ehci_soft_qh_t		*p;

	for (p = head; p != NULL && p->next != sqh; p = p->next)
		/* vazio */;

	if (p == NULL)
		panic ("ehci_rem_qh: ED not found");

	p->next = sqh->next;	p->qh.qh_link = sqh->qh.qh_link;

	ehci_sync_hc (sc);

}	/* end ehci_rem_qh */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_free_sqtd_chain (struct ehci_softc *sc, ehci_soft_qtd_t *sqtd, ehci_soft_qtd_t *sqtdend)
{
	ehci_soft_qtd_t		*p;
	int			i;

#ifdef	USB_MSG
	printf ("ehci_free_sqtd_chain: sqtd=%P sqtdend=%P\n", sqtd, sqtdend);
#endif	USB_MSG

	for (i = 0; sqtd != sqtdend; sqtd = p, i++)
	{
		p = sqtd->nextqtd;
		ehci_free_sqtd (sc, sqtd);
	}

}	/* end ehci_free_sqtd_chain */

/*
 ****************************************************************
 *	Abort a device request					*
 ****************************************************************
 *
 * If this routine is called at splusb() it guarantees that the request
 * will be removed from the hardware scheduling and that the callback
 * for it will be called with USBD_CANCELLED status.
 * It's impossible to guarantee that the requested transfer will not
 * have happened since the hardware runs concurrently.
 * If the transaction has already happened we rely on the ordinary
 * interrupt processing to process it.
 * XXX This is most probably wrong.
 */
#define exfer ((struct ehci_xfer *)xfer)

void
ehci_abort_xfer (struct usbd_xfer *xfer, int status)
{
	struct ehci_pipe		*epipe = (struct ehci_pipe *)xfer->pipe;
	struct ehci_softc		*sc = (struct ehci_softc *)epipe->pipe.device->bus;
	ehci_soft_qh_t			*sqh = epipe->sqh;
	ehci_soft_qtd_t			*sqtd;
	ulong				cur, qhstatus;
	int				s, hit;

	printf ("ehci_abort_xfer: xfer=%P pipe=%P\n", xfer, epipe);

	if (sc->sc_dying)
	{
		/* If we're dying, just do the software part. */
		s = splx (irq_pl_vec[sc->sc_bus.irq]);

		xfer->status = status;	/* make software ignore it */

#if (0)	/*******************************************************/
		usb_uncallout(xfer->timeout_handle, ehci_timeout, xfer);
#endif	/*******************************************************/

		usb_transfer_complete (xfer);

		splx(s);
		return;
	}

#if (0)	/*******************************************************/
	if (xfer->device->bus->intr_context || !curproc)
		panic("ehci_abort_xfer: not in process context");
#endif	/*******************************************************/

	/*
	 * Step 1: Make interrupt routine and hardware ignore xfer.
	 */
	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	xfer->status = status;	/* make software ignore it */

#if (0)	/*******************************************************/
	usb_uncallout(xfer->timeout_handle, ehci_timeout, xfer);
#endif	/*******************************************************/

	qhstatus = sqh->qh.qh_qtd.qtd_status;
	sqh->qh.qh_qtd.qtd_status = qhstatus | EHCI_QTD_HALTED;

	for (sqtd = exfer->sqtdstart; ; sqtd = sqtd->nextqtd)
	{
		sqtd->qtd.qtd_status |= EHCI_QTD_HALTED;

		if (sqtd == exfer->sqtdend)
			break;
	}

	splx (s);

	/*
	 * Step 2: Wait until we know hardware has finished any possible
	 * use of the xfer.  Also make sure the soft interrupt routine
	 * has run.
	 */
	ehci_sync_hc (sc);

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	usb_schedsoftintr (&sc->sc_bus);

	splx (s);

	/*
	 * Step 3: Remove any vestiges of the xfer from the hardware.
	 * The complication here is that the hardware may have executed
	 * beyond the xfer we're trying to abort.  So as we're scanning
	 * the TDs of this xfer we check if the hardware points to
	 * any of them.
	 */
	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	cur = EHCI_LINK_ADDR (sqh->qh.qh_curqtd);

	hit = 0;

	for (sqtd = exfer->sqtdstart; ; sqtd = sqtd->nextqtd)
	{
		hit |= cur == sqtd->physaddr;

		if (sqtd == exfer->sqtdend)
			break;
	}

	sqtd = sqtd->nextqtd;

	/* Zap curqtd register if hardware pointed inside the xfer. */

	if (hit && sqtd != NULL)
	{
		printf ("ehci_abort_xfer: cur=%P\n", sqtd->physaddr);

		sqh->qh.qh_curqtd = sqtd->physaddr;	/* unlink qTDs */
		sqh->qh.qh_qtd.qtd_status = qhstatus;
	}
	else
	{
		printf ("ehci_abort_xfer: no hit\n");
	}

	/*
	 *	Step 4: Execute callback.
	 */
	usb_transfer_complete (xfer);

	splx (s);
#undef exfer
}	/* end ehci_abort_xfer */

/*
 ****************************************************************
 *	Close a reqular pipe					*
 ****************************************************************
 *
 * Assumes that there are no pending transactions.
 */
void
ehci_close_pipe (struct usbd_pipe *pipe_ptr, ehci_soft_qh_t *head)
{
	struct ehci_pipe	*epipe = (struct ehci_pipe *)pipe_ptr;
	struct ehci_softc	*sc = (struct ehci_softc *)pipe_ptr->device->bus;
	ehci_soft_qh_t		*sqh = epipe->sqh;
	int			s;

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	ehci_rem_qh (sc, sqh, head);

	splx (s);

	ehci_free_sqh (sc, epipe->sqh);

}	/* end ehci_close_pipe */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_set_qh_qtd (ehci_soft_qh_t *sqh, ehci_soft_qtd_t *sqtd)
{
	/* Halt while we are messing. */

	sqh->qh.qh_qtd.qtd_status |= EHCI_QTD_HALTED;
	sqh->qh.qh_curqtd = 0;
	sqh->qh.qh_qtd.qtd_next = sqtd->physaddr;
	sqh->sqtd = sqtd;

	/* Keep toggle, clear the rest, including length. */

	sqh->qh.qh_qtd.qtd_status &= EHCI_QTD_TOGGLE;

}	/* end ehci_set_qh_qtd */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 *
 * Ensure that the HC has released all references to the QH.  We do this
 * by asking for a Async Advance Doorbell interrupt and then we wait for
 * the interrupt.
 * To make this easier we first obtain exclusive use of the doorbell.
 */
void
ehci_sync_hc (struct ehci_softc *sc)
{
	int		s;

	if (sc->sc_dying)
		{ printf ("ehci_sync_hc: dying\n"); return; }

#ifdef	USB_MSG
	printf ("ehci_sync_hc: enter\n");
#endif	USB_MSG

	/* get doorbell */

#if (0)	/*******************************************************/
	lockmgr (&sc->sc_doorbell_lock, LK_EXCLUSIVE, NULL, NULL);
#endif	/*******************************************************/

	SLEEPLOCK (&sc->sc_doorbell_lock, PBLKIO);

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	/* ask for doorbell */

	ehci_write_port_or_mem_long
	(	sc,
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD) | EHCI_CMD_IAAD,
		sc->sc_offs + EHCI_USBCMD
	);

	printf
	(	"ehci_sync_hc: cmd=%P sts=%P\n",
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS)
	);

#if (0)	/*******************************************************/
	error = tsleep (&sc->sc_async_head, PZERO, "ehcidi", hz); /* bell wait */
#endif	/*******************************************************/

	splx (s);

	SLEEPFREE (&sc->sc_doorbell_lock);


printf ("ehci_sync_hc: esperando evento sc->sc_async_head_done\n");
	EVENTWAIT (&sc->sc_async_head_done, PBLKIO);
printf ("ehci_sync_hc: ocorreu o evento sc->sc_async_head_done\n");

	printf
	(	"ehci_sync_hc: cmd=%P sts=%P\n",
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS)
	);


#if (0)	/*******************************************************/
	/* release doorbell */
	lockmgr (&sc->sc_doorbell_lock, LK_RELEASE, NULL, NULL);
#endif	/*******************************************************/

}	/* end ehci_sync_hc */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_device_clear_toggle (struct usbd_pipe *pipe_ptr)
{
	struct ehci_pipe	*epipe = (struct ehci_pipe *)pipe_ptr;

	epipe->sqh->qh.qh_qtd.qtd_status &= ~EHCI_QTD_TOGGLE;

}	/* end ehci_device_clear_toggle */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
ehci_soft_qh_t *
ehci_alloc_sqh (struct ehci_softc *sc)
{
	ehci_soft_qh_t		*sqh;
	int			i, offs, byte_size;
	void			*virt_addr;

	if (sc->sc_freeqhs == NULL)
	{
#ifdef	USB_MSG
		printf ("ehci_alloc_sqh: allocating chunk\n");
#endif	USB_MSG

		byte_size = EHCI_SQH_SIZE * EHCI_SQH_CHUNK;

		if ((virt_addr = (void *)PGTOBY (malloce (M_CORE, BYUPPG (byte_size)))) == NULL)
			return (NULL);

		for (i = 0; i < EHCI_SQH_CHUNK; i++)
		{
			offs		= i * EHCI_SQH_SIZE;
			sqh		= (ehci_soft_qh_t *)(virt_addr + offs);
			sqh->physaddr	= VIRT_TO_PHYS_ADDR (sqh);
			sqh->next	= sc->sc_freeqhs;
			sc->sc_freeqhs	= sqh;
		}
	}

	sqh		= sc->sc_freeqhs;
	sc->sc_freeqhs	= sqh->next;

	memclr (&sqh->qh, sizeof (ehci_qh_t));

	sqh->next	= NULL;

	return (sqh);

}	/* end ehci_alloc_sqh */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_free_sqh (struct ehci_softc *sc, ehci_soft_qh_t *sqh)
{
	sqh->next = sc->sc_freeqhs; sc->sc_freeqhs = sqh;

}	/* end ehci_free_sqh */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
ehci_soft_qtd_t *
ehci_alloc_sqtd (struct ehci_softc *sc)
{
	ehci_soft_qtd_t		*sqtd;
	int			i, offs, byte_size;
	void			*virt_addr;
	int			s;

	if (sc->sc_freeqtds == NULL)
	{
		byte_size = EHCI_SQTD_SIZE * EHCI_SQTD_CHUNK;

		if ((virt_addr = (void *)PGTOBY (malloce (M_CORE, BYUPPG (byte_size)))) == NULL)
			return (NULL);

		s = splx (irq_pl_vec[sc->sc_bus.irq]);

		for (i = 0; i < EHCI_SQTD_CHUNK; i++)
		{
			offs		= i * EHCI_SQTD_SIZE;
			sqtd		= (ehci_soft_qtd_t *)(virt_addr + offs);
			sqtd->physaddr	= VIRT_TO_PHYS_ADDR (sqtd);
			sqtd->nextqtd	= sc->sc_freeqtds;
			sc->sc_freeqtds = sqtd;
		}

		splx (s);
	}

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	sqtd		= sc->sc_freeqtds;
	sc->sc_freeqtds = sqtd->nextqtd;

	memclr (&sqtd->qtd, sizeof (ehci_qtd_t));

	sqtd->nextqtd	= NULL;
	sqtd->xfer	= NULL;

	splx (s);

	return (sqtd);

}	/* end ehci_alloc_sqtd */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_free_sqtd (struct ehci_softc *sc, ehci_soft_qtd_t *sqtd)
{
	int		s;

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	sqtd->nextqtd = sc->sc_freeqtds; sc->sc_freeqtds = sqtd;

	splx (s);

}	/* end ehci_free_sqtd */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_alloc_sqtd_chain (struct ehci_pipe *epipe, struct ehci_softc *sc,
		     int alen, int rd, struct usbd_xfer *xfer,
		     ehci_soft_qtd_t **sp, ehci_soft_qtd_t **ep)
{
	ehci_soft_qtd_t		*next, *cur;
	ulong			dataphys, dataphyspage, dataphyslastpage, nextphys;
	ulong			qtdstatus;
	int			len, curlen, offset, i;

#ifdef	USB_MSG
	printf ("ehci_alloc_sqtd_chain: start len=%d\n", alen);
#endif	USB_MSG

	offset = 0; 	len = alen;

	dataphys = VIRT_TO_PHYS_ADDR (xfer->dmabuf);

	dataphyslastpage = EHCI_PAGE (dataphys + len - 1);

	qtdstatus = EHCI_QTD_ACTIVE |
		    EHCI_QTD_SET_PID (rd ? EHCI_QTD_PID_IN : EHCI_QTD_PID_OUT) |
		    EHCI_QTD_SET_CERR (3);

	if ((*sp = cur = ehci_alloc_sqtd (sc)) == NULL)
		goto nomem;

	for (EVER)
	{
		dataphyspage = EHCI_PAGE (dataphys);

		/* The EHCI hardware can handle at most 5 pages. */
		/* XXX This is pretty broken: Because we do not allocate
		 * a contiguous buffer (contiguous in physical pages) we
		 * can only transfer one page in one go.
		 * So check whether the start and end of the buffer are on
		 * the same page.
		 */
		if (dataphyspage == dataphyslastpage)
		{
			curlen = len;
		}
		else
		{
			/* See comment above (XXX) */
			curlen = EHCI_PAGE_SIZE - EHCI_PAGE_MASK (dataphys);

			/* XXX true for EHCI? */
			/* the length must be a multiple of the max size */

			curlen -= curlen % UGETW (epipe->pipe.endpoint->edesc->wMaxPacketSize);

#ifdef	USB_MSG
			printf ("ehci_alloc_sqtd_chain: multiple QTDs, curlen=%d\n", curlen);
#endif	USB_MSG
		}

#ifdef	USB_MSG
		printf
		(	"ehci_alloc_sqtd_chain: dataphys=%P dataphyslastpage=%P len=%d curlen=%d\n",
			dataphys, dataphyslastpage, len, curlen
		);
#endif	USB_MSG

		len -= curlen;

		if (len != 0)
		{
			next = ehci_alloc_sqtd (sc);

			if (next == NULL)
				goto nomem;

			nextphys = next->physaddr;
		}
		else
		{
			next	 = NULL;
			nextphys = EHCI_NULL;
		}

		for (i = 0; i * EHCI_PAGE_SIZE < curlen; i++)
		{
			ulong	 a = dataphys + i * EHCI_PAGE_SIZE;

			if (i != 0)			/* use offset only in first buffer */
				a = EHCI_PAGE (a);

			cur->qtd.qtd_buffer[i] = a;

			if (i >= EHCI_QTD_NBUFFERS)
			{
				printf ("ehci_alloc_sqtd_chain: i=%d\n", i);
				goto nomem;
			}
		}

		cur->nextqtd		= next;
		cur->qtd.qtd_next	= cur->qtd.qtd_altnext = nextphys;
		cur->qtd.qtd_status	= qtdstatus | EHCI_QTD_SET_BYTES (curlen);
		cur->xfer		= xfer;
		cur->len		= curlen;

#ifdef	USB_MSG
		printf ("ehci_alloc_sqtd_chain: cbp=%P end=%P\n", dataphys, dataphys + curlen);
#endif	USB_MSG

		if (len == 0)
			break;

#ifdef	USB_MSG
		printf ("ehci_alloc_sqtd_chain: extend chain\n");
#endif	USB_MSG

		offset  += curlen;
		dataphys = VIRT_TO_PHYS_ADDR (xfer->dmabuf + offset);
		cur      = next;
	}

	cur->qtd.qtd_status |= EHCI_QTD_IOC;
	*ep = cur;

#ifdef	USB_MSG
	printf ("ehci_alloc_sqtd_chain: return sqtd=%P sqtdend=%P\n", *sp, *ep);
#endif	USB_MSG

	return (USBD_NORMAL_COMPLETION);

	/*
	 *	Em caso de erro, ...
	 */
 nomem:
	/* XXX free chain */
	printf ("ehci_alloc_sqtd_chain: no memory\n");

	return (USBD_NOMEM);

}	/* end ehci_alloc_sqtd_chain */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
ehci_device_bulk_transfer (struct usbd_xfer *xfer)
{
	int	err;

	if (err = usb_insert_transfer (xfer))
		return (err);

	return (ehci_device_bulk_start (xfer->pipe->first_xfer));

}	/* end ehci_device_bulk_transfer */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
#define exfer ((struct ehci_xfer *)xfer)

int
ehci_device_bulk_start (struct usbd_xfer *xfer)
{
	struct ehci_pipe		*epipe = (struct ehci_pipe *)xfer->pipe;
	struct usbd_device		*udev = epipe->pipe.device;
	struct ehci_softc		*sc = (struct ehci_softc *)udev->bus;
	ehci_soft_qtd_t			*data, *dataend;
	ehci_soft_qh_t			*sqh;
	int				err, len, isread, endpt, s;

#ifdef	USB_MSG
	printf ("ehci_device_bulk_start: xfer=%P len=%d flags=%d\n", xfer, xfer->length, xfer->flags);
#endif	USB_MSG

	if (sc->sc_dying)
		return (USBD_IOERROR);

	len	= xfer->length;
	endpt	= epipe->pipe.endpoint->edesc->bEndpointAddress;
	isread	= UE_GET_DIR(endpt) == UE_DIR_IN;
	sqh	= epipe->sqh;

	epipe->u.bulk.length = len;

	if (err = ehci_alloc_sqtd_chain (epipe, sc, len, isread, xfer, &data, &dataend))
	{
		printf ("ehci_device_bulk_transfer: no memory\n");
		xfer->status = err;
		usb_transfer_complete(xfer);
		return (err);
	}

#ifdef USB_DEBUG
	if (ehcidebug > 5) {
		DPRINTF(("ehci_device_bulk_transfer: data(1)\n"));
		ehci_dump_sqh(sqh);
		ehci_dump_sqtds(data);
	}
#endif

	/* Set up interrupt info. */

	exfer->sqtdstart = data;
	exfer->sqtdend = dataend;

	s = splx (irq_pl_vec[sc->sc_bus.irq]);

	ehci_set_qh_qtd (sqh, data);

#if (0)	/*******************************************************/
	if (xfer->timeout && !sc->sc_bus.use_polling)
		usb_callout(xfer->timeout_handle, MS_TO_TICKS(xfer->timeout), ehci_timeout, xfer);
#endif	/*******************************************************/

	ehci_add_intr_list (sc, exfer);

	xfer->status = USBD_IN_PROGRESS;

	splx (s);

#ifdef USB_DEBUG
	if (ehcidebug > 10) {
		DPRINTF(("ehci_device_bulk_transfer: data(2)\n"));
		DELAY (10000);
		DPRINTF(("ehci_device_bulk_transfer: data(3)\n"));
		ehci_dump_regs(sc);
#if 0
		printf("async_head:\n");
		ehci_dump_sqh(sc->sc_async_head);
#endif
		printf("sqh:\n");
		ehci_dump_sqh(sqh);
		ehci_dump_sqtds(data);
	}
#endif

#ifdef	USB_POLLING
	if (sc->sc_bus.use_polling)
		ehci_waitintr(sc, xfer);
#endif	USB_POLLING

	return (USBD_IN_PROGRESS);

}	/* end ehci_device_bulk_start */
#undef exfer

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_device_bulk_abort (struct usbd_xfer *xfer)
{
	ehci_abort_xfer (xfer, USBD_CANCELLED);

}	/* end ehci_device_bulk_abort */

/*
 ****************************************************************
 *	Close a device bulk pipe				*
 ****************************************************************
 */
void
ehci_device_bulk_close (struct usbd_pipe *pipe_ptr)
{
	struct ehci_softc		*sc = (struct ehci_softc *)pipe_ptr->device->bus;

	ehci_close_pipe (pipe_ptr, sc->sc_async_head);

}	/* end ehci_device_bulk_close */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
ehci_device_bulk_done (struct usbd_xfer *xfer)
{
	struct ehci_xfer		*ex = (struct ehci_xfer *)xfer;
	struct ehci_softc		*sc = (struct ehci_softc *)xfer->pipe->device->bus;
#if (0)	/*******************************************************/
	struct ehci_pipe		*epipe = (struct ehci_pipe *)xfer->pipe;
#endif	/*******************************************************/

#ifdef	USB_MSG
	printf ("ehci_bulk_done: xfer=%P, actlen=%d\n", xfer, xfer->actlen);
#endif	USB_MSG

	if (xfer->status != USBD_NOMEM && ehci_active_intr_list (ex))
	{
		ehci_del_intr_list(ex);	/* remove from active list */
		ehci_free_sqtd_chain (sc, ex->sqtdstart, NULL);
	}

#ifdef	USB_MSG
	printf ("ehci_bulk_done: length=%d\n", xfer->actlen);
#endif	USB_MSG

}	/* end ehci_device_bulk_done */

/*
 ****************************************************************
 *	A serem feitas ou nunca existir�o ????????		*
 ****************************************************************
 */
int
ehci_device_intr_transfer (struct usbd_xfer *xfer)
{
	return (USBD_IOERROR);
}

int
ehci_device_intr_start (struct usbd_xfer *xfer)
{
	return (USBD_IOERROR);
}

void
ehci_device_intr_abort (struct usbd_xfer *xfer)
{
}

void
ehci_device_intr_close (struct usbd_pipe *pipe_ptr)
{
}

void
ehci_device_intr_done (struct usbd_xfer *xfer)
{
}

int
ehci_device_isoc_transfer (struct usbd_xfer *xfer)
{
	return (USBD_IOERROR);
}

int
ehci_device_isoc_start (struct usbd_xfer *xfer)
{
	return (USBD_IOERROR);
}

void
ehci_device_isoc_abort (struct usbd_xfer *xfer)
{
}

void
ehci_device_isoc_close (struct usbd_pipe *pipe_ptr)
{
}

void
ehci_device_isoc_done (struct usbd_xfer *xfer)
{
}

/*
 ****************************************************************
 *	Le 1 byte de um registro (da porta ou mem�ria)		*
 ****************************************************************
 */
int
ehci_read_port_or_mem_char (const struct ehci_softc *up, int reg)
{
	if (up->sc_bus.port_mapped)
		return (read_port (up->sc_bus.base_addr + reg));
	else
		return (*(char *)(up->sc_bus.base_addr + reg));

}	/* end ehci_write_port_or_mem_char */

/*
 ****************************************************************
 *	Le 2 bytes de um registro (da porta ou mem�ria)		*
 ****************************************************************
 */
int
ehci_read_port_or_mem_short (const struct ehci_softc *up, int reg)
{
	if (up->sc_bus.port_mapped)
		return (read_port_short (up->sc_bus.base_addr + reg));
	else
		return (*(short *)(up->sc_bus.base_addr + reg));

}	/* end ehci_read_port_or_mem_short */

#ifdef	USB_MSG
/*
 ****************************************************************
 *	Escreve 2 bytes em um registro (da porta ou mem�ria)	*
 ****************************************************************
 */
void
ehci_write_port_or_mem_short (const struct ehci_softc *up, int value, int reg)
{
	if (up->sc_bus.port_mapped)
		write_port_short (value, up->sc_bus.base_addr + reg);
	else
		*(short *)(up->sc_bus.base_addr + reg) = value;

}	/* end ehci_write_port_or_mem_short */
#endif	USB_MSG

/*
 ****************************************************************
 *	Le 4 bytes de um registro (da porta ou mem�ria)		*
 ****************************************************************
 */
int
ehci_read_port_or_mem_long (const struct ehci_softc *up, int reg)
{
	if (up->sc_bus.port_mapped)
		return (read_port_long (up->sc_bus.base_addr + reg));
	else
		return (*(long *)(up->sc_bus.base_addr + reg));

}	/* end ehci_read_port_or_mem_long */

/*
 ****************************************************************
 *	Escreve 4 bytes em um registro (da porta ou mem�ria)	*
 ****************************************************************
 */
void
ehci_write_port_or_mem_long (const struct ehci_softc *up, int value, int reg)
{
	if (up->sc_bus.port_mapped)
		write_port_long (value, up->sc_bus.base_addr + reg);
	else
		*(long *)(up->sc_bus.base_addr + reg) = value;

}	/* end ehci_write_port_or_mem_long */

/*
 ****************************************************************
 *	Imprime os diversos registros				*
 ****************************************************************
 */
void
ehci_dump_regs (struct ehci_softc *sc)
{
	int		i;

	printf
	(	"cmd=%P, sts=%P, ien=%P\n",
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBCMD),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBSTS),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_USBINTR)
	);

	printf
	(	"frindex=%P ctrdsegm=%P periodic=%P async=%P\n",
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_FRINDEX),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_CTRLDSSEGMENT),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_PERIODICLISTBASE),
		ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_ASYNCLISTADDR)
	);

	for (i = 1; i <= sc->sc_noport; i++)
	{
		printf
		(	"port %d status=%P\n", i,
			ehci_read_port_or_mem_long (sc, sc->sc_offs + EHCI_PORTSC (i))
		);
	}

	getchar ();

}	/* end ehci_dump_regs */
#endif	/*******************************************************/


#if (0)	/*******************************************************/
void
ehci_timeout(void *addr)
{
	struct ehci_xfer *exfer = addr;
	struct ehci_pipe *epipe = (struct ehci_pipe *)exfer->xfer.pipe;
	struct ehci_softc *sc = (struct ehci_softc *)epipe->pipe.device->bus;

	DPRINTF(("ehci_timeout: exfer=%P\n", exfer));
#ifdef USB_DEBUG
	if (ehcidebug > 1)
		usbd_dump_pipe(exfer->xfer.pipe);
#endif

	if (sc->sc_dying) {
		ehci_abort_xfer(&exfer->xfer, USBD_TIMEOUT);
		return;
	}

	/* Execute the abort in a process context. */
	usb_init_task(&exfer->abort_task, ehci_timeout_task, addr);
	usb_add_task(exfer->xfer.pipe->device, &exfer->abort_task);
}

void
ehci_timeout_task(void *addr)
{
	struct usbd_xfer * xfer = addr;
	int s;

	DPRINTF(("ehci_timeout_task: xfer=%P\n", xfer));

	s = splusb();
	ehci_abort_xfer(xfer, USBD_TIMEOUT);
	splx(s);
}
/*
 * Unused function - this is meant to be called from a kernel
 * debugger.
 */
void
ehci_dump()
{
	ehci_dump_regs(theehci);
}

void
ehci_dump_link(ehci_link_t link, int type)
{
	link = le32toh(link);
	printf("%P", link);
	if (link & EHCI_LINK_TERMINATE)
		printf("<T>");
	else {
		printf("<");
		if (type) {
			switch (EHCI_LINK_TYPE(link)) {
			case EHCI_LINK_ITD: printf("ITD"); break;
			case EHCI_LINK_QH: printf("QH"); break;
			case EHCI_LINK_SITD: printf("SITD"); break;
			case EHCI_LINK_FSTN: printf("FSTN"); break;
			}
		}
		printf(">");
	}
}

void
ehci_dump_sqtds(ehci_soft_qtd_t *sqtd)
{
	int i;
	ulong stop;

	stop = 0;
	for (i = 0; sqtd && i < 20 && !stop; sqtd = sqtd->nextqtd, i++) {
		ehci_dump_sqtd(sqtd);
		stop = sqtd->qtd.qtd_next & EHCI_LINK_TERMINATE;
	}
	if (sqtd)
		printf("dump aborted, too many TDs\n");
}

void
ehci_dump_sqtd(ehci_soft_qtd_t *sqtd)
{
	printf("QTD(%P) at %P:\n", sqtd, sqtd->physaddr);
	ehci_dump_qtd(&sqtd->qtd);
}

void
ehci_dump_qtd(ehci_qtd_t *qtd)
{
	ulong s;
	char sbuf[128];

	printf("  next="); ehci_dump_link(qtd->qtd_next, 0);
	printf(" altnext="); ehci_dump_link(qtd->qtd_altnext, 0);
	printf("\n");
	s = le32toh(qtd->qtd_status);
	bitmask_snprintf(EHCI_QTD_GET_STATUS(s),
			 "\20\10ACTIVE\7HALTED\6BUFERR\5BABBLE\4XACTERR"
			 "\3MISSED\2SPLIT\1PING", sbuf, sizeof(sbuf));
	printf("  status=%P: toggle=%d bytes=0x%x ioc=%d c_page=0x%x\n",
	       s, EHCI_QTD_GET_TOGGLE(s), EHCI_QTD_GET_BYTES(s),
	       EHCI_QTD_GET_IOC(s), EHCI_QTD_GET_C_PAGE(s));
	printf("    cerr=%d pid=%d stat=0x%s\n", EHCI_QTD_GET_CERR(s),
	       EHCI_QTD_GET_PID(s), sbuf);
	for (s = 0; s < 5; s++)
		printf("  buffer[%d]=%P\n", s, le32toh(qtd->qtd_buffer[s]));
}

void
ehci_dump_sqh(ehci_soft_qh_t *sqh)
{
	ehci_qh_t *qh = &sqh->qh;
	ulong endp, endphub;

	printf("QH(%P) at %P:\n", sqh, sqh->physaddr);
	printf("  link="); ehci_dump_link(qh->qh_link, 1); printf("\n");
	endp = le32toh(qh->qh_endp);
	printf("  endp=%P\n", endp);
	printf("    addr=0x%02x inact=%d endpt=%d eps=%d dtc=%d hrecl=%d\n",
	       EHCI_QH_GET_ADDR(endp), EHCI_QH_GET_INACT(endp),
	       EHCI_QH_GET_ENDPT(endp),  EHCI_QH_GET_EPS(endp),
	       EHCI_QH_GET_DTC(endp), EHCI_QH_GET_HRECL(endp));
	printf("    mpl=0x%x ctl=%d nrl=%d\n",
	       EHCI_QH_GET_MPL(endp), EHCI_QH_GET_CTL(endp),
	       EHCI_QH_GET_NRL(endp));
	endphub = le32toh(qh->qh_endphub);
	printf("  endphub=%P\n", endphub);
	printf("    smask=0x%02x cmask=0x%02x huba=0x%02x port=%d mult=%d\n",
	       EHCI_QH_GET_SMASK(endphub), EHCI_QH_GET_CMASK(endphub),
	       EHCI_QH_GET_HUBA(endphub), EHCI_QH_GET_PORT(endphub),
	       EHCI_QH_GET_MULT(endphub));
	printf("  curqtd="); ehci_dump_link(qh->qh_curqtd, 0); printf("\n");
	printf("Overlay qTD:\n");
	ehci_dump_qtd(&qh->qh_qtd);
}

void
ehci_dump_exfer(struct ehci_xfer *ex)
{
	printf("ehci_dump_exfer: ex=%P\n", ex);
}
#endif	/*******************************************************/
