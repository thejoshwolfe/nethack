#ifndef SAVE_H
#define SAVE_H

int dosave(void);
void hangup(int);
int dosave0(void);
void savestateinlock(void);
void savelev(int,signed char,int);
void bufon(int);
void bufoff(int);
void bflush(int);
void bwrite(int,void *,unsigned int);
void bclose(int);
void savefruitchn(int,int);
void free_dungeons(void);
void freedynamicdata(void);

#endif // SAVE_H
