#*
#****************************************************************
#*	Makefile para o programa "boot0"			*
#****************************************************************
#*
DIR	= /usr/bin
PGM	= boot0
PROT	= a+x
MAN	= cmd
CPDIR	= /etc/boot

.virtual	all cp cmp modif gar gara sym cpfd0 cmpfd0 cpfd1 cmpfd1

all:		h.$PGM	d.$PGM

#
#	Valores de BUNIT: "0"    para disquette (3 1/2 ou 5 1/4)
#			  "0x80" para disco IDE
#
h.$PGM:		$PGM.s
		as -D BUNIT=0x80 -l h.$PGM.l -o h.$PGM.o $PGM.s
		ld -T 0xC000 h.$PGM.o
#		size a.out
		strip a.out
		dd if=a.out of=h.$PGM bs=80 skip=1
		ls -l h.$PGM
		rm -f h.$PGM.o a.out

d.$PGM:		$PGM.s
		as -D BUNIT=0 -l d.$PGM.l -o d.$PGM.o $PGM.s
		ld -T 0xC000 d.$PGM.o
#		size a.out
		strip a.out
		dd if=a.out of=d.$PGM bs=80 skip=1
		ls -l d.$PGM
		rm -f d.$PGM.o a.out

sym:		$PGM.s
		as -a -D BUNIT=0 -l $PGM.l -o $PGM.o $PGM.s

cp:		h.$PGM d.$PGM $CPDIR
		cp h.$PGM d.$PGM $CPDIR

cmp:		$CPDIR
		-cmp h.$PGM d.$PGM $CPDIR

$CPDIR:		.exist
		mkdir $CPDIR

cpfd0:		d.$PGM
		dd if=d.$PGM of=/dev/fd0
		cmp -p d.$PGM /dev/fd0

cmpfd0:
		cmp -p d.$PGM /dev/fd0

cpfd1:		d.$PGM
		dd if=d.$PGM of=/dev/fd1
		cmp -p d.$PGM /dev/rfd1

cmpfd1:
		cmp -p d.$PGM /dev/rfd1

modif:		modif.r
		sroff modif.r >$mandir/modif/$MAN/$PGM
		modif $PGM
gar:
		cd ..; gar -ovs0 $PGM
		cd ..; gar -icv0

gara:
		cd ..; gar -avs0 $PGM
		cd ..; gar -icv0
