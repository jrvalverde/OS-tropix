/*
 ****************************************************************
 *								*
 *			TMoreOrLess.c				*
 *								*
 *	"Widget" para expansão e contração por callbacks	*
 *								*
 *	Versão	3.2.3, de 28.06.00				*
 *								*
 *	Módulo: TMoreOrLess					*
 *		"Widgets" do Tropix				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "../h/TMoreOrLessP.h"

#define elif	else if

/****************************************************************
 *	TMoreOrLess Resources					*
 ****************************************************************
 */
#define offset(field) XtOffsetOf(TMoreOrLessRec, field)

static XtResource resources[] =
{
	{
		XtNmore,
		XtCMore,
		XtRCallback,
		sizeof (XtPointer),
		offset (tMoreOrLess.more),
		XtRCallback,
		NULL
	},
	{
		XtNless,
		XtCLess,
		XtRCallback,
		sizeof (XtPointer),
		offset (tMoreOrLess.less),
		XtRCallback,
		NULL
	},
	{
		XtNcompressed,
		XtCCompressed,
		XtRBoolean,
		sizeof (Boolean),
		offset (tMoreOrLess.compressed),
		XtRImmediate,
		True
	},
};

#undef offset(field)

/****************************************************************
 *	Protótipos das funções					*
 ****************************************************************
 */
static void		Initialize ();
static void		Redisplay ();
static void		Destroy ();
static void		Resize ();
static Boolean		SetValues ();
static XtGeometryResult	QueryGeometry ();

static void		Trogle ();


/*
 *	Tabela de traduções & tabela de ações
 */
static char	defaultTranslations[] = "	<Btn1Down>	: TrogleCompression()";

static XtActionsRec	actions[] =	{	{"TrogleCompression", Trogle},
					};


/****************************************************************
 *	Full class record constant				*
 ****************************************************************
 */
TMoreOrLessClassRec tMoreOrLessClassRec =
{
	{
		/* core_class fields */

		(WidgetClass) &coreClassRec,	/* superclass		*/
		"tMoreOrLess",			/* class_name		*/
		sizeof (TMoreOrLessRec),	/* widget_size		*/
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
		/* tMoreOrLess class fields */
		0,				/* empty		*/
	}
};

WidgetClass tMoreOrLessWidgetClass = (WidgetClass)&tMoreOrLessClassRec;

/*
 ****************************************************************
 *								*
 *	Definições das funções de método			*
 *								*
 ****************************************************************
 */
static void
Initialize (Widget request, Widget new)
{
	XGCValues		values;
	XtGCMask		mask;
	TMoreOrLessWidget	newmol = (TMoreOrLessWidget) new;

	/*
	 *	Inicialisa o GC
	 */
	mask = GCForeground | GCLineStyle;

	values.foreground = 1;
	values.line_style = LineSolid;

	newmol->tMoreOrLess.gc = XtGetGC ((Widget) newmol, mask, &values);


	if (newmol->core.width < 7)
		newmol->core.width = 7;

	if (newmol->core.height < 7)
		newmol->core.height = 7;

}

static void
Redisplay (Widget w, XExposeEvent *event)
{
	TMoreOrLessWidget	mol = (TMoreOrLessWidget) w;
	int			x, y, s;

	s = 5;
	x = (mol->core.width - s) / 2;
	y = (mol->core.height - s) / 2;

	XDrawLine
	(	XtDisplay (mol), mol->core.window, mol->tMoreOrLess.gc,
		x, mol->core.height / 2,
		x + s - 1, mol->core.height / 2
	);

	if (mol->tMoreOrLess.compressed)
	{
		XDrawLine
		(	XtDisplay (mol), mol->core.window, mol->tMoreOrLess.gc,
			mol->core.width / 2, y,
			mol->core.width / 2, y + s - 1
		);
	}
}

static void
Destroy (Widget w)
{
	TMoreOrLessWidget	mol = (TMoreOrLessWidget) w;

	if (mol->tMoreOrLess.gc)
		XFreeGC (XtDisplay (mol), mol->tMoreOrLess.gc);

}

static void
Resize (Widget w)
{
	TMoreOrLessWidget	mol = (TMoreOrLessWidget) w;

	if (XtIsRealized ((Widget) mol))
		XClearArea (XtDisplay (mol), mol->core.window, 0, 0, 0, 0, True);
}

static Boolean
SetValues (Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
	TMoreOrLessWidget	tMOLcurrent = (TMoreOrLessWidget) current;
	TMoreOrLessWidget	tMOLnew = (TMoreOrLessWidget) new;

	if (tMOLcurrent->tMoreOrLess.compressed != tMOLnew->tMoreOrLess.compressed)
	{
		if (tMOLnew->tMoreOrLess.compressed)
			XtCallCallbacks ((Widget) tMOLnew, XtNless, NULL);
		else
			XtCallCallbacks ((Widget) tMOLnew, XtNmore, NULL);

		return (True);
	}

	return (False);
}

static XtGeometryResult
QueryGeometry (Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply_return)
{
	XtGeometryResult	result = XtGeometryYes;

	if (request->request_mode == 0)
		return (XtGeometryYes);

	if (request->request_mode & CWHeight)
	{
		if (request->height < 7)
		{
			result = XtGeometryAlmost;
			reply_return->height = 7;
			reply_return->request_mode &= CWHeight;
		}
		else
			result = XtGeometryYes;
	}

	if (request->request_mode & CWWidth)
	{
		if (request->width < 7)
		{
			result = XtGeometryAlmost;
			reply_return->width = 7;
			reply_return->request_mode &= CWWidth;
		}
		else
			result = XtGeometryYes;
	}

	return (result);

}	/* end QueryGeometry */

static void
Trogle (Widget w, XEvent *xevent)
{
	TMoreOrLessWidget	mol = (TMoreOrLessWidget) w;

	if (mol->tMoreOrLess.compressed)
		XtVaSetValues ((Widget) mol, XtNcompressed, False, NULL);
	else
		XtVaSetValues ((Widget) mol, XtNcompressed, True, NULL);
}
