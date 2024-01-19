/*
 ****************************************************************
 *								*
 *			serv_daemon.c				*
 *								*
 *	Um esqueleto de servidor				*
 *								*
 *	Versão	2.3.0, de 08.08.91				*
 *		3.2.0, de 15.06.99				*
 *								*
 *	Módulo: Internet					*
 *		Utilitários especiais				*
 *		Categoria B					*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 * 		Copyright © 2000 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */

#include <sys/common.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <xti.h>

/*
 ****************************************************************
 *	Variáveis e Definições globais				*
 ****************************************************************
 */
#if (0)	/*************************************/
const char	pgversion[] =  "Versão: 3.0.6, de 22.08.97";
#endif	/*************************************/

#define	NOVOID	(void *)NULL
#define	elif	else if
#define	EVER	;;
#define STR(x)	# x
#define XSTR(x)	STR (x)

/*
 ****** Variáveis diversas **************************************
 */
entry int	allflag;	/* Um bit para cada letra */
entry int	vflag;		/* Verbose */

entry int	exit_code;	/* Código de retorno */

entry long	udp_pid;	/* No. do processo, se for UDP */

const char	tcp_dev_nm[] = "/dev/itntcp";

entry int	global_listen_fd; /* Para "error" */

entry char	error_msg_to_log; /* Não nulo se mensagens para "itnetlog" */

/*
 ****** Protótipos de funções ***********************************
 */
void		udp_serv (int, const char *);
void		tcp_serv (int, const char *);
void		on_SIGTERM (int, ...);
void		error (const char *, ...);

/*
 ****************************************************************
 *	Um esqueleto de servidor				*
 ****************************************************************
 */
int
main (int argc, register const char *argv[])
{
	register int	opt;
	const char	*serv_nm;
	int		well_known_port_no;
	int		proto = 0;

	error_msg_to_log++;

	/*
	 *	Verifica se é SUPERUSUÁRIO
	 */
	if (geteuid () != 0)
		error ("$O usuário efetivo não é SUPERUSUÁRIO");

	/*
	 *	Analisa as opções: no momento aceitando tôdas as letras minúsculas
	 */
	while ((opt = getopt (argc, argv, "abcdefghijklmnopqrstuvwxyz")) != EOF)
	{
		if (opt >= 'a' && opt <= 'z')
			allflag |= (1 << (opt - 'a'));

		switch (opt)
		{
		    case 'v':			/* Verbose */
			vflag++;
			break;

		}	/* end switch */

	}	/* end while */

	argv += optind;
	argc -= optind;

	/*
	 *		arg[i]:	      0		   1		  2
	 *
	 *			    "tcp"
	 *	serv_daemon [<opt>]	  <well_known_port_no> <serv_nm>
	 *			    "udp"
	 */
	if (vflag || argc != 3)
	{
		if (!vflag)
			error ("Número inválido de argumentos: %d", argc);

		error
		(	"%s <%s> <%s> <%s>",
			vflag ? "-v" : "", argv[0], argv[1], argv[2]
		);
	}

	if   (streq (argv[0], "udp"))
	{
		proto = 'u';
		signal (SIGTERM, on_SIGTERM);
	}
	elif (streq (argv[0], "tcp"))
	{
		proto = 't';
	}
	else
	{
		error ("$Protocolo inválido: \"%s\"", argv[0]);
	}

	if ((well_known_port_no = atoi (argv[1])) <= 0)
		error ("$No. do \"port\" inválido: %s", argv[1]);

	if (access (serv_nm = argv[2], X_OK) < 0)
		error ("$*O servidor \"%s\" não é executável", serv_nm);

	if (proto == 'u')
		udp_serv (well_known_port_no, serv_nm);
	else
		tcp_serv (well_known_port_no, serv_nm);

	return (exit_code);

}	/* end serv_daemon */

/*
 ****************************************************************
 *	Servidor UDP						*
 ****************************************************************
 */
