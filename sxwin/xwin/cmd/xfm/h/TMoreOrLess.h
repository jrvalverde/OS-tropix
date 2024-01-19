/*
 ****************************************************************
 *								*
 *			TMoreOrLess.h				*
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
#ifndef _TMoreOrLess_h
#define _TMoreOrLess_h

#include <X11/Core.h>

#define XtNcompressed	"compressed"
#define XtNmore		"more"
#define XtNless		"less"

#define XtCCompressed	"Compressed"
#define XtCMore		"More"
#define XtCLess		"Less"


extern WidgetClass		tMoreOrLessWidgetClass;

typedef struct _TMoreOrLessClassRec	*TMoreOrLessWidgetClass;
typedef struct _TMoreOrLessRec		*TMoreOrLessWidget;

#endif /* _TMoreOrLess_h */
