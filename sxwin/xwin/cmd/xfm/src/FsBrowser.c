/*
 ****************************************************************
 *								*
 *			FsBrowser.c				*
 *								*
 *	"Widget" para navegação visual pelo sistema de arquivos	*
 *								*
 *	Versão	3.2.1, de 15.02.00				*
 *								*
 *	Módulo: FsBrowser					*
 *		"Widgets" do Tropix				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */
#include <stdlib.h>
#include <stdio.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xatom.h>

#include <sys/types.h>
#if (0)	/*******************************************************/
#include <sys/dir.h>
#endif	/*******************************************************/
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/a.out.h>
#include <unistd.h>
#include <fcntl.h>

#include "../h/FsBrowserP.h"

/*
 ****** Variáveis e Definições globais **************************
 */
#define elif	else if

/****************************************************************
 *	Declaração das funções de métodos			*
 ****************************************************************
 */
static void		Initialize ();
static void		Redisplay ();
static void		Destroy ();
static void		Resize ();
static Boolean		SetValues ();
static XtGeometryResult	QueryGeometry ();

/****************************************************************
 *	Declaração das funções privativas			*
 ****************************************************************
 */
extern void	ChangeDir (FsBrowserWidget, char *);
extern void	MudaAparencia (FsBrowserWidget);
extern int	Select (FsBrowserWidget, int, int);
extern void	DrawIco (FsBrowserWidget, int);

/****************************************************************
 *	Declaração das funções de ações				*
 ****************************************************************
 */
static void	SelectItem ();
static void	ProcessSelection ();
static void	GoParentLevel ();


#define offset(field) XtOffsetOf(FsBrowserRec, field)

/*
 *	Lista de recursos
 */
static XtResource resources[] =
{
	{
		XtNcdNotify, 
		XtCCdNotify, 
		XtRCallback, 
		sizeof (XtPointer),
		offset (fsBrowser.cdnotify), 
		XtRCallback, 
		NULL
	},
	{
		XtNpath,
		XtCPath,
		XtRString,
		sizeof (String),
		offset (fsBrowser.path),
		XtRImmediate,
		""
	},
	{
		XtNminwidth,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.width),
		XtRImmediate,
		0
	},
	{
		XtNfocuson,
		XtCFocuson,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.focuson),
		XtRImmediate,
		0
	},
	{
		XtNminheight,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.height),
		XtRImmediate,
		0
	},
	{
		XtNcur_x,
		XtCCur_x,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.cur_x),
		XtRImmediate,
		0
	},
	{
		XtNcur_y,
		XtCCur_y,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.cur_y),
		XtRImmediate,
		0
	},
	{
		XtNmodoexib,
		XtCModoexib,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.modoexib),
		XtRImmediate,
		0
	},
	{
		XtNitenslinha,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.itenslinha),
		XtRImmediate,
		0
	},
	{
		XtNnumarqs,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.numarq),
		XtRImmediate,
		0
	},
	{
		XtNespvert,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.espvert),
		XtRImmediate,
		0
	},
	{
		XtNesphor,
		XtCReadOnly,
		XtRInt,
		sizeof (int),
		offset (fsBrowser.esphor),
		XtRImmediate,
		0
	},
};

#undef offset(field)


/*
 *	Tabela de traduções & tabela de ações
 */
static char	defaultTranslations[] =	"\
			<Btn1Down>	:	SelectItem()		\n\
			<Btn1Up>(2)	:	ProcessSelection()	\n\
			<Key>BackSpace	:	GoParentLevel()		\n\
			<Key>Return	:	ProcessSelection()	\n\
			<Key>		:	SelectItem()";

/*	~Shift ~Ctrl	<Btn1Up>(2)	:	ProcessSelection()	\n\
 *	~Shift ~Ctrl	<Btn1Down>	:	SelectItem()		\n\
 *	~Shift  Ctrl	<Btn1Down>	:	TrogleItemSel()		\n\
 *	 Shift  Ctrl	<Btn1Down>	:	AddField()		\n\
 *	 Shift ~Ctrl	<Btn1Down>	:	SelectField()		\n\
 */

