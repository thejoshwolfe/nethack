/* See LICENSE in the root of this project for change info */
/* used by ckmailstatus() to pass information to the mail-daemon in newmail() */

#ifndef MAIL_H
#define MAIL_H

#define MSG_OTHER 0     /* catch-all; none of the below... */
#define MSG_MAIL  1     /* unimportant, uninteresting mail message */
#define MSG_CALL  2     /* annoying phone/talk/chat-type interruption */

struct mail_info {
    int message_typ; /* always MSG_MAIL */
    const char *display_txt; /* text for daemon to verbalize */
    const char *object_nam; /* text to tag object with */
    const char *response_cmd; /* command to eventually execute */
};

void ckmailstatus(void);
void readmail(struct obj *);

#endif /* MAIL_H */
