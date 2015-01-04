/* See LICENSE in the root of this project for change info */
/* used by ckmailstatus() to pass information to the mail-daemon in newmail() */

#ifndef MAIL_H
#define MAIL_H

enum {
    MSG_OTHER, /* catch-all; none of the below... */
    MSG_MAIL, /* unimportant, uninteresting mail message */
    MSG_CALL, /* annoying phone/talk/chat-type interruption */
};

struct mail_info {
        int      message_typ;           /* MSG_foo value */
        const char *display_txt;        /* text for daemon to verbalize */
        const char *object_nam;         /* text to tag object with */
        const char *response_cmd;       /* command to eventually execute */
};

#endif /* MAIL_H */
