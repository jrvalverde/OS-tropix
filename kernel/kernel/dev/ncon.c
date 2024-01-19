/*
 ****************************************************************
 *								*
 *			ncon.c					*
 *								*
 *	"Driver" para o vídeo do PC				*
 *								*
 *	Versão	3.0.0, de 15.12.94				*
 *		4.5.0, de 17.06.03				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2003 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/sync.h"
#include "../h/scb.h"
#include "../h/region.h"

#include "../h/frame.h"
#include "../h/kfile.h"
#include "../h/inode.h"
#include "../h/tty.h"
#include "../h/kbd.h"
#include "../h/video.h"
#include "../h/ioctl.h"
#include "../h/intr.h"
#include "../h/devmajor.h"
#include "../h/uerror.h"
#include "../h/signal.h"
#include "../h/uproc.h"

#include "../h/extern.h"
#include "../h/proto.h"

/*
 ****************************************************************
 *	Variáveis e Definições Globais				*
 ****************************************************************
 */
#define	CON_IRQ			1	/* No. do IRQ do teclado */
#define CON_PL			1	/* Prioridade de interrupção */
#define splcon			spl1	/* Função de prioridade de interrupção */

/*
 ******	Definições das telas virtuais ***************************
 */
entry TTY	*con_tp;	/* Console Virtual corrente */

entry enum { STATE_NORMAL, STATE_STOP_SEEN } con_state = STATE_NORMAL;

entry short	con_display[NCON][LINE*COL]; /* A cópia de cada tela virtual */

/*
 ******	Definições das cores ************************************
 */
entry VT100	con_video_vt100[NCON] =	/* O estado "vt100" de cada tela virtual */
{
	/* Inicialização prévia para a console */

	0,		/* Posição do cursor */
	0,		/* Guarda a posição do cursor */

	0x0700,		/* Cor normal dos caracteres */
	0x0700,		/* Cor atual dos caracteres */

	0,		/* Estado do protocolo "vt100" */

	0,		/* O primeiro argumento */
	0,		/* O primeiro argumento */

	0,		/* Início do rolamento */
	LINE - 1	/* Final do rolamento */
};

entry VT100	*video_vt100 = &con_video_vt100[0]; /* Tela virtual corrente */

const char	con_video_color[NCON] =	/* A cor inicial de cada tela virtual */
{
	(0 << 4)|7,	/* Branco */
	(0 << 4)|2,	/* Verde */
	(0 << 4)|1,	/* Azul */
	(0 << 4)|7,	/* Branco */
	(0 << 4)|7,	/* Branco */
	(0 << 4)|7,	/* Branco */
	(0 << 4)|7,	/* Branco */
	(0 << 4)|7	/* Branco */
};

/*
 ******	Definição de "Tela 1" na cor dada ***********************
 */
#define	COR		((0 << 4)|7)		/* Branco */
#define	TELA_TXT_OFFSET (LINE*COL+COL-36)	/* Endereço do início */
#define	TELA_NO_OFFSET	(LINE*COL+COL-31)	/* Posição do dígito */

const char	tela_1[] =
{
	' ', COR,	' ', COR,
	' ', COR,	' ', COR,
	' ', COR,	'1', COR
};

/*
 ******	Variváveis globais **************************************
 */
entry TTY	con[NCON];		/* Um para cada console Virtual */

entry char	con_nopen[NCON];	/* No. de Opens de cada vídeo */

entry char	con_lock[NCON];		/* Para o "O_LOCK" */

extern TTY	ps2m_tty;		/* Para o "mouse" PS/2 */

entry time_t	key_last_time;		/* Tempo do último caratere teclado */

enum { SCREEN_SAVER_IDLE, SCREEN_SAVER_ACTIVE }; /* Estado do "screen saver" */

entry char	screen_saver_state;

extern const char function_key_table[][8];	/* Tabela de funções */

/*
 ******	Protótipos de funções ***********************************
 */
