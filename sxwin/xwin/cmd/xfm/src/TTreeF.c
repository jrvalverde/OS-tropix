/*
 ****************************************************************
 *								*
 *			TTreeF.c				*
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Label.h>
#include "../h/TMoreOrLess.h"

#include "../h/TTreeP.h"


#define elif	else if

/*
 *	Declaração das funções públicas
 */
void
FillTreeDir (Widget w, int x, int y, char *path)
{
	TTreeWidget	tree = (TTreeWidget) w;
	Widget		wid, mol;
	int		i = 0;
	int		numdirs;
	DIR		*dir_fp;
	const DIRENT	*dp;
	STAT		st;
	char		pasta[256];
	char		*chr;

	if (!streq (tree->core.widget_class->core_class.class_name, "tTree"))
	{
		error ("A \"Widget\" passada não é do tipo árvore");
		return;
	}


	if ((dir_fp = opendir (path)) == NODIR)
	{
		error ("*Não consegui abrir o diretório \"%s\"", path);
		return;
	}

#if (0)	/*******************************************************/
	if ((fd = open (path, O_RDONLY)) < 0)
        {
		error ("*Não consegui abrir o diretório \"%s\"", path);
		return;
	}
#endif	/*******************************************************/

	numdirs = 0;

#if (0)	/*******************************************************/
	while (read (fd, (void *) &entrada, sizeof (entrada)) > 0)
#endif	/*******************************************************/
	while ((dp = readdir (dir_fp)) != NODIRENT)
	{
		if
		( 	streq (dp->d_name, ".") ||
			streq (dp->d_name, "..")
		)
		{
			continue;
		}

		strcpy (pasta, path);

		if (pasta[strlen (pasta) - 1] != '/')
			strcat (pasta, "/");

		strcat (pasta, dp->d_name);
		stat (pasta, &st);

		if ((st.st_mode & S_IFMT) == S_IFDIR)
			numdirs++;
	}

	rewinddir (dir_fp);

	for (i = 0; i < tree->composite.num_children; i++)
	{
		wid = tree->composite.children[i];

		if (wid->core.y >= y)
		{
			XtConfigureWidget	(	wid,
							wid->core.x, wid->core.y + (numdirs * 18),
							wid->core.width, wid->core.height,
							wid->core.border_width
						);
		}
	}

	while ((dp = readdir (dir_fp)) != NODIRENT)
	{
		DIR		*Dir_fp;
		const DIRENT	*Dp;

		if 
		(	streq (dp->d_name, ".") ||
			streq (dp->d_name, "..")
		)
		{
			continue;
		}

		strcpy (pasta, path);

		if (pasta[strlen (pasta) - 1] != '/')
			strcat (pasta, "/");

		strcat (pasta, dp->d_name);
		numdirs = 0;
		stat (pasta, &st);

		if ((st.st_mode & S_IFMT) != S_IFDIR)
			continue;

		wid = XtVaCreateManagedWidget	(	"entry", labelWidgetClass, (Widget) tree,
							XtNlabel,	dp->d_name,
							XtNborderWidth,	0,
							XtNx,		x + 30,
							XtNy,		y,
							NULL
						);

		if ((Dir_fp = opendir (pasta)) != NODIR)
		{
			while ((Dp = readdir (Dir_fp)) != NODIRENT)
			{
				if
				( 	streq (Dp->d_name, ".") ||
					streq (Dp->d_name, "..")
				)
				{
					continue;
				}

				strcat (pasta, "/");
				strcat (pasta, Dp->d_name);
				stat (pasta, &st);

				chr = strrchr (pasta, '/');
				*chr = '\0';

				if ((st.st_mode & S_IFMT) == S_IFDIR)
					numdirs++;
			}

			closedir (Dir_fp);
		}

		if (numdirs > 0)
		{
			mol =	XtVaCreateManagedWidget
				(	"mol", tMoreOrLessWidgetClass, (Widget) tree,
					XtNx,		x + 15,
					XtNy, 		y + 4,
					XtNwidth,	7,
					XtNheight,	7,
					XtNborderWidth,	1,
					NULL
				);

			XtAddCallback (mol, XtNmore, ExpandTreeDir, (XtPointer) wid);
			XtAddCallback (mol, XtNless, CompressTreeDir, (XtPointer) wid);
		}

		y += 18;

	}	/* end while */

	closedir (dir_fp);

}	/* end FillTreeDir */

/*
 *	Callback padrão para a expansão de árvores de diretórios
 */
void
ExpandTreeDir (Widget mol, XtPointer client_data, XtPointer call_data)
{
	char		pasta[256];
	char		*dir;
	int		i, x, y;
	div_t		result;
	Widget		wid, nopai, ico = (Widget) client_data;
	TTreeWidget	tree = (TTreeWidget) mol->core.parent;

	XtVaGetValues (ico, XtNlabel, &dir, NULL);
	strcpy (pasta, "/");
	strcat (pasta, dir);
	x = ico->core.x;
	y = ico->core.y + ico->core.height + 1;

	while (True)
	{
		for (i = 0, nopai = NULL; i < tree->composite.num_children; i++)
		{
			wid = tree->composite.children[i];
			result = div (wid->core.x, 30);

			if
			(	(wid->core.y < ico->core.y) &&
				(wid->core.x < ico->core.x) &&
				(result.rem == 0)
			)
			{
				if (nopai == NULL)
					nopai = wid;
				elif (wid->core.y > nopai->core.y)
					nopai = wid;
			}
		}

		if (nopai == NULL)
			break;

		XtVaGetValues (nopai, XtNlabel, &dir, NULL);
		ico = nopai;

		if (strlen (dir) > 0)
		{
			strmove (&(pasta[strlen (dir) + 2]), pasta);
			strcpy (pasta, "/");
			strcat (pasta, dir);
			strmove (&(pasta[strlen (dir) + 1]), &(pasta[strlen (dir) + 2]));
		}
	}

	FillTreeDir ((Widget) tree, x, y, pasta);

}	/* end ExpandTreeDir */

void
CompressTreeDir (Widget mol, XtPointer client_data, XtPointer call_data)
{
	int		i, sup, inf;
	Widget		wid, ico = (Widget) client_data;
	TTreeWidget	tree = (TTreeWidget) mol->core.parent;

	inf = 999999;
	sup = ico->core.y + ico->core.height;

	for (i = 0; i < tree->composite.num_children; i++)
	{
		wid = tree->composite.children[i];

		if
		(	(wid->core.x <= ico->core.x) &&
			(wid->core.y >= sup) &&
			(wid->core.y <= inf)
		)
		{
			inf = wid->core.y - 1;
		}
	}

	for (i = 0; i < tree->composite.num_children; i++)
	{
		wid = tree->composite.children[i];

		if (wid->core.y >= sup)
		{
			if (wid->core.y > inf)
			{
				XtConfigureWidget
				(	wid, wid->core.x, wid->core.y - inf + sup,
					wid->core.width, wid->core.height,
					wid->core.border_width
				);
			}
			else
			{
				XtUnrealizeWidget (wid);
			}
		}
	}

}	/* end CompressTreeDir */