void
udp_serv (int well_known_port_no, const char *serv_nm)
{
	register long		i;
	register const char	**ap;
	char			port_nm[6];
	const char		*last_id;
	const char		*argv[32];

	/*
	 *	Fecha os arquivos não mais utilizados
	 */
	for (i = 3; i < NUFILE; i++)
		close (i);

	/*
	 *	Executa o servidor
	 */
	for (EVER)
	{
		if ((udp_pid = fork ()) < 0)
			error ("$*Não consegui dar \"fork\"");

		if (udp_pid == 0)
			break;

		for (EVER)
		{
			if ((i = wait ((int *)NULL)) < 0)
				error ("$*Erro no \"wait\"");

			if (i == udp_pid)
				break;
		}

	}	/* for (EVER) */

	/*
	 *	Prepara os conteúdos de "argv"
	 */
	if ((last_id = strrchr (serv_nm, '/')) == NOSTR)
		last_id = serv_nm;
	else
		last_id++;

	sprintf (port_nm, "%d", well_known_port_no);

	/*
	 *	Prepara os "argv"
	 */
	ap = argv;

	*ap++ = last_id;

#if (0)	/*******************************************************/
	if (vflag)
		*ap++ = "-v";
#endif	/*******************************************************/

	if (allflag)
	{
		int		allopt = allflag;
		int		mask;
		char		*str;

		for (mask = 1, i = 'a' - 'a'; i <= 'z' - 'a'; mask <<= 1, i++)
		{
			if (allopt & mask)
			{
				str = alloca (4);

				str[0] = '-';
				str[1] = 'a' + i;
				str[2] = '\0';

				*ap++ = str;

				if ((allopt &= ~mask) == 0)
					break;
			}
		}
	}

	*ap++ = "udp";
	*ap++ = port_nm;
	*ap++ = NOSTR;

	/*
	 *	Passa o controle para o servidor de verdade
	 *
	 *	<last_id> [-v] "udp" <well_known_port>
	 */
	execv (serv_nm, argv);

	error ("$*Não consegui dar \"exec\" para \"%s\"", serv_nm);

}	/* end udp_serv */

/*
 ****************************************************************
 *	Servidor TCP						*
 ****************************************************************
 */
void
tcp_serv (int well_known_port_no, const char *serv_nm)
{
	register long		pid, i;
	register const char	**ap;
	register int		listen_fd;
	INADDR			serv_addr, client_addr;
	T_BIND			bind;
	T_CALL			call;
	char			listen_port_nm[6], listen_fd_nm[6];
	char			addr_nm[16], port_nm[6], seq_nm[6];
	const char		*last_id;
	const char		*argv[32];
	T_OPTMGMT		req_optmgmt, ret_optmgmt;
	TCP_OPTIONS		options;

	/*
	 *	Abre o dispositivo
	 */
	if ((listen_fd = t_open (tcp_dev_nm, O_RDWR, (T_INFO *)NULL)) < 0)
		error ("$*Não consegui abrir o dispositivo \"%s\"", tcp_dev_nm);

	global_listen_fd = listen_fd;

	serv_addr.a_port = well_known_port_no;
	serv_addr.a_addr = 0;

	bind.addr.maxlen = sizeof (INADDR);
	bind.addr.len	 = sizeof (INADDR);
	bind.addr.buf	 = &serv_addr;

	bind.qlen	 = 5;

	if (t_bind (listen_fd, &bind, &bind) < 0)
		error ("$*Não consegui atribuir endereço");

	if (vflag)
		error ("QLEN negociado = %d", bind.qlen);

	/*
	 *	Negocia o tamanho da janela
	 */
	memset (&options, 0, sizeof (TCP_OPTIONS));

	req_optmgmt.flags = T_CHECK;

	req_optmgmt.opt.buf    = &options;
	req_optmgmt.opt.len    = sizeof (TCP_OPTIONS);

	ret_optmgmt.opt.buf    = &options;
	ret_optmgmt.opt.maxlen = sizeof (TCP_OPTIONS);

	if (t_optmgmt (listen_fd, &req_optmgmt, &ret_optmgmt) < 0)
		error ("$*Não consegui obter as opções TCP");

	req_optmgmt.flags = T_NEGOTIATE;

   /***	options.max_wait     = 0;	/* Sem limite */
   /***	options.max_silence  = 0;	/* Sem limite */
   	options.window_size  = 1024;

   /***	req_optmgmt.opt.buf    = &options; ***/
   /***	req_optmgmt.opt.len    = sizeof (TCP_OPTIONS); ***/

	ret_optmgmt.opt.buf    = &options;
	ret_optmgmt.opt.maxlen = sizeof (TCP_OPTIONS);

	if (t_optmgmt (listen_fd, &req_optmgmt, &ret_optmgmt) < 0)
		error ("$*Não consegui negociar as opções TCP");

	/*
	 *	Fecha os arquivos não mais utilizados
	 */
	for (i = 3; i < NUFILE; i++)
	{
		if (i != listen_fd)
			close (i);
	}

	/*
	 *	Um "esqueleto", esperando um pedido de conexão
	 */
	call.addr.maxlen = sizeof (INADDR);
	call.addr.buf = &client_addr;

   /***	call.opt.len = 0; ***/
   /***	call.udata.len = 0; ***/

	/*
	 *	Espera clientes pedirem conexão
	 */
	for (EVER)
	{
		if (t_listen (listen_fd, &call) < 0)
			error ("$*Erro no \"t_listen\"");
	
		/*
		 *	Quando chegar um pedido de conexão, o filho
		 *	atende e o pai continua esperando novas conexões
		 */
		if ((pid = fork ()) < 0)
			error ("$*Não consegui dar \"fork\"");

		if (pid == 0)
			break;

	}	/* for (EVER) */

	if (vflag) error
	(	"Recebi um pedido de conexão de (%s, %d), seq = %d",
		t_addr_to_str (client_addr.a_addr), client_addr.a_port,
		call.sequence
	);

	/*
	 *	O pai passa a ser o INIT
	 */
	if (setppid (0) < 0)
		error ("$*Não consegui dar \"setppid\"");

	/*
	 *	Prepara os conteúdos de "argv"
	 */
	if ((last_id = strrchr (serv_nm, '/')) == NOSTR)
		last_id = serv_nm;
	else
		last_id++;

	sprintf (listen_port_nm, "%d", well_known_port_no);
	sprintf (listen_fd_nm, "%d", listen_fd);
	sprintf (addr_nm, "%s", t_addr_to_str (client_addr.a_addr));
	sprintf (port_nm, "%d", client_addr.a_port);
	sprintf (seq_nm, "%d", call.sequence);

	/*
	 *	Prepara os "argv"
	 */
	ap = argv;

	*ap++ = last_id;

#if (0)	/*******************************************************/
	if (vflag)
		*ap++ = "-v";
#endif	/*******************************************************/

	if (allflag)
	{
		int		allopt = allflag;
		int		mask;
		char		*str;

		for (mask = 1, i = 'a' - 'a'; i <= 'z' - 'a'; mask <<= 1, i++)
		{
			if (allopt & mask)
			{
				str = alloca (4);

				str[0] = '-';
				str[1] = 'a' + i;
				str[2] = '\0';

				*ap++ = str;

				if ((allopt &= ~mask) == 0)
					break;
			}
		}
	}

	*ap++ = listen_port_nm;
	*ap++ = listen_fd_nm;
	*ap++ = addr_nm;
	*ap++ = port_nm;
	*ap++ = seq_nm;
	*ap++ = NOSTR;

	/*
	 *	Passa o controle para o servidor de verdade
	 *
	 *   <last_id> [-v] <listen_port> <listen_fd> <addr> <port> <sequence>
	 */
	execv (serv_nm, argv);

	error ("*Não consegui dar \"exec\" para \"%s\"", serv_nm);

	if (t_snddis (listen_fd, &call) < 0)
		error ("*Erro no \"t_snddis\"");

	exit (1);

}	/* end tcp_serv */