int		kbd_probe_keyboard (void);
int		kbd_write_kbd_command (int command);
int		kbd_send_kbd_command (int command);
int		kbd_send_kbd_command_and_data (int command, int data);
int		kbd_wait_for_kbd_ack (void);
void		constart (TTY *, int);
int		con_cursor_color (int color);
void		conint (struct intr_frame);
void		kbdevent (int scancode);
void		mouseint (struct intr_frame frame);
int		kbd_read_data (void);
int		kbd_wait_for_input (void);
void		kbd_wait (void);
void		aux_write_ack (int val);
void		kbd_write_cmd (int cmd);
int		kbd_send_data (char data);

/*
 ****************************************************************
 *	Função de "attach"					*
 ****************************************************************
 */
int
conattach (int major)
{
	int		i;
	VT100		*vtp;
	char		color;

	/*
	 *	Prepara as telas (exceto a console)
	 */
	for (i = 1; i < NCON; i++)
	{
		color = con_video_color[i];

		/* Coloca brancos em toda a tela */

		memsetw (con_display[i], (color << 8)|' ', LINE*COL);

		/* Coloca um cursor no primeiro caratere */

		color = (color & 0x88) | ((color & 0x70) >> 4) | ((color & 0x07) << 4);

		con_display[i][0] = (color << 8)|' ';
	}

	/*
	 *	Termina de preparar o protocolo "vt100"
	 */
	for (vtp = &con_video_vt100[0], i = 0; i < NCON; vtp++, i++)
	{
	   /***	vtp->vt100_pos			= ... */
	   /***	vtp->vt100_save_pos		= ... */

		vtp->vt100_normal_char_color	=
		vtp->vt100_char_color		= con_video_color[i] << 8;

	   /***	vtp->vt100_state		= VT100_NORMAL; ***/

	   /***	vtp->vt100_arg_1		= 0; ***/
	   /***	vtp->vt100_arg_2		= 0; ***/

	   /***	vtp->vt100_scroll_begin		= 0; ***/
		vtp->vt100_scroll_end		= LINE - 1;
	}

	/*
	 *	Escreve "Tela 1" na última linha da tela
	 */
	memmove (&VIDEOADDR[TELA_TXT_OFFSET], tela_1, sizeof (tela_1));

	/*
	 ******	Inicializa o Teclado ****************************
	 *
	 *	Esvazia a fila de entrada
	 */
	splcon ();

	if (kbd_probe_keyboard () < 0)
		printf ("conattach: Erro no teclado\n");

	/*
	 *	Prepara a interrupção
	 */
	if (set_dev_irq (CON_IRQ, CON_PL, 0, conint) < 0)
		return (-1);

	/*
	 *	Escreve qual o teclado configurado
	 */
	printf ("key: [0: 0x60, %d, ", CON_IRQ);

	if (scb.y_keyboard == 0)
		printf ("US]\n");
	else
		printf ("ABNT]\n");

	spl0 ();

	return (0);

}	/* end conattach */

/*
 ****************************************************************
 *	Inicializa o teclado					*
 ****************************************************************
 */
