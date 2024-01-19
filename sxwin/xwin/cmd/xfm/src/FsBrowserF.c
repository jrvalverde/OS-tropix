/*
 ****************************************************************
 *								*
 *			FsBrowserF.c				*
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <sys/types.h>
#include <dirent.h>
#if (0)	/*******************************************************/
#include <sys/dir.h>
#endif	/*******************************************************/
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/a.out.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "../h/FsBrowserP.h"

extern const char	cmpisotb[];
extern const char	cmpfoldtb[];
extern const char	cmpafoldtb[];

#define	elif	else if

/****************************************************************
 *	Definições relativas à ordem de bytes da CPU		*
 ****************************************************************
 */
#ifdef	HEADER_ENDIAN_CV
#define	HEADER_ENDIAN_LONG(x)	long_endian_cv (x)
#define	HEADER_ENDIAN_SHORT(x)	short_endian_cv (x)
#else
#define	HEADER_ENDIAN_LONG(x)	(x)
#define	HEADER_ENDIAN_SHORT(x)	(x)
#endif	HEADER_ENDIAN_CV

/****************************************************************
 *	Declaração das funções privativas			*
 ****************************************************************
 */
void		ChangeDir (FsBrowserWidget, char *);
void		LerDir (FsBrowserWidget);
void		Ordena (DESCRICAO *, int);
void		quick (DESCRICAO *, int, int);
void		AtribuiTipos (DESCRICAO *, int, char *);
void		MudaAparencia (FsBrowserWidget);
void		header_endian_conversion (HEADER *);
int		Select (FsBrowserWidget, int, int);
void		DrawIco (FsBrowserWidget, int);

extern void	PutGif (Display *, Drawable, GC, FILE *,
			int imageNumber, int x, int y);

/*
 ****************************************************************
 *								*
 *	Definições das funcões privativas			*
 * 								*
 ****************************************************************
 */
void
ChangeDir (FsBrowserWidget fsbw, char *camrel)
{
	char		*chr;

	if (streq (camrel, ".."))
	{
		if ((fsbw->fsBrowser.caminho[0] == '\0') || streq (fsbw->fsBrowser.caminho, "/"))
			return;

		if ((chr = strrchr (fsbw->fsBrowser.caminho, '/')) == fsbw->fsBrowser.caminho)
			fsbw->fsBrowser.caminho[1] = '\0';
		elif (chr == NOSTR)
			fsbw->fsBrowser.caminho[0] = '\0';
		else
			*chr = '\0';
	}
	elif (streq (camrel, ""))
	{
		return;
	}
	elif (camrel[0] == '/')
	{
		strcpy (fsbw->fsBrowser.caminho, camrel);
	}
	elif (!streq (camrel, "."))
	{
		if ((fsbw->fsBrowser.caminho[0] != '\0') && !streq (fsbw->fsBrowser.caminho, "/"))
			strcat (fsbw->fsBrowser.caminho, "/");

		strcat (fsbw->fsBrowser.caminho, camrel);
	}

	LerDir (fsbw);
	Ordena (fsbw->fsBrowser.descr, fsbw->fsBrowser.numarq);
	AtribuiTipos (fsbw->fsBrowser.descr, fsbw->fsBrowser.numarq, fsbw->fsBrowser.caminho);

	fsbw->fsBrowser.cur_x = 0;
	fsbw->fsBrowser.cur_y = 0;

	MudaAparencia (fsbw);

	if (XtIsRealized ((Widget) fsbw))
		XClearArea (XtDisplay (fsbw), fsbw->core.window, 0, 0, 0, 0, True);

	XtCallCallbacks ((Widget) fsbw, XtNcdNotify, (XtPointer) fsbw->fsBrowser.path);

}	/* end ChangeDir */

