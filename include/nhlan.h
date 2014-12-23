/*	SCCS Id: @(#)nhlan.h	3.4	1997/04/12	*/
/* Copyright (c) Michael Allison, 1997			*/
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NHLAN_H
#define NHLAN_H
/*
 * Here are the LAN features currently implemented:
 * LAN_MAIL		Mail facility allowing receipt and
 *			reading of mail.
 * LAN_SHARED_BONES	Allows bones files to be stored on a
 *			network share. (Does NOT imply compatibiliy
 *			between unlike platforms)
 */

# ifdef LAN_FEATURES
#  ifdef LAN_MAIL
#define MAIL
#define MAILCKFREQ	  50

#ifndef MAX_BODY_SIZE
#define MAX_BODY_SIZE 1024
#endif

struct lan_mail_struct {
	char sender[120];
	char subject[120];
	boolean body_in_ram;	/* TRUE means body in memory not file */
	char filename[_MAX_PATH];
	char body[MAX_BODY_SIZE];
};
#  endif

# endif /*LAN_FEATURES*/
#endif /*NHLAN_H*/
