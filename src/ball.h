#ifndef BALL_H
#define BALL_H

/* ### ball.c ### */

void ballfall(void);
void placebc(void);
void unplacebc(void);
void set_bc(int);
void move_bc(int,int,signed char,signed char,signed char,signed char);
bool drag_ball(signed char,signed char, int *,signed char *,signed char *,
        signed char *,signed char *, bool *,bool);
void drop_ball(signed char,signed char);
void drag_down(void);

#endif // BALL_H
