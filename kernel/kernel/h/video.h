/*
 ****************************************************************
 *								*
 *			<sys/video.h>				*
 *								*
 *	Definições relativas ao video do PC			*
 *								*
 *	Versão	3.0.0, de 07.01.95				*
 *		3.0.0, de 04.01.96				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *		/usr/include/sys				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 1999 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#define	VIDEO_H			/* Para definir os protótipos */

/*
 ****** Endereço do video ***************************************
 */
#define	VIDEOADDR	((ushort *)(SYS_ADDR + (736 * KBSZ)))

/*
 ****** Parâmetros do video *************************************
 */
#define LINE		24
#define COL 		80
#define TAB		8	/* COL deve ser múltiplo de TAB */

/*
 ******	Definições do protocolo "vt100" *************************
 */
typedef enum
{
	VT100_NORMAL = 0,	/* Estado normal */
	VT100_ESC,		/* Foi visto um "\e" (escape) */
	VT100_LB,		/* Foi visto um "[" */
	VT100_SC,		/* Foi visto o "[;" */
	VT100_QUESTION		/* Foi visto o "[?" */

}	VT100_STATE_ENUM;

typedef struct
{
	int	vt100_pos;		/* Posição do cursor */
	int	vt100_save_pos;		/* Guarda a posição do cursor */

	int	vt100_normal_char_color; /* Cor normal dos caracteres */
	int	vt100_char_color;	/* Cor atual dos caracteres */

	int	vt100_state;		/* Estado do protocolo "vt100" */

	int	vt100_arg_1;		/* O primeiro argumento */
	int	vt100_arg_2;		/* O primeiro argumento */

	int	vt100_scroll_begin;	/* Início do rolamento */
	int	vt100_scroll_end;	/* Final do rolamento */

}	VT100;

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