/*
 ****************************************************************
 *	Recebe o sinal SIGTERM					*
 ****************************************************************
 */
void
on_SIGTERM (int signo, ...)
{
	if (udp_pid)
		kill (udp_pid, SIGTERM);

	exit (1);

}	/* end on_SIGTERM */

/*
 ****************************************************************
 *	Imprime uma mensagem de erro (servidor)			*
 ****************************************************************
 */
void
error (const char *format, ...)
{
	register va_list	 args;
	register const char	*fmt = format;
	register FILE		*log_fp = NOFILE;
	char			err, abrt;
	int			event = 0, save_errno = errno;
	time_t			time_loc;
	static int		static_log_fd = 0;
	static FILE		*static_log_fp = NOFILE;

	va_start (args, format);

	/*
	 *	Procura o arquivo de "log"
	 */
	if   (error_msg_to_log == 0)
	{
		log_fp = stderr;
	}
	elif (static_log_fd == 0)
	{
		if ((static_log_fd = open ("/var/adm/itnetlog", O_WRONLY|O_APPEND)) >= 0)
		{
			if ((static_log_fp = log_fp = fdopen (static_log_fd, "w")) == NOFILE)
				static_log_fd = -1;
		}
	}
	else
	{
		log_fp = static_log_fp;
	}

	/*
	 *	Processa os argumentos da cadeia
	 */
	if (fmt == NOSTR)
		fmt = "$*Erro de entrada/saída de TCP";

	if ((abrt = *fmt) == '$')
		fmt++;

	if ((err = *fmt) == '*')
	{
		fmt++;

		if (save_errno == TLOOK)
			event = t_look (global_listen_fd);
	}

	/*
	 *	Escreve as mensagens
	 */
	if (log_fp != NOFILE)
	{
		/* Escreve a mensagem com o tempo */

		fprintf  (log_fp, pgname);

		time (&time_loc);
		fprintf (log_fp, " (%.24s)", btime (&time_loc));

		fprintf  (log_fp, ": ", pgname);
		vfprintf (log_fp, fmt, args);

		/* Escreve o tipo do erro */

		if (err == '*')
		{
			if (event != 0)
				fprintf (log_fp, " (%s)", t_strevent (global_listen_fd, event));
			elif (save_errno != 0)
				fprintf (log_fp, " (%s)", strerror (save_errno));
		}

		putc ('\n', log_fp); fflush (log_fp);
	}

	if (abrt == '$' || event ==  T_DISCONNECT)
		exit (1);

	va_end (args);

}	/* end error */
