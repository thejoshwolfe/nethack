all:

GENERATED_LEVELS = build/asmodeus.lev build/baalz.lev build/bigrm-?.lev build/castle.lev build/fakewiz?.lev \
  build/juiblex.lev build/knox.lev build/medusa-?.lev build/minend-?.lev build/minefill.lev \
  build/minetn-?.lev build/oracle.lev build/orcus.lev build/sanctum.lev build/soko?-?.lev \
  build/tower?.lev build/valley.lev build/wizard?.lev \
  build/astral.lev build/air.lev build/earth.lev build/fire.lev build/water.lev \
  build/???-goal.lev build/???-fil?.lev build/???-loca.lev build/???-strt.lev

DATA_INPUTS1 = dat/help dat/hh dat/cmdhelp dat/history dat/opthelp dat/wizhelp build/dungeon
DATA_INPUTS2 = build/data build/oracles dat/options build/quest.dat build/rumors
DATDLB = $(DATA_INPUTS1) $(GENERATED_LEVELS) $(DATA_INPUTS2)

MAKEDEFS_OBJS = build/makedefs.o build/monst.o build/objects.o
DLB_OBJS = build/dlb_main.o build/dlb.o build/alloc.o build/panic.o
DGN_COMP_OBJS = build/dgn_yacc.o build/dgn_lex.o build/dgn_main.o build/alloc.o build/panic.o
LEV_COMP_OBJS = build/lev_yacc.o build/lev_lex.o build/lev_main.o build/alloc.o build/panic.o build/drawing.o build/decl.o build/monst.o build/objects.o
RECOVER_OBJS = build/recover.o
BUILD_DIR_CHILDREN += $(MAKEDEFS_OBJS) $(DLB_OBJS) $(DGN_COMP_OBJS) $(LEV_COMP_OBJS) $(RECOVER_OBJS)

CFLAGS = -Ibuild -Iinclude -g

MAKEDEFS = cd dat && ../build/makedefs

HACK_H = build/onames.h build/pm.h

MAKEDEFS_NEEDS_THESE = build/monst.o build/objects.o
NORMAL_ASS_O_FILES = build/allmain.o build/alloc.o build/apply.o build/artifact.o build/attrib.o build/ball.o \
  build/bones.o build/botl.o build/cmd.o build/dbridge.o build/decl.o build/detect.o build/dig.o build/display.o build/dlb.o \
  build/do.o build/do_name.o build/do_wear.o build/dog.o build/dogmove.o build/dokick.o build/dothrow.o \
  build/drawing.o build/dungeon.o build/eat.o build/end.o build/engrave.o build/exper.o build/explode.o \
  build/files.o build/fountain.o build/hack.o build/hacklib.o build/invent.o build/light.o \
  build/lock.o build/mail.o build/makemon.o build/mapglyph.o build/mcastu.o build/mhitm.o build/mhitu.o \
  build/minion.o build/mklev.o build/mkmap.o \
  build/mkmaze.o build/mkobj.o build/mkroom.o build/mon.o build/mondata.o build/monmove.o build/monstr.o \
  build/mplayer.o build/mthrowu.o build/muse.o build/music.o build/o_init.o build/objnam.o build/options.o \
  build/pager.o build/pickup.o build/pline.o build/polyself.o build/potion.o build/pray.o build/priest.o \
  build/quest.o build/questpgr.o build/read.o build/rect.o build/region.o build/restore.o build/rip.o build/rnd.o \
  build/role.o build/rumors.o build/save.o build/shk.o build/shknam.o build/sit.o build/sounds.o build/sp_lev.o build/spell.o \
  build/steal.o build/steed.o build/teleport.o build/timeout.o build/topten.o build/track.o build/trap.o build/u_init.o \
  build/uhitm.o build/vault.o build/vision.o build/vis_tab.o build/weapon.o build/were.o build/wield.o build/windows.o \
  build/wizard.o build/worm.o build/worn.o build/write.o build/zap.o \
  build/ioctl.o build/unixmain.o build/unixtty.o build/unixunix.o \
  build/getline.o build/termcap.o build/topl.o build/wintty.o \
	build/version.o

NETHACK_OBJS = $(MAKEDEFS_NEEDS_THESE) $(NORMAL_ASS_O_FILES)
BUILD_DIR_CHILDREN += $(NETHACK_OBJS)

all: build/nethack build/recover build/nhdat

