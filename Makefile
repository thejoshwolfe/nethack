all:

-include $(wildcard build/*.d)

GENERATED_LEVELS = build/asmodeus.lev build/baalz.lev build/bigrm-?.lev build/castle.lev build/fakewiz?.lev \
  build/juiblex.lev build/knox.lev build/medusa-?.lev build/minend-?.lev build/minefill.lev \
  build/minetn-?.lev build/oracle.lev build/orcus.lev build/sanctum.lev build/soko?-?.lev \
  build/tower?.lev build/valley.lev build/wizard?.lev \
  build/astral.lev build/air.lev build/earth.lev build/fire.lev build/water.lev \
  build/???-goal.lev build/???-fil?.lev build/???-loca.lev build/???-strt.lev
LEVEL_SOURCES = \
  dat/bigroom.des dat/castle.des dat/endgame.des dat/gehennom.des dat/knox.des \
  dat/medusa.des dat/mines.des dat/oracle.des dat/sokoban.des dat/tower.des \
  dat/yendor.des dat/Arch.des dat/Barb.des dat/Caveman.des dat/Healer.des \
  dat/Knight.des dat/Monk.des dat/Priest.des dat/Ranger.des dat/Rogue.des \
  dat/Samurai.des dat/Tourist.des dat/Valkyrie.des dat/Wizard.des

DATA_INPUTS1 = dat/help dat/hh dat/cmdhelp dat/history dat/opthelp dat/wizhelp build/dungeon
DATA_INPUTS2 = build/data build/oracles dat/options build/quest.dat build/rumors
DATDLB = $(DATA_INPUTS1) $(GENERATED_LEVELS) $(DATA_INPUTS2)

MAKEDEFS_OBJS = build/makedefs.o build/monst.o build/objects.o
MAKE_ONAMES_OBJS = build/make_onames.o build/objects.o
MAKE_ARTIFACT_NAMES_OBJS = build/make_artifact_names.o
MAKE_PM_OBJS = build/make_pm.o build/monst.o
DLB_OBJS = build/dlb_main.o build/dlb.o build/panic.o
DGN_COMP_OBJS = build/dgn_yacc.o build/dgn_lex.o build/dgn_main.o build/panic.o
LEV_COMP_OBJS = build/lev_yacc.o build/lev_lex.o build/lev_main.o build/panic.o build/drawing.o build/decl.o build/monst.o build/objects.o
RECOVER_OBJS = build/recover.o
BUILD_DIR_CHILDREN += $(MAKEDEFS_OBJS) $(MAKE_ONAMES_OBJS) $(MAKE_ARTIFACT_NAMES_OBJS) \
					  $(MAKE_PM_OBJS) $(DLB_OBJS) $(DGN_COMP_OBJS) $(LEV_COMP_OBJS) $(RECOVER_OBJS)

CC = clang
C_FLAGS = -Ibuild -Isrc -g -Wimplicit-function-declaration -Werror
# TODO: remove this and make all id fields not pointers
C_FLAGS += -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
# add extra flags from the command line, such as `EXTRA_C_FLAGS=-ferror-limit=2 make`
EXTRA_C_FLAGS ?=
C_FLAGS += $(EXTRA_C_FLAGS)
COMPILE_C = $(CC) -c -o $@ -MMD -MP -MF $@.d $(C_FLAGS) $<

MAKEDEFS = cd dat && ../build/makedefs

HACK_H = build/onames.h build/pm.h build/artifact_names.h

MAKEDEFS_NEEDS_THESE = build/monst.o build/objects.o
NORMAL_ASS_O_FILES = build/allmain.o build/apply.o build/artifact.o build/attrib.o build/ball.o \
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
  build/uhitm.o build/vault.o build/vision.o build/weapon.o build/were.o build/wield.o build/windows.o \
  build/wizard.o build/worm.o build/worn.o build/write.o build/zap.o \
  build/unixtty.o build/unixunix.o \
  build/getline.o build/termcap.o build/topl.o build/wintty.o \
  build/version.o build/util.o

NETHACK_OBJS = $(MAKEDEFS_NEEDS_THESE) $(NORMAL_ASS_O_FILES)
BUILD_DIR_CHILDREN += $(NETHACK_OBJS)

all: build/nethack build/recover build/nhdat

# make is incapable of tracking the explosion of .lev files from lev_comp,
# so bundle the entire operation here atomicly culminating in the complete archive.
build/nhdat: build/lev_comp $(LEVEL_SOURCES) build/dlb $(DATA_INPUTS1) $(DATA_INPUTS2)
	cd build && ./lev_comp $(foreach f,$(LEVEL_SOURCES),../$f)
	./build/dlb cf build/nhdat $(DATDLB)

build/makedefs: $(MAKEDEFS_OBJS)
	$(CC) -o $@ $(MAKEDEFS_OBJS)

build/make_onames: $(MAKE_ONAMES_OBJS)
	$(CC) -o $@ $(MAKE_ONAMES_OBJS)

build/make_artifact_names: $(MAKE_ARTIFACT_NAMES_OBJS)
	$(CC) -o $@ $(MAKE_ARTIFACT_NAMES_OBJS)

build/make_artifact_names.o: build/onames.h build/pm.h

build/makedefs.o: build/onames.h build/pm.h

build/make_pm: $(MAKE_PM_OBJS)
	$(CC) -o $@ $(MAKE_PM_OBJS)

build/%.o: src/%.c
	$(COMPILE_C)
build/%.o: util/%.c
	$(COMPILE_C)
build/%.o: build/%.c
	$(COMPILE_C)

build/data: dat/data.base build/makedefs
	$(MAKEDEFS) -d

build/rumors: dat/rumors.tru dat/rumors.fal build/makedefs
	$(MAKEDEFS) -r

build/quest.dat: dat/quest.txt build/makedefs
	$(MAKEDEFS) -q

build/oracles: dat/oracles.txt build/makedefs
	$(MAKEDEFS) -h

build/dlb: $(DLB_OBJS)
	$(CC) -o $@ $(DLB_OBJS)

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

build/dgn_lex.o: build/dgn_lex.c build/dgn_yacc.o build/pm.h

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
	$(CC) -o $@ $(DGN_COMP_OBJS)

build/lev_comp: $(LEV_COMP_OBJS)
	$(CC) -o $@ $(LEV_COMP_OBJS)

build/lev_yacc.o: $(HACK_H)

build/lev_main.o: $(HACK_H)

build/onames.h: build/make_onames
	./build/make_onames $@

build/artifact_names.h: build/make_artifact_names
	./build/make_artifact_names $@

build/pm.h: build/make_pm
	./build/make_pm $@

$(NORMAL_ASS_O_FILES): $(HACK_H)

build/nethack: $(NETHACK_OBJS)
	$(CC) -o $@ $(NETHACK_OBJS) -lncurses

build/monstr.c: build/makedefs
	$(MAKEDEFS) -m

# recover can be used when the checkpoint option is true
build/recover: $(RECOVER_OBJS)
	$(CC) -o $@ $(RECOVER_OBJS)


$(BUILD_DIR_CHILDREN): | build
build:
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf build
	rm -rf run
