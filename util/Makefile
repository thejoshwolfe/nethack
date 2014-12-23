CFLAGS = -O -I../include
LFLAGS =
LIBS =
OBJDIR = ../src
YACC     = yacc
LEX      = lex
 
# these are the names of the output files from YACC/LEX.
YTABC = y.tab.c
YTABH = y.tab.h
LEXYYC = lex.yy.c


# timestamps for primary header files, matching src/Makefile
CONFIG_H = ../src/config.h-t
HACK_H	 = ../src/hack.h-t

# utility .c files
MAKESRC = makedefs.c
SPLEVSRC = lev_yacc.c lev_lex.c lev_main.c
DGNCOMPSRC = dgn_yacc.c dgn_lex.c dgn_main.c
RECOVSRC = recover.c
DLBSRC = dlb_main.c
UTILSRCS = $(MAKESRC) panic.c $(SPLEVSRC) $(DGNCOMPSRC) $(RECOVSRC) $(DLBSRC)

# files that define all monsters and objects
CMONOBJ = ../src/monst.c ../src/objects.c
OMONOBJ = $(OBJDIR)/monst.o $(OBJDIR)/objects.o
# files that provide access to NetHack's names
CNAMING = ../src/drawing.c ../src/decl.c $(CMONOBJ)
ONAMING = $(OBJDIR)/drawing.o $(OBJDIR)/decl.o $(OMONOBJ)
# dynamic memory allocation
CALLOC = ../src/alloc.c panic.c
OALLOC = $(OBJDIR)/alloc.o panic.o

# object files for makedefs
MAKEOBJS = makedefs.o $(OMONOBJ)

# object files for special levels compiler
SPLEVOBJS = lev_yacc.o lev_lex.o lev_main.o $(OALLOC) $(ONAMING)

# object files for dungeon compiler
DGNCOMPOBJS = dgn_yacc.o dgn_lex.o dgn_main.o $(OALLOC)

# object files for recovery utility
RECOVOBJS = recover.o

# object files for the data librarian
DLBOBJS = dlb_main.o $(OBJDIR)/dlb.o $(OALLOC)

FLEXDIST =
YACCDIST =

makedefs:	$(MAKEOBJS)
	$(CC) $(LFLAGS) -o makedefs $(MAKEOBJS)

makedefs.o: makedefs.c $(CONFIG_H) ../include/permonst.h \
		../include/objclass.h ../include/monsym.h \
		../include/artilist.h ../include/dungeon.h ../include/obj.h \
		../include/monst.h ../include/you.h ../include/flag.h \
		../include/dlb.h ../include/patchlevel.h ../include/qtext.h

../include/onames.h: makedefs
	./makedefs -o
../include/pm.h: makedefs
	./makedefs -p
../src/monstr.c: makedefs
	./makedefs -m
../include/vis_tab.h: makedefs
	./makedefs -z
# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
../src/vis_tab.c: ../include/vis_tab.h

lintdefs:
	@lint -axbh -I../include -DLINT $(MAKESRC) $(CMONOBJ) | sed '/_flsbuf/d'


# we defer this makedefs call to the src Makefile, since it knows all about
# the main src and include files date.h is a timestamp for
../include/date.h::
	@( cd ../src ; $(MAKE) ../include/date.h )

# support code used by several of the utility programs (but not makedefs)
panic.o:     panic.c $(CONFIG_H)


lev_comp:  $(SPLEVOBJS)
	$(CC) $(LFLAGS) -o lev_comp $(SPLEVOBJS) $(LIBS)

lev_yacc.o:  lev_yacc.c $(HACK_H) ../include/sp_lev.h
lev_main.o:  lev_main.c $(HACK_H) ../include/sp_lev.h ../include/tcap.h ../include/date.h

# see lev_comp.l for WEIRD_LEX discussion
# egrep will return failure if it doesn't find anything, but we know there
# is one "_cplusplus" inside a comment
lev_lex.o:   lev_lex.c $(HACK_H) ../include/lev_comp.h ../include/sp_lev.h
	@echo $(CC) -c $(CFLAGS) lev_lex.c
	@$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus lev_lex.c` lev_lex.c

../include/lev_comp.h: lev_yacc.c

lev_yacc.c: lev_comp.y
	$(YACC) $(YACCDIST) -d lev_comp.y
	mv $(YTABC) lev_yacc.c
	mv $(YTABH) ../include/lev_comp.h

lev_lex.c: lev_comp.l
	$(LEX) $(FLEXDIST) lev_comp.l
	mv $(LEXYYC) lev_lex.c


dgn_comp:  $(DGNCOMPOBJS)
	$(CC) $(LFLAGS) -o dgn_comp $(DGNCOMPOBJS) $(LIBS)

dgn_yacc.o:  dgn_yacc.c $(CONFIG_H) ../include/dgn_file.h ../include/date.h
dgn_main.o:  dgn_main.c $(CONFIG_H) ../include/dlb.h

# see dgn_comp.l for WEIRD_LEX discussion
dgn_lex.o:   dgn_lex.c $(CONFIG.H) ../include/dgn_comp.h ../include/dgn_file.h
	@echo $(CC) -c $(CFLAGS) dgn_lex.c
	@$(CC) -c $(CFLAGS) -DWEIRD_LEX=`egrep -c _cplusplus dgn_lex.c` dgn_lex.c


../include/dgn_comp.h: dgn_yacc.c

dgn_yacc.c: dgn_comp.y
	$(YACC) $(YACCDIST) -d dgn_comp.y
	mv $(YTABC) dgn_yacc.c
	mv $(YTABH) ../include/dgn_comp.h

dgn_lex.c: dgn_comp.l
	$(LEX) $(FLEXDIST) dgn_comp.l
	mv $(LEXYYC) dgn_lex.c


recover: $(RECOVOBJS)
	$(CC) $(LFLAGS) -o recover $(RECOVOBJS) $(LIBS)

recover.o: recover.c $(CONFIG_H) ../include/date.h


dlb:	$(DLBOBJS)
	$(CC) $(LFLAGS) -o dlb $(DLBOBJS) $(LIBS)

dlb_main.o: dlb_main.c $(CONFIG_H) ../include/dlb.h ../include/date.h
	$(CC) $(CFLAGS) -c dlb_main.c


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
