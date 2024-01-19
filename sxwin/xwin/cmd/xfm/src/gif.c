/*
 ****************************************************************
 *								*
 *			gif.c					*
 *								*
 *	Leitura de imagens em formato GIF			*
 *								*
 *	Versão	3.0.0, de 15.03.98				*
 *		3.1.6, de 01.02.99				*
 *								*
 *	Módulo: xc/programs/xgifshow				*
 *		X Windows - Clientes				*
 *		Baseado no software homônimo do XFree86		*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright (c) 1998 TROPIX Technologies Ltd.	*
 * 								*
 ****************************************************************
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "../h/colormap.h"

extern XStandardColormap	xstdcmap;

/*
 ****************************************************************
 *	Constantes e Macros					*
 ****************************************************************
 */
#define	MAX_LWZ_BITS		12

#define	TRUE			1
#define	FALSE			0

#define INTERLACE		0x40
#define COLORMAP_PRESENT	0x80

#define	gif_read(file,buffer,len)	(fread (buffer, len, 1, file) != 0)

/*
 ****************************************************************
 *	Definições de tipos					*
 ****************************************************************
 */
/*
 ****** Cabeçalho global ****************************************
 */
typedef struct
{
	char	g_magic[6];		/* Cadeia mágica (sem o '\0') */

	ushort	g_width,
		g_height;

	uchar	g_info,
		g_background,
		g_aspectratio;

}	GIFHEADER;

/*
 ****** Cabeçalho local *****************************************
 */
typedef struct
{
	ushort	l_left,
		l_top,
		l_width,
		l_height;

	uchar	l_info;

}	IMHEADER;

/*
 ****** Sei lá o que é isso  ************************************
 */
static struct
{
	int	transparent;
	int	delayTime;
	int	inputFlag;
	int	disposal;

}	Gif89 = { -1, -1, -1, 0 };

static GIFHEADER	header;

/*
 ****************************************************************
 *	Variáveis globais					*
 ****************************************************************
 */
extern	int	vflag;

/*
 ****************************************************************
 *	Protótipos de Funções					*
 ****************************************************************
 */
int		read_colormap (FILE *, COLORMAP *);
int		do_extension (FILE *, int);
int		get_data_block (FILE *, uchar *);
int		get_code (FILE *, int, int);
int		get_LZW_byte (FILE *, int, int);
XImage		*read_image (FILE *, int, int, COLORMAP *, int, int, Display *);

/*
 ****************************************************************
 *	Lê um arquivo GIF, gerando uma "XImage"			*
 ****************************************************************
 */
void
PutGif (Display *dpy, Drawable drawable, GC gc, FILE *fp,
	int imageNumber, int x, int y)
{
	XImage		*ximage = NULL;
	uchar		c;
	int		imageCount = 0;
	COLORMAP	global_cmap, local_cmap, *cmapp;
	IMHEADER	lheader;

	if (fp == NOFILE)
		goto finish;

	rewind (fp);

	if (!gif_read (fp, &header, 13))
		goto finish;

	if
	(	strncmp (header.g_magic, "GIF87a", 6) != 0 &&
		strncmp (header.g_magic, "GIF89a", 6) != 0
	)
		goto finish;

	/*
	 *	Obtém o número de cores utilizadas (máximo).
	 */
	global_cmap.c_size = 2 << (header.g_info & 0x07);

	if (header.g_info & COLORMAP_PRESENT)
	{	/*
		 *	Lê o mapa de cores global.
		 */
		if (read_colormap (fp, &global_cmap))
		{
			error ("Erro na leitura do mapa de cores global");
			goto finish;
		}
	}

	/*
	 *	Processa as imagens.
	 */
	do
	{
		c = getc (fp);

		if (c == ';')		/* GIF terminator */
		{
			if (imageCount < imageNumber)
			{
				error
				(	"Arquivo contém apenas %d image%s",
					imageCount, imageCount > 1 ? "ns" : "m"
				);

				goto finish;
			}
		}

		if (c == '!')		/* Extension */
		{
			c = getc (fp);
			do_extension (fp, c);
			continue;
		}

		if (c != ',')		/* Not a valid start character */
			continue;

		/*
		 *	Lê o cabeçalho da próxima imagem.
		 */
		++imageCount;
		if (!gif_read (fp, &lheader, 9))
		{
			error ("Erro na leitura do cabeçalho da imagem %d", imageCount);
			goto finish;
		}

		if (lheader.l_info & COLORMAP_PRESENT)
		{	/*
			 *	A imagem tem seu próprio mapa de cores.
			 */
			local_cmap.c_size = 1 << ((lheader.l_info & 0x07) + 1);

			if (read_colormap (fp, &local_cmap))
			{
				error ("Erro na leitura do mapa de cores local");
				goto finish;
			}

			cmapp = &local_cmap;
		}
		else
		{
			cmapp = &global_cmap;
		}

		ximage =	read_image
				(	fp,
					lheader.l_width, lheader.l_height,
					cmapp,
					lheader.l_info & INTERLACE,
					imageCount != imageNumber, dpy
				);

	}
	while (ximage == NULL);

finish:
	if (ximage == NULL)
	{
		XDrawRectangle (dpy, drawable, gc, x, y, 15, 15);
		XDrawLine (dpy, drawable, gc, x, y, x + 16, y + 16);
		XDrawLine (dpy, drawable, gc, x, y + 16, x + 16, y);
	}
	else
	{
		XPutImage (dpy, drawable, gc, ximage, 0, 0, x, y, ximage->width, ximage->height);
		XDestroyImage (ximage);
	}

	return;

}	/* end PutGif */

