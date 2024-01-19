/*
 ****************************************************************
 *								*
 *			FsBrowser.h				*
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

#ifndef _FsBrowser_h
#define _FsBrowser_h

#include <X11/Core.h>

/*
 *	Valores dos tipos
 */
enum tp	{	T_INVALIDO,
		T_ESP_CARACTERES,
		T_ESP_BLOCOS,
		T_FIFO,
		T_DIRETORIO,
		T_REGULAR,
		T_EXECUTAVEL
	};

#define EXIB_ICONES	0
#define EXIB_LISTA	1
#define EXIB_DETALHES	2

/*
 *	Estruturas públicas
 */
typedef struct
{
	char		nome[15];
	enum tp		tipo;
	off_t		tamanho;	/* = sp->st_size	*/
	long		modo;		/* modo do arquivo = sp->st_mode	*/
	ushort		nlinks;		/* quantidade de links = sp->st_nlink	*/
	ushort		uid, gid;	/* = sp->st_uid, sp->st_gid	*/
	time_t		atime, mtime, ctime;
	/* Tempos de acesso, modificação e criação = sp->st_atime, sp->st_mtime, sp->st_ctime	*/
	int		largetiq;	/* largura da etiqueta de texto */

} DESCRICAO;

/*
 *	Constantes XtN, XtC and XtR usadas pelos recursos
 */ 
#define XtNpath			"path"
#define XtNcdNotify		"cdNotify"
#define XtNcur_x		"cur_x"
#define XtNcur_y		"cur_y"
#define XtNminwidth		"minwidth"
#define XtNminheight		"minheight"
#define XtNmodoexib		"modoexib"
#define XtNnumarqs		"numarqs"
#define XtNfocuson		"focuson"
#define XtNitenslinha		"itenslinha"
#define XtNespvert		"espvert"
#define XtNesphor		"esphor"

#define XtCPath			"Path"
#define XtCCdNotify		"CdNotify"
#define XtCCur_x		"Cur_x"
#define XtCCur_y		"Cur_y"
#define XtCFocuson		"Focuson"
#define XtCModoexib		"Modoexib"

/*
 *	Declarações de funções públicas
 */

/*
 *	Constantes da "Class record"
 */
extern WidgetClass			fsBrowserWidgetClass;

typedef struct _FsBrowserClassRec	*FsBrowserWidgetClass;
typedef struct _FsBrowserRec		*FsBrowserWidget;

#endif /* _FsBrowser_h */
