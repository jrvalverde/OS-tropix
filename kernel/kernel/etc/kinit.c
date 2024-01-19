/*
 ****************************************************************
 *								*
 *			kinit.c					*
 *								*
 *	Inicialização do KERNEL					*
 *								*
 *	Versão	3.0.0, de 29.08.94				*
 *		4.6.0, de 08.11.04				*
 *								*
 *	Módulo: Núcleo						*
 *		NÚCLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright © 2004 NCE/UFRJ - tecle "man licença"	*
 *								*
 ****************************************************************
 */

#include "../h/common.h"
#include "../h/bcb.h"
#include "../h/scb.h"
#include "../h/sync.h"
#include "../h/region.h"

#include "../h/a.out.h"
#include "../h/seg.h"
#include "../h/tss.h"
#include "../h/intr.h"
#include "../h/disktb.h"
#include "../h/video.h"
#include "../h/signal.h"
#include "../h/uproc.h"

#include "../h/proto.h"
#include "../h/extern.h"

/*
 ******	Variáveis globais ***************************************
 */
#define	CLOCK_IRQ	0	 /* No. do IRQ do relógio */
#define	CLOCK_PL	7	 /* Nível de prioridade da interrupção */

#define MILHAO	 1000000	/* No. de microsegundos por segundo */
#define PIT_FREQ 1193182	/* Freqüência do relógio do PIT */

entry BCB	bcb; 	/* A estrutura herdada do BOOT */

entry DISKTB	*disktb,	/* Início da tabela de discos */
		*next_disktb,	/* Próxima entrada livre da tabela  de discos */
		*end_disktb;	/* Final da tabela  de discos */

entry TSS	tss;	/* O segmento de estado da "task" do 486 */

entry char	preemption_flag[NCPU];	/* Habilita troca de proc. em modo SUP */

entry char	fpu_present; 	/* Inicializado no fpu.s */

entry int	PIT_init;	/* Valor inicial do contador do PIT */

entry long	DELAY_value;	/* Valor para DELAY */

/*
 ****** Referências externas ************************************
 */
extern void	div_vector (),		debug_vector (),
		nmi_vector (), 		breakpoint_vector (),
		overflow_vector (),	bound_vector (),
		inv_opcode_vector (),	coprocessor_vector (),
		double_fault_vector (),	segment_overrun_vector (),
		invalid_tss_vector (),	segment_not_present_vector (),
		stack_vector (),	general_protection_vector (),
		page_fault_vector (),	reser_15_vector (),
		coprocessor_error_vector (), alignment_vector (),
		reser_18_vector (),	reser_19_vector (),
		reser_20_vector (),	reser_21_vector (),
		reser_22_vector (),	reser_23_vector (),
		reser_24_vector (),	reser_25_vector (),
		reser_26_vector (),	reser_27_vector (),
		reser_28_vector (),	reser_29_vector (),
		reser_30_vector (),	reser_31_vector ();

extern void	spurious_32_vector (),	spurious_33_vector (),
		spurious_34_vector (),	spurious_35_vector (),
		spurious_36_vector (),	spurious_37_vector (),
		spurious_38_vector (),	spurious_39_vector (),
		spurious_40_vector (),	spurious_41_vector (),
		spurious_42_vector (),	spurious_43_vector (),
		spurious_44_vector (),	spurious_45_vector (),
		spurious_46_vector (),	spurious_47_vector (),
		spurious_255_vector ();

/*
 ******	Vetor para a inicialização ******************************
 */
