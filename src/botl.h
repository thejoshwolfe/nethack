#ifndef BOTL_H
#define BOTL_H

#include <stdbool.h>

int xlev_to_rank(int);
int title_to_mon(const char *,int *,int *);
void max_rank_sz(void);
int describe_level(char *);
const char *rank_of(int,short,bool);
void bot(void);
void bot1str(char *);
void bot2str(char *);

#endif // BOTL_H