/*
 ****************************************************************
 *	Lê um mapa de cores					*
 ****************************************************************
 */
int
read_colormap (FILE *fp, COLORMAP *cmp)
{
	int	i, size;
	uchar	rgb[3];

	for (i = 0, size = cmp->c_size; i < size; i++)
	{
		if (!gif_read (fp, rgb, sizeof (rgb)))
		{
			error ("Erro na leitura do mapa de cores");
			return (1);
		}

		cmp->c_red  [i] = rgb[0];
		cmp->c_green[i] = rgb[1];
		cmp->c_blue [i] = rgb[2];
	}

	cmp->c_grayscale = 0;

	return (0);

}	/* end read_colormap */

/*
 ****************************************************************
 *	Processa as extensões					*
 ****************************************************************
 */
int
do_extension (FILE *fp, int label)
{
	static char	buf[CMAPSZ];
	char		*str;

	switch (label)
	{
	    case 0x01:			/* Plain Text Extension */
		str = "Plain Text Extension";
		break;

	    case 0xFF:			/* Application Extension */
		str = "Application Extension";
		break;

	    case 0xFE:			/* Comment Extension */
		str = "Comment Extension";
		while (get_data_block (fp, (uchar *)buf) != 0)
			/* vazio */;

		return (FALSE);

	    case 0xF9:			/* Graphic Control Extension */
		str = "Graphic Control Extension";
		(void)get_data_block (fp, (uchar *)buf);

		Gif89.disposal  = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag = (buf[0] >> 1) & 0x1;
		Gif89.delayTime = (buf[2] << 8) | buf[1]; /* LM_to_uint(buf[1], buf[2]); */

		if ((buf[0] & 0x1) != 0)
			Gif89.transparent = buf[3];

		while (get_data_block (fp, (uchar *)buf) != 0)
			/* vazio */;

		return (FALSE);

	    default:
		str = buf;
		sprintf (buf, "UNKNOWN (0x%02x)", label);
		break;
	}

	while (get_data_block (fp, (uchar *)buf) != 0)
		/* vazio */;

	return (FALSE);

}	/* end do_extension */


/*
 ****************************************************************
 *	Lê um bloco de dados, precedido de seu tamanho		*
 ****************************************************************
 */
static int ZeroDataBlock = FALSE;	/* Para quê ? */

int
get_data_block (FILE *fp, uchar *buf)
{
	uchar	count;

	count = getc (fp);

	ZeroDataBlock = count == 0;

	if (count && !gif_read (fp, buf, count))
		return (-1);

	return (count);

}	/* end get_data_block */

/*
 ****************************************************************
 *	x							*
 ****************************************************************
 */
int
get_code (FILE *fp, int code_size, int flag)
{
	static uchar	buf[280];
	static int	curbit, lastbit, done, last_byte;
	int		i, j, ret;
	uchar		count;

	if (flag)
	{	/*
		 *	Inicialização.
		 */
		curbit = lastbit = 0;
		done = FALSE;

		return (0);
	}

	if ((curbit + code_size) >= lastbit)
	{
		if (done)
		{
			if (curbit >= lastbit)
				error ("ran off the end of my bits");

			return (-1);
		}

		buf[0] = buf[last_byte - 2];
		buf[1] = buf[last_byte - 1];

		if ((count = get_data_block (fp, &buf[2])) == 0)
			done = TRUE;

		last_byte	= 2 + count;
		curbit		= (curbit - lastbit) + 16;
		lastbit		= (2 + count) << 3;
	}

	ret = 0;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[i >> 3] & (1 << (i & 7))) != 0) << j;

	curbit += code_size;

	return (ret);

}	/* end get_code */

/*
 ****************************************************************
 *	Obtém o próximo byte da imagem sendo descomprimida	*
 ****************************************************************
 */
