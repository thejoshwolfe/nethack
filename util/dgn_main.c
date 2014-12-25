/* See LICENSE in the root of this project for change info */
/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include "config.h"
#include "dlb.h"

#ifndef MPWTOOL
# define SpinCursor(x)
#endif

#define MAX_ERRORS 25

extern int  yyparse(void);
extern int line_number;
const char *fname = "(stdin)";
int fatal_error = 0;

int  main(int,char **);
void yyerror(const char *);
void yywarning(const char *);
int  yywrap(void);
void init_yyin(FILE *);
void init_yyout(FILE *);

#ifdef AZTEC_36
FILE *freopen(char *,char *,FILE *);
#endif
#define Fprintf (void)fprintf

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "error: usage: dgn_comp input output\n");
    return 1;
  }
  char * infile = argv[1];
  char * outfile = argv[2];
  FILE * fin = freopen(infile, "r", stdin);
  if (!fin) {
    Fprintf(stderr, "Can't open %s for input.\n", infile);
    perror(infile);
    return 1;
  }
  FILE * fout = freopen(outfile, WRBMODE, stdout);
  if (!fout) {
    Fprintf(stderr, "Can't open %s for output.\n", outfile);
    perror(outfile);
    return 1;
  }
  init_yyin(fin);
  init_yyout(fout);
  (void) yyparse();
  line_number = 1;
  if (fout && fclose(fout) < 0) {
    Fprintf(stderr, "Can't finish output file.");
    perror(outfile);
    return 1;
  }
  if (fatal_error > 0) return 1;
  return 0;
}

/*
 * Each time the parser detects an error, it uses this function.
 * Here we take count of the errors. To continue farther than
 * MAX_ERRORS wouldn't be reasonable.
 */

void yyerror (const char *s)
{
  (void) fprintf(stderr,"%s : line %d : %s\n",fname,line_number, s);
  if (++fatal_error > MAX_ERRORS) {
    (void) fprintf(stderr,"Too many errors, good bye!\n");
    exit(EXIT_FAILURE);
  }
}

/*
 * Just display a warning (that is : a non fatal error)
 */

void yywarning (const char *s)
{
  (void) fprintf(stderr,"%s : line %d : WARNING : %s\n",fname,line_number,s);
}

int yywrap (void)
{
  SpinCursor(3); /* Don't know if this is a good place to put it ?
                    Is it called for our grammar ? Often enough ?
                    Too often ? -- h+ */
  return 1;
}

/*dgn_main.c*/