void
MudaAparencia (FsBrowserWidget fsbw)
{
	switch (fsbw->fsBrowser.modoexib)
	{
	    case EXIB_ICONES:
		fsbw->fsBrowser.esphor = 96;
		fsbw->fsBrowser.espvert = 64;
		fsbw->fsBrowser.itenslinha = fsbw->core.width / fsbw->fsBrowser.esphor;

		if (fsbw->fsBrowser.itenslinha == 0)
			fsbw->fsBrowser.itenslinha = 1;

		fsbw->fsBrowser.width = fsbw->fsBrowser.itenslinha * fsbw->fsBrowser.esphor;
		fsbw->fsBrowser.height = ((fsbw->fsBrowser.numarq - 1) / fsbw->fsBrowser.itenslinha) + 1;
		fsbw->fsBrowser.height = fsbw->fsBrowser.height * fsbw->fsBrowser.espvert;
		break;

	    case EXIB_LISTA:
		fsbw->fsBrowser.esphor = 25 + fsbw->fsBrowser.largmax;

		if (fsbw->fsBrowser.esphor < 64)
			fsbw->fsBrowser.esphor = 64;

		fsbw->fsBrowser.espvert =
			(2 + fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent);

		if (fsbw->fsBrowser.espvert < 18)
			fsbw->fsBrowser.espvert = 18;

		fsbw->fsBrowser.itenslinha = fsbw->core.height / fsbw->fsBrowser.espvert;

		if (fsbw->fsBrowser.itenslinha == 0)
			fsbw->fsBrowser.itenslinha = 1;

		fsbw->fsBrowser.width = ((fsbw->fsBrowser.numarq - 1) / fsbw->fsBrowser.itenslinha) + 1;
		fsbw->fsBrowser.width = fsbw->fsBrowser.width * fsbw->fsBrowser.esphor;
		fsbw->fsBrowser.height = fsbw->fsBrowser.itenslinha * fsbw->fsBrowser.espvert;
		break;

	    case EXIB_DETALHES:
		fsbw->fsBrowser.esphor = 694;

		fsbw->fsBrowser.espvert =
			(2 + fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent);

		if (fsbw->fsBrowser.espvert < 18)
			fsbw->fsBrowser.espvert = 18;

		fsbw->fsBrowser.itenslinha = 30000;
		fsbw->fsBrowser.width = fsbw->fsBrowser.esphor;
		fsbw->fsBrowser.height = fsbw->fsBrowser.numarq * fsbw->fsBrowser.espvert;

	}	/* end switch */

}	/* end MudaAparencia */

