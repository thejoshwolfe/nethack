all:

SHELL=/bin/sh

GENERATED_LEVELS = dat/asmodeus.lev dat/baalz.lev dat/bigrm-?.lev dat/castle.lev dat/fakewiz?.lev \
  dat/juiblex.lev dat/knox.lev dat/medusa-?.lev dat/minend-?.lev dat/minefill.lev \
  dat/minetn-?.lev dat/oracle.lev dat/orcus.lev dat/sanctum.lev dat/soko?-?.lev \
  dat/tower?.lev dat/valley.lev dat/wizard?.lev \
  dat/astral.lev dat/air.lev dat/earth.lev dat/fire.lev dat/water.lev \
  dat/???-goal.lev dat/???-fil?.lev dat/???-loca.lev dat/???-strt.lev

DATA_INPUTS1 = dat/help dat/hh dat/cmdhelp dat/history dat/opthelp dat/wizhelp dat/dungeon
DATA_INPUTS2 = dat/data dat/oracles dat/options dat/quest.dat dat/rumors
DATDLB = $(DATA_INPUTS1) $(GENERATED_LEVELS) $(DATA_INPUTS2)

MAKEDEFS_OBJS = util/makedefs.o src/monst.o src/objects.o
DLB_OBJS = util/dlb_main.o src/dlb.o src/alloc.o util/panic.o
DGN_COMP_OBJS = util/dgn_yacc.o util/dgn_lex.o util/dgn_main.o src/alloc.o util/panic.o
LEV_COMP_OBJS = util/lev_yacc.o util/lev_lex.o util/lev_main.o src/alloc.o util/panic.o src/drawing.o src/decl.o src/monst.o src/objects.o
RECOVER_OBJS = util/recover.o

CFLAGS = -Iinclude -g

MAKEDEFS = cd dat && ../util/makedefs

HACK_H = include/onames.h include/pm.h

MAKEDEFS_NEEDS_THESE = src/monst.o src/objects.o
NORMAL_ASS_O_FILES = src/allmain.o src/alloc.o src/apply.o src/artifact.o src/attrib.o src/ball.o \
  src/bones.o src/botl.o src/cmd.o src/dbridge.o src/decl.o src/detect.o src/dig.o src/display.o src/dlb.o \
  src/do.o src/do_name.o src/do_wear.o src/dog.o src/dogmove.o src/dokick.o src/dothrow.o \
  src/drawing.o src/dungeon.o src/eat.o src/end.o src/engrave.o src/exper.o src/explode.o \
  src/extralev.o src/files.o src/fountain.o src/hack.o src/hacklib.o src/invent.o src/light.o \
  src/lock.o src/mail.o src/makemon.o src/mapglyph.o src/mcastu.o src/mhitm.o src/mhitu.o \
  src/minion.o src/mklev.o src/mkmap.o \
  src/mkmaze.o src/mkobj.o src/mkroom.o src/mon.o src/mondata.o src/monmove.o src/monstr.o \
  src/mplayer.o src/mthrowu.o src/muse.o src/music.o src/o_init.o src/objnam.o src/options.o \
  src/pager.o src/pickup.o src/pline.o src/polyself.o src/potion.o src/pray.o src/priest.o \
  src/quest.o src/questpgr.o src/read.o src/rect.o src/region.o src/restore.o src/rip.o src/rnd.o \
  src/role.o src/rumors.o src/save.o src/shk.o src/shknam.o src/sit.o src/sounds.o src/sp_lev.o src/spell.o \
  src/steal.o src/steed.o src/teleport.o src/timeout.o src/topten.o src/track.o src/trap.o src/u_init.o \
  src/uhitm.o src/vault.o src/vision.o src/vis_tab.o src/weapon.o src/were.o src/wield.o src/windows.o \
  src/wizard.o src/worm.o src/worn.o src/write.o src/zap.o \
  src/ioctl.o src/unixmain.o src/unixtty.o src/unixunix.o src/unixres.o \
  src/getline.o src/termcap.o src/topl.o src/wintty.o \
	src/version.o

NETHACK_OBJS = $(MAKEDEFS_NEEDS_THESE) $(NORMAL_ASS_O_FILES)

all: src/nethack util/recover dat/nhdat

