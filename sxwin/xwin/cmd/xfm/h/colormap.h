/*
 ****************************************************************
 *								*
 *			colormap.h				*
 *								*
 *	Estrutura Universal do Mapa de Cores			*
 *								*
 *	Versão	3.0.0, de 05.04.98				*
 *								*
 *	Módulo: xc/programs/xpaint				*
 *		X Windows - Clientes				*
 *		Baseado no software homônimo do XFree86		*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright (c) 1998 TROPIX Technologies Ltd.	*
 * 								*
 ****************************************************************
 */

#define	CMAPSZ	256

/*
 ****** Mapa de Cores *******************************************
 */
typedef struct
{
	uchar	c_red  [CMAPSZ],
		c_green[CMAPSZ],
		c_blue [CMAPSZ];

	ulong	c_size;

	int	c_grayscale;

}	COLORMAP;
