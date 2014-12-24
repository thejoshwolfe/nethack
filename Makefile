all:

GAME     = nethack

VARDATD = data oracles options quest.dat rumors

DATHELP = help hh cmdhelp history opthelp wizhelp

SPEC_LEVS = asmodeus.lev baalz.lev bigrm-?.lev castle.lev fakewiz?.lev \
	juiblex.lev knox.lev medusa-?.lev minend-?.lev minefill.lev \
	minetn-?.lev oracle.lev orcus.lev sanctum.lev soko?-?.lev \
	tower?.lev valley.lev wizard?.lev \
	astral.lev air.lev earth.lev fire.lev water.lev
QUEST_LEVS = ???-goal.lev ???-fil?.lev ???-loca.lev ???-strt.lev

DATDLB = $(DATHELP) dungeon $(SPEC_LEVS) $(QUEST_LEVS) $(VARDATD)

all:	$(GAME) recover dlb
	@echo "Done."

$(GAME):
	( cd src ; $(MAKE) )

data: $(GAME)
	( cd dat && $(MAKE) data )

rumors: $(GAME)
	( cd dat && $(MAKE) rumors )

oracles: $(GAME)
	( cd dat && $(MAKE) oracles )

#	Note: options should have already been made with make, but...
options: $(GAME)
	( cd dat && $(MAKE) options )

quest.dat: $(GAME)
	( cd dat && $(MAKE) quest.dat )

spec_levs: dungeon
	( cd util && $(MAKE) lev_comp )
	( cd dat && $(MAKE) spec_levs )
	( cd dat && $(MAKE) quest_levs )

dungeon: $(GAME)
	( cd util && $(MAKE) dgn_comp )
	( cd dat && $(MAKE) dungeon )

dlb: $(GAME) $(VARDATD) dungeon spec_levs
	( cd util && $(MAKE) dlb )
	( cd dat && ../util/dlb cf nhdat $(DATDLB) )

# recover can be used when INSURANCE is defined in include/config.h
# and the checkpoint option is true
recover: $(GAME)
	( cd util && $(MAKE) recover )
