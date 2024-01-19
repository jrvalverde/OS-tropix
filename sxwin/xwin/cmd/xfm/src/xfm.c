/*
 ****************************************************************
 *								*
 *			xfm.c					*
 *								*
 *	Gerenciador de arquivos para o X Windows		*
 *								*
 *	Versão	3.2.1, de 15.02.00				*
 *								*
 *	Módulo: xfm						*
 *		Utilitário para o sistema X Windows		*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "../h/xlogo16.v"
#include <X11/cursorfont.h>

#include <X11/Xatom.h>
#include <X11/Xos.h>		/* for types.h */

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <sys/stat.h>
#include "../h/FsBrowser.h"
#include "../h/Fsb.h"
#include "../h/TMoreOrLess.h"
#include "../h/TTree.h"

/*
 ****** Protótipos de funções ***********************************
 */
void		mudamodo (Widget, XtPointer, XtPointer);
void		abrecom (Widget, XtPointer, XtPointer);
void		quit ();
void		AskForMenu ();
void		cdnotify (Widget, XtPointer, XtPointer);
extern void	ProcessWith (Widget, char *);

/*
 ****** Variáveis e Definições globais **************************
 */
#define elif	else if

static XtActionsRec	actions[] =	{	{"quit",	quit},
						{"AskForMenu",	AskForMenu},
					};

static Atom		wm_delete_window;
Display			*CurDpy;

XStandardColormap	xstdcmap;

Pixmap			mark;
Widget			principal, ultimo, abremenu;

/*
 ****************************************************************
 *	Gerenciador de arquivos para o X Windows		*
 ****************************************************************
 */
