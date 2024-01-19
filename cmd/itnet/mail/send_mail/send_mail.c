/*
 ****************************************************************
 *								*
 *			send_mail.c				*
 *								*
 *	Interface para enviar cartas do correio eletrônico	*
 *								*
 *	Versão	3.1.0, de 01.12.98				*
 *		4.8.0, de 01.02.05				*
 *								*
 *	Módulo: ITNET						*
 *		Utilitários de comunicação			*
 *		Categoria B					*
 *								*
 *	TROPIX: Sistema Operacional Multiprocessado		*
 *		Copyright © 2005 NCE/UFRJ - tecle "man licença"	*
 * 								*
 ****************************************************************
 */

#include <sys/types.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <xti.h>

/*
 ****************************************************************
 *	Variáveis e Definições globais				*
 ****************************************************************
 */
const char	pgversion[] =  "Versão:	4.8.0, de 01.02.05";

#define	elif	else if
#define	EVER	;;
#define STR(x)	# x
#define XSTR(x)	STR (x)

#define MAIL_PORT	25

/*
 ****** Variáveis globais ***************************************
 */
entry const char	tcp_dev_nm[] = "/dev/itntcp";

entry const char	sendmailrc_nm[] = "/etc/.sendmailrc";

entry const char	*relay_host_nm;		/* Computador retransmissor */

int			global_tcp_fd;		/* Para o TCP */

entry FILE		*tcp_in,		/* Conexão de controle */
			*tcp_out;

entry FILE		*error_fp;		/* Para "error" */
entry FILE		*log_fp;		/* Para "log" */

char			my_name[256];		/* Nome do computador */

const char		*my_domain;		/* Nome do domínio */

entry const char	*file_nm;		/* O arquivo contendo a carta */
entry FILE		*file_fp;

entry long		file_offset;		/* Deslocamento do início da "carta" */

entry const char	dead_letter[] = "/dead.letter";

entry int		fflag;			/* De um ".forward" */
entry int		aflag;			/* Envia a todos servidores */
entry int		vflag;			/* Verbose */

entry int		exit_code = 0;		/* Código de retorno */

entry char		*sender_nm;		/* Nome completo do remetente */
entry char		*sender_user_nm;	/* Somente a conta do nome do remetente */

entry const char	**receiver_nm_list;	/* Lista de destinatários */
entry int		receiver_nm_total;
entry int		receiver_nm_sz;

/*
 ****** Estrutura de dados para listas **************************
 */
typedef struct list	LIST;

struct list
{
	char	     l_code;	/* Veja o enum abaixo */
	const char  *l_nm;	/* Nome do domínio ou dst. */
	LIST	    *l_dst;	/* Destinatário seguinte */
	LIST	    *l_domain;	/* Domínio seguinte */
};

enum { DST_NULL, DST_SENT, DST_UNKNOWN };

#define	NOLIST	(LIST *)NULL

entry LIST	list_head;	/* Cabeça da lista */

/*
 ****** Protótipos de funções ***********************************
 */
void		read_rc_file (void);
void		analyse_domain_and_connect (const char *[]);
void		analyse_domain_and_relay (const char *, const char *[]);
void		connect_with_mail_server (const char *, LIST *);
void		connect_with_relay_server (const char *, LIST *);
int		try_to_send_text (LIST *, const char *, long, int preference);
void		write_header (void);
char		*fnrgets (char *, int, FILE *);
char		*strtoupper (char *);
int		mail_read_line (char *, int);
void		mail_write_line (const char *, ...);
void		save_text_file (FILE *);
void		log_msg (const char *, ...);
void		error (const char *, ...);
void		tcp_abort (void);
void		quit (int);
void		on_signal (int signo, ...);
void		help (void);

/*
 ****************************************************************
 *	Recebe ou envia cartas do correio eletrônico		*
 ****************************************************************
 */