const void (* const vector_vec[]) () =
{
		div_vector,		debug_vector,
		nmi_vector, 		breakpoint_vector,
		overflow_vector,	bound_vector,
		inv_opcode_vector,	coprocessor_vector,
		double_fault_vector,	segment_overrun_vector,
		invalid_tss_vector,	segment_not_present_vector,
		stack_vector,		general_protection_vector,
		page_fault_vector,	reser_15_vector,
		coprocessor_error_vector, alignment_vector,
		reser_18_vector,	reser_19_vector,
		reser_20_vector,	reser_21_vector,
		reser_22_vector,	reser_23_vector,
		reser_24_vector,	reser_25_vector,
		reser_26_vector,	reser_27_vector,
		reser_28_vector,	reser_29_vector,
		reser_30_vector,	reser_31_vector,

		spurious_32_vector,	spurious_33_vector,
		spurious_34_vector,	spurious_35_vector,
		spurious_36_vector,	spurious_37_vector,
		spurious_38_vector,	spurious_39_vector,
		spurious_40_vector,	spurious_41_vector,
		spurious_42_vector,	spurious_43_vector,
		spurious_44_vector,	spurious_45_vector,
		spurious_46_vector,	spurious_47_vector
};

/*
 ****************************************************************
 *   Obtém as informações do BOOT (enquanto ainda acessíveis)	*
 ****************************************************************
 */
void
get_bcb (BCB *bi)
{
	BCB		*bp = &bcb;
	DISKTB		*dp;
	int		n_sym = 0, disktb_sz, sz;
	const SYM	*sp, *endsymtb;

	/*
	 *	Em primeiro lugar, confere a versão e copia a estrutura BCB
	 */
	memmove (bp, bi, sizeof (BCB));

	video_vt100->vt100_pos = bp->y_video_pos;

	end_base_bss += bp->y_ssize;
	phys_mem      = bp->y_physmem;

	scb.y_DELAY_value = DELAY_value = bp->y_DELAY_value;

	if (bp->y_version != BCB_VERSION && bp->y_version != BCB_VERSION - 1)
	{
		printf
		(	"\nA versão do BCB NÃO confere (%d :: " XSTR (BCB_VERSION) ")\n",
			bi->y_version
		);

		for (EVER)
			/* vazio */;
	}

	/*
	 *	Copia a "disktb" (já deixando espaço
	 *	para os 2 "ramd"s, 3 "meta_dos" e 32 "extras")
	 */
	disktb_sz = bp->y_disktb_sz + (2 + 3 + 32) * sizeof (DISKTB);

	if (end_base_bss + disktb_sz >= (void *)BASE_END)
	{
		disktb	      = end_ext_bss;	/* Acima de 1MB */
		end_ext_bss  += disktb_sz;
		end_disktb    = end_ext_bss;
	}
	else
	{
		disktb	      = end_base_bss;	/* Cabe abaixo de 640Kb */
		end_base_bss += disktb_sz;
		end_disktb    = end_base_bss;
	}

	memmove (disktb, bp->y_disktb, bp->y_disktb_sz);

	for (dp = disktb; dp->p_name[0] != '\0'; dp++)
		/* vazio */;

	next_disktb = dp;	/* Aponta para a primeira entrada vazia */

	/*
	 *	Prepara a área de "dmesg"
	 */
	if (end_base_bss + scb.y_dmesg_sz >= (void *)BASE_END)
	{
		dmesg_area    = 
		dmesg_ptr     = end_ext_bss;
		end_ext_bss  += scb.y_dmesg_sz;
		dmesg_end     = end_ext_bss;
	}
	else
	{
		dmesg_area    = 
		dmesg_ptr     = end_base_bss;
		end_base_bss += scb.y_dmesg_sz;
		dmesg_end     = end_base_bss;
	}

	memclr (dmesg_area, scb.y_dmesg_sz);

	/*
	 *	Tenta obter o DMESG do boot
	 */
	if (bp->y_version == BCB_VERSION)
	{
		if ((sz = bcb.y_dmesg_ptr - bcb.y_dmesg_area) > scb.y_dmesg_sz)
			sz = scb.y_dmesg_sz;

		memmove (dmesg_area, bcb.y_dmesg_area, sz); dmesg_ptr += sz;
	}

	/*
	 *	Arredonda os dois finais para páginas
	 */
	end_base_bss = (void *)PGROUND (end_base_bss);
	end_ext_bss  = (void *)PGROUND (end_ext_bss);

	/*
	 *	Verifica se a tabela de símbolos está correta
	 */
	endsymtb = (SYM *)((char *)&end + bcb.y_ssize);

	for (sp = (SYM *)&end; sp < endsymtb; sp = SYM_NEXT_PTR (sp))
		n_sym++;

	if (sp != endsymtb)
		printf ("****** Defasamento na tabela de símbolos\n");

	/*
	 *	Imprime o "bcb"
	 */
#ifdef	DEBUG
	printf ("Tabela de símbolos com %d símbolos\n", n_sym);

	printf ("Tipo da CPU = %d\n", bp->y_cputype);

	printf ("basemem = %d\n", bp->y_basemem);
	printf ("extmem  = %d\n", bp->y_extmem);

	printf ("NR_WD   = %d\n", bp->y_NR_WD);

	printf ("ssize   = %d\n", bp->y_ssize);
	printf ("video_pos = %d\n", bp->y_video_pos);

	printf ("disktb   = %P\n", bp->y_disktb);
	printf ("disktb_sz = %d\n", bp->y_disktb_sz);

	getchar ();
#endif	DEBUG

}	/* end get_bcb */