int
kbd_probe_keyboard (void)
{
	int		retry, again, c = KBD_RESEND;
	int		command_byte;

	kbd_write_controller_command (KBDC_DISABLE_KBD_PORT);

	kbd_empty_both_buffers (200);

	if ((command_byte = kbd_get_controller_command_byte ()) < 0)
		{ printf ("%g: NÃO consegui obter o byte de comando\n"); return (-1); }


	if (kbd_set_controller_command_byte (KBD_KBD_CONTROL_BITS, KBD_ENABLE_KBD_PORT|KBD_DISABLE_KBD_INT) < 0)
		{ printf ("%g: NÃO consegui enviar um comando de controle\n"); return (-1); }

	/*
	 *	Coloca o teclado no estado inicial
	 */
	for (retry = KBD_MAXRETRY; /* abaixo */; retry--)
	{
		if (retry <= 0)
			return (-1);

		kbd_empty_both_buffers (10);

		if (kbd_write_kbd_command (KBDC_RESET_KBD) < 0)
			continue;

		kbd_emptyq (&kbd_kbd_q);

		if (kbd_read_controller_data () == KBD_ACK)
			break;
	}

	for (again = KBD_MAXWAIT * 100; again > 0; again--)
	{
		if ((c = kbd_read_controller_data ()) >= 0)
			break;

		DELAY (KBD_RESETDELAY * 10);
	}

#if (0)	/*******************************************************/
	for (again = KBD_MAXWAIT; again > 0; again--)
	{
		if ((c = kbd_read_controller_data ()) >= 0)
			break;

		DELAY (KBD_RESETDELAY * 1000);
	}
#endif	/*******************************************************/

	if (c != KBD_RESET_DONE)	/* reset status */
		return (-1);

	/*
	 *	Configura a velocidade
	 */
	if (kbd_send_kbd_command_and_data (KBDC_SET_TYPEMATIC, (0 << 5) | 0x04) != KBD_ACK)
	        kbd_send_kbd_command (KBDC_ENABLE_KBD); 

	/*
	 *	Habilita o teclado
	 */
	command_byte &= (KBD_TRANSLATION|KBD_OVERRIDE_KBD_LOCK);

	kbd_set_controller_command_byte
	(
		KBD_KBD_CONTROL_BITS|KBD_TRANSLATION|KBD_OVERRIDE_KBD_LOCK,
		command_byte|KBD_ENABLE_KBD_PORT|KBD_ENABLE_KBD_INT
	);

	return (0);

}	/* end kbd_probe_keyboard */

/*
 ****************************************************************
 *	Escreve um commando na porta do teclado			*
 ****************************************************************
 */
int
kbd_write_kbd_command (int command)
{
	if (kbd_wait_while_controller_busy () < 0)
		return (-1);

	write_port (command, KBD_DATA_PORT);

	return (0);

}	/* end kbd_write_kbd_command */

/*
 ****************************************************************
 *	Envia um comando para o teclado e aguarda o ACK		*
 ****************************************************************
 */
int
kbd_send_kbd_command (int command)
{
	int		retry, res = -1;

	for (retry = KBD_MAXRETRY; /* abaixo */; retry--)
	{
		if (retry <= 0)
			return (-1);

		if (kbd_write_kbd_command (command) < 0)
			continue;

		if ((res = kbd_wait_for_kbd_ack ()) == KBD_ACK)
			break;
	}

	return (res);

}	/* end kbd_send_kbd_command */

/*
 ****************************************************************
 *	Envia um comando e um dado para a porta auxiliar	*
 ****************************************************************
 */
int
kbd_send_kbd_command_and_data (int command, int data)
{
	int		retry, res = -1;

	for (retry = KBD_MAXRETRY; /* abaixo */; retry--)
	{
		if (retry <= 0)
			return (res);

		if (kbd_write_kbd_command (command) < 0)
			continue;

		if   ((res = kbd_wait_for_kbd_ack ()) == KBD_ACK)
			break;
		elif (res != KBD_RESEND)
			return (res);
	}

	for (retry = KBD_MAXRETRY, res = -1; retry > 0; retry--)
	{
		if (kbd_write_kbd_command (data) < 0)
			continue;

		if ((res = kbd_wait_for_kbd_ack ()) != KBD_RESEND)
			break;
	}

	return (res);

}	/* end kbd_send_kbd_command_and_data */

/*
 ****************************************************************
 *	Espera por um ACK, RESEND, RESET_FAIL, ...		*
 ****************************************************************
 */
int
kbd_wait_for_kbd_ack (void)
{
	int		retry;
	char		status, data;

	for (retry = 10000; /* abaixo */; retry--)
	{
		if (retry <= 0)
			return (-1);

		if ((status = read_port (KBD_STATUS_PORT)) & KBDS_ANY_BUFFER_FULL)
		{
			DELAY (KBDD_DELAYTIME);

			data = read_port (KBD_DATA_PORT);

			if   ((status & KBDS_BUFFER_FULL) == KBDS_KBD_BUFFER_FULL)
			{
				if (data == KBD_ACK || data == KBD_RESEND || data == KBD_RESET_FAIL)
					return (data);

				kbd_putq (&kbd_kbd_q, data);
			}
			elif ((status & KBDS_BUFFER_FULL) == KBDS_AUX_BUFFER_FULL)
			{
				kbd_putq (&kbd_mouse_q, data);
			}
		}

		DELAY (KBDC_DELAYTIME);
	}

}	/* end kbd_wait_for_kbd_ack */

