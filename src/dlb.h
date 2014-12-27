/* See LICENSE in the root of this project for change info */
#ifndef DLB_H
#define DLB_H

#include <stdio.h>
#include "global.h"

/* directory structure in memory */
typedef struct dlb_directory {
    char *fname;        /* file name as seen from calling code */
    long foffset;       /* offset in lib file to start of this file */
    long fsize;         /* file size */
    char handling;      /* how to handle the file (compression, etc) */
} libdir;

/* information about each open library */
typedef struct dlb_library {
    FILE *fdata;        /* opened data file */
    long fmark;         /* current file mark */
    libdir *dir;        /* directory of library file */
    char *sspace;       /* pointer to string space */
    long nentries;      /* # of files in directory */
    long rev;           /* dlb file revision */
    long strsize;       /* dlb file string size */
} library;

/* library definitions */
#define DLBFILE "build/nhdat"


typedef struct dlb_handle {
    FILE *fp;           /* pointer to an external file, use if non-null */
    library *lib;       /* pointer to library structure */
    long start;         /* offset of start of file */
    long size;          /* size of file */
    long mark;          /* current file marker */
} dlb;

bool dlb_init(void);
void dlb_cleanup(void);

dlb *dlb_fopen(const char *,const char *);
int dlb_fclose(dlb *);
int dlb_fread(char *,int,int,dlb *);
int dlb_fseek(dlb *,long,int);
char *dlb_fgets(char *,int,dlb *);
int dlb_fgetc(dlb *);
long dlb_ftell(dlb *);

#endif  /* DLB_H */
