/* See LICENSE in the root of this project for change info */
#ifndef TRADSTDC_H
#define TRADSTDC_H

/*
 * Used for robust ANSI parameter forward declarations:
 * int VDECL(sprintf, (char *, const char *, ...));
 *
 * NDECL() is used for functions with zero arguments;
 * FDECL() is used for functions with a fixed number of arguments;
 * VDECL() is used for functions with a variable number of arguments.
 * Separate macros are needed because ANSI will mix old-style declarations
 * with prototypes, except in the case of varargs, and the OVERLAY-specific
 * trampoli.* mechanism conflicts with the ANSI <<f(void)>> syntax.
 */

#define NDECL(f)	f(void) /* overridden later if USE_TRAMPOLI set */

#define FDECL(f,p)	f p

#define VDECL(f,p)	f p


#endif /* TRADSTDC_H */
