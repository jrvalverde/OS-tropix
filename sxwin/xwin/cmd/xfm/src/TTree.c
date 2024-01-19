/*
 ****************************************************************
 *								*
 *			TTree.c					*
 *								*
 *	"Widget" de itens arranjados em árvore			*
 *								*
 *	Versão	3.2.2, de 16.06.00				*
 *								*
 *	Módulo: TTree						*
 *		"Widgets" do Tropix				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "../h/TTreeP.h"

#define elif	else if

/****************************************************************
 *	TTree Resources						*
 ****************************************************************
 */
static XtResource resources[] =
{
	{
		XtNprocess,
		XtCProcess,
		XtRCallback,
		sizeof (XtPointer),
		XtOffset (TTreeWidget, tTree.process),
		XtRCallback,
		NULL
	},
};

/****************************************************************
 *	Protótipos das funções					*
 ****************************************************************
 */
static void		Initialize ();
static void		Redisplay ();
static void		Destroy ();
static void		Resize ();
static Boolean		SetValues ();
static void		ChangeManaged ();
static XtGeometryResult	QueryGeometry ();
static XtGeometryResult	GeometryManager ();

/****************************************************************
 *	Full class record constant				*
 ****************************************************************
 */
TTreeClassRec tTreeClassRec =
{
	{
		/* core_class fields      */
		(WidgetClass) &compositeClassRec,	/* superclass		*/
		"tTree",				/* class_name		*/
		sizeof (TTreeRec),			/* widget_size		*/
		NULL,					/* class_initialize	*/
		NULL,					/* classpart_init	*/
		FALSE,					/* class_inited		*/
		Initialize,				/* initialize		*/
		NULL,					/* initialize_hook	*/
		XtInheritRealize,			/* realize		*/
		NULL,					/* actions		*/
		0,					/* num_actions		*/
		resources,				/* resources		*/
		XtNumber (resources),			/* num_resources	*/
		NULLQUARK,				/* xrm_class		*/
		TRUE,					/* compress_motion	*/
		TRUE,					/* compress_exposure	*/
		TRUE,					/* compress_enterleave	*/
		FALSE,					/* visible_interest	*/
		Destroy,				/* destroy		*/
		Resize,					/* resize		*/
		Redisplay,				/* expose		*/
		SetValues,				/* set_values		*/
		NULL,					/* set_values_hook	*/
		XtInheritSetValuesAlmost,		/* set_values_almost	*/
		NULL,					/* get_values_hook	*/
		NULL,					/* accept_focus		*/
		XtVersion,				/* version		*/
		NULL,					/* callback_private	*/
		NULL,					/* tm_table		*/
		QueryGeometry,				/* query_geometry	*/
		XtInheritDisplayAccelerator,		/* display_accelerator	*/
		NULL					/* extension		*/
	},
	{
		/* composite_class fields */
		GeometryManager,			/* geometry_manager	*/
		ChangeManaged,				/* change_managed	*/
		XtInheritInsertChild,			/* insert_child		*/
		XtInheritDeleteChild,			/* delete_child		*/
		NULL					/* extension		*/
	},
	{
		/* tTree class fields */
		0,					/* empty		*/
	}
};

WidgetClass tTreeWidgetClass = (WidgetClass)&tTreeClassRec;


static XtGeometryResult
GeometryManager (Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply)
{
	XtWidgetGeometry	allowed;

	if (request->request_mode & ~(XtCWQueryOnly | CWWidth | CWHeight))
		return (XtGeometryNo);

	if (request->request_mode & CWWidth)
		allowed.width = request->width;
	else
		allowed.width = w->core.width;

	if (request->request_mode & CWHeight)
		allowed.height = request->height;
	else
		allowed.height = w->core.height;

	if ((allowed.width == w->core.width) && (allowed.height == w->core.height))
		return (XtGeometryNo);

	return (XtGeometryYes);
}

static XtGeometryResult
QueryGeometry (Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply_return)
{
	XtGeometryResult	result = XtGeometryYes;
	TTreeWidget		tree = (TTreeWidget) w;
	int			i = 0, min_width = 0, min_height = 0;
	Widget			wid;

	if (request->request_mode == 0)
		return (XtGeometryYes);

	while (i < tree->composite.num_children)
	{
		wid = tree->composite.children[i];

		if (min_height < (wid->core.height + wid->core.y))
			min_height = (wid->core.height + wid->core.y);

		if (min_width < (wid->core.width + wid->core.x))
			min_width = (wid->core.width + wid->core.x);

		i++;
	}

	if (request->request_mode & CWHeight)
	{
		if (request->height < min_height)
		{
			result = XtGeometryAlmost;
			reply_return->height = min_height;
			reply_return->request_mode &= CWHeight;
		}
		else
			result = XtGeometryYes;
	}

	if (request->request_mode & CWWidth)
	{
		if (request->width < min_width)
		{
			result = XtGeometryAlmost;
			reply_return->width = min_width;
			reply_return->request_mode &= CWWidth;
		}
		else
			result = XtGeometryYes;
	}

	return (result);
}

static void
Resize (Widget w)
{
	TTreeWidget	tree = (TTreeWidget) w;

}

static void
ChangeManaged (Widget w)
{
	TTreeWidget	tree = (TTreeWidget) w;

}

static void
Initialize (Widget request, Widget new)
{
	XGCValues		values;
        XtGCMask		mask;
	TTreeWidget		newtree = (TTreeWidget) new;

	if (newtree->core.width <= 0)
		newtree->core.width = 50;

	if (newtree->core.height <= 0)
		newtree->core.height = 50;

	/*
	 *	Inicialisa o GC
	 */
	mask = GCForeground | GCDashOffset | GCDashList | GCLineStyle;

	values.foreground = 1;
	values.dashes = 1;
	values.dash_offset = 0;
	values.line_style = LineOnOffDash;

	newtree->tTree.gc = XtGetGC ((Widget) newtree, mask, &values);

}	/* end Initialize */

static void
Destroy (Widget w)
{
	TTreeWidget	tree = (TTreeWidget) w;

	if (tree->tTree.gc)
		XFreeGC (XtDisplay (tree), tree->tTree.gc);

}

static void
Redisplay (Widget w, XExposeEvent *event)
{
	TTreeWidget	tree = (TTreeWidget) w;
	int		i = 0;
	Widget		wid = NULL;
	int		y = 0;
	div_t		result;

	XClearArea (XtDisplay (tree), tree->core.window, 0, 0, 0, 0, False);

	for (i = 0; i < tree->composite.num_children; i++)
	{
		wid = tree->composite.children[i];
		result = div (wid->core.x, 30);

		if (!XtIsRealized (wid))
		{
/*
			XtDestroyWidget (wid);
*/
		}
		elif (result.rem == 0)
		{
			XDrawLine
			(	XtDisplay (tree), tree->core.window, tree->tTree.gc,
				wid->core.x - 22,	wid->core.y + 8,
				wid->core.x,		wid->core.y + 8
			);
		}
	}
}

static Boolean
SetValues (Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
	TTreeWidget	tTreecurrent = (TTreeWidget) current;
	TTreeWidget	tTreenew = (TTreeWidget) new;

	return (False);
}