void
LerDir (FsBrowserWidget tnew)
{
	int		i = 0;
	DIR		*dir_fp;
	const DIRENT	*dp;
	int		direction, ascent, descent;
	XCharStruct	overall;
#if (0)	/*******************************************************/
	struct dir	entrada;
#endif	/*******************************************************/
	DESCRICAO	*det;
	STAT		st;
	char		*chr;

	if (tnew->fsBrowser.descr != NULL)
	{
		free (tnew->fsBrowser.descr);
		tnew->fsBrowser.descr = NULL;
	}

	tnew->fsBrowser.numarq = 0;
	tnew->fsBrowser.largmax = 0;
	tnew->fsBrowser.focuson = 0;
	memset (tnew->fsBrowser.selected, 0, sizeof (int) * 20);

	if ((dir_fp = opendir (tnew->fsBrowser.caminho)) == NODIR)
	{
		error ("*Não consegui abrir o diretório \"%s\"", tnew->fsBrowser.caminho);
		return;
	}

	while ((dp = readdir (dir_fp)) != NODIRENT)
		tnew->fsBrowser.numarq++;

#if (0)	/*******************************************************/
	fd = open (tnew->fsBrowser.caminho, O_RDONLY);

	if (fd < 0)
	{
		error ("*Não consegui abrir o diretório \"%s\"", tnew->fsBrowser.caminho);
		return;
	}

	while (read (fd, (void *) &entrada, sizeof (entrada)) > 0)
		tnew->fsBrowser.numarq++;
#endif	/*******************************************************/

	tnew->fsBrowser.descr = malloc (sizeof (DESCRICAO) * tnew->fsBrowser.numarq);
	det = tnew->fsBrowser.descr;

	rewinddir (dir_fp);
#if (0)	/*******************************************************/
	lseek (fd, 0, L_SET);
#endif	/*******************************************************/

	if (tnew->fsBrowser.descr == NULL)
	{
		error ("FsBrowser: Não consegui obter memória para variáveis internas");
		closedir (dir_fp);
		return;
	}

#if (0)	/*******************************************************/
	while (read (fd, (void *) &entrada, sizeof (entrada)) > 0)
#endif	/*******************************************************/
	while ((dp = readdir (dir_fp)) != NODIRENT)
	{
		if (i >= tnew->fsBrowser.numarq)
			break;

		if (streq (dp->d_name, "."))
			continue;

		strncpy (det->nome, dp->d_name, 14);
		det->nome[14] = '\0';

		if (!streq (tnew->fsBrowser.caminho, "/"))
			strcat (tnew->fsBrowser.caminho, "/");

		strcat (tnew->fsBrowser.caminho, dp->d_name);
		stat (tnew->fsBrowser.caminho, &st);

		if ((chr = strrchr (tnew->fsBrowser.caminho, '/')) == tnew->fsBrowser.caminho)
			chr[1] = '\0';
		else
			chr[0] = '\0';

		det->tamanho	= st.st_size;
		det->modo	= st.st_mode;
		det->nlinks	= st.st_nlink;
		det->uid	= st.st_uid;
		det->gid	= st.st_gid;
		det->atime	= st.st_atime;
		det->mtime	= st.st_mtime;
		det->ctime	= st.st_ctime;

		switch (st.st_mode & S_IFMT)
		{
		    case S_IFREG:
			det->tipo	= T_REGULAR;
			break;

		    case S_IFDIR:
			det->tipo	= T_DIRETORIO;
			break;

		    case S_IFIFO:
			det->tipo	= T_FIFO;
			break;

		    case S_IFBLK:
			det->tipo	= T_ESP_BLOCOS;
			break;

		    case S_IFCHR:
			det->tipo	= T_ESP_CARACTERES;
			break;

		    default:
			det->tipo	= T_INVALIDO;
		}

		if (tnew->fsBrowser.fsp != NULL)
		{
			XTextExtents
			(	tnew->fsBrowser.fsp, det->nome, strlen (det->nome),
				&direction, &ascent, &descent, &overall
			);

			det->largetiq = overall.width;

			if (det->largetiq > tnew->fsBrowser.largmax)
				tnew->fsBrowser.largmax = det->largetiq;
		}

		i++;
		det++;

	}	/* end while (tem entradas no diretório) */

	tnew->fsBrowser.numarq = i;
	closedir (dir_fp);

	return;

}	/* end LerDir */

void
Ordena (DESCRICAO *descri, int numarq)
{
	quick (descri, 0, numarq - 1);
}

void
quick (DESCRICAO *descri, int left, int right)
{
	register int	i, j;
	DESCRICAO	x, y;

	i = left;
	j = right;
	memcpy (&x, &descri[(left + right) / 2], sizeof (DESCRICAO));

	while (i <= j)
	{
		while
		(	(	(descri[i].tipo == T_DIRETORIO && x.tipo != T_DIRETORIO) ||
				( (	(descri[i].tipo == T_DIRETORIO && x.tipo == T_DIRETORIO) ||
					(descri[i].tipo != T_DIRETORIO && x.tipo != T_DIRETORIO)
				  ) &&	strttcmp (descri[i].nome, x.nome, cmpfoldtb, cmpisotb) < 0
				)
			) &&
			i < right
		)
			i++;

		while
		(
			(	(x.tipo == T_DIRETORIO && descri[j].tipo != T_DIRETORIO) ||
				( (	(x.tipo == T_DIRETORIO && descri[j].tipo == T_DIRETORIO) ||
					(x.tipo != T_DIRETORIO && descri[j].tipo != T_DIRETORIO)
				  ) &&	strttcmp (x.nome, descri[j].nome, cmpfoldtb, cmpisotb) < 0
				)
			) &&
			j > left
		)
			j--;

		if (i <= j)
		{
			memcpy (&y, &descri[i], sizeof (DESCRICAO));
			memcpy (&descri[i], &descri[j], sizeof (DESCRICAO));
			memcpy (&descri[j], &y, sizeof (DESCRICAO));
			i++; j--;
		}
	}

	if (left < j)
		quick (descri, left, j);

	if (i < right)
		quick (descri, i, right);

}	/* end quick */

