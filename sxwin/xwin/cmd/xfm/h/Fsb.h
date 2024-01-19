/*
 ****************************************************************
 *								*
 *			Fsb.h					*
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
#ifndef _Fsb_h
#define _Fsb_h

#include <X11/Composite.h>

#define XtNfsbw		"fsbw"
#define XtNhorSpace	"horSpace"
#define XtNvertSpace	"vertSpace"

#define XtCHorSpace	"HorSpace"
#define XtCVertSpace	"VertSpace"


extern WidgetClass		fsbWidgetClass;

typedef struct _FsbClassRec	*FsbWidgetClass;
typedef struct _FsbRec		*FsbWidget;

#endif /* _Fsb_h */
