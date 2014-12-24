# make NetHack
GAME     = nethack
GAMEUID  = games
GAMEGRP  = bin

# Permissions - some places use setgid instead of setuid, for instance
# See also the option "SECURE" in include/config.h
GAMEPERM = 04755
FILEPERM = 0644
EXEPERM  = 0755
DIRPERM  = 0755

VARDATND = 

VARDATD = data oracles options quest.dat rumors
VARDAT = $(VARDATD) $(VARDATND)

# Some versions of make use the SHELL environment variable as the shell
# for running commands.  We need this to be a Bourne shell.
# SHELL = /bin/sh
# for Atari
# SHELL=E:/GEMINI2/MUPFEL.TTP

# Commands for setting the owner and group on files during installation.
# Some systems fail with one or the other when installing over NFS or for
# other permission-related reasons.  If that happens, you may want to set the
# command to "true", which is a no-op. Note that disabling chown or chgrp
# will only work if setuid (or setgid) behavior is not desired or required.
CHOWN = chown
CHGRP = chgrp

#
# end of configuration
#

DATHELP = help hh cmdhelp history opthelp wizhelp

SPEC_LEVS = asmodeus.lev baalz.lev bigrm-?.lev castle.lev fakewiz?.lev \
	juiblex.lev knox.lev medusa-?.lev minend-?.lev minefill.lev \
	minetn-?.lev oracle.lev orcus.lev sanctum.lev soko?-?.lev \
	tower?.lev valley.lev wizard?.lev \
	astral.lev air.lev earth.lev fire.lev water.lev
QUEST_LEVS = ???-goal.lev ???-fil?.lev ???-loca.lev ???-strt.lev

DATNODLB = $(VARDATND) license
DATDLB = $(DATHELP) dungeon $(SPEC_LEVS) $(QUEST_LEVS) $(VARDATD)
DAT = $(DATNODLB) $(DATDLB)

all:	$(GAME) recover dlb
	@echo "Done."

$(GAME):
	( cd src ; $(MAKE) )

# Note: many of the dependencies below are here to allow parallel make
# to generate valid output

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

nhtiles.bmp: $(GAME)
	( cd dat && $(MAKE) nhtiles.bmp )

x11tiles: $(GAME)
	( cd util && $(MAKE) tile2x11 )
	( cd dat && $(MAKE) x11tiles )

beostiles: $(GAME)
	( cd util && $(MAKE) tile2beos )
	( cd dat && $(MAKE) beostiles )

NetHack.ad: $(GAME)
	( cd dat && $(MAKE) NetHack.ad )

pet_mark.xbm:
	( cd dat && $(MAKE) pet_mark.xbm )

rip.xpm:
	( cd dat && $(MAKE) rip.xpm )

mapbg.xpm:
	(cd dat && $(MAKE) mapbg.xpm )

nhsplash.xpm:
	( cd dat && $(MAKE) nhsplash.xpm )

nh16.img: $(GAME)
	( cd util && $(MAKE) tile2img.ttp )
	( cd dat && $(MAKE) nh16.img )

rip.img:
	( cd util && $(MAKE) xpm2img.ttp )
	( cd dat && $(MAKE) rip.img )
GEM_RSC.RSC:
	( cd dat && $(MAKE) GEM_RSC.RSC )

title.img:
	( cd dat && $(MAKE) title.img )

dlb: $(GAME) $(VARDAT) dungeon spec_levs
	( cd util && $(MAKE) dlb )
	( cd dat && ../util/dlb cf nhdat $(DATDLB) )

# recover can be used when INSURANCE is defined in include/config.h
# and the checkpoint option is true
recover: $(GAME)
	( cd util && $(MAKE) recover )