void
main (int argc, const char *argv[])
{
	int		fd, opt, tcp_fd, len;
	char		*cp, *at;
	char		area[BLSZ];

	/*
	 *	Modo de chamar:
	 *
	 *		send_mail [-fav] <file_nm>
	 */

	/*
	 *	Analisa as opções
	 */
	while ((opt = getopt (argc, argv, "favH")) != EOF)
	{
		switch (opt)
		{
		    case 'f':			/* De um ".forward" */
			fflag++;
			break;

		    case 'a':			/* Envia para todos os servidores */
			aflag++;
			break;

		    case 'v':			/* Verbose */
			vflag++;
			break;

		    case 'H':			/* Help */
			help ();

		    default:			/* Erro */
			putc ('\n', stderr);
			help ();

		}	/* end switch */

	}	/* end while */

	argv += optind;
	argc -= optind;

	/*
	 *	Prepara alguns sinais
	 */
	signal (SIGHUP,  SIG_IGN);
	signal (SIGINT,  SIG_IGN);
	signal (SIGTERM, on_signal);

	/*
	 *	Verifica se é uma chamada de "mail_s" para ".forward"
	 */
	if (fflag)
	{
		if ((fd = open ("/var/adm/itnetlog", O_WRONLY|O_APPEND)) >= 0)
			error_fp = fdopen (fd, "w");

		if (error_fp == NOFILE)
			error_fp = fopen ("/dev/null", "w");

		vflag = 0;
	}
	else
	{
		error_fp = stderr;
	}

	/*
	 *	Abre o "log" para coletar o envio de cartas
	 */
	if ((fd = open ("/var/adm/maillog", O_WRONLY|O_APPEND)) >= 0)
		log_fp = fdopen (fd, "w");
	else
		log_fp = NOFILE;

	/*
	 *	Analisa o arquivo de configuração (se presente)
	 */
	read_rc_file ();

	/*
	 *	Abre o arquivo da carta
	 */
	if ((file_nm = *argv) == NOSTR)
		error ("$Não foi dado o nome do arquivo contendo a carta");

	if ((file_fp = fopen (file_nm, "r")) == NOFILE)
		error ("$*Não consegui abrir \"%s\"", file_nm);

	/*
	 *	Retira o remetente
	 */
	if (fnrgets (area, sizeof (area), file_fp) == NOSTR)
		error ("$Não obtive o remetente");

	if ((sender_nm = malloc (strlen (area) + 1)) == NOSTR)
		error (NOSTR);

	strcpy (sender_nm, area);

	if ((at = strchr (area, '@')) != NOSTR)
	{
		len = at - area;

		if ((sender_user_nm = malloc (len + 1)) == NOSTR)
			error (NOSTR);

		memcpy (sender_user_nm, area, len); sender_user_nm[len] = '\0';
	}

	/*
	 *	Retira a lista de destinatários
	 */
	for (EVER)
	{
		if (fnrgets (area, sizeof (area), file_fp) == NOSTR)
			error ("$Não obtive o destinatário");

		if (receiver_nm_sz <= receiver_nm_total)
		{
			receiver_nm_sz += 8;

			receiver_nm_list = realloc (receiver_nm_list, receiver_nm_sz * sizeof (char *));

			if (receiver_nm_list == NULL)
				error (NOSTR);
		}

		if (area[0] == '\0')
			{ receiver_nm_list[receiver_nm_total++] = NOSTR; break; }

		if ((cp = malloc (strlen (area) + 1)) == NOSTR)
			error (NOSTR);

		strcpy (cp, area);

		receiver_nm_list[receiver_nm_total++] = cp;
	}

	/*
	 *	Guarda o ponto do arquivo, onde começa a "carta"
	 */
	file_offset = ftell (file_fp);

	/*
	 *	Abre o "endpoint"
	 */
	if ((tcp_fd = t_open (tcp_dev_nm, O_RDWR, (T_INFO *)NULL)) < 0)
		error ("$*Não consegui abrir \"%s\"", tcp_dev_nm);

	global_tcp_fd = tcp_fd;

	/*
	 *	Obtém o nome do próprio computador
	 */
	cp = t_addr_to_node (tcp_fd, 0);

	if (cp[0] >= '0' && cp[0] <= '9')
		error ("$NÃO consegui obter o nome do próprio computador");

	strcpy (my_name, cp); cp = strchr (my_name, '.');

	if (cp == NOSTR)
		error ("$NÃO consegui obter o nome do próprio domínio");

	my_domain = cp + 1;

#ifdef	DEBUG
	if (vflag)
		printf ("Meu nome/domínio: %s/%s\n", my_name, my_domain);
#endif	DEBUG

	close (tcp_fd);

	/*
	 *	Conecta e envia para os diversos destinatários
	 */
	if (relay_host_nm == NOSTR)
		analyse_domain_and_connect (receiver_nm_list);
	else
		analyse_domain_and_relay (relay_host_nm, receiver_nm_list);

	/*
	 *	Se houve algum erro, salva o arquivo
	 */
	if (!fflag && exit_code > 0)
		save_text_file (file_fp);

	quit (exit_code);

}	/* end send_mail */

