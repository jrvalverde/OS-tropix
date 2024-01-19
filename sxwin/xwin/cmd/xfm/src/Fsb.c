/*
 ****************************************************************
 *								*
 *			Fsb.c					*
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
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Scrollbar.h>

#include "../h/FsbP.h"
#include "../h/FsBrowser.h"

#define elif	else if

/****************************************************************
 *	fsb Resources						*
 ****************************************************************
 */
static XtResource resources[] =
{
	{	XtNhorSpace,
		XtCHorSpace,
		XtRDimension,
		sizeof (Dimension),
		XtOffset (FsbWidget, fsb.h_space),
		XtRImmediate,
		(XtPointer) 2
	},
	{	XtNvertSpace,
		XtCVertSpace,
		XtRDimension,
		sizeof (Dimension),
		XtOffset (FsbWidget, fsb.v_space),
		XtRImmediate,
		(XtPointer) 2
	},
	{
		XtNfsbw,
		XtCReadOnly,
		XtRWidget,
		sizeof (Widget),
		XtOffset (FsbWidget, fsb.fsbw),
		XtRImmediate,
		NULL
	},
};

/****************************************************************
 *	Protótipos das funções					*
 ****************************************************************
 */
static void		Initialize ();
static void		Resize ();
static Boolean		SetValues ();
static void		ChangeManaged ();
static XtGeometryResult	QueryGeometry ();
static XtGeometryResult	GeometryManager ();

static void	DoLayout (FsbWidget);
static void	cdnotify (Widget, XtPointer, XtPointer);

static void	Resize_thumbs (Widget, XEvent *, String *, Cardinal *);
static void	Scroll_up_down (Widget, XtPointer, XtPointer);
static void	Scroll_left_right (Widget, XtPointer, XtPointer);
static void	Thumb_up_down (Widget, XtPointer, XtPointer);
static void	Thumb_left_right (Widget, XtPointer, XtPointer);


/****************************************************************
 *	Full class record constant				*
 ****************************************************************
 */
FsbClassRec fsbClassRec =
{
	{
		/* core_class fields      */
		(WidgetClass) &compositeClassRec,	/* superclass		*/
		"fsb",					/* class_name		*/
		sizeof (FsbRec),			/* widget_size		*/
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
		NULL,					/* destroy		*/
		Resize,					/* resize		*/
		NULL,					/* expose		*/
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
		/* fsb class fields */
		0,					/* empty		*/
	}
};

WidgetClass fsbWidgetClass = (WidgetClass)&fsbClassRec;


