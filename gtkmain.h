/* ************ Copyright 2003 Steven Grafton

    This program is part of Linpal.

    Linpal is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Linpal is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Linpal; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef GTKMAIN_H
#define GTKMAIN_H

#include <list>
#include <gtk/gtk.h>

#include "gtkbigpage.h"
#include "gtkasset.h"

// some globals
extern GtkAssetMgr	PropMgr;
extern GdkDrawable *	BackBuffer;
extern GdkPixbuf*	ColorPixbuf[16],
		*	ShadowPixbuf,
		*	FacePixbuf[13],
		*	TransPixbuf,
		*	IconPixbuf16,
		*	IconPixbuf32,
		*	IconPixbuf48;
extern GtkTooltips	*	Tooltips;
extern GtkTreeStore	*	UserList,
			*	RoomList;
extern GdkColor 	ColorWhite,
	 		ColorBlack;

extern const gint	PalaceWidth,
			PalaceHeight,
			PropSize,
			BubbleBorder,
			LogChoice_Window,
			LogChoice_Bottom,
			LogChoice_Left,
			LogChoice_Right,
			BrowserChoice_Manual;

// Config loaded variables
extern gint		myLogChoice,
			myBrowserChoice;
extern gchar	*	myFont,
		*	ConfigPath,
		*	SettingsFile,
		*	DefaultBGFile,
		*	ProgramPath;
extern gboolean		myShowTabs,
			myShowSend,
			myShowTabbedLog,
			myAutoSwitch,
			myLogShown,
			BlankPage;

gboolean on_image_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_entry_activate(GtkWidget *widget, gpointer data);
gboolean on_send_clicked(GtkButton * button, gpointer data);
gboolean on_nickname_clicked(GtkButton * button, gpointer data);
gboolean close_tab(GtkWidget *widget, gpointer data);
//gboolean destroy_tab(GtkWidget *widget, GdkEvent * event, gpointer data);
gboolean on_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean on_link_clicked(GtkTextTag *texttag, GObject *arg1, GdkEvent *event, GtkTextIter *arg2, gpointer user_data);

gint ChatTimeout(gpointer param);
void PropRedraw();
void sync_tabs();
void adjust_log(int newchoice);

#endif