/*
 ****************************************************************
 *	Analisa o arquivo "/etc/.sendmailrc" (se presente)	*
 ****************************************************************
 */
void
read_rc_file (void)
{
	FILE		*rc_fp;
	char		area[128];

	if ((rc_fp = fopen (sendmailrc_nm, "r")) == NOFILE)
		return;

	while (fnrgets (area, sizeof (area), rc_fp) == area)
	{
		if (area[0] == '\0' || area[0] == '#')
			continue;

		if (strncmp (area, "relay=", 6) == 0)
		{
			if ((relay_host_nm = strdup (area + 6)) == NOSTR)
				error (NOSTR);
		}
	}

	fclose (rc_fp);

}	/* end read_rc_file */

/*
 ****************************************************************
 *	Procura domínios em comum entre os destinatários	*
 ****************************************************************
 */
void
analyse_domain_and_connect (const char *argv[])
{
	LIST			*dp, *new_dp;
	LIST			*np, *new_np;
	const char		*domain_nm, *dst_nm;

	/*
	 *	Insere todos os destinatários
	 */
	while (dst_nm = *argv++)
	{
		if ((domain_nm = strchr (dst_nm, '@')) == NOSTR)
		{
			error ("Faltando o \"@\" no nome do usuário");
			log_msg ("<%s>: Faltando o \"@\" em <%s>", sender_nm, dst_nm);
			exit_code++;
			continue;
		}

		domain_nm++;

		for (dp = &list_head; /* abaixo */; dp = dp->l_domain)
		{
			if   (dp->l_domain == NOLIST)
			{
				np = alloca (sizeof (LIST));

				np->l_code   = DST_NULL;
				np->l_nm     = dst_nm;
				np->l_dst    = NOLIST;
				np->l_domain = NOLIST;

				new_dp = alloca (sizeof (LIST));

			   /***	new_dp->l_code   = 0; ***/
				new_dp->l_nm     = domain_nm;
				new_dp->l_dst    = np;
			   	new_dp->l_domain = NOLIST;

				dp->l_domain = new_dp;
			}
			elif (!streq (dp->l_domain->l_nm, domain_nm))
			{
				continue;
			}
			else for (np = dp->l_domain; /* abaixo */; np = np->l_dst)
			{
				if   (np->l_dst == NOLIST)
				{
					new_np = alloca (sizeof (LIST));

					new_np->l_code   = DST_NULL;
					new_np->l_nm     = dst_nm;
				   	new_np->l_dst    = NOLIST;
					new_np->l_domain = NOLIST;

					np->l_dst = new_np;
				}
				elif (!streq (np->l_dst->l_nm, dst_nm))
				{
					continue;
				}

				break;
			}

			break;

		}	/* end percorrendo a lista de domínios */

	}	/* end for (EVER) */

	/*
	 *	Finalmente, conecta com cada domínio
	 */
#undef	LIST_DEBUG
#ifndef	LIST_DEBUG
	for (dp = list_head.l_domain; dp != NOLIST; dp = dp->l_domain)
		connect_with_mail_server (dp->l_nm, dp->l_dst);
#else
	for (dp = list_head.l_domain; dp != NOLIST; dp = dp->l_domain)
	{
		printf ("Domínio \"%s\":", dp->l_nm);

		for (np = dp->l_dst; np != NOLIST; np = np->l_dst)
			printf (" \"%s\"", np->l_nm);

		printf ("\n");
	}
	quit (0);
#endif	LIST_DEBUG

}	/* end analyse_domain_and_connect */

