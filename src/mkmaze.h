#ifndef MKMAZE_H
#define MKMAZE_H

void wallification(int,int,int,int);
void walkfrom(int,int);
void makemaz(const char *);
void mazexy(coord *);
void bound_digging(void);
void mkportal(signed char,signed char,signed char,signed char);
bool bad_location(signed char,signed char,signed char,signed char,signed char,signed char);
void place_lregion(signed char,signed char,signed char,signed char,
        signed char,signed char,signed char,signed char,
        signed char,d_level *);
void movebubbles(void);
void water_friction(void);
void save_waterlevel(int,int);
void restore_waterlevel(int);
const char *waterbody_name(signed char,signed char);

#endif