int
get_LZW_byte (FILE *fp, int flag, int input_code_size)
{
	static int	fresh = FALSE;
	static int	code_size, set_code_size;
	static int	max_code, max_code_size;
	static int	firstcode, oldcode;
	static int	clear_code, end_code;
	static int	table[2][(1 << MAX_LWZ_BITS)];
	static int	stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;
	int		i, code, incode;

	if (flag)
	{	/*
		 *	Inicialização.
		 */
		set_code_size	= input_code_size;
		code_size	= set_code_size + 1;
		clear_code	= 1 << set_code_size;
		end_code	= clear_code + 1;
		max_code_size	= 2 * clear_code;
		max_code	= clear_code + 2;

		get_code (fp, 0, TRUE);

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i)
		{
			table[0][i] = 0;
			table[1][i] = i;
		}

		for (/* vazio */; i < (1 << MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][0] = 0;

		sp = stack;

		return (0);
	}

	if (fresh)
	{
		fresh = FALSE;

		do
		{
			firstcode = oldcode = get_code (fp, code_size, FALSE);

		}	while (firstcode == clear_code);

		return (firstcode);
	}

	if (sp > stack)
		return (*--sp);

	while ((code = get_code (fp, code_size, FALSE)) >= 0)
	{
		if (code == clear_code)
		{
			for (i = 0; i < clear_code; ++i)
			{
				table[0][i] = 0;
				table[1][i] = i;
			}

			for (/* vazio */; i < (1 << MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = 0;

			code_size	= set_code_size + 1;
			max_code_size	= 2 * clear_code;
			max_code	= clear_code + 2;
			sp		= stack;
			firstcode	= oldcode = get_code (fp, code_size, FALSE);

			return (firstcode);
		}

		if (code == end_code)
		{
			int	count;
			uchar	buf[260];

			if (ZeroDataBlock)
				return (-2);

			while ((count = get_data_block (fp, buf)) > 0)
				/* vazio */;

#if (0)	/****************************************************/
			if (count != 0)
			{
		/*
		 * pm_message("missing EOD in data stream (common occurence)");
		 */
			}
#endif	/****************************************************/

			return (-2);
		}

		incode = code;

		if (code >= max_code)
		{
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code)
		{
			*sp++ = table[1][code];
			if (code == table[0][code])
				error ("circular table entry BIG ERROR");

			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) < (1 << MAX_LWZ_BITS))
		{
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;

			if
			(	(max_code >= max_code_size) &&
				(max_code_size < (1 << MAX_LWZ_BITS))
			)
			{
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return (*--sp);
	}

	return (code);

}	/* end get_LZW_byte */

/*
 ****************************************************************
 *	Lê uma imagem, guardando na memória			*
 ****************************************************************
 */
XImage *
read_image (FILE *fp, int len, int height, COLORMAP *cmp,
		int interlace, int ignore, Display *dpy)
{
	char		*image, c;
	XImage		*ximage;
	int		depth, xpos, ypos, screen, size;
	ulong		pass, pixi, pixval;
	static int	yinc[] = { 8, 8, 4, 2 };

	/*
	 *	Inicializa o decompressor.
	 */
	c = getc (fp);

	if (get_LZW_byte (fp, TRUE, c) < 0)
	{
		error ("Erro na leitura da imagem");
		return (NULL);
	}

	/*
	 *	If this is an "uninteresting picture" ignore it.
	 */
	if (ignore)
	{
		while (get_LZW_byte (fp, FALSE, c) >= 0)
			/* vazio */;

		return (NULL);
	}

	screen = DefaultScreen (dpy);
	depth = DefaultDepth (dpy, screen);

        /*
         *      Aloca a imagem de trabalho.
         */
        image = (char *) malloc (len * height * (depth >> 3));

        if (image == NULL)
                error ("$Não há memória para criar a imagem");

        ximage =	XCreateImage
                        (       dpy, DefaultVisual (dpy, screen),
				depth, ZPixmap, 0, image,
                                len, height, 8, 0
                        );

        if (!ximage)
                error ("$Não foi possível criar a imagem");

	/*
	 *	Inicializa a contagem de cores efetivamente usadas.
	 */
	size = (xstdcmap.red_max + 1) * (xstdcmap.green_max + 1) * (xstdcmap.blue_max + 1);

	xpos = 0; ypos = 0; pass = 0;

	while ((pixi = get_LZW_byte (fp, FALSE, c)) >= 0)
	{
		int	red = cmp->c_red[pixi];
		int	green = cmp->c_green[pixi];
		int	blue = cmp->c_blue[pixi];

		pixval = xstdcmap.base_pixel +
			(((red * xstdcmap.red_max) >> 8) * xstdcmap.red_mult) +
			(((green * xstdcmap.green_max) >> 8) * xstdcmap.green_mult) +
			(((blue * xstdcmap.blue_max) >> 8) * xstdcmap.blue_mult);
		/*
		 *	Acrescenta o pixel obtido à imagem sendo gerada.
		 *	Verifica se é um valor novo, ainda não usado.
		 */
		XPutPixel (ximage, xpos, ypos, pixval);

		if (++xpos == len)
		{
			xpos = 0;

			if (interlace)
			{
				ypos += yinc[pass];

				if (ypos >= height)
				{
					if (++pass > 3)
						break;

					ypos = yinc[pass] >> 1;
				}
			}
			else
			{
				++ypos;
			}
		}

		if (ypos >= height)
			break;
	}

	return (ximage);

}	/* end read_image */