/*
 ****************************************************************
 *	Conecta com o servidor de um destinatário		*
 ****************************************************************
 */
void
connect_with_mail_server (const char *domain, LIST *dst_list)
{
	LIST			*np;
	int			tcp_fd;
	int			preference, sent = 0;
	long			server_addr;
	const char		*mail_server_nm;

	/*
	 *	Abre o "endpoint"
	 */
	if ((tcp_fd = t_open (tcp_dev_nm, O_RDWR, (T_INFO *)NULL)) < 0)
		error ("$*Não consegui abrir \"%s\"", tcp_dev_nm);

	global_tcp_fd = tcp_fd;

	if ((tcp_in = fdopen (tcp_fd, "r")) == NOFILE)
		error ("$*Não consegui obter um FILE para \"%s\"", tcp_dev_nm);

	if ((tcp_out = fdopen (tcp_fd, "w")) == NOFILE)
		error ("$*Não consegui obter um FILE para \"%s\"", tcp_dev_nm);

	if ((mail_server_nm = t_mail_to_node (tcp_fd, domain, 0, &preference)) == NOSTR)
		{ mail_server_nm = domain; domain = NOSTR; }

	/*
	 *	Examina os vários servidores disponíveis
	 */
	for (EVER)
	{
		if ((server_addr = t_node_to_addr (tcp_fd, mail_server_nm, (long *)NULL)) == -1)
		{
			error ("*Não consegui obter o endereço de \"%s\"", mail_server_nm);
			log_msg ("<%s>: Não consegui obter o endereço de \"%s\"", sender_nm, mail_server_nm);
		}
		else
		{
			if (try_to_send_text (dst_list, mail_server_nm, server_addr, preference) >= 0)
			{
				sent++;

				if (!aflag)
					break;
			}
		}

		/*
		 *	Verifica se tem outros
		 */
		if
		(	domain == NOSTR ||
			(mail_server_nm = t_mail_to_node (tcp_fd, domain, preference + 1, &preference)) == NOSTR
		)
		{
			if (!sent)
			{
				for (np = dst_list; np != NOLIST; np = np->l_dst)
				{
					error ("NÃO consegui enviar para \"%s\"", np->l_nm);
					log_msg ("<%s>: Não consegui enviar para <%s>", sender_nm, np->l_nm);
				}

				exit_code++;
			}
			break;
		}

	}	/* end for (EVER) */

	/*
	 *	Fecha o dispositivo
	 */
	fclose (tcp_in);
	fclose (tcp_out);
	t_close (tcp_fd);

}	/* end connect_with_mail_server */

/*
 ****************************************************************
 *	Faz uma lista única, para o retransmissor ("relay")	*
 ****************************************************************
 */
void
analyse_domain_and_relay (const char *relay_host, const char *argv[])
{
	LIST			*np, *new_np;
	const char		*dst_nm;

	/*
	 *	Insere todos os destinatários na mesma lista
	 */
	while (dst_nm = *argv++)
	{
		if (strchr (dst_nm, '@') == NOSTR)
		{
			error ("Faltando o \"@\" no nome do usuário");
			log_msg ("<%s>: Faltando o \"@\" em <%s>", sender_nm, dst_nm);
			exit_code++;
			continue;
		}

		for (np = &list_head; /* abaixo */; np = np->l_dst)
		{
			if   (np->l_dst == NOLIST)
			{
				new_np = alloca (sizeof (LIST));

				new_np->l_code   = DST_NULL;
				new_np->l_nm     = dst_nm;
			   	new_np->l_dst    = NOLIST;
				new_np->l_domain = NOLIST;

				np->l_dst = new_np;
			}
			elif (!streq (np->l_dst->l_nm, dst_nm))
			{
				continue;
			}

			break;
		}

	}	/* end for (EVER) */

	/*
	 *	Finalmente, conecta com o retransmissor
	 */
#ifndef	LIST_DEBUG
	connect_with_relay_server (relay_host, list_head.l_dst);
#else
	for (np = list_head.l_dst; np != NOLIST; np = np->l_dst)
			printf (" \"%s\"", np->l_nm);

	printf ("\n");

	quit (0);
#endif	LIST_DEBUG

}	/* end analyse_domain_and_relay */

