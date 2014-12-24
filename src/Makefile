SHELL=/bin/sh
YACC     = yacc
LEX      = lex


# these are the names of the output files from YACC/LEX.
YTABC = ../util/y.tab.c
YTABH = ../util/y.tab.h
LEXYYC = ../util/lex.yy.c


# timestamps for primary header files, matching src/Makefile
CONFIG_H = config.h-t
HACK_H	 = hack.h-t

# files that define all monsters and objects
OMONOBJ = monst.o objects.o
# files that provide access to NetHack's names
ONAMING = drawing.o decl.o $(OMONOBJ)
# dynamic memory allocation
OALLOC = alloc.o ../util/panic.o

# object files for makedefs
MAKEOBJS = ../util/makedefs.o $(OMONOBJ)

# object files for special levels compiler
SPLEVOBJS = ../util/lev_yacc.o ../util/lev_lex.o ../util/lev_main.o $(OALLOC) $(ONAMING)

# object files for dungeon compiler
DGNCOMPOBJS = ../util/dgn_yacc.o ../util/dgn_lex.o ../util/dgn_main.o $(OALLOC)

# object files for recovery utility
RECOVOBJS = ../util/recover.o

# object files for the data librarian
DLBOBJS = ../util/dlb_main.o dlb.o $(OALLOC)

../util/makedefs:	$(MAKEOBJS)
	$(CC) $(LFLAGS) -o $@ $(MAKEOBJS)

../util/makedefs.o: ../util/makedefs.c $(CONFIG_H) ../include/permonst.h \
		../include/objclass.h ../include/monsym.h \
		../include/artilist.h ../include/dungeon.h ../include/obj.h \
		../include/monst.h ../include/you.h ../include/flag.h \
		../include/dlb.h ../include/patchlevel.h ../include/qtext.h

../include/onames.h: ../util/makedefs
	../util/./makedefs -o
../include/pm.h: ../util/makedefs
	../util/./makedefs -p
monstr.c: ../util/makedefs
	../util/./makedefs -m
../include/vis_tab.h: ../util/makedefs
	../util/./makedefs -z
# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
vis_tab.c: ../include/vis_tab.h


# support code used by several of the utility programs (but not makedefs)
../util/panic.o:     ../util/panic.c $(CONFIG_H)


../util/lev_comp:  $(SPLEVOBJS)
	$(CC) -o $@ $(SPLEVOBJS)

../util/lev_yacc.o:  ../util/lev_yacc.c $(HACK_H) ../include/sp_lev.h
../util/lev_main.o:  ../util/lev_main.c $(HACK_H) ../include/sp_lev.h ../include/tcap.h ../include/date.h

# see lev_comp.l for WEIRD_LEX discussion
# egrep will return failure if it doesn't find anything, but we know there
# is one "_cplusplus" inside a comment
../util/lev_lex.o:   ../util/lev_lex.c $(HACK_H) ../include/lev_comp.h ../include/sp_lev.h
	$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus ../util/lev_lex.c` -o $@ ../util/lev_lex.c

../include/lev_comp.h: ../util/lev_yacc.c

../util/lev_yacc.c: ../util/lev_comp.y
	(cd ../util && $(YACC) -d ../util/lev_comp.y)
	mv $(YTABC) ../util/lev_yacc.c
	mv $(YTABH) ../include/lev_comp.h

../util/lev_lex.c: ../util/lev_comp.l
	(cd ../util && $(LEX) ../util/lev_comp.l)
	mv $(LEXYYC) ../util/lev_lex.c


../util/dgn_comp:  $(DGNCOMPOBJS)
	$(CC) $(LFLAGS) -o $@ $(DGNCOMPOBJS)

../util/dgn_yacc.o:  ../util/dgn_yacc.c $(CONFIG_H) ../include/dgn_file.h ../include/date.h
../util/dgn_main.o:  ../util/dgn_main.c $(CONFIG_H) ../include/dlb.h

# see dgn_comp.l for WEIRD_LEX discussion
../util/dgn_lex.o:   ../util/dgn_lex.c $(CONFIG.H) ../include/dgn_comp.h ../include/dgn_file.h
	$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus ../util/dgn_lex.c` -o $@ ../util/dgn_lex.c


../include/dgn_comp.h: ../util/dgn_yacc.c

../util/dgn_yacc.c: ../util/dgn_comp.y
	(cd ../util && $(YACC) -d ../util/dgn_comp.y)
	mv $(YTABC) ../util/dgn_yacc.c
	mv $(YTABH) ../include/dgn_comp.h