static void
DoLayout (FsbWidget fsb)
{
	Widget		main, vscroll, hscroll;
	Dimension	mw, mh, vh, hw;
	Position	vx, hy;
	int		modoexib, esphor, espvert, numarqs;
	int		altura = 1, largura = 1;
	Boolean		vneed, hneed;

	main = fsb->fsb.fsbw;
	vscroll = fsb->fsb.vsbar;
	hscroll = fsb->fsb.hsbar;

	XtVaGetValues
	(	main,
		XtNmodoexib,	&modoexib,
		XtNesphor,	&esphor,
		XtNespvert,	&espvert,
		XtNnumarqs,	&numarqs,
		NULL
	);

	/*
	 *	Tenta primeiro sem as barras
	 */
	mw = fsb->core.width - (2 * fsb->fsb.h_space) - (2 * main->core.border_width);
	mh = fsb->core.height - (2 * fsb->fsb.v_space) - (2 * main->core.border_width);
	vneed = False;
	hneed = False;

	switch (modoexib)
	{
	    case EXIB_ICONES:
		if ((largura = mw / esphor) == 0)
			largura ++;

		altura = ((numarqs - 1) / largura) + 1;

		break;

	    case EXIB_LISTA:
		if ((altura = mh / espvert) == 0)
			altura ++;

		largura = ((numarqs - 1) / altura) + 1;

		break;

	    case EXIB_DETALHES:
		largura = 1;
		altura = numarqs;
	}

	largura *= esphor;
	altura *= espvert;

	if ((largura > mw) && (altura > mh))
	{
		mw = fsb->core.width - (3 * fsb->fsb.h_space) - 
			vscroll->core.width - (2 * vscroll->core.border_width) -
			(2 * main->core.border_width);

		mh = fsb->core.height - (3 * fsb->fsb.v_space) - 
			hscroll->core.height - (2 * hscroll->core.border_width) -
			(2 * main->core.border_width);

		vneed = True;
		hneed = True;
	}
	elif ((largura > mw) || (altura > mh))
	{
		if (largura > mw)
		{
			mh = fsb->core.height - (3 * fsb->fsb.v_space) - 
				hscroll->core.height - (2 * hscroll->core.border_width) -
				(2 * main->core.border_width);

			hneed = True;
		}
		elif (altura > mh)
		{
			mw = fsb->core.width - (3 * fsb->fsb.h_space) - 
				vscroll->core.width - (2 * vscroll->core.border_width) -
				(2 * main->core.border_width);

			vneed = True;
		}

		switch (modoexib)
		{
		    case EXIB_ICONES:
			if ((largura = mw / esphor) == 0)
				largura ++;

			altura = ((numarqs - 1) / largura) + 1;

			break;

		    case EXIB_LISTA:
			if ((altura = mh / espvert) == 0)
				altura ++;

			largura = ((numarqs - 1) / altura) + 1;

			break;

		    case EXIB_DETALHES:
			largura = 1;
			altura = numarqs;
		}

		largura *= esphor;
		altura *= espvert;

		if ((largura > mw) && (altura > mh))
		{
			mw = fsb->core.width - (3 * fsb->fsb.h_space) -
				vscroll->core.width - (2 * vscroll->core.border_width) -
				(2 * main->core.border_width);

			mh = fsb->core.height - (3 * fsb->fsb.v_space) -
				hscroll->core.height - (2 * hscroll->core.border_width) -
				(2 * main->core.border_width);

			vneed = True;
			hneed = True;
		}

	}	/* end elif */

	vx = fsb->fsb.h_space + mw + fsb->fsb.h_space + main->core.border_width + vscroll->core.border_width; 
	hy = fsb->fsb.v_space + mh + fsb->fsb.v_space + main->core.border_width + hscroll->core.border_width; 
	vh = mh;
	hw = mw;

	XtConfigureWidget (main, fsb->fsb.h_space, fsb->fsb.v_space, mw, mh, main->core.border_width);
	XtConfigureWidget (vscroll, vx, fsb->fsb.v_space, vscroll->core.width, vh, vscroll->core.border_width);
	XtConfigureWidget (hscroll, fsb->fsb.h_space, hy, hw, hscroll->core.height, hscroll->core.border_width);

}	/* end DoLayout */

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

static void
cdnotify (Widget w, XtPointer client_data, XtPointer call_data)
{
	FsbWidget	fsb = (FsbWidget) client_data;
	Dimension	width, height;
	int		minwidth, minheight;

	DoLayout (fsb);

	XtVaGetValues
	(	((FsbWidget) (w->core.parent))->fsb.fsbw,
		XtNheight,      &height,
		XtNwidth,       &width,
		XtNminheight,   &minheight,
		XtNminwidth,    &minwidth,
		NULL
	);

	XawScrollbarSetThumb (fsb->fsb.vsbar, 0, (float) height / minheight);
	XawScrollbarSetThumb (fsb->fsb.hsbar, 0, (float) width / minwidth);

}