void
AtribuiTipos (DESCRICAO *descri, int numarq, char *path)
{
	register int	fd;
	HEADER		h;
	int		magic;
	char		*chr;

	for (/* vazio */; numarq > 0; numarq--, descri++)
	{
		if (descri->tipo != T_REGULAR)
			continue;

		if ((path[0] != '\0') && !streq (path, "/"))
			strcat (path, "/");

		strcat (path, descri->nome);

		/*
		 *	Abre o arquivo
		 */
		if ((fd = open (path, O_RDONLY)) < 0)
		{
			if ((chr = strrchr (path, '/')) != path)
				chr[0] = '\0';
			else
				chr[1] = '\0';

			continue;
		}

		if ((chr = strrchr (path, '/')) != path)
			chr[0] = '\0';
		else
			chr[1] = '\0';

		/*
		 *	Le o cabeçalho do arquivo
		 */
		if ((read (fd, &h, sizeof (HEADER))) != sizeof (HEADER))
		{
			close (fd);
			continue;
		}

		/*
		 *	Verifica se é um <módulo> objeto
		 */
		magic = HEADER_ENDIAN_SHORT (h.h_magic);

		if   (magic == FMAGIC || magic == NMAGIC || magic == FMAGIC)
		{
#ifdef	HEADER_ENDIAN_CV
			header_endian_conversion (&h);
#endif	HEADER_ENDIAN_CV
		}

		if (h.h_magic == NMAGIC)
		{
			descri->tipo = T_EXECUTAVEL;
		}

		close (fd);

	}	/* end for */

}	/* end AtribuiTipos */

#ifdef	HEADER_ENDIAN_CV
void
header_endian_conversion (register HEADER *hp)
{
	hp->h_machine	= short_endian_cv (hp->h_machine);	
	hp->h_magic	= short_endian_cv (hp->h_magic);
	hp->h_version	= short_endian_cv (hp->h_version);
	hp->h_flags	= short_endian_cv (hp->h_flags);
	hp->h_time	= long_endian_cv  (hp->h_time);	
	hp->h_serial	= long_endian_cv  (hp->h_serial);
	hp->h_tstart	= long_endian_cv  (hp->h_tstart);
	hp->h_dstart	= long_endian_cv  (hp->h_dstart);
	hp->h_entry	= long_endian_cv  (hp->h_entry);
	hp->h_tsize	= long_endian_cv  (hp->h_tsize);
	hp->h_dsize	= long_endian_cv  (hp->h_dsize);
	hp->h_bsize	= long_endian_cv  (hp->h_bsize);
	hp->h_ssize	= long_endian_cv  (hp->h_ssize);
	hp->h_rtsize	= long_endian_cv  (hp->h_rtsize);
	hp->h_rdsize	= long_endian_cv  (hp->h_rdsize);
	hp->h_lnosize	= long_endian_cv  (hp->h_lnosize);
	hp->h_dbsize	= long_endian_cv  (hp->h_dbsize);

}	/* end header_endian_conversion */
#endif	HEADER_ENDIAN_CV

