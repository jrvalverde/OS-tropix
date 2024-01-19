/*
 ****************************************************************
 *								*
 *			FsbP.h					*
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

#ifndef _FsbP_h
#define _FsbP_h

#include <X11/CompositeP.h>
#include "Fsb.h"

/*
 *	Class Structures
 */
typedef struct
{
	int		make_compiler_happy;

}	FsbClassPart;

typedef struct _FsbClassRec
{
 	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	FsbClassPart		fsb_class;

}	FsbClassRec;

extern FsbClassRec	fsbClassRec;

/*
 *	Instance Structures
 */
typedef struct
{
	Widget		fsbw, hsbar, vsbar;
	Dimension	h_space, v_space;
	Dimension	preferred_width, preferred_height;
	Dimension	last_query_width, last_query_height;
	XtGeometryMask	last_query_mode;

}	FsbPart;

typedef struct _FsbRec
{
	CorePart	core;
	CompositePart	composite;
	FsbPart		fsb;

}	FsbRec;

#endif	/* _FsbP_h */
