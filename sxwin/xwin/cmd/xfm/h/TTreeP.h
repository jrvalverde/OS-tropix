/*
 ****************************************************************
 *								*
 *			TTreeP.h				*
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

#ifndef _TTreeP_h
#define _TTreeP_h

#include <X11/CompositeP.h>
#include "TTree.h"

/*
 *	Class Structures
 */
typedef struct
{
	int		make_compiler_happy;

}	TTreeClassPart;

typedef struct _TTreeClassRec
{
 	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	TTreeClassPart		tTree_class;

}	TTreeClassRec;

extern TTreeClassRec	tTreeClassRec;

/*
 *	Instance Structures
 */
typedef struct
{
	/* recursos */
	XtCallbackList	process;

	/* privado */
	Dimension	min_width, min_height;
	GC		gc;

}	TTreePart;

typedef struct _TTreeRec
{
	CorePart	core;
	CompositePart	composite;
	TTreePart	tTree;

}	TTreeRec;

#endif	/* _TTreeP_h */