int
Select (FsBrowserWidget fsbw, int indice, int modo)
{
	int		x = 0, y = 0, i = indice / (sizeof (int) * 8);
	unsigned	mask = 1 << (indice - (i * sizeof (int) * 8));

	if (i >= 20)
		return (0);

	if (fsbw->fsBrowser.selected[i] & mask)
	{
		if (modo == 1)
			return (0);

		fsbw->fsBrowser.selected[i] -= mask;
	}
	else
	{
		if (modo == -1)
			return (0);

		fsbw->fsBrowser.selected[i] += mask;
	}

	switch (fsbw->fsBrowser.modoexib)
	{
	    case EXIB_ICONES:
		y = indice / fsbw->fsBrowser.itenslinha;
		x = indice - (y * fsbw->fsBrowser.itenslinha);
		break;

	    case EXIB_LISTA:
		x = indice / fsbw->fsBrowser.itenslinha;
		y = indice - (x * fsbw->fsBrowser.itenslinha);
		break;

	    case EXIB_DETALHES:
		x = 0;
		y = indice;
		break;
	}

	XClearArea
	(	XtDisplay (fsbw), fsbw->core.window,
		(x * fsbw->fsBrowser.esphor) - fsbw->fsBrowser.cur_x,
		(y * fsbw->fsBrowser.espvert) - fsbw->fsBrowser.cur_y,
		fsbw->fsBrowser.esphor - 1, fsbw->fsBrowser.espvert - 1,
		False
	);
	DrawIco (fsbw, indice);

	return (1);

}	/* end Select */