/*
 ****************************************************************
 *	Função de "open"					*
 ****************************************************************
 */
void
conopen (dev_t dev, int oflag)
{
	TTY		*tp;
	int		vterm;

	/*
	 *	Prólogo
	 */
	if ((unsigned)(vterm = MINOR (dev)) >= NCON)
		{ u.u_error = ENXIO; return; }

	if (con_lock[vterm] || (oflag & O_LOCK) && con_nopen[vterm])
		{ u.u_error = EBUSY; return; }

	tp = &con[vterm];

	if (con_nopen[vterm] > 0)
		{ ttyopen (dev, tp, oflag); con_nopen[vterm]++; return; }
	

	/*
	 *	Abre esta console virtual
	 */
	if ((tp->t_state & ISOPEN) == 0)
	{
		tp->t_irq   = CON_IRQ;
		tp->t_vterm = vterm;
	   /***	tp->t_canal = ...; ***/
	   /***	tp->t_addr  = ...; ***/
		tp->t_oproc = constart;

		tp->t_iflag = ICRNL|IXON;
		tp->t_oflag = OPOST|ONLCR;
		tp->t_cflag = B9600|CS8|CLOCAL;
		tp->t_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK|CNTRLX|ISO|VIDEO;

		tp->t_color = con_video_color[vterm];

		tp->t_nline = LINE;
		tp->t_ncol  = COL;

		if (con_tp == NOTTY)
		{
			VT100	*vtp = &con_video_vt100[vterm];

			con_tp = tp;

			vtp->vt100_normal_char_color	=
			vtp->vt100_char_color		= tp->t_color << 8;

			vtp->vt100_scroll_begin		= 0;
			vtp->vt100_scroll_end		= LINE - 1;
		}
	}

	ttyopen (dev, tp, oflag);

	/*
	 *	Epílogo
	 */
	con_nopen[vterm] = 1;

	if (oflag & O_LOCK)
		con_lock[vterm] = 1;

}	/* end conopen */

/*
 ****************************************************************
 *	Função de "close"					*
 ****************************************************************
 */
void
conclose (dev_t dev, int flag)
{
	TTY		*tp;
	int		vterm;

	/*
	 *	Prólogo
	 */
	vterm = MINOR (dev);

	if (--con_nopen[vterm] > 0)
		return;

	tp = &con[vterm];

   /***	EVENTWAIT (&tp->t_outqempty, PTTYOUT);	***/

	/*
	 *	Epílogo
	 */
	con_lock[vterm] = 0;

	ttyclose (tp);

}	/* end conclose */

/*
 ****************************************************************
 *	Função de "read"					*
 ****************************************************************
 */
void
conread (IOREQ *iop)
{
	ttyread (iop, &con[MINOR (iop->io_dev)]);

}	/* end conread */

/*
 ****************************************************************
 *	Função  de "write"					*
 ****************************************************************
 */
void
conwrite (IOREQ *iop)
{
	TTY		*tp;

	tp = &con[MINOR (iop->io_dev)];

	if (con_tp == tp)
		key_last_time = time;

	ttywrite (iop, tp);

}	/* end conwrite */

/*
 ****************************************************************
 *	Função de IOCTL						*
 ****************************************************************
 */
