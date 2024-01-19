/*
 ****************************************************************
 *								*
 *			TMoreOrLessP.h				*
 *								*
 *	"Widget" para expans�o e contra��o por callbacks	*
 *								*
 *	Vers�o	3.2.3, de 28.06.00				*
 *								*
 *	M�dulo: TMoreOrLess					*
 *		"Widgets" do Tropix				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 1999 NCE/UFRJ - tecle "man licen�a"	*
 * 								*
 ****************************************************************
 */

#ifndef _TMoreOrLessP_h
#define _TMoreOrLessP_h

#include <X11/CoreP.h>
#include "TMoreOrLess.h"

/*
 *	Class Structures
 */
typedef struct
{
	int		make_compiler_happy;

}	TMoreOrLessClassPart;

typedef struct _TMoreOrLessClassRec
{
 	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	TMoreOrLessClassPart	tMoreOrLess_class;

}	TMoreOrLessClassRec;

extern TMoreOrLessClassRec	tMoreOrLessClassRec;

/*
 *	Instance Structures
 */
typedef struct
{
	GC		gc;
	Boolean		compressed;
	XtCallbackList	more, less;

}	TMoreOrLessPart;

typedef struct _TMoreOrLessRec
{
	CorePart	core;
	CompositePart	composite;
	TMoreOrLessPart	tMoreOrLess;

}	TMoreOrLessRec;

#endif	/* _TMoreOrLessP_h */