void
DrawIco (FsBrowserWidget fsbw, int indice)
{
	int		icox, icoy;
	int		x, y;
	int		aux;
	div_t		result;
	unsigned	mask;
	FILE		*fp = NOFILE;
	char		string[30];
	DESCRICAO	desc;

	memcpy (&desc, &(fsbw->fsBrowser.descr[indice]), sizeof (DESCRICAO));

	if (!XtIsRealized ((Widget) fsbw))
		return;

	if (indice >= fsbw->fsBrowser.numarq)
		return;

	result = div (indice, fsbw->fsBrowser.itenslinha);

	if (fsbw->fsBrowser.modoexib == EXIB_ICONES)
	{
		icox = result.rem;
		icoy = result.quot;
	}
	else
	{
		icox = result.quot;
		icoy = result.rem;
	}

	aux = indice / (sizeof (int) * 8);
	mask = 1 << (indice - (aux * sizeof (int) * 8));

	if (aux >= 20)
	{
		aux = 0;
		mask = 0;
	}

	switch (desc.tipo)
	{
	    case T_DIRETORIO:
		fp = fopen ("/usr/xwin/gifmaps/xfm.dir.gif", "rb");
		break;

	    case T_EXECUTAVEL:
		fp = fopen ("/usr/xwin/gifmaps/xfm.exe.gif", "rb");
		break;

	    default:
		fp = fopen ("/usr/xwin/gifmaps/xfm.reg.gif", "rb");
		break;
	}

	switch (fsbw->fsBrowser.modoexib)
	{
	    case EXIB_ICONES:
		PutGif
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			fp, 1,
			(icox * fsbw->fsBrowser.esphor) + ((fsbw->fsBrowser.esphor >> 1) - 16) - fsbw->fsBrowser.cur_x,
			(icoy * fsbw->fsBrowser.espvert) + 3 - fsbw->fsBrowser.cur_y
		);

		if (fsbw->fsBrowser.selected[aux] & mask)
		{
			XFillRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gcs,
				(icox * fsbw->fsBrowser.esphor) + ((fsbw->fsBrowser.esphor >> 1) -
					(desc.largetiq >> 1)) - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 38 - fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window,
			fsbw->fsBrowser.gc,
			(icox * fsbw->fsBrowser.esphor) + ((fsbw->fsBrowser.esphor >> 1)
				- (desc.largetiq >> 1)) - fsbw->fsBrowser.cur_x,
			(icoy * fsbw->fsBrowser.espvert) + 38 + fsbw->fsBrowser.fsp->ascent - fsbw->fsBrowser.cur_y,
			desc.nome,
			strlen (desc.nome)
		);

		if (fsbw->fsBrowser.focuson == indice)
		{
			XDrawRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
				(icox * fsbw->fsBrowser.esphor) + ((fsbw->fsBrowser.esphor >> 1)
					- (desc.largetiq >> 1)) - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 38 - fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

		break;

	    case EXIB_LISTA:
		XFillRectangle
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			(icox * fsbw->fsBrowser.esphor) + 2 - fsbw->fsBrowser.cur_x,
			(icoy * fsbw->fsBrowser.espvert) + 2 - fsbw->fsBrowser.cur_y,
			16, 16
		);

		if (fsbw->fsBrowser.selected[aux] & mask)
		{
			XFillRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gcs,
				(icox * fsbw->fsBrowser.esphor) + 21 - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 9 -
					((fsbw->fsBrowser.fsp->ascent +
					fsbw->fsBrowser.fsp->descent) >> 1) -
					fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window,
			fsbw->fsBrowser.gc,
			(icox * fsbw->fsBrowser.esphor) + 21 - fsbw->fsBrowser.cur_x,
			(icoy * fsbw->fsBrowser.espvert) + 9 +
				(fsbw->fsBrowser.fsp->ascent >> 1) - fsbw->fsBrowser.cur_y,
			desc.nome,
			strlen (desc.nome)
		);

		if (fsbw->fsBrowser.focuson == indice)
		{
			XDrawRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
				(icox * fsbw->fsBrowser.esphor) + 21 - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 9 -
					((fsbw->fsBrowser.fsp->ascent +
					fsbw->fsBrowser.fsp->descent) >> 1) -
					fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

		break;

	    case EXIB_DETALHES:
		XFillRectangle
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			2 - fsbw->fsBrowser.cur_x,
			(icoy * fsbw->fsBrowser.espvert) + 2 - fsbw->fsBrowser.cur_y,
			16, 16
		);

		if (fsbw->fsBrowser.selected[aux] & mask)
		{
			XFillRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gcs,
				21 - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 9 -
					((fsbw->fsBrowser.fsp->ascent +
					fsbw->fsBrowser.fsp->descent) >> 1) -
					fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

		x = 21 - fsbw->fsBrowser.cur_x;
		y = (icoy * fsbw->fsBrowser.espvert) + 9 + (fsbw->fsBrowser.fsp->ascent >> 1) - fsbw->fsBrowser.cur_y;

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window,
			fsbw->fsBrowser.gc, x, y,
			desc.nome,
			strlen (desc.nome)
		);

		x += fsbw->fsBrowser.largmax + 5;
		sprintf (string, "%d", desc.tamanho);

		while (strlen (string) < 9)
		{
			char		strcopia[30];

			strcpy (strcopia, string);
			sprintf (string, " %s", strcopia);
		}

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			x, y, string, strlen (string)
		);

		x += 90;
		strcpy (string, (modetostr (desc.modo) + 9));

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			x, y, string, strlen (string)
		);

		x += 130;
		strcpy (string, (char *)btime (&desc.mtime));
		string[24] = '\0';

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			x, y, string, strlen (string)
		);

		x += 210;
		strcpy (string, pwcache (desc.uid));

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			x, y, string, strlen (string)
		);

		x += 90;
		strcpy (string, grcache (desc.gid));

		XDrawString
		(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
			x, y, string, strlen (string)
		);

		if (fsbw->fsBrowser.focuson == indice)
		{
			XDrawRectangle
			(	XtDisplay (fsbw), fsbw->core.window, fsbw->fsBrowser.gc,
				21 - fsbw->fsBrowser.cur_x,
				(icoy * fsbw->fsBrowser.espvert) + 9 -
					((fsbw->fsBrowser.fsp->ascent +
					fsbw->fsBrowser.fsp->descent) >> 1) -
					fsbw->fsBrowser.cur_y,
				desc.largetiq,
				fsbw->fsBrowser.fsp->ascent + fsbw->fsBrowser.fsp->descent
			);
		}

	}	/* end switch */

	if (fp != NOFILE)
		fclose (fp);

}	/* end DrawIco */
