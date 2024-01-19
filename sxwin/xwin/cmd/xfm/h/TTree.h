/*
 ****************************************************************
 *								*
 *			TTree.h					*
 *								*
 *	"Widget" "Widget" de itens arranjados em �rvore		*
 *								*
 *	Vers�o	3.2.2, de 16.06.00				*
 *								*
 *	M�dulo: TTree						*
 *		"Widgets" do Tropix				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 1999 NCE/UFRJ - tecle "man licen�a"	*
 * 								*
 ****************************************************************
 */
#ifndef _TTree_h
#define _TTree_h

#include <X11/Composite.h>

#define XtNprocess	"process"

#define XtCProcess	"Process"

/*
 *	Prot�tipos das fun��es p�blicas
 */
void		FillTreeDir (Widget, int, int, char *);
void		ExpandTreeDir (Widget, XtPointer, XtPointer);
void		CompressTreeDir (Widget, XtPointer, XtPointer);


extern WidgetClass		tTreeWidgetClass;

typedef struct _TTreeClassRec	*TTreeWidgetClass;
typedef struct _TTreeRec	*TTreeWidget;

#endif /* _TTree_h */