static XtActionsRec	actions[] =	{	{"SelectItem", SelectItem},
						{"ProcessSelection", ProcessSelection},
						{"GoParentLevel", GoParentLevel},
					};

/*
 *	Inicialização da "Class Record"
 */
FsBrowserClassRec fsBrowserClassRec =
{
	{
		/* core_class fields */

		(WidgetClass) &coreClassRec,	/* superclass		*/
		"FsBrowser",			/* class_name		*/
		sizeof (FsBrowserRec),		/* widget_size		*/
		NULL,				/* class_initialize	*/
		NULL,				/* class_part_initialize*/
		FALSE,				/* class_inited		*/
		Initialize,			/* initialize		*/
		NULL,				/* initialize_hook	*/
		XtInheritRealize,		/* realize		*/
		actions,			/* actions		*/
		XtNumber (actions),		/* num_actions		*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/
		NULLQUARK,			/* xrm_class		*/
		TRUE,				/* compress_motion	*/
		XtExposeCompressMultiple,	/* compress_exposure	*/
		TRUE,				/* compress_enterleave	*/
		FALSE,				/* visible_interest	*/
		Destroy,			/* destroy		*/
		Resize,				/* resize		*/
		Redisplay,			/* expose		*/
		SetValues,			/* set_values		*/
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,				/* get_values_hook	*/
		NULL,				/* accept_focus		*/
		XtVersion,			/* version		*/
		NULL,				/* callback_private	*/
		defaultTranslations,		/* tm_table		*/
		QueryGeometry,			/* query_geometry	*/
		XtInheritDisplayAccelerator,	/* display_accelerator	*/
		NULL				/* extension		*/
	},
	{
		/* dummy_field */
		0
	},
};

WidgetClass fsBrowserWidgetClass = (WidgetClass) &fsBrowserClassRec;


/*
 ****************************************************************
 *								*
 *	Definições das funções de método			*
 *								*
 ****************************************************************
 */
static void
Initialize (Widget treq, Widget tnew, ArgList args, Cardinal *num_args)
{
	XGCValues	values;
	XtGCMask	mask;
	char		*chr;
	STAT		st;
	int		fd;
	FsBrowserWidget	new = (FsBrowserWidget) tnew;

	/* 
	 *  Verifica os valores da instância atribuidos pelos recursos que podem ser inválidos
	 */
	if (strcpy (new->fsBrowser.caminho, new->fsBrowser.path) != NOSTR)
		new->fsBrowser.path = new->fsBrowser.caminho;

	fd = open (new->fsBrowser.caminho, O_RDONLY);

	if ((fd < 0) || streq (new->fsBrowser.caminho, ""))
		strcpy (new->fsBrowser.caminho, getenv ("HOME"));

	if (fd >= 0)
		close (fd);

	new->fsBrowser.descr = NULL;
	new->fsBrowser.numarq = 0;
	new->fsBrowser.fsp = XLoadQueryFont (XtDisplay (new), "8x13");

	if (new->fsBrowser.modoexib > 2)
		new->fsBrowser.modoexib = EXIB_ICONES;

	if (new->fsBrowser.fsp == NULL)
	{
		error ("Não consegui ler as informações métricas da fonte");
		return;
	}

	/*
	 *	Inicialisa o GC
	 */
	mask =	GCForeground | GCBackground | GCFont | GCDashOffset | GCDashList | GCLineStyle;

	values.foreground = 1;
	values.background = 0;
	values.font = new->fsBrowser.fsp->fid;
	values.dashes = 1;
	values.dash_offset = 0;
	values.line_style = LineOnOffDash;

	new->fsBrowser.gc = XtGetGC ((Widget) new, mask, &values);

	mask =	GCForeground | GCBackground;

	values.foreground = 254;
	values.background = 0;

	new->fsBrowser.gcs = XtGetGC ((Widget) new, mask, &values);

	/*
	 *	Valida o caminho
	 */
	stat (new->fsBrowser.caminho, &st);

	while ((st.st_mode & S_IFMT) != S_IFDIR)
	{
		stat (new->fsBrowser.caminho, &st);

		chr = strrchr (new->fsBrowser.caminho, '/');

		if (chr != NOSTR)
		{
			if (chr == new->fsBrowser.caminho)
				strcpy (new->fsBrowser.caminho, "/");
			else
				new->fsBrowser.caminho[0] = '\0';
		}
		else
		{
			if (streq (new->fsBrowser.caminho, ""))
				strcpy (new->fsBrowser.caminho, "/");
			else
				strcpy (new->fsBrowser.caminho, "");
		}
	}

	if (new->core.width < new->fsBrowser.esphor)
		new->core.width = new->fsBrowser.esphor;

	if (new->core.height < new->fsBrowser.espvert)
		new->core.height = new->fsBrowser.espvert;

	/*
	 *	Muda o diretório
	 */
	ChangeDir (new, ".");

}	/* end initialize */