dat/nhdat: util/dlb util/lev_comp $(DATA_INPUTS1) $(DATA_INPUTS2)
	cd dat && ../util/lev_comp bigroom.des
	cd dat && ../util/lev_comp castle.des
	cd dat && ../util/lev_comp endgame.des
	cd dat && ../util/lev_comp gehennom.des
	cd dat && ../util/lev_comp knox.des
	cd dat && ../util/lev_comp medusa.des
	cd dat && ../util/lev_comp mines.des
	cd dat && ../util/lev_comp oracle.des
	cd dat && ../util/lev_comp sokoban.des
	cd dat && ../util/lev_comp tower.des
	cd dat && ../util/lev_comp yendor.des
	cd dat && ../util/lev_comp Arch.des
	cd dat && ../util/lev_comp Barb.des
	cd dat && ../util/lev_comp Caveman.des
	cd dat && ../util/lev_comp Healer.des
	cd dat && ../util/lev_comp Knight.des
	cd dat && ../util/lev_comp Monk.des
	cd dat && ../util/lev_comp Priest.des
	cd dat && ../util/lev_comp Ranger.des
	cd dat && ../util/lev_comp Rogue.des
	cd dat && ../util/lev_comp Samurai.des
	cd dat && ../util/lev_comp Tourist.des
	cd dat && ../util/lev_comp Valkyrie.des
	cd dat && ../util/lev_comp Wizard.des
	cd dat && ../util/dlb cf nhdat $(notdir $(DATDLB))

util/makedefs: $(MAKEDEFS_OBJS)
	gcc -o $@ $(MAKEDEFS_OBJS)

%.o: %.c
	gcc -o $@ -c $(CFLAGS) $<

dat/data: dat/data.base util/makedefs
	$(MAKEDEFS) -d

dat/rumors: dat/rumors.tru dat/rumors.fal util/makedefs
	$(MAKEDEFS) -r

dat/quest.dat: dat/quest.txt util/makedefs
	$(MAKEDEFS) -q

dat/oracles: dat/oracles.txt util/makedefs
	$(MAKEDEFS) -h

util/dlb: $(DLB_OBJS)
	gcc -o $@ $(DLB_OBJS)

dat/dungeon: util/dgn_comp dat/dungeon.pdf
	cd dat && ../util/dgn_comp dungeon.pdf

dat/dungeon.pdf: util/makedefs dat/dungeon.def
	cd dat && ../util/makedefs -e

util/dgn_yacc.c: util/dgn_comp.y
	cd util && yacc -d ../util/dgn_comp.y
	mv util/y.tab.c util/dgn_yacc.c
	mv util/y.tab.h include/dgn_comp.h

util/lev_yacc.c: util/lev_comp.y
	cd util && yacc -d ../util/lev_comp.y
	mv util/y.tab.c util/lev_yacc.c
	mv util/y.tab.h include/lev_comp.h


include/dgn_comp.h: util/dgn_yacc.c

include/lev_comp.h: util/lev_yacc.c

# see dgn_comp.l for WEIRD_LEX discussion
util/dgn_lex.o: util/dgn_lex.c util/dgn_yacc.o
	gcc -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus util/dgn_lex.c` -o $@ util/dgn_lex.c

# see lev_comp.l for WEIRD_LEX discussion
# egrep will return failure if it doesn't find anything, but we know there
# is one "_cplusplus" inside a comment
util/lev_lex.o: util/lev_lex.c $(HACK_H) util/lev_yacc.o
	gcc -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus util/lev_lex.c` -o $@ util/lev_lex.c

util/dgn_lex.c: util/dgn_comp.l
	cd util && lex ../util/dgn_comp.l
	mv util/lex.yy.c util/dgn_lex.c

util/lev_lex.c: util/lev_comp.l
	cd util && lex ../util/lev_comp.l
	mv util/lex.yy.c util/lev_lex.c

util/dgn_comp: $(DGN_COMP_OBJS)
	gcc -o $@ $(DGN_COMP_OBJS)

util/lev_comp: $(LEV_COMP_OBJS)
	gcc -o $@ $(LEV_COMP_OBJS)

util/lev_yacc.o: $(HACK_H)

util/lev_main.o: $(HACK_H)

include/onames.h: util/makedefs
	$(MAKEDEFS) -o

include/pm.h: util/makedefs
	$(MAKEDEFS) -p

$(NORMAL_ASS_O_FILES): $(HACK_H)

src/nethack: $(NETHACK_OBJS)
	gcc -o $@ $(NETHACK_OBJS) -lncurses

# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
src/vis_tab.c: include/vis_tab.h

src/monstr.c: util/makedefs
	$(MAKEDEFS) -m

include/vis_tab.h: util/makedefs
	$(MAKEDEFS) -z

# recover can be used when INSURANCE is defined in include/config.h
# and the checkpoint option is true
util/recover: $(RECOVER_OBJS)
	gcc -o $@ $(RECOVER_OBJS)
