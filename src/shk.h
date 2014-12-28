#ifndef SHK_H
#define SHK_H

#include <stdbool.h>

const char *shkname(const struct monst *mtmp);
void shkgone(struct monst *);
void set_residency(struct monst *,bool);
void replshk(struct monst *,struct monst *);
void restshk(struct monst *,bool);
char inside_shop(signed char,signed char);
void u_left_shop(char *,bool);
void remote_burglary(signed char,signed char);
void u_entered_shop(char *);
bool same_price(struct obj *,struct obj *);
void shopper_financial_report(void);
int inhishop(struct monst *);
struct monst *shop_keeper(char);
bool tended_shop(struct mkroom *);
void delete_contents(struct obj *);
void obfree(struct obj *,struct obj *);
void home_shk(struct monst *,bool);
void make_happy_shk(struct monst *,bool);
void hot_pursuit(struct monst *);
void make_angry_shk(struct monst *,signed char,signed char);
int dopay(void);
bool paybill(int);
void finish_paybill(void);
struct obj *find_oid(unsigned);
long contained_cost(struct obj *,struct monst *,long,bool, bool);
long contained_gold(struct obj *);
void picked_container(struct obj *);
long unpaid_cost(struct obj *);
void addtobill(struct obj *,bool,bool,bool);
void splitbill(struct obj *,struct obj *);
void subfrombill(struct obj *,struct monst *);
long stolen_value(struct obj *,signed char,signed char,bool,bool);
void sellobj_state(int);
void sellobj(struct obj *,signed char,signed char);
int doinvbill(int);
struct monst *shkcatch(struct obj *,signed char,signed char);
void add_damage(signed char,signed char,long);
int repair_damage(struct monst *,struct damage *,bool);
int shk_move(struct monst *);
void after_shk_move(struct monst *);
bool is_fshk(const struct monst *);
void shopdig(int);
void pay_for_damage(const char *,bool);
bool costly_spot(signed char,signed char);
struct obj *shop_object(signed char,signed char);
void price_quote(struct obj *);
void shk_chat(struct monst *);
void check_unpaid_usage(struct obj *,bool);
void check_unpaid(struct obj *);
void costly_gold(signed char,signed char,long);
bool block_door(signed char,signed char);
bool block_entry(signed char,signed char);
char *shk_your(char *,struct obj *);
char *Shk_Your(char *,struct obj *);

#endif // SHK_H
