#ifndef OPTIONS_H
#define OPTIONS_H

bool match_optname(const char *,const char *,int,bool);
void initoptions(void);
void parseoptions(char *,bool,bool);
int doset(void);
int dotogglepickup(void);
void option_help(void);
void next_opt(winid,const char *);
int fruitadd(char *);
int choose_classes_menu(const char *,int,bool,char *,char *);
void add_menu_cmd_alias(char, char);
char map_menu_cmd(char);
void assign_warnings(unsigned char *);
char *nh_getenv(const char *);
void set_duplicate_opt_detection(int);
void set_wc_option_mod_status(unsigned long, int);
void set_wc2_option_mod_status(unsigned long, int);
void set_option_mod_status(const char *,int);

#endif // OPTIONS_H