/*
 ****************************************************************
 *	Conecta com o computador retransmissor ("relay")	*
 ****************************************************************
 */
void
connect_with_relay_server (const char *relay_host, LIST *dst_list)
{
	LIST			*np;
	int			tcp_fd;
	long			server_addr;

	/*
	 *	Abre o "endpoint"
	 */
	if ((tcp_fd = t_open (tcp_dev_nm, O_RDWR, (T_INFO *)NULL)) < 0)
		error ("$*Não consegui abrir \"%s\"", tcp_dev_nm);

	global_tcp_fd = tcp_fd;

	if ((tcp_in = fdopen (tcp_fd, "r")) == NOFILE)
		error ("$*Não consegui obter um FILE para \"%s\"", tcp_dev_nm);

	if ((tcp_out = fdopen (tcp_fd, "w")) == NOFILE)
		error ("$*Não consegui obter um FILE para \"%s\"", tcp_dev_nm);

	/*
	 *	Obtém o endereço do retransmissor e envia o texto
	 */
	if   ((server_addr = t_node_to_addr (tcp_fd, relay_host, (long *)NULL)) == -1)
	{
		error ("*Não consegui obter o endereço de \"%s\"", relay_host);
		log_msg ("<%s>: Não consegui obter o endereço de \"%s\"", sender_nm, relay_host);

		exit_code++;
	}
	elif (try_to_send_text (dst_list, relay_host, server_addr, -1) < 0)
	{
		for (np = dst_list; np != NOLIST; np = np->l_dst)
		{
			error ("NÃO consegui enviar para \"%s\"", np->l_nm);
			log_msg ("<%s>: Não consegui enviar para <%s>", sender_nm, np->l_nm);
		}

		exit_code++;
	}

	/*
	 *	Fecha o dispositivo
	 */
	fclose (tcp_in);
	fclose (tcp_out);
	t_close (tcp_fd);

}	/* end connect_with_relay_server */

/*
 ****************************************************************
 *	Tenta enviar o texto para um destinatário		*
 ****************************************************************
 */
