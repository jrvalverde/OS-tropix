#*
#****************************************************************
#*	Makefile para o cliente "xfm"				*
#****************************************************************
#*
DIR	=	/usr/xwin/bin
PGM	=	xfm
PROT	=	a+x
MAN	=	xwin

CONF	=	/usr/xwin/conf/Xt

INC	=	-I /usr/xwin/include

OBJ	=	obj/$PGM.o			\
		obj/Fsb.o			\
		obj/FsBrowser.o			\
		obj/FsBrowserF.o		\
		obj/SimpleMenu.o		\
		obj/TMoreOrLess.o		\
		obj/TTree.o			\
		obj/TTreeF.o			\
		obj/gif.o

CC	=	cc

.virtual	lint cp cmp cmpq cmpqcp man modif

#*
#****************************************************************
#	Link-edição final					*
#****************************************************************
#*
$PGM:		obj $OBJ
		$CC -s -o $PGM $OBJ -lx.o -ly.o
		size $PGM
		ls -l $PGM

obj:		.exist
		mkdir -f obj

#*
#****************************************************************
#	Compilação dos Diversos Módulos				*
#****************************************************************
#*
obj/$PGM.o:		src/$PGM.c h/FsBrowser.h
			$CC $INC -c -O -o obj src/$PGM.c

obj/SimpleMenu.o:	src/SimpleMenu.c
			$CC $INC -c -O -o obj src/SimpleMenu.c

obj/TMoreOrLess.o:	src/TMoreOrLess.c h/TMoreOrLess.h h/TMoreOrLessP.h
			$CC $INC -c -O -o obj src/TMoreOrLess.c

obj/TTree.o:		src/TTree.c h/TTree.h h/TTreeP.h
			$CC $INC -c -O -o obj src/TTree.c

obj/TTreeF.o:		src/TTreeF.c h/TTree.h h/TTreeP.h
			$CC $INC -c -O -o obj src/TTreeF.c

obj/Fsb.o:		src/Fsb.c h/Fsb.h h/FsbP.h
			$CC $INC -c -O -o obj src/Fsb.c

obj/FsBrowser.o:	src/FsBrowser.c h/FsBrowser.h h/FsBrowserP.h
			$CC $INC -c -O -o obj src/FsBrowser.c

obj/FsBrowserF.o:	src/FsBrowserF.c h/FsBrowser.h h/FsBrowserP.h
			$CC $INC -c -O -o obj src/FsBrowserF.c

obj/gif.o:		src/gif.c h/colormap.h
			$CC $INC -c -O -o obj src/gif.c

lint:
			$CC $INC -T src/$PGM.c
			$CC $INC -T src/Fsb.c
			$CC $INC -T src/FsBrowser.c
			$CC $INC -T src/FsBrowserF.c
			$CC $INC -T src/SimpleMenu.c
			$CC $INC -T src/TMoreOrLess.c
			$CC $INC -T src/TTree.c
			$CC $INC -T src/TTreeF.c
			$CC $INC -T src/gif.c

#*
#****************************************************************
#	Cópia/Comparação					*
#****************************************************************
#*
cp:			$DIR $CONF $PGM
			cp -f $PGM $DIR
			chmod $PROT $DIR/$PGM
			cp -f Xfm.ad.v $CONF/Xfm
			chmod a=r $CONF/Xfm
			cp -f xfm.*.gif /usr/xwin/gifmaps
			chmod a=r /usr/xwin/gifmaps/xfm.*.gif
			ls -L $DIR/$PGM
			ls -L $CONF/Xfm
			ls -L /usr/xwin/gifmaps/xfm.*.gif
			sync

$DIR:			.exist
			mkdir -f $DIR

$CONF:			.exist
			mkdir -f $CONF

cmp:
			-cmpobj $PGM $DIR
			-cmp Xfm.ad.v $CONF/Xfm

cmpq:
			-cmpobj -q $PGM $DIR

cmpqcp:
			-cmpobj -q $PGM $DIR || make cp

#*
#****************************************************************
#	Manuais							*
#****************************************************************
#*
man:			man.r
			sroff man.r >$mandir/$MAN/$PGM
			chmod a+r $mandir/$MAN/$PGM
			man $PGM

modif:			modif.r
			sroff modif.r >$mandir/modif/$MAN/$PGM
			modif $PGM
