/*
 ****************************************************************
 *								*
 *			FsBrowserP.h				*
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

#ifndef _FsBrowserP_h
#define _FsBrowserP_h

#include <X11/CoreP.h>
#include "FsBrowser.h"

/*
 *	Class Structures
 */
typedef struct
{
	int		make_compiler_happy;

}	FsBrowserClassPart;

typedef struct _FsBrowserClassRec
{
	CoreClassPart		core_class;
	FsBrowserClassPart	fsBrowser_class;

}	FsBrowserClassRec;

extern FsBrowserClassRec	fsBrowserClassRec;

/*
 *	Instance Structures
 */
typedef struct
{
	/* recursos */
	XtCallbackList	cdnotify;
	const char	*path;
	int		modoexib;
	int		cur_x, cur_y;
	int		focuson;

	/* somente leitura */
	int		width, height;
	int		numarq, esphor, espvert;

	/* privado */
	char		caminho[512];
	XFontStruct	*fsp;
	int		altpad;
	GC		gc, gcs;
	DESCRICAO	*descr;
	int		selected[20];
	int		largmax;
	int		itenslinha;

}	FsBrowserPart;

typedef struct _FsBrowserRec
{
	CorePart	core;
	FsBrowserPart	fsBrowser;

}	FsBrowserRec;


#endif	/* _FsBrowserP_h */