static void
Redisplay (Widget w, XExposeEvent *event)
{
	FsBrowserWidget	cw = (FsBrowserWidget) w;
	register int	x, y;
	unsigned int	width, height;
	int		icox, icoy, icoa, icob;
	int		i, j, indice;

	if (!XtIsRealized ((Widget) cw))
		return;

	if (event)		/* called from btn-event or expose */
	{
		x = event->x;
		y = event->y; 
		width = event->width;
		height = event->height;
	} 
	else			/* called because complete redraw */
	{
		x = 0;
		y = 0; 
		width = cw->core.width;
		height = cw->core.height;
	}

	icox = (x + cw->fsBrowser.cur_x) / cw->fsBrowser.esphor;
	icoy = (y + cw->fsBrowser.cur_y) / cw->fsBrowser.espvert;
	icoa = (width + x + cw->fsBrowser.cur_x) / cw->fsBrowser.esphor;
	icob = (height + y + cw->fsBrowser.cur_y) / cw->fsBrowser.espvert;

	XClearArea
	(	XtDisplay (cw), cw->core.window,
		(icox * cw->fsBrowser.esphor) - cw->fsBrowser.cur_x,
		(icoy * cw->fsBrowser.espvert) - cw->fsBrowser.cur_y,
		((icoa - icox + 1) * cw->fsBrowser.esphor) - 1,
		((icob - icoy + 1) * cw->fsBrowser.espvert) - 1,
		False
	);

	if (cw->fsBrowser.modoexib == EXIB_DETALHES)
	{
		if (icox > 0)
			return;

		icoa = 0;
	}

	if (cw->fsBrowser.modoexib == EXIB_ICONES)
	{
		icox += icoy;
		icoy = icox - icoy;
		icox -= icoy;
		icoa += icob;
		icob = icoa - icob;
		icoa -= icob;
	}

	for (i = icox; i <= icoa; i++)
	{
		if (i < 0)
			continue;

		for (j = icoy; j <= icob, j < cw->fsBrowser.itenslinha; j++)
		{
			indice = (i * cw->fsBrowser.itenslinha) + j;

			if ((indice >= cw->fsBrowser.numarq) || (j < 0))
				continue;

			DrawIco (cw, indice);
		}
	}



}	/* end redisplay */

static Boolean
SetValues (Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
	FsBrowserWidget	curcw		= (FsBrowserWidget) current;
	FsBrowserWidget	newcw		= (FsBrowserWidget) new;
	Boolean		do_redisplay	= False;

	if
	(	(curcw->core.background_pixel != newcw->core.background_pixel) ||
		(curcw->core.background_pixmap != newcw->core.background_pixmap) ||
		(curcw->fsBrowser.cur_x != newcw->fsBrowser.cur_x) ||
		(curcw->fsBrowser.cur_y != newcw->fsBrowser.cur_y)
	)
	{
		do_redisplay = True;
	}

	if (curcw->fsBrowser.modoexib != newcw->fsBrowser.modoexib)
	{
		newcw->fsBrowser.cur_x = 0;
		newcw->fsBrowser.cur_y = 0;
		do_redisplay = True;

		MudaAparencia (newcw);
		XtCallCallbacks ((Widget) newcw, XtNcdNotify, (XtPointer) newcw->fsBrowser.path);
	}

	if (curcw->fsBrowser.path != newcw->fsBrowser.path)
	{
		ChangeDir (newcw, (char *) newcw->fsBrowser.path);
		do_redisplay = False;
	}

	return (do_redisplay);
}