../util/dgn_lex.c: ../util/dgn_comp.l
	(cd ../util && $(LEX) ../util/dgn_comp.l)
	mv $(LEXYYC) ../util/dgn_lex.c


../util/recover: $(RECOVOBJS)
	$(CC) $(LFLAGS) -o $@ $(RECOVOBJS)

../util/recover.o: ../util/recover.c $(CONFIG_H) ../include/date.h


../util/dlb:	$(DLBOBJS)
	$(CC) $(LFLAGS) -o $@ $(DLBOBJS)

../util/dlb_main.o: ../util/dlb_main.c $(CONFIG_H) ../include/dlb.h ../include/date.h
	$(CC) $(CFLAGS) -o $@ -c ../util/dlb_main.c


# make sure object files from src are available when needed
#
alloc.o: alloc.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c alloc.c -o $@
drawing.o: drawing.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c drawing.c -o $@
decl.o: decl.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c decl.c -o $@
monst.o: monst.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c monst.c -o $@
objects.o: objects.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c objects.c -o $@
dlb.o: dlb.c $(HACK_H) ../include/dlb.h
	$(CC) $(CFLAGS) -c dlb.c -o $@







SYSSRC = ../sys/share/ioctl.c ../sys/share/unixtty.c ../sys/unix/unixmain.c \
	../sys/unix/unixunix.c ../sys/unix/unixres.c
SYSOBJ = ioctl.o unixmain.o unixtty.o unixunix.o unixres.o

CC = gcc
CFLAGS = -I../include -g

# files for a straight tty port using no native windowing system
WINTTYSRC = ../win/tty/getline.c ../win/tty/termcap.c ../win/tty/topl.c \
	../win/tty/wintty.c
WINTTYOBJ = getline.o termcap.o topl.o wintty.o

WINSRC = $(WINTTYSRC)
WINOBJ = $(WINTTYOBJ)

MAKEDEFS = ../util/makedefs

# all .c that are part of the main NetHack program and are not operating- or
# windowing-system specific
HACKCSRC = allmain.c alloc.c apply.c artifact.c attrib.c ball.c bones.c \
	   botl.c cmd.c dbridge.c decl.c detect.c dig.c display.c dlb.c do.c \
	   do_name.c do_wear.c dog.c dogmove.c dokick.c dothrow.c drawing.c \
	   dungeon.c eat.c end.c engrave.c exper.c explode.c extralev.c \
	   files.c fountain.c hack.c hacklib.c invent.c light.c lock.c \
	   mail.c makemon.c mapglyph.c mcastu.c mhitm.c mhitu.c minion.c \
	   mklev.c mkmap.c \
	   mkmaze.c mkobj.c mkroom.c mon.c mondata.c monmove.c monst.c \
	   mplayer.c mthrowu.c muse.c music.c o_init.c objects.c objnam.c \
	   options.c pager.c pickup.c pline.c polyself.c potion.c pray.c \
	   priest.c quest.c questpgr.c read.c rect.c region.c restore.c rip.c \
	   rnd.c role.c rumors.c save.c shk.c shknam.c sit.c sounds.c sp_lev.c \
	   spell.c steal.c steed.c teleport.c timeout.c topten.c track.c trap.c \
	   u_init.c uhitm.c vault.c version.c vision.c weapon.c were.c wield.c \
	   windows.c wizard.c worm.c worn.c write.c zap.c

# all operating-system-dependent .c (for dependencies and such)
SYSCSRC = ../sys/atari/tos.c \
	../sys/share/pcunix.c ../sys/share/random.c \
	../sys/share/ioctl.c ../sys/share/unixtty.c ../sys/unix/unixmain.c \
	../sys/unix/unixunix.c ../sys/unix/unixres.c ../sys/be/bemain.c

# generated source files
GENCSRC = monstr.c vis_tab.c

# .c files for this version (for date.h)
VERSOURCES = $(HACKCSRC) $(SYSSRC) $(WINSRC) $(GENCSRC)