int
conctl (dev_t dev, int cmd, void *arg, int flag)
{
	TTY		*tp;
	int		vterm;
	int		ret;

	/*
	 *	Prólogo
	 */
	vterm = MINOR (dev); tp = &con[vterm];

	/*
	 *	Tratamento especial para certos comandos
	 */
	switch (cmd)
	{
		/*
		 *	Processa/prepara o ATTENTION
		 */
	    case U_ATTENTION:
#if (0)	/*******************************************************/
		splcon ();
#endif	/*******************************************************/
		splx (irq_pl_vec[tp->t_irq]);

		if (EVENTTEST (&tp->t_inqnempty) == 0)
			{ spl0 (); return (1); }

		tp->t_uproc	= u.u_myself;
		tp->t_index	= (int)arg;

		tp->t_attention_set = 1;
		*(char **)flag	= &tp->t_attention_set;

		spl0 ();

		return (0);

#ifdef	UNATTENTION
	    case U_UNATTENTION:
		tp->t_state &= ~ATTENTION;

		return (0);
#endif	UNATTENTION

	}	/* end switch */

	/*
	 *	Demais IOCTLs
	 */
   /***	SLEEPLOCK (&tp->t_olock, PTTYOUT); ***/

	if ((ret = ttyctl (tp, cmd, arg)) >= 0)
	{
		if (cmd == TCSETS || cmd == TCSETAW || cmd == TCSETAF)
		{
			if (tp == con_tp)
			{
				VT100	*vtp = &con_video_vt100[vterm];

				vtp->vt100_normal_char_color	=
				vtp->vt100_char_color		= tp->t_color << 8;
			}
		}
	}

   /***	SLEEPFREE (&tp->t_olock); ***/

	return (ret);

}	/* end conctl */

/*
 ****************************************************************
 *	Função de interrupção do teclado/mouse			*
 ****************************************************************
 */
void
conint (struct intr_frame frame)
{
	char		status;
	int		scancode;

	while ((status = read_port (KBD_STATUS_PORT)) & KBDS_ANY_BUFFER_FULL)
	{
		scancode = read_port (KBD_DATA_PORT);

		if (status & (KBDS_STAT_GTO|KBDS_STAT_PERR))
			continue;

		if (status & KBDS_ONLY_AUX_BUFFER)
			ttyin (scancode, &ps2m_tty);
		else
			kbdevent (scancode);
	}

}	/* end conint */

/*
 ****************************************************************
 *	Função de interrupção do teclado			*
 ****************************************************************
 */
void
kbdevent (int scancode)
{
	int		c, old_vterm, new_vterm;
	TTY		*tp;

	/*
	 *	x
	 */
	key_last_time = time;

	if (screen_saver_state != SCREEN_SAVER_IDLE)
		return;

	if ((tp = con_tp) == NOTTY)
		return;

	if ((c = key_code_to_char (scancode)) < 0)
		return;

	/*
	 *	Analisa o estado das telas virtuais
	 */
	if (c == CSTOP)
		{ con_state = STATE_STOP_SEEN; ttyin (c, tp); return; }

	/*
	 *	Processa o estado normal (com as cadeias)
	 */
	if (con_state == STATE_NORMAL)
	{
		const char	*cp;

		if (c > 0)
			{ ttyin (c, tp); return; }

		cp = function_key_table[scancode];

		while ((c = *cp++) != 0)
			ttyin (c, tp);

		return;
	}

	/*
	 *	Estamos no STATE_STOP_SEEN
	 */
	if (c == 0)
		return;

	con_state = STATE_NORMAL;

	if ((unsigned)(new_vterm = c - '1') >= NCON || new_vterm == tp->t_vterm)
		{ ttyin (c, tp); return; }

	/*
	 *	Vai ser iniciada uma troca de T.V.
	 */
	old_vterm = tp->t_vterm; tp = &con[new_vterm];

	if ((tp->t_state & ISOPEN) == 0)
		return;

	memmove (con_display[old_vterm], VIDEOADDR, LINE*COL*sizeof (short));

	memmove (VIDEOADDR, con_display[new_vterm], LINE*COL*sizeof (short));

	video_vt100 = &con_video_vt100[new_vterm];

	*(char *)&VIDEOADDR[TELA_NO_OFFSET] = '1' + new_vterm;

	con_tp = tp; constart (tp, 1);

}	/* end kbdevent */