static void
Destroy (Widget w)
{
	FsBrowserWidget cw = (FsBrowserWidget) w;

	if (cw->fsBrowser.gc)
		XFreeGC (XtDisplay(cw), cw->fsBrowser.gc);

	if (cw->fsBrowser.gcs)
		XFreeGC (XtDisplay(cw), cw->fsBrowser.gcs);

	if (cw->fsBrowser.descr != NULL)
		free (cw->fsBrowser.descr);

}

static void
Resize (Widget w)
{
	FsBrowserWidget cw = (FsBrowserWidget) w;

	switch (cw->fsBrowser.modoexib)
	{
	    case EXIB_ICONES:
		cw->fsBrowser.itenslinha = cw->core.width / cw->fsBrowser.esphor;

		if (cw->fsBrowser.itenslinha == 0)
			cw->fsBrowser.itenslinha = 1;

		cw->fsBrowser.width = cw->fsBrowser.itenslinha * cw->fsBrowser.esphor;
		cw->fsBrowser.height = ((cw->fsBrowser.numarq - 1) / cw->fsBrowser.itenslinha) + 1;
		cw->fsBrowser.height = cw->fsBrowser.height * cw->fsBrowser.espvert;
		break;

	    case EXIB_LISTA:
		cw->fsBrowser.itenslinha = cw->core.height / cw->fsBrowser.espvert;

		if (cw->fsBrowser.itenslinha == 0)
			cw->fsBrowser.itenslinha = 1;

		cw->fsBrowser.width = ((cw->fsBrowser.numarq - 1) / cw->fsBrowser.itenslinha) + 1;
		cw->fsBrowser.width = cw->fsBrowser.width * cw->fsBrowser.esphor;
		cw->fsBrowser.height = cw->fsBrowser.itenslinha * cw->fsBrowser.espvert;
		break;

	    case EXIB_DETALHES:
		cw->fsBrowser.itenslinha = 30000;
		cw->fsBrowser.width = cw->fsBrowser.esphor;
		cw->fsBrowser.height = cw->fsBrowser.numarq * cw->fsBrowser.espvert;
	}

}	/* end Resize */

static XtGeometryResult
QueryGeometry (Widget w, XtWidgetGeometry *proposed, XtWidgetGeometry *answer)
{
	FsBrowserWidget cw = (FsBrowserWidget) w;

	answer->request_mode = CWWidth | CWHeight;

	if (proposed->width < cw->fsBrowser.esphor)
		answer->width = cw->fsBrowser.esphor;
	else
		answer->width = proposed->width;

	if (proposed->height < cw->fsBrowser.espvert)
		answer->height = cw->fsBrowser.espvert;
	else
		answer->height = proposed->height;

	if
	(	((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight)) &&
		proposed->width == answer->width &&
		proposed->height == answer->height
	)
	{
		return XtGeometryYes;
	}
	else
	{
		if (answer->width == cw->core.width && answer->height == cw->core.height)
			return XtGeometryNo;
		else
			return XtGeometryAlmost;
	}

}	/* end QueryGeometry */

/*
 ********************************************************
 *							*
 *	Definições das funcões de ações			*
 *							*
 ********************************************************
 */