# all .h files except date.h, onames.h, pm.h, and vis_tab.h which would
# cause dependency loops if run through "make depend"
# and dgn_comp.h, dgn_file.h, lev_comp.h, special level & dungeon files.
#
HACKINCL = align.h artifact.h artilist.h attrib.h color.h \
	config.h config1.h coord.h decl.h def_os2.h display.h dlb.h dungeon.h \
	edog.h emin.h engrave.h epri.h eshk.h extern.h flag.h func_tab.h \
	global.h hack.h lev.h mfndpos.h micro.h mkroom.h \
	monattk.h mondata.h monflag.h monst.h monsym.h obj.h objclass.h \
	patchlevel.h permonst.h prop.h rect.h region.h rm.h \
	sp_lev.h spell.h tcap.h timeout.h tradstdc.h \
	trampoli.h trap.h unixconf.h vault.h vision.h wintty.h \
	winprocs.h wintype.h you.h youprop.h

HSOURCES = $(HACKINCL) date.h onames.h pm.h vis_tab.h\
		lev_comp.h dgn_comp.h dgn_file.h

HOBJ = monst.o objects.o allmain.o alloc.o apply.o artifact.o attrib.o ball.o \
	bones.o botl.o cmd.o dbridge.o decl.o detect.o dig.o display.o dlb.o \
	do.o do_name.o do_wear.o dog.o dogmove.o dokick.o dothrow.o \
	drawing.o dungeon.o eat.o end.o engrave.o exper.o explode.o \
	extralev.o files.o fountain.o hack.o hacklib.o invent.o light.o \
	lock.o mail.o makemon.o mapglyph.o mcastu.o mhitm.o mhitu.o \
	minion.o mklev.o mkmap.o \
	mkmaze.o mkobj.o mkroom.o mon.o mondata.o monmove.o monstr.o \
	mplayer.o mthrowu.o muse.o music.o o_init.o objnam.o options.o \
	pager.o pickup.o pline.o polyself.o potion.o pray.o priest.o \
	quest.o questpgr.o read.o rect.o region.o restore.o rip.o rnd.o \
	role.o rumors.o save.o shk.o shknam.o sit.o sounds.o sp_lev.o spell.o \
	steal.o steed.o teleport.o timeout.o topten.o track.o trap.o u_init.o \
	uhitm.o vault.o vision.o vis_tab.o weapon.o were.o wield.o windows.o \
	wizard.o worm.o worn.o write.o zap.o \
	$(SYSOBJ) $(WINOBJ) version.o

nethack:	$(HOBJ)
	$(CC) -o $@ $(HOBJ) -lncurses

all:	nethack


# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
vis_tab.c: ../include/vis_tab.h

../win/gnome/gn_rip.h: ../win/X11/rip.xpm
	cp ../win/X11/rip.xpm ../win/gnome/gn_rip.h

#	date.h should be remade any time any of the source or include code
#	is modified.  Unfortunately, this would make the contents of this
#	file far more complex.  Since "hack.h" depends on most of the include
#	files, we kludge around this by making date.h dependent on hack.h,
#	even though it doesn't include this file.
#
#	hack.h depends on makedefs' output, so we know makedefs will be
#	up to date before being executed
../include/date.h:	$(VERSOURCES) $(HACK_H)
	../util/makedefs -v


# config.h timestamp
$(CONFIG_H): ../include/config.h ../include/config1.h ../include/tradstdc.h \
		../include/global.h ../include/coord.h \
		../include/unixconf.h ../include/micro.h ../include/nhlan.h
	touch $(CONFIG_H)
# hack.h timestamp
$(HACK_H): ../include/hack.h $(CONFIG_H) ../include/align.h \
		../include/dungeon.h ../include/monsym.h ../include/mkroom.h \
		../include/objclass.h ../include/youprop.h ../include/prop.h \
		../include/permonst.h ../include/monattk.h \
		../include/monflag.h ../include/mondata.h ../include/pm.h \
		../include/wintype.h ../include/decl.h ../include/quest.h \
		../include/spell.h ../include/color.h ../include/obj.h \
		../include/you.h ../include/attrib.h ../include/monst.h \
		../include/skills.h ../include/onames.h ../include/timeout.h \
		../include/trap.h ../include/flag.h ../include/rm.h \
		../include/vision.h ../include/display.h ../include/engrave.h \
		../include/rect.h ../include/region.h ../include/winprocs.h \
		../include/wintty.h ../include/trampoli.h
	touch $(HACK_H)

pcunix.o: ../sys/share/pcunix.c $(HACK_H)
	$(CC) $(CFLAGS) -c ../sys/share/pcunix.c
random.o: ../sys/share/random.c $(HACK_H)
	$(CC) $(CFLAGS) -c ../sys/share/random.c
