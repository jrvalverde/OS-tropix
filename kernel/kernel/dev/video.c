/*
 ****************************************************************
 *								*
 *			video.c					*
 *								*
 *	Pequeno "driver" para o video				*
 *								*
 *	Versão	3.0.0, de 26.06.94				*
 *		4.3.0, de 06.09.02				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2002 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/scb.h"
#include "../h/sync.h"

#include "../h/cpu.h"
#include "../h/video.h"
#include "../h/timeout.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Definições globais					*
 ****************************************************************
 */
entry int	video_beep_period;		/* Período em ação */

entry int	video_line = 0;			/* Para controlar a tela cheia */

/*
 ******	Protótipos de funções ***********************************
 */
void		video_beep (int, int);
void		video_beep_stop (int);

/*
 ****************************************************************
 *	Inicializa os protocolos do video			*
 ****************************************************************
 */
void
init_video (void)
{
	/* A inicialização agora está em "con.c" */

}	/* end init_video */

/*
 ****************************************************************
 *	Escreve um caractere no video, na posição corrente	*
 ****************************************************************
 */
void
writechar (int c)
{
	VT100		*vtp = video_vt100;

	/*
	 *	Verifica se está no meio do protocolo "vt100"
	 */
	if (vtp->vt100_state != VT100_NORMAL)
		{ video_vt100_protocol (c); return; }

	/*
	 *	Verifica se encheu a tela: repare que
	 *	"wait_for_ctl_Q ()" zera "video_line"
	 */
	if (scb.y_boot_verbose && video_line >= LINE - 1 && !scb.y_std_boot)
		wait_for_ctl_Q ();

	/*
	 *	Se o caractere for gráfico, imprime e termina
	 */
	if (c >= ' ')
	{
	   /***	video_cursor_off (vtp->vt100_pos); ***/
		VIDEOADDR[vtp->vt100_pos] = vtp->vt100_char_color|c;
		vtp->vt100_pos++;
		goto set_cursor;
	}

	/*
	 *	O caractere não é gráfico
	 */
	switch (c)
	{
	    case '\a':
		video_beep (1193180 / 880, scb.y_hz >> 2);
		return;

	    case '\e':
		video_vt100_protocol (c);
		return;

	    case '\n':
		video_cursor_off (vtp->vt100_pos);
		vtp->vt100_pos += COL; 
		break;

	    case '\r':
		video_cursor_off (vtp->vt100_pos);
		vtp->vt100_pos = vtp->vt100_pos / COL * COL;
		break;

	    case '\t':
		video_cursor_off (vtp->vt100_pos);
		vtp->vt100_pos = (vtp->vt100_pos + TAB) / TAB * TAB;
		break;

	    case '\b':
		if (vtp->vt100_pos <= 0)
			return;
		video_cursor_off (vtp->vt100_pos);
		vtp->vt100_pos--;
		break;

	    case '\f':
	   /***	video_cursor_off (vtp->vt100_pos); ***/
		memsetw (&VIDEOADDR[0], vtp->vt100_char_color|' ', LINE * COL);
		vtp->vt100_pos = 0;
		break;

	    default:				/* Ignora os outros caracteres */
		return;

	}	/* end switch */
	
	/*
	 *	Dá scrolls, se necessário
	 */
    set_cursor:
	while (vtp->vt100_pos >= LINE * COL)
	{
		memmove
		(	&VIDEOADDR[vtp->vt100_scroll_begin * COL],
			&VIDEOADDR[vtp->vt100_scroll_begin * COL + COL],
			2 * (vtp->vt100_scroll_end - vtp->vt100_scroll_begin) * COL
		);

		memsetw
		(	&VIDEOADDR[vtp->vt100_scroll_end * COL],
			vtp->vt100_normal_char_color|' ',
			COL
		);

		vtp->vt100_pos -= COL; video_line++;
	}

	/*
	 *	Põe o cursor
	 */
	video_cursor_on (vtp->vt100_pos);

}	/* end writechar */

/*
 ****************************************************************
 *	Limpa o início da última linha (que eventualmente suja)	*
 ****************************************************************
 */
void
video_clr_last_line (void)
{
	memsetw
	(	&VIDEOADDR[LINE * COL],
		' ',
		40 /* Lembrar de "Tela" */
	);

}	/* end video_clr_last_line */

/*
 ****************************************************************
 *	Liga o cursor						*
 ****************************************************************
 */
void
video_cursor_on (int pos)
{
	char		*cp;
	char		color;

	cp = (char *)&VIDEOADDR[pos] + 1;

	color = *cp;

	*cp = (color & 0x88) | ((color & 0x70) >> 4) | ((color & 0x07) << 4);

}	/* end video_cursor_on */

#if (0)	/*******************************************************/
	cp = (char *)&VIDEOADDR[pos] + 1;
  
	color = video_vt100->vt100_char_color >> 8;

	*cp = (color & 0x88) | ((color & 0x70) >> 4) | ((color & 0x07) << 4);
#endif	/*******************************************************/

/*
 ****************************************************************
 *	Desliga o cursor					*
 ****************************************************************
 */
void
video_cursor_off (int pos)
{
	char		*cp;
	char		color;

	cp = (char *)&VIDEOADDR[pos] + 1;

	color = *cp;

	*cp = (color & 0x88) | ((color & 0x70) >> 4) | ((color & 0x07) << 4);

}	/* end video_cursor_on */

#if (0)	/*******************************************************/
	cp = (char *)&VIDEOADDR[pos] + 1;

	*cp = video_vt100->vt100_char_color >> 8;
#endif	/*******************************************************/
  
/*
 ****************************************************************
 *	Faz o video começar a apitar				*
 ****************************************************************
 */
void
video_beep (int frequency, int period)
{
	if (video_beep_period != 0)
		return;

	write_port (read_port (0x61) | 3, 0x61);
	write_port (0xB6, 0x43);
	
	write_port (frequency,      0x42);
	write_port (frequency >> 8, 0x42);
	
	video_beep_period = period;

	toutset (period, video_beep_stop, period);

}	/* end video_beep */

/*
 ****************************************************************
 *	Faz o video parar de apitar				*
 ****************************************************************
 */
void
video_beep_stop (int period)
{
	write_port (read_port (0x61) & ~3, 0x61);

	video_beep_period = 0;

}	/* end video_beep_stop */