int
try_to_send_text (LIST *dst_list, const char *mail_server_nm, long server_addr, int preference)
{
	int		c;
	LIST		*np;
	int		tcp_fd;
	int		code;
	const char	*dst_nm;
	T_BIND		bind;
	INADDR		addr;
	T_CALL		call;
	T_OPTMGMT	req_optmgmt, ret_optmgmt;
	TCP_OPTIONS	options;
	char		area[BLSZ];

	/*
	 *	Associa um endereço local
	 */
	tcp_fd = global_tcp_fd;

	bind.addr.len	 = sizeof (INADDR);
	bind.addr.maxlen = sizeof (INADDR);
	bind.addr.buf	 = &addr;

	bind.qlen	 = 0;

	addr.a_port = 0;

	if (t_bind (tcp_fd, &bind, &bind) < 0)
		{ error ("*Não consegui associar um endereço"); return (-1); }

	/*
	 *	Altera o "timeout" do TCP/IP
	 */
	req_optmgmt.flags = T_DEFAULT;

   /***	req_optmgmt.opt.buf    = (TCP_OPTIONS *)NULL; ***/
   /***	req_optmgmt.opt.len    = 0; ***/

	ret_optmgmt.opt.buf    = &options;
	ret_optmgmt.opt.maxlen = sizeof (TCP_OPTIONS);

	if (t_optmgmt (tcp_fd, &req_optmgmt, &ret_optmgmt) < 0)
		error ("$*Não consegui negociar (DEFAULT)");

	req_optmgmt.flags = T_NEGOTIATE;

	options.max_silence    = 120;	/* Apenas 2 minutos */

	req_optmgmt.opt.buf    = &options;
	req_optmgmt.opt.len    = sizeof (TCP_OPTIONS);

   /***	ret_optmgmt.opt.buf    = &options; ***/
   /***	ret_optmgmt.opt.maxlen = sizeof (TCP_OPTIONS); ***/

	if (t_optmgmt (tcp_fd, &req_optmgmt, &ret_optmgmt) < 0)
		error ("$*Não consegui negociar (NEGOCIATE)");

	/*
	 *	Dá o CONNECT
	 */
	addr.a_port	 = MAIL_PORT;
	addr.a_addr	 = server_addr;

	call.addr.len	 = sizeof (INADDR);
	call.addr.maxlen = sizeof (INADDR);
	call.addr.buf	 = &addr;

	if (vflag)
	{
		printf ("Tentando %s (%s", mail_server_nm, t_addr_to_str (server_addr));

		if (preference >= 0)
			printf (", %d", preference);

		printf (") ... ");

		fflush (stdout);
	}

	if (t_connect (tcp_fd, &call, &call) < 0)
	{
		if (errno == TLOOK)
		{
			switch (t_look (tcp_fd))
			{
			    case T_DISCONNECT:
				if (vflag)
					printf ("conexão recusada por \"%s\"\n", mail_server_nm);
				break;

			    case T_TIMEOUT:
				if (vflag)
					printf ("tempo de espera esgotado de \"%s\"\n", mail_server_nm);
				break;

			    default:
				goto print_error;
			}
		}
		else
		{
		    print_error:
			error ("*Não consegui estabelecer conexão com \"%s\"", mail_server_nm);
		}

		t_unbind (tcp_fd); return (-1);
	}

	if (vflag)
		printf ("conectado\n");

#ifdef	DEBUG
	if (vflag)
		printf ("port_no do servidor obtido: %d\n", ((INADDR *)call.addr.buf)->a_port);
#endif	DEBUG

	/*
	 *	Espera o Prefixo do servidor
	 */
	code = mail_read_line (area, BLSZ);

	if (code != 220)
		goto mail_server_error;

	/*
	 *	Processa o HELO
	 */
	mail_write_line ("HELO %s", my_name);

	code = mail_read_line (area, BLSZ);

	if (code != 250)
		goto mail_server_error;

	/*
	 *	Processa o MAIL FROM
	 */
	mail_write_line ("MAIL From: <%s>", sender_nm);

	code = mail_read_line (area, BLSZ);

	if (code != 250)
		goto mail_server_error;

	/*
	 *	Processa o RCPT TO
	 */
	for (np = dst_list; np != NOLIST; np = np->l_dst)
	{
		dst_nm = np->l_nm;

		mail_write_line ("RCPT To: <%s>", dst_nm);

		if ((code = mail_read_line (area, BLSZ)) < 0)
			goto mail_server_error;

		if (code != 250 && code != 251)
			{ np->l_code = DST_UNKNOWN; exit_code++; }
		else
			np->l_code = DST_SENT;
	}

	/*
	 *	Processa o DATA
	 */
	mail_write_line ("DATA");

	code = mail_read_line (area, BLSZ);

	if (code != 354)
		goto mail_server_error;

	/*
	 *	Envia o texto
	 */
	fseek (file_fp, file_offset, SEEK_SET);

	while ((c = getc (file_fp)) >= 0)
	{
		if   (c == '\r')
			continue;
		elif (c == '\n')
			putc ('\r', tcp_out);

		putc (c, tcp_out);
	}

	/*
	 *	Indica o final do texto
	 */
	mail_write_line (".");

	code = mail_read_line (area, BLSZ);

	if (code != 250)
		goto mail_server_error;

	/*
	 *	Processa o QUIT
	 */
	mail_write_line ("QUIT");

	code = mail_read_line (area, BLSZ);

	/*
	 *	Em caso de erro, ...
	 */
	if (code == 221)
	{
		for (np = dst_list; np != NOLIST; np = np->l_dst)
		{
			if (np->l_code == DST_SENT)
			{
				if (vflag)
					printf ("Carta enviada para \"%s\"\n", np->l_nm);

				log_msg ("<%s> => <%s>", sender_nm, np->l_nm);
			}
			else
			{
				error ("Destinatário desconhecido: \"%s\"", np->l_nm);
				log_msg ("<%s>: Destinatário desconhecido: <%s>", sender_nm, np->l_nm);
			}
		}
	}
	else
	{
	    mail_server_error:
		if (!vflag && code >= 0)
		{
			error ("%s", area);
			log_msg ("<%s>: %s", sender_nm, area);
		}

		t_snddis (tcp_fd, (T_CALL *)NULL);
		t_unbind (tcp_fd); return (-1);
	}

	/*
	 *	Termina a conexão
	 */
	if (t_sndrel (tcp_fd) < 0)
		error ("*Erro no SNDREL");

	if (t_rcvrel (tcp_fd) < 0)
		error ("*Erro no RCVREL");

	if (t_unbind (tcp_fd) < 0)
		error ("*Não consegui desassociar um endereço");

	return (0);

}	/* end try_to_send_text */

