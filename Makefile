all:

VARDATD = data oracles options quest.dat rumors

DATHELP = help hh cmdhelp history opthelp wizhelp

SPEC_LEVS = asmodeus.lev baalz.lev bigrm-?.lev castle.lev fakewiz?.lev \
	juiblex.lev knox.lev medusa-?.lev minend-?.lev minefill.lev \
	minetn-?.lev oracle.lev orcus.lev sanctum.lev soko?-?.lev \
	tower?.lev valley.lev wizard?.lev \
	astral.lev air.lev earth.lev fire.lev water.lev
QUEST_LEVS = ???-goal.lev ???-fil?.lev ???-loca.lev ???-strt.lev

DATDLB = $(DATHELP) dungeon $(SPEC_LEVS) $(QUEST_LEVS) $(VARDATD)

all:	nethack recover dlb
	@echo "Done."

nethack:
	( cd src ; $(MAKE) nethack )

data: nethack
	( cd dat && $(MAKE) data )

rumors: nethack
	( cd dat && $(MAKE) rumors )

oracles: nethack
	( cd dat && $(MAKE) oracles )

options: nethack
	( cd dat && $(MAKE) options )

quest.dat: nethack
	( cd dat && $(MAKE) quest.dat )

spec_levs: dungeon
	( cd src && $(MAKE) ../util/lev_comp )
	( cd dat && $(MAKE) spec_levs )
	( cd dat && $(MAKE) quest_levs )

dungeon: nethack
	( cd src && $(MAKE) ../util/dgn_comp )
	( cd dat && $(MAKE) dungeon )

dlb: nethack $(VARDATD) dungeon spec_levs
	( cd src && $(MAKE) ../util/dlb )
	( cd dat && ../util/dlb cf nhdat $(DATDLB) )

# recover can be used when INSURANCE is defined in include/config.h
# and the checkpoint option is true
recover: nethack
	( cd src && $(MAKE) ../util/recover )