/*
 ****************************************************************
 *	Inicia a transmissão para o vídeo			*
 ****************************************************************
 */
void
constart (TTY *tp, int flag)
{
	int		c;

	/*
	 *	O Flag indica a origem da chamada:
	 *	    0 => Chamada fora de conint
	 *	    1 => Chamada de conint
	 */

	/*
	 *	Verifica se é uma operação para a tela corrente
	 */
	if (tp != con_tp)
		return;

	if (screen_saver_state != SCREEN_SAVER_IDLE)
		return;

	/*
	 *	Verifica se está no estado STOP
	 */
	if (tp->t_state & TTYSTOP)
		return;

	/*
	 *	Verifica se tem operação em andamento
	 */
	if (TAS (&tp->t_obusy) < 0)
		return;

	/*
	 *	Escreve o(s) caracter(es)
	 */
	for (EVER)
	{
		SPINLOCK (&tp->t_outq.c_lock); c = cget (&tp->t_outq); SPINFREE (&tp->t_outq.c_lock);

		if (c < 0)
			break;

		writechar (tp->t_cvtb[c]);

		if (c == '\n')
		{
			if ((tp->t_oflag & OPOST) == 0)
				continue;

			tp->t_lno++;

			if (tp->t_page != 0 && tp->t_lno >= tp->t_page)
				tp->t_state |= TTYSTOP;
		}

		if (tp->t_state & TTYSTOP)
			break;
	}

	CLEAR (&tp->t_obusy);

	EVENTDONE (&tp->t_outqempty);

#if (0)	/*************************************/
	if (tp->t_outq.c_count <= TTYMINQ)
#endif	/*************************************/
		EVENTDONE (&tp->t_outqnfull);

}	/* end constart */

/*
 ****************************************************************
 *	Processamento de segundo em segundo			*
 ****************************************************************
 */
void
screen_saver_every_second_work (void)
{
	TTY		*tp;
	int		vterm;

	if (scb.y_screen_saver_time <= 0)
		return;

	if (key_last_time == 0)
		{ key_last_time = time; return; }

	if ((tp = con_tp) == NOTTY)
		return;

	vterm = tp->t_vterm;

	if (screen_saver_state == SCREEN_SAVER_IDLE)
	{
		if (time > key_last_time + scb.y_screen_saver_time)
		{
			/* Liga o "screen_saver" */

			screen_saver_state = SCREEN_SAVER_ACTIVE;
			memmove (con_display[vterm], VIDEOADDR, LINE*COL*sizeof (short));
			memsetw (VIDEOADDR, ' ', LINE*COL);
			screen_saver_on ();
			*(char *)&VIDEOADDR[TELA_NO_OFFSET] = ' ';
		}
	}
	elif (time <= key_last_time + scb.y_screen_saver_time)
	{
		/* Desliga o "screen_saver" */

		memmove (VIDEOADDR, con_display[vterm], LINE*COL*sizeof (short));
		*(char *)&VIDEOADDR[TELA_NO_OFFSET] = '1' + vterm;
		screen_saver_state = SCREEN_SAVER_IDLE;
		constart (tp, 1);
	}
	else	/* screen_saver_state == SCREEN_SAVER_ACTIVE */
	{
		/* Avança o "screen_saver" */

		screen_saver_inc ();
	}

}	/* screen_saver_second_work */

/*
 ****************************************************************
 *	Interrompe as atividades do "screen saver"		*
 ****************************************************************
 */
void
screen_saver_off (void)
{
	int		vterm;

	if (screen_saver_state != SCREEN_SAVER_ACTIVE)
		return;

	vterm = con_tp->t_vterm;

	memmove (VIDEOADDR, con_display[vterm], LINE*COL*sizeof (short));
	*(char *)&VIDEOADDR[TELA_NO_OFFSET] = '1' + vterm;

	screen_saver_state = SCREEN_SAVER_IDLE;
	key_last_time	   = time;	/* Para não religar de imediato */

}	/* end screen_saver_off */