void
main (int argc, char *argv[])
{
	XtAppContext		appcon;
	Widget			top, outer, outh, label;
	Widget			box, exibbutton, exibmenu, w;
	Widget			sidetree, fsb, fsbw;
	int			pid, i, modoexib;
	char			*path;
	String			names[] =	{	"exib_icones",
							"exib_lista",
							"exib_detalhes",
							"ac_vi",
							"ac_xedit",
						};
	String			labels[] =	{	"Icones",
							"Lista",
							"Detalhes",
							"vi",
							"xedit",
						};
	String			command[] =	{	"*vi",
							"/usr/xwin/bin/xedit",
						};

	static XrmOptionDescRec	table[] =
	{
		{"-i",	"*modoexib",	XrmoptionNoArg,	(XtPointer) "0"},
		{"-l",	"*modoexib",	XrmoptionNoArg,	(XtPointer) "1"},
		{"-d",	"*modoexib",	XrmoptionNoArg,	(XtPointer) "2"},
		{"-caminho",	"*path",	XrmoptionSepArg,	NULL},
	};

	pid = fork();

	if (pid > 0)
		exit (0);

	if (pid < 0)
		error ("Não consegui dar um fork");

	top = XtAppInitialize (&appcon, "Xfm", table, XtNumber (table), &argc, argv, NULL, NULL, 0);

	XtAppAddActions (appcon, actions, XtNumber(actions));
	XtOverrideTranslations (top, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

	CurDpy = XtDisplay (top);

	outer = XtCreateManagedWidget ("paned", panedWidgetClass, top, NULL, 0);
	label = XtVaCreateManagedWidget	(	"etiqueta", labelWidgetClass, outer,
						XtNshowGrip,	False,
						XtNheight,	20,
						NULL
					);
	box = XtVaCreateManagedWidget	(	"box", boxWidgetClass, outer,
						XtNorientation,	"horizontal",
						XtNshowGrip,	False,
						XtNheight,	20,
						XtNvSpace,	0,
						XtNhSpace,	0,
						NULL
					);

	exibbutton = XtVaCreateManagedWidget	(	"exibButton", menuButtonWidgetClass, box,
							XtNlabel,	"Exibir",
							XtNmenuName,	"exibMenu",
							XtNshowGrip,	False,
							XtNwidth,	60,
							XtNheight,	20,
							XtNborderWidth,	0,
							NULL
						);

	exibmenu = XtVaCreatePopupShell	(	"exibMenu", simpleMenuWidgetClass, exibbutton,
						XtNrowHeight,	16,
						NULL
					);

	outh = XtVaCreateManagedWidget	(	"paned", panedWidgetClass, outer,
						XtNorientation,	XtorientHorizontal,
						XtNborderWidth,	0,
						NULL
					);
	sidetree = XtVaCreateManagedWidget	(	"sideTree", tTreeWidgetClass, outh,
							XtNborderWidth,	0,
							NULL
						);
	fsb = XtVaCreateManagedWidget	(	"fsb", fsbWidgetClass, outh,
						XtNborderWidth,	0,
						NULL
					);

	FillTreeDir (sidetree, 0, 1, "/");

	XtVaGetValues (fsb, "fsbw", &fsbw, NULL);

	abremenu = XtVaCreatePopupShell	(	"abreMenu", simpleMenuWidgetClass, fsbw,
						XtNrowHeight,	16,
						NULL
					);

	XtOverrideTranslations
	(	fsbw, XtParseTranslationTable	(	"<Btn2Down>:	SelectItem()	\
									AskForMenu()	\n\
							 <Btn3Down>:	SelectItem()	\
									AskForMenu()"
						)
	);

	XtAddCallback (fsbw, XtNcdNotify, cdnotify, (XtPointer) label);

	XtVaGetValues (fsbw, XtNpath, &path, XtNmodoexib, &modoexib, NULL);
	XtVaSetValues (label, XtNlabel, path, NULL);

	mark = XCreateBitmapFromData	(	XtDisplay (top),
						RootWindowOfScreen (XtScreen (top)),
						(char *) xlogo16_bits, xlogo16_width, xlogo16_height
					);
	principal = fsbw;

	for (i = 0; i < 3; i++)
	{
		w = XtVaCreateManagedWidget	(	names[i], smeBSBObjectClass, exibmenu,
							XtNlabel,	labels[i],
							XtNleftMargin,	20,
							NULL
						);

		if (i == modoexib)
		{
			ultimo = w;
			XtVaSetValues (w, XtNleftBitmap, mark, NULL);
		}

		XtAddCallback (w, XtNcallback, mudamodo, (XtPointer) i);
	}

	for (i = 0; i < 2; i++)
	{
		w = XtVaCreateManagedWidget	(	names[i+3], smeBSBObjectClass, abremenu,
							XtNlabel,	labels[i+3],
							NULL
						);

		XtAddCallback (w, XtNcallback, abrecom, (XtPointer) command[i]);
	}

/*	XtRegisterGrabAction	(	AskForMenu, True,
 *					(unsigned int)(ButtonPressMask | ButtonReleaseMask),
 *					GrabModeAsync, GrabModeAsync
 *				);
 */

	XtRealizeWidget (top);
	XDefineCursor
	(	XtDisplay (top), XtWindow (top),
		XCreateFontCursor (XtDisplay (top), XC_left_ptr)
	);

	/*
	 *	Inicializa o mapa de cores padrão
	 *	Precisa ser depois do XtRealizeWidget porque o display precisa estar aberto
	 */
	if (!XGetStandardColormap (CurDpy, RootWindow (CurDpy, 0), &xstdcmap, XA_RGB_BEST_MAP))
		error ("$Mapa de cores padrão não disponível");

	wm_delete_window = XInternAtom (XtDisplay (top), "WM_DELETE_WINDOW", False);
	XSetWMProtocols (XtDisplay (top), XtWindow (top), &wm_delete_window, 1);

	XtAppMainLoop (appcon);

}	/* end xfm */

void
mudamodo (Widget w, XtPointer client_data, XtPointer call_data)
{
	if (ultimo == w)
		return;

	XtVaSetValues (ultimo, XtNleftBitmap, None, NULL);
	ultimo = w;
	XtVaSetValues (ultimo, XtNleftBitmap, mark, NULL);
	XtVaSetValues (principal, XtNmodoexib, (int) client_data, NULL);
}

void
abrecom (Widget w, XtPointer client_data, XtPointer call_data)
{
	ProcessWith (principal, (char *) client_data);
}

void
quit (void)
{
	exit (0);
}

void
cdnotify (Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaSetValues ((Widget) client_data, XtNlabel, (const char *) call_data, NULL);
}

void
AskForMenu (Widget w, XEvent *xevent)
{
	Position	x, y;
	div_t		result;
	int		focuson, itenslinha, modoexib, esphor, espvert, cur_x, cur_y;

	if (!XtIsRealized (abremenu))
		XtRealizeWidget (abremenu);

	XtTranslateCoords (w, 0, 0, &x, &y);

	if (xevent->type == KeyPress)
	{
		XtVaGetValues	(	w,
					XtNfocuson,	&focuson,
					XtNitenslinha,	&itenslinha,
					XtNmodoexib,	&modoexib,
					XtNesphor,	&esphor,
					XtNespvert,	&espvert,
					XtNcur_x,	&cur_x,
					XtNcur_y,	&cur_y,
					NULL
				);

		result = div (focuson, itenslinha);

		if (modoexib == EXIB_ICONES)
		{
			x += ((result.rem + 0.5) * esphor) - cur_x;
			y += ((result.quot + 0.5) * espvert) - cur_y;
		}
		else
		{
			x += ((result.quot + 0.5) * esphor) - cur_x;
			y += ((result.rem + 0.5) * espvert) - cur_y;
		}
	}
	elif (xevent->type == ButtonPress)
	{
		x += ((XButtonEvent *) xevent) ->x;
		y += ((XButtonEvent *) xevent) ->y;
	}

	x -= 20;
	y -= 10;

	XtVaSetValues (abremenu, XtNx, x, XtNy, y, NULL);
	XtPopupSpringLoaded (abremenu);

}	/* AskForMenu */
