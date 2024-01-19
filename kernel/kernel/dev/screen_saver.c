/*
 ****************************************************************
 *								*
 *			screen_saver.c				*
 *								*
 *	Pequeno protetor de video do PC				*
 *								*
 *	Versão	3.0.0, de 06.09.96				*
 *		3.0.0, de 09.09.96				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/scb.h"

#include "../h/tty.h"
#include "../h/video.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Variáveis e Definições Globais				*
 ****************************************************************
 */
#define	PAT_LETTER	6
#define	PAT_LINE	8
#define	PAT_SPACE	2
#define	PAT_COL		(8 + PAT_SPACE)
#define	PAT_COLOR	7		/* Cor inicial: Branca */
#define C		PAT_COLOR

entry int	screen_saver_line,	/* Linha de início no momento */
		screen_saver_col;	/* Coluna de início no momento */
entry char	screen_saver_down,	/* Indica o sentido vertical */
		screen_saver_right;	/* Indica o sentido horizontal */

/*
 ******	Tabela contendo o padrão "TROPIX" ***********************
 */
entry char	screen_saver_table[PAT_LINE][2*PAT_LETTER*PAT_COL] =
{
/* 0 */	'*', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	'*', C,	'*', C,		' ', C, ' ', C,
 	'*', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,

/* 1 */	'*', C,	'*', C,	' ', C,	'*', C,		'*', C,	' ', C,	'*', C,	'*', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,

/* 2 */	'*', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	'*', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,

/* 4 */	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	'*', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,

/* 5 */	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	'*', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,

/* 7 */	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,

/* 8 */	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	' ', C,		' ', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	' ', C,	'*', C,		'*', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,

/* 9 */	' ', C,	' ', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	'*', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C,
	' ', C,	'*', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	'*', C,	'*', C,		' ', C,	' ', C,	' ', C,	' ', C,		' ', C, ' ', C,
	' ', C,	' ', C,	'*', C,	'*', C,		'*', C,	'*', C,	' ', C,	' ', C,		' ', C, ' ', C,
	'*', C,	'*', C,	' ', C,	' ', C,		' ', C,	'*', C,	'*', C,	' ', C,		' ', C, ' ', C
};

/*
 ****************************************************************
 *	Liga o "screen_saver"					*
 ****************************************************************
 */
void
screen_saver_on (void)
{
	register int	i;

	for (i = 0; i < PAT_LINE; i++)
	{
		memmove
		(	VIDEOADDR + COL*i,
			screen_saver_table[i],
			2*PAT_LETTER*PAT_COL
		);
	}

	screen_saver_line = 0; 	screen_saver_col = 0;
	screen_saver_down = 1; 	screen_saver_right = 1;

}	/* end screen_saver_on */

/*
 ****************************************************************
 *	Avança o "screen_saver"					*
 ****************************************************************
 */
void
screen_saver_inc (void)
{
	register char		*cp, *end_cp;
	int			i, position;
	char			new_color;

	position = screen_saver_line * COL + screen_saver_col;

	/* Apaga o desenho anterior */

	for (i = 0; i < PAT_LINE; i++)
	{
		memsetw
		(	VIDEOADDR + position + COL*i,
			' ',
			PAT_LETTER*PAT_COL
		);
	}

	/* Troca a cor (1 a 7) */

	if ((new_color = screen_saver_table[0][1]) >= 7)
		new_color = 1;
	else
		new_color++;

	end_cp = &screen_saver_table[PAT_LINE-1][2*PAT_LETTER*PAT_COL];

	for (cp = screen_saver_table[0] + 1; cp < end_cp; cp += 2)
		*cp = new_color;

	/* Avança a posição */

	if (screen_saver_down)
	{
		if (screen_saver_line + PAT_LINE >= LINE )
			{ screen_saver_down = 0; screen_saver_line--; }
		else
			screen_saver_line++;
	}
	else	/* screen_saver_up */
	{
		if (screen_saver_line <= 0)
			{ screen_saver_down = 1; screen_saver_line++; }
		else
			screen_saver_line--;
	}

	if (screen_saver_right)
	{
		if (screen_saver_col + PAT_LETTER*PAT_COL - PAT_SPACE >= COL)
			{ screen_saver_right = 0; screen_saver_col--; }
		else
			screen_saver_col++;
	}
	else	/* screen_saver_left */
	{
		if (screen_saver_col <= 0)
			{ screen_saver_right = 1; screen_saver_col++; }
		else
			screen_saver_col--;
	}

	position = screen_saver_line * COL + screen_saver_col;

	for (i = 0; i < PAT_LINE; i++)
	{
		memmove
		(	VIDEOADDR + position + COL*i,
			screen_saver_table[i],
			2 * (PAT_LETTER*PAT_COL - PAT_SPACE)
		);
	}

}	/* end screen_saver_inc */
