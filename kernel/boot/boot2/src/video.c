/*
 ****************************************************************
 *								*
 *			video.c					*
 *								*
 *	Pequeno "driver" para o video do PC			*
 *								*
 *	Versão	3.0.0, de 26.06.94				*
 *		3.2.0, de 12.06.99				*
 *								*
 *	Módulo: Boot2						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include <common.h>
#include <bcb.h>

#include "../h/common.h"
#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Definições globais					*
 ****************************************************************
 */
#define	VIDEOADDR	((ushort *)(736 * KBSZ))

/*
 ****** Parâmetros do video *************************************
 */
#define LINE		24
#define COL 		80
#define TAB		8	/* COL deve ser múltiplo de TAB */

/*
 ****** Tabela das cores ****************************************
 *
 *	0 == Black		Preto
 *	1 == Blue		Azul escuro
 *	2 == Green		Verde
 *	3 == Cyan		Azul claro
 *	4 == Red		Vermelho
 *	5 == Magenta		Violeta
 *	6 == Brown		Laranja
 *	7 == White		Branco
 *	8 == Gray
 *	9 == Bright blue
 *	A == Bright green
 *	B == Bright cyan
 *	C == Bright red
 *	D == Bright magenta
 *	E == Yellow
 *	F == Bright white
 */

/*
 ****** Variáveis ***********************************************
 */
extern long		video_pos;

#define	BACK_COLOR	0x0000		/* Preto */
#define	DISPLAY_COLOR	0x0700		/* white */

#define	BLANK		 (BACK_COLOR|DISPLAY_COLOR|' ')
#define	BLACK		 (0	    |0		  |' ')

entry long	video_cor  = (BACK_COLOR|DISPLAY_COLOR);

entry int	video_line = 0;		/* Para controlar a tela cheia */

enum  {	DMESG_SZ = 4 * KBSZ };

entry char	dmesg_area[DMESG_SZ];	/* Área de DMESG */

/*
 ******	Protótipos de funções ***********************************
 */
void		swap_color (ushort *);

/*
 ****************************************************************
 *	Inicializa toda a tela com a cor de fundo		*
 ****************************************************************
 */
void
init_console (void)
{
	/*
	 *	Dá dois scrolls e limpa as últimas duas linhas (23 e 24)
	 */
	memmove (&VIDEOADDR[0], &VIDEOADDR[2*COL], 2*(LINE-1)*COL);
	memsetw (&VIDEOADDR[(LINE-1)*COL], BLANK, 2*COL);

	video_pos -= 2*COL;

	/*
	 *	Prepara a área de DMESG
	 */
	bcb.y_dmesg_area =
	bcb.y_dmesg_ptr  = dmesg_area;
	bcb.y_dmesg_end  = dmesg_area + DMESG_SZ;

}	/* end init_console */

/*
 ****************************************************************
 *	Escreve um caractere no video, na posição corrente	*
 ****************************************************************
 */
void
putchar (int c)
{
	enum { V_REG, V_ESC, V_ARG };
	static int	video_state = V_REG;
	static int	video_arg = 0;

	/*
	 *	Guarda o caractere
	 */
	if (bcb.y_dmesg_ptr < bcb.y_dmesg_end)
		*bcb.y_dmesg_ptr++ = c;

	/*
	 *	Examina o protocolo VT100 de cores
	 */
	switch (video_state)
	{
	    case V_REG:
		break;

	    case V_ESC:
		if (c == '[')
		{
			video_state = V_ARG;
			video_arg   = 0;
			return;
		}

		video_state = V_REG;
		break;

	    case V_ARG:
		if   (c >= '0' && c <= '9')
		{
			video_arg *= 10;
			video_arg += c - '0';
			return;
		}

		video_state = V_REG;

		if (c == 'm')
		{
			if (video_arg == 0)
				video_cor = (BACK_COLOR|DISPLAY_COLOR);
			else
				video_cor = (BACK_COLOR|((video_arg - 30) << 8));
			return;
		}

		break;
	}

#undef	WAIT_FOR_CTL_Q
#ifdef	WAIT_FOR_CTL_Q
	/*
	 *	Verifica se encheu a tela: repare que
	 *	"wait_for_ctl_Q ()" zera "video_line"
	 */
	if (video_line >= LINE - 3 && !time_out)
		wait_for_ctl_Q ();
#endif	WAIT_FOR_CTL_Q

	/*
	 *	Se o caractere for gráfico, imprime e termina
	 */
	if (c >= ' ')
	{
		VIDEOADDR[video_pos] = video_cor|c;
		video_pos++;
		goto set_cursor;
	}

	/*
	 *	O caractere não é gráfico
	 */
	switch (c)
	{
	    case '\n':		/* Na verdade <nl> + <cr> */
		swap_color (&VIDEOADDR[video_pos]);	/* Apaga o cursor */
		video_pos = (video_pos / COL * COL) + COL;
		break;

	    case '\r':
		swap_color (&VIDEOADDR[video_pos]);	/* Apaga o cursor */
		video_pos = video_pos / COL * COL;
		break;

	    case '\t':
		swap_color (&VIDEOADDR[video_pos]);	/* Apaga o cursor */
		video_pos = (video_pos + TAB) / TAB * TAB;
		break;

	    case '\b':
		if (video_pos <= 0)
			return;
		swap_color (&VIDEOADDR[video_pos]);	/* Apaga o cursor */
		video_pos--;
		break;

	    case '\f':
		memsetw (&VIDEOADDR[0], BLANK, LINE * COL);
		video_pos = 0;
		break;

	    case '\e':
		video_state = V_ESC;
		return;

	    default:				/* Ignora os outros caracteres */
		return;

	}	/* end switch */
	
	/*
	 *	Dá scrolls, se necessário
	 */
    set_cursor:
	while (video_pos >= LINE * COL)
	{
		memmove (&VIDEOADDR[0], &VIDEOADDR[COL], 2 * (LINE - 1) * COL);
		memsetw (&VIDEOADDR[(LINE - 1) * COL], BLANK, COL);

		video_pos -= COL;

		video_line++;
	}

	/*
	 *	Põe o cursor
	 */
	swap_color (&VIDEOADDR[video_pos]);

}	/* end putchar */

/*
 ****************************************************************
 *	Liga/desliga o cursor invertendo as cores		*
 ****************************************************************
 */
void
swap_color (ushort *vp)
{
	char		*cp;
	char		c;
  
	cp = (char *)vp + 1;	c = *cp;

	*cp = ((c & 0xF0) >> 4) | ((c & 0x0F) << 4);

}	/* end swap_color */