static XtGeometryResult
QueryGeometry (Widget w, XtWidgetGeometry *request, XtWidgetGeometry *reply_return)
{
	XtGeometryResult	result = XtGeometryYes;

	request->request_mode &= CWWidth | CWHeight;

	if (request->request_mode == 0)
		return (XtGeometryYes);

	if (request->request_mode & CWHeight)
	{
		if (request->height < 32)
		{
			result = XtGeometryAlmost;
			reply_return->height = 32;
			reply_return->request_mode &= CWHeight;
		}
		else
			result = XtGeometryYes;
	}

	if (request->request_mode & CWWidth)
	{
		if (request->width < 64)
		{
			result = XtGeometryAlmost;
			reply_return->width = 64;
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
	FsbWidget	fsb = (FsbWidget) w;

	DoLayout (fsb);
}

static void
ChangeManaged (FsbWidget w)
{
	FsbWidget	fsb = (FsbWidget) w;

	DoLayout (fsb);
}

static void
Initialize (Widget request, Widget new)
{
	FsbWidget		newfsb = (FsbWidget) new;
	static XtActionsRec	window_actions[] =
				{
					{"resize_thumbs", Resize_thumbs}
				};

	XtAppAddActions (XtWidgetToApplicationContext ((Widget) newfsb), window_actions, 1);

	if (newfsb->core.width == 0)
		newfsb->core.width = 300;

	if (newfsb->core.height == 0)
		newfsb->core.height = 300;

	newfsb->fsb.fsbw =	XtCreateManagedWidget
				(	"fsBrowser", fsBrowserWidgetClass,
					(Widget) newfsb, NULL, 0
				);

	XtAddCallback (newfsb->fsb.fsbw, XtNcdNotify, cdnotify, (XtPointer) newfsb);

	newfsb->fsb.vsbar =	XtVaCreateManagedWidget
				(	"scrollVert", scrollbarWidgetClass, (Widget) newfsb,
					XtNorientation,	XtorientVertical,
					XtNheight,	newfsb->fsb.fsbw->core.height,
					XtNwidth,	15,
					NULL
				);

	XtAddCallback (newfsb->fsb.vsbar, XtNscrollProc, Scroll_up_down, newfsb->fsb.fsbw);
	XtAddCallback (newfsb->fsb.vsbar, XtNthumbProc, Thumb_up_down, newfsb->fsb.fsbw);

	XtOverrideTranslations
	(	newfsb->fsb.vsbar,
		XtParseTranslationTable
		(	"<Configure>:	resize_thumbs(v)\n\
			 <Expose>:	resize_thumbs(v)"
		)
	);

	newfsb->fsb.hsbar =	XtVaCreateManagedWidget
				(	"scrollHoriz", scrollbarWidgetClass, (Widget) newfsb,
					XtNorientation,	XtorientHorizontal,
					XtNwidth,	newfsb->fsb.fsbw->core.width,
					XtNheight,	15,
					NULL
				);

	XtAddCallback (newfsb->fsb.hsbar, XtNscrollProc, Scroll_left_right, newfsb->fsb.fsbw);
	XtAddCallback (newfsb->fsb.hsbar, XtNthumbProc, Thumb_left_right, newfsb->fsb.fsbw);

	XtOverrideTranslations
	(	newfsb->fsb.hsbar,
		XtParseTranslationTable
		(	"<Configure>:	resize_thumbs(h)\n\
			 <Expose>:	resize_thumbs(h)"
		)
	);

}	/* end Initialize */

static Boolean
SetValues (Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
{
	FsbWidget	fsbcurrent = (FsbWidget) current;
	FsbWidget	fsbnew = (FsbWidget) new;

	if
	(	(fsbnew->fsb.h_space != fsbcurrent->fsb.h_space) ||
		(fsbnew->fsb.v_space != fsbcurrent->fsb.v_space)
	)
		DoLayout (fsbnew);

	return (False);
}

static void
Resize_thumbs (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	String		*orientation = (String *) params;
	Dimension	width, height;
	int		minwidth, minheight;
	int		cur_x, cur_y;
	Boolean		reposiciona = False;

	XtVaGetValues
	(	((FsbWidget) (w->core.parent))->fsb.fsbw, 
		XtNheight,	&height,
		XtNwidth,	&width,
		XtNminheight,	&minheight,
		XtNminwidth,	&minwidth,
		XtNcur_x,	&cur_x,
		XtNcur_y,	&cur_y,
		NULL
	);

	if (width > minwidth - cur_x)
	{
		cur_x = minwidth - width;
		reposiciona = True;
	}

	if (width >= minwidth)
	{
		cur_x = 0;
		reposiciona = True;
	}

	if (height > minheight - cur_y)
	{
		cur_y = minheight - height;
		reposiciona = True;
	}

	if (height >= minheight)
	{
		cur_y = 0;
		reposiciona = True;
	}

	if (reposiciona)
	{
		XtVaSetValues
		(
			((FsbWidget) (w->core.parent))->fsb.fsbw,
			XtNcur_x,	cur_x,
			XtNcur_y,	cur_y,
			NULL
		);
	}

	if (*orientation[0] == 'h')
		XawScrollbarSetThumb
		(	w,
			(float) cur_x / minwidth,
		        (float) width / minwidth
		);
	else 
		XawScrollbarSetThumb
		(	w,
			(float) cur_y / minheight,
		        (float) height / minheight
		);
}

static void
Scroll_up_down (Widget w, XtPointer client_data, XtPointer call_data)
{
	int		pixels = (int) call_data;
	Dimension	height;
	int		minheight;
	int		cur_y;

	XtVaGetValues
	(	client_data, 
		XtNheight,	&height,
		XtNcur_y,	&cur_y,
		XtNminheight,	&minheight,
		NULL
	);

	cur_y += pixels;

	if (cur_y < 0)
		cur_y = 0;
	elif (cur_y > minheight - height )
		cur_y = minheight - height;

	if (height >= minheight)
		cur_y = 0;

	XtVaSetValues
	(	client_data, 
		XtNcur_y,	cur_y,
		NULL
	);

	XawScrollbarSetThumb
	(	w,
		(float) cur_y / minheight,
		(float) height / minheight
	);
}

static void
Scroll_left_right (Widget w, XtPointer client_data, XtPointer call_data)
{
	int		pixels = (int) call_data;
	Dimension	width;
	int		minwidth;
	int		cur_x;

	XtVaGetValues
	(	client_data, 
		XtNwidth,	&width,
		XtNcur_x,	&cur_x,
		XtNminwidth,	&minwidth,
		NULL
	);

	cur_x += pixels;

	if (cur_x < 0)
		cur_x = 0;
	elif (cur_x > minwidth - width)
		cur_x = minwidth - width;

	if (width >= minwidth)
		cur_x = 0;

	XtVaSetValues
	(	client_data, 
		XtNcur_x,	cur_x,
		NULL
	);

	XawScrollbarSetThumb
	(	w,
		(float) cur_x / minwidth,
		(float) width / minwidth
	);
}

static void
Thumb_up_down (Widget w, XtPointer client_data, XtPointer call_data)
{
	float		percent = *((float *) call_data);
	Dimension	height;
	int		minheight;
	int		cur_y;

	XtVaGetValues
	(	client_data, 
		XtNheight,	&height,
		XtNcur_y,	&cur_y,
		XtNminheight,	&minheight,
		NULL
	);

	cur_y = (int) (minheight * percent);

	if (cur_y >= minheight - height)
		cur_y = minheight - height;

	if (height >= minheight)
		cur_y = 0;

	XtVaSetValues
	(	client_data, 
		XtNcur_y,	cur_y,
		NULL
	);

	XawScrollbarSetThumb
	(	w,
		(float) cur_y / minheight,
		(float) height / minheight
	);
}

static void
Thumb_left_right (Widget w, XtPointer client_data, XtPointer call_data)
{
	float		percent = *((float *) call_data);
	Dimension	width;
	int		minwidth;
	int		cur_x;

	XtVaGetValues
	(	client_data, 
		XtNwidth,	&width,
		XtNcur_x,	&cur_x,
		XtNminwidth,	&minwidth,
		NULL
	);

	cur_x = (int) (minwidth * percent);

	if (cur_x >= minwidth - width)
		cur_x = minwidth - width;

	if (width >= minwidth)
		cur_x = 0;

	XtVaSetValues
	(	client_data, 
		XtNcur_x,	cur_x,
		NULL
	);

	XawScrollbarSetThumb
	(	w,
		(float) cur_x / minwidth,
		(float) width / minwidth
	);
}