ioctl.o: ../sys/share/ioctl.c $(HACK_H) ../include/tcap.h
	$(CC) $(CFLAGS) -c ../sys/share/ioctl.c
unixtty.o: ../sys/share/unixtty.c $(HACK_H)
	$(CC) $(CFLAGS) -c ../sys/share/unixtty.c
unixmain.o: ../sys/unix/unixmain.c $(HACK_H) ../include/dlb.h
	$(CC) $(CFLAGS) -c ../sys/unix/unixmain.c
unixunix.o: ../sys/unix/unixunix.c $(HACK_H)
	$(CC) $(CFLAGS) -c ../sys/unix/unixunix.c
unixres.o: ../sys/unix/unixres.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../sys/unix/unixres.c
getline.o: ../win/tty/getline.c $(HACK_H) ../include/func_tab.h
	$(CC) $(CFLAGS) -c ../win/tty/getline.c
termcap.o: ../win/tty/termcap.c $(HACK_H) ../include/tcap.h
	$(CC) $(CFLAGS) -c ../win/tty/termcap.c
topl.o: ../win/tty/topl.c $(HACK_H) ../include/tcap.h
	$(CC) $(CFLAGS) -c ../win/tty/topl.c
wintty.o: ../win/tty/wintty.c $(HACK_H) ../include/dlb.h \
		../include/patchlevel.h ../include/tcap.h
	$(CC) $(CFLAGS) -c ../win/tty/wintty.c
monstr.o: monstr.c $(CONFIG_H)
vis_tab.o: vis_tab.c $(CONFIG_H) ../include/vis_tab.h
allmain.o: allmain.c $(HACK_H)
alloc.o: alloc.c $(CONFIG_H)
apply.o: apply.c $(HACK_H) ../include/edog.h
artifact.o: artifact.c $(HACK_H) ../include/artifact.h ../include/artilist.h
attrib.o: attrib.c $(HACK_H)
ball.o: ball.c $(HACK_H)
bones.o: bones.c $(HACK_H) ../include/lev.h
botl.o: botl.c $(HACK_H)
cmd.o: cmd.c $(HACK_H) ../include/func_tab.h
dbridge.o: dbridge.c $(HACK_H)
decl.o: decl.c $(HACK_H)
detect.o: detect.c $(HACK_H) ../include/artifact.h
dig.o: dig.c $(HACK_H) ../include/edog.h
display.o: display.c $(HACK_H)
dlb.o: dlb.c $(CONFIG_H) ../include/dlb.h
do.o: do.c $(HACK_H) ../include/lev.h
do_name.o: do_name.c $(HACK_H)
do_wear.o: do_wear.c $(HACK_H)
dog.o: dog.c $(HACK_H) ../include/edog.h
dogmove.o: dogmove.c $(HACK_H) ../include/mfndpos.h ../include/edog.h
dokick.o: dokick.c $(HACK_H) ../include/eshk.h
dothrow.o: dothrow.c $(HACK_H) ../include/edog.h
drawing.o: drawing.c $(HACK_H) ../include/tcap.h
dungeon.o: dungeon.c $(HACK_H) ../include/dgn_file.h ../include/dlb.h
eat.o: eat.c $(HACK_H)
end.o: end.c $(HACK_H) ../include/eshk.h ../include/dlb.h
engrave.o: engrave.c $(HACK_H) ../include/lev.h
exper.o: exper.c $(HACK_H)
explode.o: explode.c $(HACK_H)
extralev.o: extralev.c $(HACK_H)
files.o: files.c $(HACK_H) ../include/dlb.h
fountain.o: fountain.c $(HACK_H)
hack.o: hack.c $(HACK_H)
hacklib.o: hacklib.c $(HACK_H)
invent.o: invent.c $(HACK_H)
light.o: light.c $(HACK_H) ../include/lev.h
lock.o: lock.c $(HACK_H)
mail.o: mail.c $(HACK_H) ../include/mail.h
makemon.o: makemon.c $(HACK_H) ../include/epri.h ../include/emin.h \
		../include/edog.h