/*
 ****************************************************************
 *	Inicializa os vetores de interrupções			*
 ****************************************************************
 */
void
init_idt (void)
{
	int 		i;

	/*
	 *	Inicializa as exceções 0 - 31
	 */
	for (i = 0; i < 32; i++)
		set_idt (i, vector_vec[i], TRAP_GATE, KERNEL_PL);

	/*
	 *	A exceção 3 tem prioridade de usuário
	 */
	set_idt (3, vector_vec[3], TRAP_GATE, USER_PL);

	/*
	 *	Inicializa as interrupções 32 - 48
	 */
	for (i = 32; i < 48; i++)
		set_idt (i, vector_vec[i], INTR_GATE, KERNEL_PL);

	/*
	 *	Inicializa as interrupções 48 - 255
	 */
	for (i = 48; i < 256; i++)
		set_idt (i, spurious_255_vector, INTR_GATE, KERNEL_PL);

}	/* end init_idt */

/*
 ****************************************************************
 *	Programa o controlador de interrupções 8259		*
 ****************************************************************
 */
void
init_irq (void)
{
#define	IO_ICU1		0x20
#define	IO_ICU2		0xA0

	/*
	 *	initialize 8259's
	 */
	write_port (0x11, IO_ICU1);		/* reset; program device, four bytes */
	write_port (32,	  IO_ICU1+1);		/* starting at this vector index */
	write_port (1<<2, IO_ICU1+1);		/* slave on line 2 */
	write_port (1,    IO_ICU1+1);		/* 8086 mode */
	write_port (0xFF, IO_ICU1+1);		/* leave interrupts masked */
	write_port (2,    IO_ICU1);		/* default to ISR on read */

	write_port (0x11, IO_ICU2);		/* reset; program device, four bytes */
	write_port (32+8, IO_ICU2+1);		/* starting at this vector index */
	write_port (2,    IO_ICU2+1);		/* my slave id is 2 */
	write_port (1,    IO_ICU2+1);		/* 8086 mode */
	write_port (0xFF, IO_ICU2+1);		/* leave interrupts masked */
	write_port (2,    IO_ICU2);		/* default to ISR on read */

}	/* end init_irq */

/*
 ****************************************************************
 *	Inicializa o TSS e SYSCALL				*
 ****************************************************************
 */
void
init_syscall (void)
{
	ulong		func; 
	EXCEP_DC	*ip;

	tss.tss_sp0   = (void *)&u + UPROCSZ;
	tss.tss_ss0   = KERNEL_DS;
	tss.tss_iomap = 0xDFFF << 16;

	ip = &gdt[SYS_CALL >> 3];

	func = (ulong)syscall_vector;
	ip->ed_low_offset  = func;
	ip->ed_high_offset = func >> 16;

}	/* end init_syscall */

/*
 ****************************************************************
 *	Inicia os relógios					*
 ****************************************************************
 */
