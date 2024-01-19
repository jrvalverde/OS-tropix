#*
#****************************************************************
#*	Makefile para o programa "init"				*
#****************************************************************
#*
ROOT	=
DIR	= $ROOT/etc
PGM	= init
PROT	= u=rwx
MAN	= cmd
CC	= cc

.virtual	unld lint cp cmp cmpq cmpqcp man modif prman gar gara

$PGM:		$PGM.c
		$CC -u -s -O -o $PGM $PGM.c
		size $PGM
		ls -l $PGM

unld:
		-rm -f $PGM

lint:
		$CC -T $PGM.c

cp:		$PGM
		-mv -f $DIR/$PGM $DIR/old.$PGM
		cp -f $PGM $DIR/$PGM
		chmod $PROT $DIR/$PGM
		ls -L $DIR/*$PGM

cmp:
		-cmpobj $PGM $DIR/$PGM

cmpq:
		-cmpobj -q $PGM $DIR

cmpqcp:
		-cmpobj -q $PGM $DIR || make cp

man:		man.r
		sroff man.r >$mandir/$MAN/$PGM
		chmod a+r $mandir/$MAN/$PGM
		man $PGM

modif:		modif.r
		sroff modif.r >$mandir/modif/$MAN/$PGM
		modif $PGM

prman:
		sroff man.r | lpr

gar:
		cd ..; gar -ovs0 $PGM
		cd ..; gar -icv0

gara:
		cd ..; gar -avs0 $PGM
		cd ..; gar -icv0
