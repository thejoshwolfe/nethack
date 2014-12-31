#ifndef O_INIT_H
#define O_INIT_H

void init_objects(void);
int find_skates(void);
void oinit(void);
void savenames(int,int);
void restnames(int);
void discover_object(int,bool,bool);
void undiscover_object(int);
int dodiscovered(void);

#endif