/*
 ****************************************************************
 *	Le uma linha de um arquivo (ignorando "\r")		*
 ****************************************************************
 */
char *
fnrgets (char *str, int n, FILE *fp)
{
	char		*cp = str;
	int		c = 0;

	while ((c = getc (fp)) >= 0)
	{
		if (c == '\r')
			continue;

		if (c == '\n')
			break;

		if (--n > 0)
			*cp++ = c;
	}

	if (cp == str && c < 0)
		return (NOSTR);

	*cp = '\0';

	return (str);

}	/* end fnrgets */

/*
 ****************************************************************
 *	Lê uma linha do TCP até um <nl>				*
 ****************************************************************
 */
int
mail_read_line (char *area, int count)
{
	char		*cp = area;
	const char	*end_area = cp + count - 1;
	const char	*str;
	int		c;
	int		code;

	/*
	 *	Retorna o código SMTP
	 */
	for (EVER)
	{
		c = getc (tcp_in);

		if (c < 0)	/* EOF ou erro */
			return (-1);

		if (c == '\r')	/* Ignora <cr> */
			continue;

		if (c != '\n')
		{
			if (cp < end_area)
				*cp++ = c;

			continue;
		}

		/* Terminou a linha */

		*cp = '\0';

		if (vflag)
			printf ("%s\n", area);

		code = strtol (area, &str, 10);

		if (*str != '-')
			return (code);

		/* Linha do tipo "123-" */

		cp = area;

	}	/* end for (EVER) */

}	/* end mail_read_line */

/*
 ****************************************************************
 *	Escreve uma linha de TCP				*
 ****************************************************************
 */
void
mail_write_line (const char *format, ...)
{
	va_list		args;

	va_start (args, format);

	if (vflag)
	{
		printf (">>> ");
		vprintf (format, args);
		printf ("\n");
	}

	vfprintf (tcp_out, format, args);
	fprintf  (tcp_out, "\r\n");
	fflush   (tcp_out); t_push (global_tcp_fd);

	va_end (args);

}	/* end mail_write_line */

/*
 ****************************************************************
 *	Salva o arquivo em "dead.letter"			*
 ****************************************************************
 */