mapglyph.o: mapglyph.c $(HACK_H)
mcastu.o: mcastu.c $(HACK_H)
mhitm.o: mhitm.c $(HACK_H) ../include/artifact.h ../include/edog.h
mhitu.o: mhitu.c $(HACK_H) ../include/artifact.h ../include/edog.h
minion.o: minion.c $(HACK_H) ../include/emin.h ../include/epri.h
mklev.o: mklev.c $(HACK_H)
mkmap.o: mkmap.c $(HACK_H) ../include/sp_lev.h
mkmaze.o: mkmaze.c $(HACK_H) ../include/sp_lev.h ../include/lev.h
mkobj.o: mkobj.c $(HACK_H)
mkroom.o: mkroom.c $(HACK_H)
mon.o: mon.c $(HACK_H) ../include/mfndpos.h ../include/edog.h
mondata.o: mondata.c $(HACK_H) ../include/eshk.h ../include/epri.h
monmove.o: monmove.c $(HACK_H) ../include/mfndpos.h ../include/artifact.h \
		../include/epri.h
monst.o: monst.c $(CONFIG_H) ../include/permonst.h ../include/align.h \
		../include/monattk.h ../include/monflag.h ../include/monsym.h \
		../include/dungeon.h ../include/eshk.h ../include/vault.h \
		../include/epri.h ../include/color.h
mplayer.o: mplayer.c $(HACK_H)
mthrowu.o: mthrowu.c $(HACK_H)
muse.o: muse.c $(HACK_H) ../include/edog.h
music.o: music.c $(HACK_H) #interp.c
o_init.o: o_init.c $(HACK_H) ../include/lev.h
objects.o: objects.c $(CONFIG_H) ../include/obj.h ../include/objclass.h \
		../include/prop.h ../include/skills.h ../include/color.h
objnam.o: objnam.c $(HACK_H)
options.o: options.c $(CONFIG_H) ../include/objclass.h ../include/flag.h \
		$(HACK_H) ../include/tcap.h
pager.o: pager.c $(HACK_H) ../include/dlb.h
pickup.o: pickup.c $(HACK_H)
pline.o: pline.c $(HACK_H) ../include/epri.h ../include/edog.h
polyself.o: polyself.c $(HACK_H)
potion.o: potion.c $(HACK_H)
pray.o: pray.c $(HACK_H) ../include/epri.h
priest.o: priest.c $(HACK_H) ../include/mfndpos.h ../include/eshk.h \
		../include/epri.h ../include/emin.h
quest.o: quest.c $(HACK_H) ../include/qtext.h
questpgr.o: questpgr.c $(HACK_H) ../include/dlb.h ../include/qtext.h
read.o: read.c $(HACK_H)
rect.o: rect.c $(HACK_H)
region.o: region.c $(HACK_H) ../include/lev.h
restore.o: restore.c $(HACK_H) ../include/lev.h ../include/tcap.h
rip.o: rip.c $(HACK_H)
rnd.o: rnd.c $(HACK_H)
role.o: role.c $(HACK_H)
rumors.o: rumors.c $(HACK_H) ../include/lev.h ../include/dlb.h
save.o: save.c $(HACK_H) ../include/lev.h
shk.o: shk.c $(HACK_H) ../include/eshk.h
shknam.o: shknam.c $(HACK_H) ../include/eshk.h
sit.o: sit.c $(HACK_H) ../include/artifact.h
sounds.o: sounds.c $(HACK_H) ../include/edog.h
sp_lev.o: sp_lev.c $(HACK_H) ../include/dlb.h ../include/sp_lev.h
spell.o: spell.c $(HACK_H)
steal.o: steal.c $(HACK_H)
steed.o: steed.c $(HACK_H)
teleport.o: teleport.c $(HACK_H)
timeout.o: timeout.c $(HACK_H) ../include/lev.h
topten.o: topten.c $(HACK_H) ../include/dlb.h ../include/patchlevel.h
track.o: track.c $(HACK_H)
trap.o: trap.c $(HACK_H)
u_init.o: u_init.c $(HACK_H)
uhitm.o: uhitm.c $(HACK_H)
vault.o: vault.c $(HACK_H) ../include/vault.h
version.o: version.c $(HACK_H) ../include/date.h ../include/patchlevel.h
vision.o: vision.c $(HACK_H) ../include/vis_tab.h
weapon.o: weapon.c $(HACK_H)
were.o: were.c $(HACK_H)
wield.o: wield.c $(HACK_H)
windows.o: windows.c $(HACK_H)
wizard.o: wizard.c $(HACK_H) ../include/qtext.h ../include/epri.h
worm.o: worm.c $(HACK_H) ../include/lev.h
worn.o: worn.c $(HACK_H)
write.o: write.c $(HACK_H)
zap.o: zap.c $(HACK_H)