static void
SelectItem (Widget w, XEvent *xevent)
{
	FsBrowserWidget	fsbw = (FsBrowserWidget) w;
	int		indice = -1, oldfocus = -1;
	int		i, icox, icoy, xlim, ylim;
	Boolean		dosel = False;
	div_t		result;
	unsigned int	key;

	if (xevent->type == KeyPress)
	{
		result = div (fsbw->fsBrowser.focuson, fsbw->fsBrowser.itenslinha);

		if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
		{
			icox = result.rem;
			icoy = result.quot;

			if (((icoy + 1) * fsbw->fsBrowser.itenslinha) < fsbw->fsBrowser.numarq)
				xlim = fsbw->fsBrowser.itenslinha - 1;
			else
				xlim = fsbw->fsBrowser.numarq - (icoy * fsbw->fsBrowser.itenslinha) - 1;

			ylim = (fsbw->fsBrowser.numarq - 1) / fsbw->fsBrowser.itenslinha;

			if ((ylim * fsbw->fsBrowser.itenslinha) + icox >= fsbw->fsBrowser.numarq)
				ylim--;
		}
		else
		{
			icox = result.quot;
			icoy = result.rem;

			xlim = (fsbw->fsBrowser.numarq - 1) / fsbw->fsBrowser.itenslinha;

			if ((xlim * fsbw->fsBrowser.itenslinha) + icoy >= fsbw->fsBrowser.numarq)
				xlim--;

			if (((icox + 1) * fsbw->fsBrowser.itenslinha) < fsbw->fsBrowser.numarq)
				ylim = fsbw->fsBrowser.itenslinha - 1;
			else
				ylim = fsbw->fsBrowser.numarq - (icox * fsbw->fsBrowser.itenslinha) - 1;
		}

		key = ((XKeyEvent *) xevent) ->keycode;
		oldfocus = fsbw->fsBrowser.focuson;
		dosel = True;

		switch (key)
		{
		    case 100:
			if (icox > 0)
				icox--;

			break;

		    case 102:
			if (icox < xlim)
				icox++;

			break;

		    case 98:
			if (icoy > 0)
				icoy--;

			break;

		    case 104:
			if (icoy < ylim)
				icoy++;

			break;

		    case 65:
			break;

		    default:
			dosel = False;
		}

		if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
			indice = icox + (icoy * fsbw->fsBrowser.itenslinha);
		else
			indice = (icox * fsbw->fsBrowser.itenslinha) + icoy;

		fsbw->fsBrowser.focuson = indice;

	}	/* end if (xevent->type == KeyPress) */
	elif (xevent->type == ButtonPress)
	{
		icox = (((XButtonEvent *) xevent) ->x + fsbw->fsBrowser.cur_x) / fsbw->fsBrowser.esphor;
		icoy = (((XButtonEvent *) xevent) ->y + fsbw->fsBrowser.cur_y) / fsbw->fsBrowser.espvert;

		if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
			indice = icox + (icoy * fsbw->fsBrowser.itenslinha);
		else
			indice = (icox * fsbw->fsBrowser.itenslinha) + icoy;

		if ((indice < fsbw->fsBrowser.numarq) && (indice > 0))
		{
			oldfocus = fsbw->fsBrowser.focuson;
			fsbw->fsBrowser.focuson = indice;
		}

		dosel = True;
	}

	if (dosel)
	{
		for (i = 0; i < fsbw->fsBrowser.numarq; i++)
		{
			if (i != indice)
			{
				if (!Select (fsbw, i, -1))
				{
					if ((fsbw->fsBrowser.focuson == i) || (oldfocus == i))
					{
						result = div (i, fsbw->fsBrowser.itenslinha);

						if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
						{
							icox = result.rem;
							icoy = result.quot;
						}
						else
						{
							icox = result.quot;
							icoy = result.rem;
						}

						XClearArea
						(	XtDisplay (fsbw), fsbw->core.window,
							(icox * fsbw->fsBrowser.esphor) - fsbw->fsBrowser.cur_x,
							(icoy * fsbw->fsBrowser.espvert) - fsbw->fsBrowser.cur_y,
							fsbw->fsBrowser.esphor - 1,
							fsbw->fsBrowser.espvert - 1,
							False
						);
						DrawIco (fsbw, i);
					}
				}
			}
			else
			{
				if (!Select (fsbw, i, 1))
				{
					if (fsbw->fsBrowser.focuson != oldfocus)
					{
						result = div (i, fsbw->fsBrowser.itenslinha);

						if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
						{
							icox = result.rem;
							icoy = result.quot;
						}
						else
						{
							icox = result.quot;
							icoy = result.rem;
						}

						XClearArea
						(	XtDisplay (fsbw), fsbw->core.window,
							(icox * fsbw->fsBrowser.esphor) - fsbw->fsBrowser.cur_x,
							(icoy * fsbw->fsBrowser.espvert) - fsbw->fsBrowser.cur_y,
							fsbw->fsBrowser.esphor - 1,
							fsbw->fsBrowser.espvert - 1,
							False
						);
						DrawIco (fsbw, i);
					}
				}
			}

		}	/* end for */

	}	/* end if */

}	/* end SelectItem */

