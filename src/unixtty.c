/* See LICENSE in the root of this project for change info */
#include "hack.h"
#include "extern.h"
#include "winprocs.h"
#include "flag.h"

#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <curses.h>
#define kill_sym       c_cc[VKILL]
#define erase_sym      c_cc[VERASE]
#define intr_sym       c_cc[VINTR]
#define EXTABS        TAB3
#define tabflgs        c_oflag
#define echoflgs       c_lflag
#define cbrkflgs       c_lflag
#define CBRKMASK       ICANON
#define CBRKON         ! /* reverse condition */
#define OSPEED(x)      (speednum(cfgetospeed(&x)))
#define IS_7BIT(x)     ((x).c_cflag & CS7)
#define inputflags     c_iflag
#define STRIPHI        ISTRIP
#define GTTY(x)       (tcgetattr(0, x))
#define STTY(x)       (tcsetattr(0, TCSADRAIN, x))
#define GTTY2(x)      1
#define STTY2(x)      1
#define nonesuch      (fpathconf(0, _PC_VDISABLE))
#define inittyb2       inittyb
#define curttyb2       curttyb

static struct termios termio;

short   ospeed = 0;     /* gets around "not defined" error message */

        char erase_char, intr_char, kill_char;
static bool settty_needed = false;
struct termios inittyb, curttyb;

static void getioctls (void) {
    tcgetattr(fileno(stdin), &termio);
}

static void setioctls(void) {
    tcsetattr(fileno(stdin), TCSADRAIN, &termio);
}


static int speednum(speed_t speed) {
        switch (speed) {
                case B0:        return 0;
                case B50:       return 1;
                case B75:       return 2;
                case B110:      return 3;
                case B134:      return 4;
                case B150:      return 5;
                case B200:      return 6;
                case B300:      return 7;
                case B600:      return 8;
                case B1200:     return 9;
                case B1800:     return 10;
                case B2400:     return 11;
                case B4800:     return 12;
                case B9600:     return 13;
                case B19200:    return 14;
                case B38400:    return 15;
        }

        return 0;
}

static void
setctty (void)
{
        if(STTY(&curttyb) < 0 || STTY2(&curttyb2) < 0)
                perror("NetHack (setctty)");
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty (void)
{
        if(GTTY(&inittyb) < 0 || GTTY2(&inittyb2) < 0)
                perror("NetHack (gettty)");
        curttyb = inittyb;
        curttyb2 = inittyb2;
        ospeed = OSPEED(inittyb);
        erase_char = inittyb.erase_sym;
        kill_char = inittyb.kill_sym;
        intr_char = inittyb2.intr_sym;
        getioctls();

        /* do not expand tabs - they might be needed inside a cm sequence */
        if(curttyb.tabflgs & EXTABS) {
                curttyb.tabflgs &= ~EXTABS;
                setctty();
        }
        settty_needed = true;
}

/* reset terminal to original state */
void
settty (const char *s)
{
        end_screen();
        if(s) raw_print(s);
        if(STTY(&inittyb) < 0 || STTY2(&inittyb2) < 0)
                perror("NetHack (settty)");
        iflags.echo = (inittyb.echoflgs & ECHO) ? ON : OFF;
        iflags.cbreak = (CBRKON(inittyb.cbrkflgs & CBRKMASK)) ? ON : OFF;
        curttyb.inputflags |= STRIPHI;
        setioctls();
}

void setftty(void) {
int ef = 0;                     /* desired value of flags & ECHO */
int cf = CBRKON(CBRKMASK);      /* desired value of flags & CBREAK */
int change = 0;
        iflags.cbreak = ON;
        iflags.echo = OFF;
        /* Should use (ECHO|CRMOD) here instead of ECHO */
        if((curttyb.echoflgs & ECHO) != ef){
                curttyb.echoflgs &= ~ECHO;
/*              curttyb.echoflgs |= ef;                                 */
                change++;
        }
        if((curttyb.cbrkflgs & CBRKMASK) != cf){
                curttyb.cbrkflgs &= ~CBRKMASK;
                curttyb.cbrkflgs |= cf;
                /* be satisfied with one character; no timeout */
                curttyb.c_cc[VMIN] = 1;         /* was VEOF */
                curttyb.c_cc[VTIME] = 0;        /* was VEOL */
                /* turn off system suspend character
                 * due to differences in structure layout, this has to be
                 * here instead of in ioctl.c:getioctls() with the BSD
                 * equivalent
                 */
                curttyb.c_cc[VSUSP] = nonesuch;
                curttyb.c_cc[VREPRINT] = nonesuch;
                curttyb.c_cc[VDISCARD] = nonesuch;
                curttyb.c_cc[VWERASE] = nonesuch;
                curttyb.c_cc[VLNEXT] = nonesuch;
                change++;
        }
        if(!IS_7BIT(inittyb)) curttyb.inputflags &=~ STRIPHI;
        /* If an interrupt character is used, it will be overriden and
         * set to ^C.
         */
        if(intr_char != nonesuch && curttyb2.intr_sym != '\003') {
            curttyb2.intr_sym = '\003';
            change++;
        }

        if(change) setctty();
        start_screen();
}

void
intron (void)           /* enable kbd interupts if enabled when game started */
{
        /* Ugly hack to keep from changing tty modes for non-tty games -dlc */
        if (!strcmp(windowprocs.name, "tty") &&
            intr_char != nonesuch && curttyb2.intr_sym != '\003') {
            curttyb2.intr_sym = '\003';
            setctty();
        }
}

void
introff (void)          /* disable kbd interrupts if required*/
{
        /* Ugly hack to keep from changing tty modes for non-tty games -dlc */
        if (!strcmp(windowprocs.name, "tty") &&
           curttyb2.intr_sym != nonesuch) {
            curttyb2.intr_sym = nonesuch;
            setctty();
        }
}



#include <sys/vt.h>

int linux_flag_console = 0;

void linux_mapon(void);
void linux_mapoff(void);
void check_linux_console(void);
void init_linux_cons(void);

void linux_mapon (void) {
        if (!strcmp(windowprocs.name, "tty") && linux_flag_console) {
                write(1, "\033(B", 3);
        }
}

void linux_mapoff (void) {
        if (!strcmp(windowprocs.name, "tty") && linux_flag_console) {
                write(1, "\033(U", 3);
        }
}

void check_linux_console (void) {
        struct vt_mode vtm;

        if (isatty(0) && ioctl(0,VT_GETMODE,&vtm) >= 0) {
                linux_flag_console = 1;
        }
}

void init_linux_cons (void) {
        if (!strcmp(windowprocs.name, "tty") && linux_flag_console) {
                atexit(linux_mapon);
                linux_mapoff();
                if (has_colors())
                        iflags.use_color = true;
        }
}


/* fatal error */
void error (const char * s, ...) {
    va_list the_args;
        va_start(the_args, s);
        if(settty_needed)
                settty((char *)0);
        vprintf(s,the_args);
        (void) putchar('\n');
    va_end(the_args);
        exit(EXIT_FAILURE);
}