void
save_text_file (FILE *fp)
{
	char		*dead_letter_nm;
	FILE		*dead_fp;
	const PASSWD	*pwd;
	int		c;

	/*
	 *	Obtém o nome do arquivo
	 */
	if (sender_user_nm == NOSTR)
		return;

	if ((pwd = getpwnam (sender_user_nm)) == NOPASSWD)
		{ endpwent (); return; }

	dead_letter_nm = alloca (strlen (pwd->pw_dir) + strlen (dead_letter) + 1);
	strcpy (dead_letter_nm, pwd->pw_dir);
	strcat (dead_letter_nm, dead_letter);

	endpwent ();

	/*
	 *	Abre o arquivo
	 */
	if ((dead_fp = fopen (dead_letter_nm, "a")) == NOFILE)
	{
		error ("*NÃO consegui abrir o arquivo \"%s\"", dead_letter_nm);
		return;
	}

	fseek (file_fp, file_offset, SEEK_SET);

	while ((c = getc (fp)) >= 0)
	{
		if (c == '\r')	/* Ignora <cr> */
			continue;

		putc (c, dead_fp);
	}

	fclose (dead_fp);

	error ("O texto foi salvo em \"%s\"", dead_letter_nm);

}	/* end save_text_file */

/*
 ****************************************************************
 *	Escreve uma mensagem no arquivo do "mail"		*
 ****************************************************************
 */
void
log_msg (const char *format, ...)
{
	va_list		args;
	const char	*fmt = format;
	time_t		time_loc;

	if (log_fp == NOFILE)
		return;

	va_start (args, format);

	fprintf  (log_fp, pgname);

	time (&time_loc);
	fprintf (log_fp, " (%.24s): ", btime (&time_loc));

	vfprintf (log_fp, fmt, args);
	fprintf (log_fp, "\n");

	va_end (args);

}	/* end log_msg */

/*
 ****************************************************************
 *	Imprime uma mensagem de erro (cliente)			*
 ****************************************************************
 */
void
error (const char *format, ...)
{
	va_list		args;
	const char	*fmt = format;
	char		err, abrt;
	time_t		time_loc;
	int		event, save_errno = errno;

	va_start (args, format);

	if (fmt == NOSTR)
		fmt = "$*Erro de entrada/saída de TCP";

	if ((abrt = *fmt) == '$')
		fmt++;

	if ((err = *fmt) == '*')
		fmt++;

	fprintf  (error_fp, pgname);

	time (&time_loc);
	fprintf (error_fp, " (%.24s): ", btime (&time_loc));

	vfprintf (error_fp, fmt, args);

	if (err == '*')
	{
		if (save_errno == TLOOK)
		{
			event = t_look (global_tcp_fd);
	
			fprintf (error_fp, " (%s)", t_strevent (global_tcp_fd, event));
		}
		elif (save_errno != 0)
		{
			fprintf (error_fp, " (%s)", strerror (save_errno));
		}
	}

	fprintf (error_fp, "\r\n");

	if (abrt == '$')
		tcp_abort ();

	va_end (args);

}	/* end error */

/*
 ****************************************************************
 *	Encerra as atividades (desconectando o TCP)		*
 ****************************************************************
 */
void
tcp_abort (void)
{
	t_snddis (global_tcp_fd, (T_CALL *)NULL);

	quit (1);

}	/* end tcp_abort */

/*
 ****************************************************************
 *	Restaura o estado do terminal e termina			*
 ****************************************************************
 */
void
quit (int code)
{
	if (file_nm != NOSTR && unlink (file_nm) < 0)
		error ("*NÃO consegui remover \"%s\"", file_nm);

	exit (code);

}	/* end quit */

/*
 ****************************************************************
 *	Para alguns sinais, ...					*
 ****************************************************************
 */
void
on_signal (int signo, ...)
{
	tcp_abort ();

}	/* end on_signal */

/*
 ****************************************************************
 *	Resumo de utilização do programa			*
 ****************************************************************
 */
void
help (void)
{
	fprintf
	(	stderr,
		"%s - interface para enviar cartas do correio eletrônico\n"
		"\n%s\n"
		"\nSintaxe:\n"
		"\t%s [-fav] <carta>\n",
		pgname, pgversion, pgname
	);

	fprintf
	(	stderr,
		"\nOpções:"
		"\t-f: A <carta> já contém o <cabeçalho> (para \"forward\")\n"
		"\t-a: Envia para todos os servidores disponíveis\n"
		"\t-v: Verboso\n"
	);

	exit (2);

}	/* end help */