void
init_clock (void)
{
   	extern void	clock_int (); 

	PIT_init = mul_div_64 (PIT_FREQ, 1, scb.y_hz);

	write_port (0x34, 0x43);
	write_port (PIT_init,	   0x40);
	write_port (PIT_init >> 8, 0x40);

	/*
	 *	Inicializa o tempo
	 */
	init_time_from_cmos ();

	/*
	 *	Prepara a interrupção
	 */
	if (set_dev_irq (CLOCK_IRQ, CLOCK_PL, 0, clock_int) < 0)
	{
		printf
		(	"init_clock: O IRQ %d "
			"já está sendo usado por outro dispositivo\n",
			CLOCK_IRQ
		);
	   /***	return (-1); ***/
	}

	spl0 ();

}	/* end init_clock */

/*
 ****************************************************************
 *	Inicialização dependente do processador			*
 ****************************************************************
 */
void
cpusetup (void)
{
	int		i;
	SCB		*sp = &scb;

	/*
	 *	Processa a identificação do sistema
	 */
	strcpy (sp->y_uts.uts_sysname,	"TROPIX");
	strcpy (sp->y_uts.uts_systype,	"INTEL x86");

	strcpy (sp->y_uts.uts_machine,	"PC");
	strcpy (sp->y_uts.uts_customer,	"");
	strcpy (sp->y_uts.uts_depto,	"");
	strcpy (sp->y_uts.uts_sysserno,	"");

	printf
	(	"\n%s %s - Versão %s de %s às %s em um %s (%s)\n",
		sp->y_uts.uts_sysname,	sp->y_uts.uts_systype,
		sp->y_uts.uts_version, 	sp->y_uts.uts_date,
		sp->y_uts.uts_time,	sp->y_uts.uts_machine,
		sp->y_uts.uts_nodename
	);

	/*
	 *	Imprime o cabeçalho variável e o padrão
	 */
	printf ("\e[31m");
	printf ("\nCopyright © 1988-2003 NCE/UFRJ\n\n");
	printf ("O sistema operacional TROPIX é distribuído ABSOLUTAMENTE SEM GARANTIA.\n");
	printf ("Este é um \"software\" livre, e você é bem-vindo para redistribuí-lo sob\n");
	printf ("certas condições; para detalhes, tecle \"man licença\".\n");
	printf	("\e[0m");

	/*
	 *	Conta o número de processadores
	 */
	i = 1; scb.y_cputype = bcb.y_cputype;

#if (0)	/*******************************************************/
	printf ("\nNo. de CPUs Operacionais = %d ", i);

	switch (scb.y_cputype = bcb.y_cputype)
	{
	    case 386:
		printf ("(INTEL 386)\n\n");
		break;

	    case 486:
		printf ("(INTEL 486)\n\n");
		break;

	    case 586:
		printf ("(INTEL PENTIUM)\n\n");
		break;

	    default:
		printf ("\n\nCódigo de CPU inválido: 0x%x\n", scb.y_cputype);

		for (EVER)
			;

	}	/* end switch */

	if (i < 1)
	{
		printf ("MÁGICA: Funcionando sem CPUs\n");

		for (EVER)
			;
	}
#endif	/*******************************************************/

	scb.y_ncpu = i;
   /***	scb.y_initpid = i + 1; ***/

   /***	pidsource = 0; ***/

	/*
	 *	Verifica se o Coprocessador existe
	 */
	fpu_test ();

#if (0)	/*******************************************************/
	printf ("Processador Numérico: ");
	printf (fpu_present == 0 ? "AUSENTE\n\n" : "PRESENTE\n\n");
#endif	/*******************************************************/

	/*
	 *	Verifica se deve desligar o coprocessador
	 */
	if (fpu_present)
		sp->y_fpu_enabled = 1;

}	/* end cpustartup */

#if (0)	/*******************************************************/
/*
 ****************************************************************
 *	x			*
 ****************************************************************
 */
void
nt_bit_on (void)
{
	printf ("Bit NT ligado!, processo %d\n", u.u_proc->p_pid);

}	/* end nt_bit_on */
#endif	/*******************************************************/
