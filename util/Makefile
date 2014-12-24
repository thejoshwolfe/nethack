CFLAGS = -O -I../include
LFLAGS =
LIBS =
OBJDIR = ../src
YACC     = yacc
LEX      = lex
 
# these are the names of the output files from YACC/LEX.
YTABC = ../util/y.tab.c
YTABH = ../util/y.tab.h
LEXYYC = ../util/lex.yy.c


# timestamps for primary header files, matching src/Makefile
CONFIG_H = ../src/config.h-t
HACK_H	 = ../src/hack.h-t

# utility .c files
MAKESRC = ../util/makedefs.c
SPLEVSRC = ../util/lev_yacc.c ../util/lev_lex.c ../util/lev_main.c
DGNCOMPSRC = ../util/dgn_yacc.c ../util/dgn_lex.c ../util/dgn_main.c
RECOVSRC = ../util/recover.c
DLBSRC = ../util/dlb_main.c
UTILSRCS = $(MAKESRC) ../util/panic.c $(SPLEVSRC) $(DGNCOMPSRC) $(RECOVSRC) $(DLBSRC)

# files that define all monsters and objects
CMONOBJ = ../src/monst.c ../src/objects.c
OMONOBJ = $(OBJDIR)/monst.o $(OBJDIR)/objects.o
# files that provide access to NetHack's names
CNAMING = ../src/drawing.c ../src/decl.c $(CMONOBJ)
ONAMING = $(OBJDIR)/drawing.o $(OBJDIR)/decl.o $(OMONOBJ)
# dynamic memory allocation
CALLOC = ../src/alloc.c ../util/panic.c
OALLOC = $(OBJDIR)/alloc.o ../util/panic.o

# object files for makedefs
MAKEOBJS = ../util/makedefs.o $(OMONOBJ)

# object files for special levels compiler
SPLEVOBJS = ../util/lev_yacc.o ../util/lev_lex.o ../util/lev_main.o $(OALLOC) $(ONAMING)

# object files for dungeon compiler
DGNCOMPOBJS = ../util/dgn_yacc.o ../util/dgn_lex.o ../util/dgn_main.o $(OALLOC)

# object files for recovery utility
RECOVOBJS = ../util/recover.o

# object files for the data librarian
DLBOBJS = ../util/dlb_main.o $(OBJDIR)/dlb.o $(OALLOC)

FLEXDIST =
YACCDIST =

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
../src/monstr.c: ../util/makedefs
	../util/./makedefs -m
../include/vis_tab.h: ../util/makedefs
	../util/./makedefs -z
# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
../src/vis_tab.c: ../include/vis_tab.h


# we defer this makedefs call to the src Makefile, since it knows all about
# the main src and include files date.h is a timestamp for
../include/date.h::
	@( cd ../src ; $(MAKE) ../include/date.h )

# support code used by several of the utility programs (but not makedefs)
../util/panic.o:     ../util/panic.c $(CONFIG_H)


../util/lev_comp:  $(SPLEVOBJS)
	$(CC) $(LFLAGS) -o ../util/lev_comp $(SPLEVOBJS) $(LIBS)

../util/lev_yacc.o:  ../util/lev_yacc.c $(HACK_H) ../include/sp_lev.h
../util/lev_main.o:  ../util/lev_main.c $(HACK_H) ../include/sp_lev.h ../include/tcap.h ../include/date.h

# see lev_comp.l for WEIRD_LEX discussion
# egrep will return failure if it doesn't find anything, but we know there
# is one "_cplusplus" inside a comment
../util/lev_lex.o:   ../util/lev_lex.c $(HACK_H) ../include/lev_comp.h ../include/sp_lev.h
	@echo $(CC) -c $(CFLAGS) ../util/lev_lex.c
	@$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus ../util/lev_lex.c` ../util/lev_lex.c

../include/lev_comp.h: ../util/lev_yacc.c

../util/lev_yacc.c: ../util/lev_comp.y
	$(YACC) $(YACCDIST) -d ../util/lev_comp.y
	mv $(YTABC) ../util/lev_yacc.c
	mv $(YTABH) ../include/lev_comp.h

../util/lev_lex.c: ../util/lev_comp.l
	$(LEX) $(FLEXDIST) ../util/lev_comp.l
	mv $(LEXYYC) ../util/lev_lex.c


../util/dgn_comp:  $(DGNCOMPOBJS)
	$(CC) $(LFLAGS) -o ../util/dgn_comp $(DGNCOMPOBJS) $(LIBS)

../util/dgn_yacc.o:  ../util/dgn_yacc.c $(CONFIG_H) ../include/dgn_file.h ../include/date.h
../util/dgn_main.o:  ../util/dgn_main.c $(CONFIG_H) ../include/dlb.h

# see dgn_comp.l for WEIRD_LEX discussion
../util/dgn_lex.o:   ../util/dgn_lex.c $(CONFIG.H) ../include/dgn_comp.h ../include/dgn_file.h
	@echo $(CC) -c $(CFLAGS) ../util/dgn_lex.c
	@$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus ../util/dgn_lex.c` ../util/dgn_lex.c


../include/dgn_comp.h: ../util/dgn_yacc.c

../util/dgn_yacc.c: ../util/dgn_comp.y
	$(YACC) $(YACCDIST) -d ../util/dgn_comp.y
	mv $(YTABC) ../util/dgn_yacc.c
	mv $(YTABH) ../include/dgn_comp.h

../util/dgn_lex.c: ../util/dgn_comp.l
	$(LEX) $(FLEXDIST) ../util/dgn_comp.l
	mv $(LEXYYC) ../util/dgn_lex.c


../util/recover: $(RECOVOBJS)
	$(CC) $(LFLAGS) -o ../util/recover $(RECOVOBJS) $(LIBS)

../util/recover.o: ../util/recover.c $(CONFIG_H) ../include/date.h


../util/dlb:	$(DLBOBJS)
	$(CC) $(LFLAGS) -o ../util/dlb $(DLBOBJS) $(LIBS)

../util/dlb_main.o: ../util/dlb_main.c $(CONFIG_H) ../include/dlb.h ../include/date.h
	$(CC) $(CFLAGS) -c ../util/dlb_main.c


# make sure object files from src are available when needed
#
$(OBJDIR)/alloc.o: ../src/alloc.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../src/alloc.c -o $@
$(OBJDIR)/drawing.o: ../src/drawing.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../src/drawing.c -o $@
$(OBJDIR)/decl.o: ../src/decl.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../src/decl.c -o $@
$(OBJDIR)/monst.o: ../src/monst.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../src/monst.c -o $@
$(OBJDIR)/objects.o: ../src/objects.c $(CONFIG_H)
	$(CC) $(CFLAGS) -c ../src/objects.c -o $@
$(OBJDIR)/dlb.o: ../src/dlb.c $(HACK_H) ../include/dlb.h
	$(CC) $(CFLAGS) -c ../src/dlb.c -o $@

# make sure hack.h dependencies get transitive information
$(HACK_H): $(CONFIG_H)
	@( cd ../src ; $(MAKE) $(HACK_H) )
$(CONFIG_H): ../include/config.h
	@( cd ../src ; $(MAKE) $(CONFIG_H) )
