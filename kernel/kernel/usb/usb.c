/*
 ****************************************************************
 *								*
 *			usb.c					*
 *								*
 *	"Driver" para dispositivo USB				*
 *								*
 *	Vers�o	4.3.0, de 07.10.02				*
 *		4.6.0, de 06.10.04				*
 *								*
 *	M�dulo: N�cleo						*
 *		N�CLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 2004 NCE/UFRJ - tecle "man licen�a"	*
 *		Baseado no FreeBSD 5.0				*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/scb.h"

#include "../h/signal.h"
#include "../h/region.h"
#include "../h/uproc.h"

#include "../h/usb.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ******	Declara��o do Driver ************************************
 */
int	usb_probe  (struct device *);
int	usb_attach (struct device *);
int	usb_detach (struct device *);

const struct driver usb_driver =
{
	"usb",

	usb_probe,
	usb_attach,
	usb_detach
};

/*
 ******	Estrutura da parte espec�fica ***************************
 */
#define	EXPLORE_TIMEOUT		5

struct usb_softc
{
	struct device		*sc_dev;	/* base device */
	struct usbd_bus 	*sc_bus;	/* USB controller */
	struct usbd_port	sc_port;	/* dummy port for root hub */

	int			sc_dying;
};

/*
 ******	Informa��es sobre as Unidades ***************************
 */
entry void			*usb_data[NUSB];
entry struct usb_softc		*usb_softc_data[NUSB];

entry int			usb_unit = -1;
entry int			usb_active;

entry EVENT			usb_explore_event;

void				usb_explorer (void);

/*
 ****************************************************************
 *	Fun��o de "probe"					*
 ****************************************************************
 */
int
usb_probe (struct device *dev)
{
	return (UPROBE_GENERIC);

}	/* end usb_probe */

/*
 ****************************************************************
 *	Fun��o de "attach"					*
 ****************************************************************
 */
int
usb_attach (struct device *dev)
{
	struct usb_softc	*sc;
	struct usbd_device	*udev;
	int			speed;

	/*
	 *	Aloca a estrutura "softc"
	 */
	if ((sc = dev->softc = malloc_byte (sizeof (struct usb_softc))) == NULL)
		return (-1);

	memclr (sc, sizeof (struct usb_softc));

	sc->sc_dev		= dev;
	sc->sc_bus		= dev->ivars;
	sc->sc_bus->usbctl	= sc;
	sc->sc_port.power	= USB_MAX_POWER;

#ifdef	USB_MSG
	printf ("usb_attach: %s: USB revision ", dev->nameunit);
#endif	USB_MSG

	switch (sc->sc_bus->usbrev)
	{
	    case USBREV_1_0:
		speed = USB_SPEED_FULL;
		break;
		
	    case USBREV_1_1:
		speed = USB_SPEED_FULL;
		break;

	    case USBREV_2_0:
		speed = USB_SPEED_HIGH;
		break;

	    default:
		goto bad;
	}

#ifdef	USB_POLLING
	if (usb_cold)
		sc->sc_bus->use_polling++;
#endif	USB_POLLING

	if (usbd_new_device (sc->sc_dev, sc->sc_bus, 0, speed, 0, &sc->sc_port) != 0)
		{ printf ("%s: HUB raiz defeituoso\n", dev->nameunit); goto bad; }

	udev = sc->sc_port.device;

	if (udev->hub == NULL)
		{ printf ("%s: Hub raiz N�O encontrado\n", dev->nameunit); goto bad; }

	sc->sc_bus->root_hub = udev;

	/*
	 *	Explora o "hub" raiz
	 */
	(*udev->hub->explore) (sc->sc_bus->root_hub);

	/*
	 *	Inclui a unidade na lista de explor�veis
	 */
	usb_softc_data[usb_unit] = sc;	usb_active++;

#ifdef	USB_POLLING
	if (usb_cold)
		sc->sc_bus->use_polling--;
#endif	USB_POLLING

	return (0);

	/*
	 *	Em caso de erro, ...
	 */
    bad:
	free_byte (sc);	dev->softc = NULL;
	return (-1);

}	/* end usb_attach */

/*
 ****************************************************************
 *	Fun��o de "detach"					*
 ****************************************************************
 */
int
usb_detach (struct device *dev)
{
	return (-1);

}	/* end usb_detach */

/*
 ****************************************************************
 *	Varre todas as unidades, em busca de altera��es		*
 ****************************************************************
 */
void
usb_explorer (void)
{
	struct usb_softc	*sc;
	int			unit, nactive;

	do
	{
		EVENTWAIT  (&usb_explore_event, PBLKIO);
		EVENTCLEAR (&usb_explore_event);

		for (nactive = unit = 0; unit <= usb_unit; unit++)
		{
			if ((sc = usb_softc_data[unit]) == NULL)
				continue;

			if (sc->sc_dying)
				{ usb_softc_data[unit] = NULL; continue; }

			/* Achou uma unidade a explorar */

			nactive++;
#ifdef	MSG
			if (CSWT (39))
				printf ("%s: iniciando explora��o\n", sc->sc_dev->nameunit);
#endif	MSG
			(*sc->sc_bus->root_hub->hub->explore) (sc->sc_bus->root_hub);
		}

	}	while ((usb_active = nactive) > 0);

	/* N�o h� mais unidades ativas */

	printf ("usb: explora��o peri�dica encerrada\n");

	kexit (0, 0, 0);

}	/* end usb_explorer */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
usb_needs_explore (struct usbd_device *udev)
{
	EVENTDONE (&usb_explore_event);

}	/* end usb_needs_explore */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
usb_schedsoftintr (struct usbd_bus *bus)
{
       (*bus->methods->soft_intr) (bus);

} 	/* end usb_schedsoftintr */

/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
int
usb_str (struct usb_string_descriptor *p, int l, char *s)
{
	int	i;

	if (l == 0)
		return (0);

	p->bLength = 2 * strlen(s) + 2;

	if (l == 1)
		return (1);

	p->bDescriptorType = UDESC_STRING; l -= 2;

	for (i = 0; s[i] && l > 1; i++, l -= 2)
		USETW2 (p->bString[i], 0, s[i]);

	return (2 * i + 2);

}	/* end usb_str */
