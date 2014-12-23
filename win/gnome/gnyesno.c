/*	SCCS Id: @(#)gnyesno.c	3.4	2000/07/16	*/
/* Copyright (C) 1998 by Erik Andersen <andersee@debian.org> */
/* NetHack may be freely redistributed.  See license for details. */

#include "gnbind.h"
#include "gnyesno.h"



int ghack_yes_no_dialog( const char *question, 
		      const char *choices, int def)
{
    int i=0, ret;
    gchar button_name[BUFSZ];
    GtkWidget *box;
    GtkWidget* mainWnd=NULL;

    box = gnome_message_box_new ( question, GNOME_MESSAGE_BOX_QUESTION, NULL);
    /* add buttons for each choice */
    if (!strcmp(GNOME_STOCK_BUTTON_OK, choices)) {
	gnome_dialog_append_button ( GNOME_DIALOG(box), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_set_default( GNOME_DIALOG(box), 0);
	gnome_dialog_set_accelerator( GNOME_DIALOG(box), 0, 'o', 0);
    }
    else {
	for( ; choices[i]!='\0'; i++) {
	    if (choices[i]=='y') {
		sprintf( button_name, GNOME_STOCK_BUTTON_YES);
	    }
	    else if (choices[i]=='n') {
		sprintf( button_name, GNOME_STOCK_BUTTON_NO);
	    }
	    else if (choices[i] == 'q') {
	        sprintf( button_name, "Quit");
	    } else {
		sprintf( button_name, "%c", choices[i]);
	    }
	    if (def==choices[i])
		gnome_dialog_set_default( GNOME_DIALOG(box), i);
	    gnome_dialog_append_button ( GNOME_DIALOG(box), button_name);
	    gnome_dialog_set_accelerator( GNOME_DIALOG(box), i, choices[i], 0);
	}
    }

    gnome_dialog_set_close(GNOME_DIALOG (box), TRUE);
    mainWnd = ghack_get_main_window ();
    gtk_window_set_modal( GTK_WINDOW(box), TRUE);
    gtk_window_set_title( GTK_WINDOW(box), "GnomeHack");
    if ( mainWnd != NULL ) {
	gnome_dialog_set_parent (GNOME_DIALOG (box), 
		GTK_WINDOW ( mainWnd) );
    }

    ret=gnome_dialog_run_and_close ( GNOME_DIALOG (box));
    
    if (ret==-1)
	return( '\033');
    else
	return( choices[ret]);
}
