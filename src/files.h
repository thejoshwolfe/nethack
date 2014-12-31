#ifndef FILES_H
#define FILES_H

char *fname_encode(const char *, char, char *, char *, int);
char *fname_decode(char, char *, char *, int);
const char *fqname(const char *, int, int);
FILE *fopen_datafile(const char *,const char *,int);
bool uptodate(int,const char *);
void store_version(int);
void set_levelfile_name(char *,int);
int create_levelfile(int,char *);
int open_levelfile(int,char *);
void delete_levelfile(int);
void clearlocks(void);
int create_bonesfile(d_level*,char **, char *);
void commit_bonesfile(d_level *);
int open_bonesfile(d_level*,char **);
int delete_bonesfile(d_level*);
void set_savefile_name(void);
void save_savefile_name(int);
void set_error_savefile(void);
int create_savefile(void);
int open_savefile(void);
int delete_savefile(void);
int restore_saved_game(void);
bool lock_file(const char *,int,int);
void unlock_file(const char *);
void read_config_file(const char *);
void check_recordfile(const char *);
void read_wizkit(void);
void paniclog(const char *, const char *);
int validate_prefix_locations(char *);
char** get_saved_games(void);
void free_saved_games(char**);

#endif // FILES_H