build/nhdat: build/dlb build/lev_comp $(DATA_INPUTS1) $(DATA_INPUTS2)
	cd build && ./lev_comp ../dat/bigroom.des
	cd build && ./lev_comp ../dat/castle.des
	cd build && ./lev_comp ../dat/endgame.des
	cd build && ./lev_comp ../dat/gehennom.des
	cd build && ./lev_comp ../dat/knox.des
	cd build && ./lev_comp ../dat/medusa.des
	cd build && ./lev_comp ../dat/mines.des
	cd build && ./lev_comp ../dat/oracle.des
	cd build && ./lev_comp ../dat/sokoban.des
	cd build && ./lev_comp ../dat/tower.des
	cd build && ./lev_comp ../dat/yendor.des
	cd build && ./lev_comp ../dat/Arch.des
	cd build && ./lev_comp ../dat/Barb.des
	cd build && ./lev_comp ../dat/Caveman.des
	cd build && ./lev_comp ../dat/Healer.des
	cd build && ./lev_comp ../dat/Knight.des
	cd build && ./lev_comp ../dat/Monk.des
	cd build && ./lev_comp ../dat/Priest.des
	cd build && ./lev_comp ../dat/Ranger.des
	cd build && ./lev_comp ../dat/Rogue.des
	cd build && ./lev_comp ../dat/Samurai.des
	cd build && ./lev_comp ../dat/Tourist.des
	cd build && ./lev_comp ../dat/Valkyrie.des
	cd build && ./lev_comp ../dat/Wizard.des
	./build/dlb cf build/nhdat $(DATDLB)

build/makedefs: $(MAKEDEFS_OBJS)
	gcc -o $@ $(MAKEDEFS_OBJS)

build/%.o: src/%.c
	gcc -o $@ -c $(CFLAGS) $<
build/%.o: util/%.c
	gcc -o $@ -c $(CFLAGS) $<
build/%.o: build/%.c
	gcc -o $@ -c $(CFLAGS) $<

build/data: dat/data.base build/makedefs
	$(MAKEDEFS) -d

build/rumors: dat/rumors.tru dat/rumors.fal build/makedefs
	$(MAKEDEFS) -r

build/quest.dat: dat/quest.txt build/makedefs
	$(MAKEDEFS) -q

build/oracles: dat/oracles.txt build/makedefs
	$(MAKEDEFS) -h

build/dlb: $(DLB_OBJS)
	gcc -o $@ $(DLB_OBJS)

build/dungeon: build/dgn_comp dat/dungeon.def
	./build/dgn_comp dat/dungeon.def $@

build/dgn_yacc.c: util/dgn_comp.y
	mkdir -p build/dgn.tmp
	cd build/dgn.tmp && yacc -d ../../util/dgn_comp.y
	mv build/dgn.tmp/y.tab.c build/dgn_yacc.c
	mv build/dgn.tmp/y.tab.h build/dgn_comp.h

build/lev_yacc.c: util/lev_comp.y
	mkdir -p build/lev.tmp
	cd build/lev.tmp && yacc -d ../../util/lev_comp.y
	mv build/lev.tmp/y.tab.c build/lev_yacc.c
	mv build/lev.tmp/y.tab.h build/lev_comp.h


build/dgn_comp.h: build/dgn_yacc.c

build/lev_comp.h: build/lev_yacc.c

build/dgn_lex.o: build/dgn_lex.c build/dgn_yacc.o

build/lev_lex.o: build/lev_lex.c $(HACK_H) build/lev_yacc.o

build/dgn_lex.c: util/dgn_comp.l
	mkdir -p build/dgn.tmp
	cd build/dgn.tmp && lex ../../util/dgn_comp.l
	mv build/dgn.tmp/lex.yy.c build/dgn_lex.c

build/lev_lex.c: util/lev_comp.l
	mkdir -p build/lev.tmp
	cd build/lev.tmp && lex ../../util/lev_comp.l
	mv build/lev.tmp/lex.yy.c build/lev_lex.c

build/dgn_comp: $(DGN_COMP_OBJS)
	gcc -o $@ $(DGN_COMP_OBJS)

build/lev_comp: $(LEV_COMP_OBJS)
	gcc -o $@ $(LEV_COMP_OBJS)

build/lev_yacc.o: $(HACK_H)

build/lev_main.o: $(HACK_H)

build/onames.h: build/makedefs
	$(MAKEDEFS) -o

build/pm.h: build/makedefs
	$(MAKEDEFS) -p

$(NORMAL_ASS_O_FILES): $(HACK_H)

build/nethack: $(NETHACK_OBJS)
	gcc -o $@ $(NETHACK_OBJS) -lncurses

build/monstr.c: build/makedefs
	$(MAKEDEFS) -m

build/vis_tab.h: build/makedefs
	$(MAKEDEFS) -z

# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
build/vis_tab.c: build/vis_tab.h

# recover can be used when INSURANCE is defined in include/config.h
# and the checkpoint option is true
build/recover: $(RECOVER_OBJS)
	gcc -o $@ $(RECOVER_OBJS)


$(BUILD_DIR_CHILDREN): | build
build:
	mkdir -p $@
