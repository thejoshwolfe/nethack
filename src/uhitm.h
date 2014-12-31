#ifndef UHITM_H
#define UHITM_H

void hurtmarmor(struct monst *,int);
bool attack_checks(struct monst *,struct obj *);
void check_caitiff(struct monst *);
signed char find_roll_to_hit(struct monst *);
bool attack(struct monst *);
bool hmon(struct monst *,struct obj *,int);
int damageum(struct monst *,struct attack *);
void missum(struct monst *,struct attack *);
int passive(struct monst *,bool,int,unsigned char);
void passive_obj(struct monst *,struct obj *,struct attack *);
void stumble_onto_mimic(struct monst *);
int flash_hits_mon(struct monst *,struct obj *);

#endif
