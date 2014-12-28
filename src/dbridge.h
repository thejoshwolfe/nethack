#ifndef DBRIDGE_H
#define DBRIDGE_H

#include <stdbool.h>

bool is_pool(int,int);
bool is_lava(int,int);
bool is_ice(int,int);
int is_drawbridge_wall(int,int);
bool is_db_wall(int,int);
bool find_drawbridge(int *,int*);
bool create_drawbridge(int,int,int,bool);
void open_drawbridge(int,int);
void close_drawbridge(int,int);
void destroy_drawbridge(int,int);

#endif // DBRIDGE_H