void
ProcessWith (Widget w, char *command)
{
	FsBrowserWidget	fsbw = (FsBrowserWidget) w;
	int		i, j;
	char		*chr;

	for (i = 0; i < fsbw->fsBrowser.numarq; i++)
	{
		j = i / (sizeof (int) * 8);

		if (fsbw->fsBrowser.selected[j] & (1 << (i - (j * sizeof (int) * 8))))
		{
				if
				(	(fsbw->fsBrowser.caminho[0] != '\0') &&
					!streq (fsbw->fsBrowser.caminho, "/")
				)
					strcat (fsbw->fsBrowser.caminho, "/");

				strcat (fsbw->fsBrowser.caminho, fsbw->fsBrowser.descr[i].nome);

				if (fork() == 0)
				{
					if (command[0] == '*')
					{
						execl	(	"/usr/xwin/bin/xterm", "xterm",
								"-e", (command + 1), fsbw->fsBrowser.caminho,
								NULL
							);
					}
					else
					{
						execl (command, command, fsbw->fsBrowser.caminho, NULL);
					}

					error ("$*Exec falhou");
				}

				if ((chr = strrchr (fsbw->fsBrowser.caminho, '/')) != fsbw->fsBrowser.caminho)
					*chr = '\0';
				else
					chr[1] = '\0';

				if (chr == NOSTR)
					fsbw->fsBrowser.caminho[0] = '\0';
		}
	}

	memset (fsbw->fsBrowser.selected, 0, sizeof (int) * 20);
	XClearArea (XtDisplay (fsbw), fsbw->core.window, 0, 0, 0, 0, True);

}	/* end ProcessWith */

static void
ProcessSelection (Widget w, XEvent *xevent)
{
	FsBrowserWidget	fsbw = (FsBrowserWidget) w;
	int		i, j, dirs = 0;
	char		*chr, *dir = ".";

	for (i = 0; i < fsbw->fsBrowser.numarq; i++)
	{
		j = i / (sizeof (int) * 8);

		if (fsbw->fsBrowser.selected[j] & (1 << (i - (j * sizeof (int) * 8))))
		{
			if (fsbw->fsBrowser.descr[i].tipo == T_DIRETORIO)
			{
				dirs++;
				dir = fsbw->fsBrowser.descr[i].nome;
			}
			elif (fsbw->fsBrowser.descr[i].tipo == T_EXECUTAVEL)
			{
				if
				(	(fsbw->fsBrowser.caminho[0] != '\0') &&
					!streq (fsbw->fsBrowser.caminho, "/")
				)
					strcat (fsbw->fsBrowser.caminho, "/");

				strcat (fsbw->fsBrowser.caminho, fsbw->fsBrowser.descr[i].nome);

				if (fork() == 0)
				{
					execl (fsbw->fsBrowser.caminho, fsbw->fsBrowser.descr[i].nome, 0);
					exit (1);
				}

				if ((chr = strrchr (fsbw->fsBrowser.caminho, '/')) != fsbw->fsBrowser.caminho)
					*chr = '\0';
				else
					chr[1] = '\0';

				if (chr == NOSTR)
					fsbw->fsBrowser.caminho[0] = '\0';
			}
		}
	}

	memset (fsbw->fsBrowser.selected, 0, sizeof (int) * 20);
	XClearArea (XtDisplay (fsbw), fsbw->core.window, 0, 0, 0, 0, True);

	if (dirs == 1)
		ChangeDir (fsbw, dir);

}	/* end ProcessSelection */

static void
GoParentLevel (Widget w, XEvent *xevent)
{
	FsBrowserWidget	fsbw = (FsBrowserWidget) w;

	ChangeDir (fsbw, "..");

}	/* end GoParentLevel */
