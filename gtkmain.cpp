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

#include <string.h>	// strcmp
#include <stdio.h>
#include <stdlib.h>	// atoi
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <gdk/gdkpango.h>
#include <pango/pango.h>
#include <list>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "palclient.h"
#include "gtkmain.h"
#include "gtkbigpage.h"

BigPage * LastPage;
GtkAssetMgr PropMgr;

GtkWidget	*	MainWindow,
		*	LogWindow,
		*	PrefWindow,
		*	FileSelWindow,
		*	ConnWindow,
		*	AboutWindow,
		*	NameWindow,
		*	FontWindow,
		*	UserWindow,
		*	RoomWindow,
		*	ExpressionWindow,
		*	DissMenu,
		*	ExpressionMenu,
		*	ClearLogMenu,
		*	SaveLogMenu,
		*	UserListMenu,
		*	RoomListMenu,
		*	ColorMenu,
		*	FaceMenu,
		*	PreviewArea,
		*	LogMenu,
		*	ServerEntry,
		*	WhisperEntry,
		*	WhisperLabel,
		*	WhisperButton,
		*	PortEntry,
		*	FontDrawArea,
		*	NicknameEntry,
		*	HomePalace,
		*	HomePort,
		*	DefaultName,
		*	ConnectName,
		*	ConnectAddr,
		*	ConnectPort,
		*	OptionLogPos,
		*	CheckAutoSwitch,
		*	CheckTabs,
		*	RoomTreeView,
		*	UserTreeView,
		*	CheckTabbedLog,
		*	CheckShowSend,
		*	JumpRoomButton,
		*	JumpUserButton,
		*	WhisperUserButton,
		*	VBoxMain,
		*	TableMain,
		*	LogNotebook,
		*	OptionBrowser,
		*	BrowserEntry,
		*	PropDrawArea,
		*	Notebook;
GdkPixbuf	*	ColorPixbuf[16],
		*	ShadowPixbuf,
		*	FacePixbuf[13],
		*	TransPixbuf,
		*	IconPixbufWorld;
GtkTooltips	*	Tooltips;
GdkColor 		ColorWhite,
	 		ColorBlack;
GdkDrawable	*	BackBuffer,
		*	PreviewBuffer,
		*	PropBackBuffer;
GList		*	WindowIconList = NULL,
		*	BigPageList = NULL;

GtkTreeStore	*	UserList = NULL,
		*	RoomList = NULL;

const gint		PalaceWidth = 512,
			PalaceHeight = 384,
			PropSize = 44,
			BubbleBorder = 6,
			LogChoice_Window = 0,
			LogChoice_Bottom = 1,
			LogChoice_Left = 2,
			LogChoice_Right = 3,
			BrowserChoice_Manual = 5;

const gchar *Browser[5] = {	"mozilla %s &",
				"MozillaFirebird %s &",
				"galeon %s &",
				"epiphany %s &",
				"konqueror %s &" };

gboolean RoomWindowvisible = TRUE;
gboolean UserWindowvisible = TRUE;
gboolean myExpressionWindowShown = FALSE;

// Config loaded variables
gint			myLogChoice = 0,
			myBrowserChoice = 0;
gchar		*	myFont = 0,
		*	ConfigPath = 0,
		*	SettingsFile = 0,
		*	DefaultBGFile = 0,
		*	ProgramPath = 0;
gboolean		myShowTabs,
			myShowSend,
			myShowTabbedLog,
			myAutoSwitch,
			myLogShown = TRUE,
			BlankPage = TRUE;


void on_user_list_delete_event();
void on_room_list_delete_event();
void on_user_list_activate();
void on_room_list_activate();

// ============================================
void PropRedraw(){
	return;
	if(PropMgr.Head() == NULL){
		gtk_widget_set_size_request(PropDrawArea, PropSize, 1);
		return;
	}
	gint px, py;
	gtk_widget_get_size_request(PropDrawArea, &px, &py);
	int props_x = px / PropSize;
	gtk_widget_set_size_request(PropDrawArea,  px,
					PropSize * (g_list_length(PropMgr.Head()) / props_x));

	g_free(PropBackBuffer);
	PropBackBuffer = gdk_pixmap_new(GDK_DRAWABLE(ExpressionWindow->window), px,
					PropSize *
					 ((g_list_length(PropMgr.Head()) / props_x) +
					  1),
					-1);
	GdkGC	*gc = gdk_gc_new(PropBackBuffer);

	//gdk_gc_set_foreground(gc, &ColorWhite);
	/*GdkRectangle * rect = new GdkRectangle();
	rect->x = 0;
	rect->y = 0;
	rect->width = PropSize;
	rect->height = PropSize * g_list_length(PropMgr.Head());
	gdk_gc_set_clip_rectangle(gc, rect);
*/
	int y = 0;
	for(GList * i = g_list_first(PropMgr.Head()); i != NULL; i = g_list_next(i)){
		for(int x = 0; x < props_x; x++){
			if(i->data != NULL){
			gdk_draw_pixbuf(PropBackBuffer, gc, TransPixbuf,
					0, 0, x * PropSize, y, PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
			gdk_draw_pixbuf(PropBackBuffer, gc, ((AssetNode *)(i->data))->data,
					0, 0, x * PropSize, y, PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
			y += PropSize;
			}
		}
	}
	gdk_draw_drawable(PropDrawArea->window, gc, PropBackBuffer,
				0, 0, 0, 0, props_x * PropSize, y);

	g_object_unref(gc);
}

gpointer ConnRunner(gpointer local){

	gdk_threads_enter();
	gtk_widget_hide(ConnWindow);
	gtk_widget_set_sensitive(ClearLogMenu, TRUE);
	gtk_widget_set_sensitive(SaveLogMenu, TRUE);
	BigPage * page = new BigPage(Notebook, LogNotebook);

	if(BlankPage == TRUE){
		gtk_notebook_remove_page(GTK_NOTEBOOK(Notebook), 0);	
		gtk_notebook_remove_page(GTK_NOTEBOOK(LogNotebook), 0);	
		// Should already be destroyed
		//BigPageList = g_list_remove(BigPageList, g_list_first(BigPageList));
		BlankPage = FALSE;
		if(LastPage != NULL){
			if(BigPageList != NULL){
				g_list_free(BigPageList);
				BigPageList = NULL;
			}
			delete LastPage;
			LastPage = NULL;
		}
	} 
	sync_tabs();

	gdk_threads_leave();

	gboolean ret = page->Connect(	gtk_entry_get_text(GTK_ENTRY(ConnectAddr)),
					atoi(gtk_entry_get_text(GTK_ENTRY(ConnectPort))),
					gtk_entry_get_text(GTK_ENTRY(ConnectName)));

	if(ret != FALSE){
		gdk_threads_enter();
		gtk_widget_set_sensitive(DissMenu, TRUE);
		gtk_widget_set_sensitive(UserListMenu, TRUE);
		gtk_widget_set_sensitive(RoomListMenu, TRUE);
		gtk_widget_set_sensitive(ExpressionMenu, TRUE);
		BigPageList = g_list_append(BigPageList, page);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(Notebook), -1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(LogNotebook), -1);
		gdk_threads_leave();
	} else if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(Notebook)) == 1){
		BlankPage = TRUE;
		LastPage = page;
		
	}

	return NULL;
}


// ========= UTITLITY FUNCTIONS ====================
void update_preview(){
	gint	color = gtk_option_menu_get_history(GTK_OPTION_MENU(ColorMenu)),
		face = gtk_option_menu_get_history(GTK_OPTION_MENU(FaceMenu));

	if(TransPixbuf == NULL || ShadowPixbuf == NULL)
		return;

	GdkGC	*gc = gdk_gc_new(PreviewBuffer);

	for(int x = 0; x < 3; x++)
		for(gint y = 0; y < 3; y++)
			gdk_draw_pixbuf(PreviewBuffer, gc, TransPixbuf,
					0, 0, x * PropSize, y * PropSize,
					PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);

	gdk_draw_pixbuf(PreviewBuffer, gc, ShadowPixbuf,
				0, 0, PropSize, PropSize, PropSize, PropSize,
				GDK_RGB_DITHER_NONE, 0, 0);
	gdk_draw_pixbuf(PreviewBuffer, gc, ColorPixbuf[color],
				0, 0, PropSize, PropSize, PropSize, PropSize,
				GDK_RGB_DITHER_NONE, 0, 0);
	gdk_draw_pixbuf(PreviewBuffer, gc, FacePixbuf[face],
				0, 0, PropSize, PropSize, PropSize, PropSize,
				GDK_RGB_DITHER_NONE, 0, 0);

	/*for(int x = 0; x < (PropSize * 3); x += PropSize){
		for(gint y = 0; y < (PropSize * 3); y += PropSize){
			gdk_draw_pixbuf(PreviewArea->window, gc, prop,
					0, 0, x * PropSize, y * PropSize,
					PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
		}
	}
	*/

	g_object_unref(gc);
	gtk_widget_queue_draw_area(PreviewArea, 0, 0, PropSize * 3, PropSize * 3);

}

void adjust_log(int newchoice){
	if(myLogChoice == newchoice)
		return;

	if(newchoice == LogChoice_Window){
		gtk_window_set_resizable(GTK_WINDOW(MainWindow), FALSE);	
		gtk_container_remove(GTK_CONTAINER(TableMain), LogNotebook);
		gtk_container_add(GTK_CONTAINER(LogWindow), LogNotebook);
		if(myLogShown)	gtk_widget_show(LogWindow);
		else		gtk_widget_hide(LogWindow);
	} else {
		gtk_window_set_resizable(GTK_WINDOW(MainWindow), FALSE);
		gtk_widget_hide(LogWindow);
		if(myLogChoice == LogChoice_Window)
			gtk_container_remove(GTK_CONTAINER(LogWindow), LogNotebook);
		else
			gtk_container_remove(GTK_CONTAINER(TableMain), LogNotebook);

		if(myLogShown)	gtk_widget_show(LogNotebook);
		else		gtk_widget_hide(LogNotebook);
		
		if	(newchoice == LogChoice_Left)
			gtk_table_attach_defaults(GTK_TABLE(TableMain), LogNotebook, 0, 1, 0, 1);
		else if	(newchoice == LogChoice_Right)
			gtk_table_attach_defaults(GTK_TABLE(TableMain), LogNotebook, 2, 3, 0, 1);
		else if (newchoice == LogChoice_Bottom)
			gtk_table_attach_defaults(GTK_TABLE(TableMain), LogNotebook, 1, 2, 1, 2);
		
		gtk_window_set_resizable(GTK_WINDOW(MainWindow), TRUE);	
	}

	myLogChoice = newchoice;
}

void sync_tabs(){
	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(Notebook)) == 1){
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(Notebook), myShowTabs);
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(LogNotebook), myShowTabs);
	} else{
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(Notebook), TRUE);
		if(myShowTabbedLog == TRUE)
			gtk_notebook_set_show_tabs(GTK_NOTEBOOK(LogNotebook), TRUE);
	}
	if(myShowTabbedLog == FALSE)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(LogNotebook), FALSE);
}

void refresh_roomlists(){
	GtkTreeIter iter; 
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(RoomList), &iter) == TRUE)
		while(gtk_tree_store_remove(RoomList, &iter) == TRUE);

	for(GList * i = g_list_first(BigPageList); i != NULL; i = g_list_next(i))
		((BigPage *)(i->data))->RequestRoomList();

	gtk_widget_set_sensitive(JumpRoomButton, FALSE);
}

void refresh_userlists(){
	GtkTreeIter iter; 
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(UserList), &iter) == TRUE)
		while(gtk_tree_store_remove(UserList, &iter) == TRUE);

	for(GList * i = g_list_first(BigPageList); i != NULL; i = g_list_next(i))
		((BigPage *)(i->data))->RequestUserList();
	gtk_widget_set_sensitive(JumpUserButton, FALSE);
	gtk_widget_set_sensitive(WhisperEntry, FALSE);
	gtk_widget_set_sensitive(WhisperLabel, FALSE);
	gtk_widget_set_sensitive(WhisperButton, FALSE);
}

// ======= GTK CALLBACKS =======================

gboolean on_room_cursor(GtkTreeView * tv, gpointer data){
	GtkTreeIter iter;
	GtkTreeModel * tm = GTK_TREE_MODEL(RoomList);
	gint room;
	GtkTreeSelection * select = gtk_tree_view_get_selection(tv);
	if(gtk_tree_selection_get_selected(select, &tm, &iter) == FALSE){
		gtk_widget_set_sensitive(JumpRoomButton, FALSE);
		return TRUE;
	}

	gtk_tree_model_get(tm, &iter, 3, (gpointer)&room, -1);
	gtk_widget_set_sensitive(JumpRoomButton, (room == 0 ? FALSE : TRUE));

	return TRUE;
}


gboolean on_user_cursor(GtkTreeView * tv, gpointer data){
	GtkTreeIter iter;
	GtkTreeModel * tm = GTK_TREE_MODEL(UserList);
	gint room;
	GtkTreeSelection * select = gtk_tree_view_get_selection(tv);
	if(gtk_tree_selection_get_selected(select, &tm, &iter) == FALSE){
		gtk_widget_set_sensitive(JumpUserButton, FALSE);
		gtk_widget_set_sensitive(WhisperEntry, FALSE);
		gtk_widget_set_sensitive(WhisperLabel, FALSE);
		gtk_widget_set_sensitive(WhisperButton, FALSE);
		return TRUE;
	}

	gtk_tree_model_get(tm, &iter, 2, (gpointer)&room, -1);
	gtk_widget_set_sensitive(JumpUserButton, (room == 0 ? FALSE : TRUE));
	gtk_widget_set_sensitive(WhisperEntry, (room == 0 ? FALSE : TRUE));
	gtk_widget_set_sensitive(WhisperLabel, (room == 0 ? FALSE : TRUE));
	gtk_widget_set_sensitive(WhisperButton, (room == 0 ? FALSE : TRUE));

	return TRUE;
}

void on_room_clicked(GtkWidget * w, gpointer data){
	GtkTreeIter iter;
	GtkTreeModel * tm = GTK_TREE_MODEL(RoomList);
	gint room;
	GtkTreeSelection * select = gtk_tree_view_get_selection(GTK_TREE_VIEW(RoomTreeView));
	if(gtk_tree_selection_get_selected(select, &tm, &iter) == FALSE){
		gtk_widget_set_sensitive(JumpRoomButton, FALSE);
		return;
	}

	gtk_tree_model_get(tm, &iter, 3, (gpointer)&room, -1);
	if(room != 0){
		// do the jump
		BigPage * p;
		gint roomid;

		gtk_tree_model_get(tm, &iter, 5, &p, -1);
		gtk_tree_model_get(tm, &iter, 0, &roomid, -1);
		p->Navigate(roomid);
		on_room_list_delete_event();
	}
}

void on_userjump_clicked(GtkWidget * w, gpointer data){
	GtkTreeIter iter;
	GtkTreeModel * tm = GTK_TREE_MODEL(UserList);
	gint room;
	GtkTreeSelection * select = gtk_tree_view_get_selection(GTK_TREE_VIEW(UserTreeView));
	if(gtk_tree_selection_get_selected(select, &tm, &iter) == FALSE){
		gtk_widget_set_sensitive(JumpUserButton, FALSE);
		gtk_widget_set_sensitive(WhisperButton, FALSE);
		gtk_widget_set_sensitive(WhisperEntry, FALSE);
		gtk_widget_set_sensitive(WhisperLabel, FALSE);
		return;
	}

	gtk_tree_model_get(tm, &iter, 2, (gpointer)&room, -1);
	if(room != 0){
		// do the jump
		BigPage * p;
		gint roomid;

		gtk_tree_model_get(tm, &iter, 4, &p, -1);
		gtk_tree_model_get(tm, &iter, 3, &roomid, -1);
		p->Navigate(roomid);
		on_user_list_delete_event();
	}
}

gboolean on_link_clicked(GtkTextTag *texttag, GObject *arg1, GdkEvent *event,
				GtkTextIter *arg2, gpointer user_data){
	if(event->type != GDK_BUTTON_PRESS)
		return FALSE;
	gchar	*url,
		*exec;
	GtkTextIter * iter =  gtk_text_iter_copy(arg2);
	GtkTextIter * iter2 = gtk_text_iter_copy(iter);
	gtk_text_iter_backward_to_tag_toggle(iter, texttag);
	gtk_text_iter_forward_to_tag_toggle(iter2, texttag);
	url = gtk_text_iter_get_text(iter, iter2);
	g_free(iter);
	g_free(iter2);
	
	if(myBrowserChoice == BrowserChoice_Manual)
		exec = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(BrowserEntry)), url);
	else
		exec = g_strdup_printf(Browser[myBrowserChoice], url);

	g_free(url);
	system(exec);
	g_free(exec);

	return TRUE;
}

void on_homepage_clicked(GtkWidget * widget, gpointer data){
	gchar * exec;
	if(myBrowserChoice == BrowserChoice_Manual)
		exec = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(BrowserEntry)),
					"http://www.ruinedsoft.com");
	else
		exec = g_strdup_printf(Browser[myBrowserChoice],
					"http://www.ruinedsoft.com/");
	system(exec);
	g_free(exec);
}

void on_browser_changed(){
	myBrowserChoice = gtk_option_menu_get_history(GTK_OPTION_MENU(OptionBrowser));
	gtk_widget_set_sensitive(BrowserEntry, (myBrowserChoice == BrowserChoice_Manual ? TRUE : FALSE));
}

void on_change_page(){
	if(myAutoSwitch)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(LogNotebook), 
					gtk_notebook_get_current_page(GTK_NOTEBOOK(Notebook)));
}

void on_mainwindow_delete_event(){
	gtk_main_quit();
	g_list_free(BigPageList);
	BigPageList = NULL;
	g_free(myFont);
	g_free(ConfigPath);
	g_free(DefaultBGFile);
	g_free(SettingsFile);
	g_free(ProgramPath);
	g_object_unref(LogNotebook);
	for(int i = 0; i < 16; i++)
		g_free(ColorPixbuf[i]);
	for(int i = 0; i < 13; i++)
		g_free(FacePixbuf[i]);
	g_free(ShadowPixbuf);
	g_free(IconPixbufWorld);
//	gtk_object_destory(GTK_OBJECT(FileSelWindow));
}

gboolean on_send_clicked(GtkButton * button, gpointer data){
	((BigPage *)data)->Send();
	return TRUE;
}

gboolean close_tab(GtkWidget *widget, gpointer data){
	BigPage * p = (BigPage *)data;
	int num = p->PageIndex();
	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(Notebook)) == 1){
		p->Disconnect();
		LastPage = p;
		BlankPage = TRUE;
		gtk_widget_set_sensitive(DissMenu, FALSE);
		gtk_widget_set_sensitive(UserListMenu, FALSE);
		gtk_widget_set_sensitive(RoomListMenu, FALSE);
		gtk_widget_set_sensitive(ExpressionMenu, FALSE);
		return TRUE;
	}
//	gtk_notebook_remove_page(GTK_NOTEBOOK(LogNotebook), num);
//	gtk_notebook_remove_page(GTK_NOTEBOOK(Notebook), num);

	BigPageList = g_list_remove(BigPageList, p);

	sync_tabs();
	delete p;
	
	return TRUE;
}

BigPage * nickPage = NULL;
void on_nickname_close(){
	gtk_widget_hide(NameWindow);
}

gboolean on_nickname_okay(GtkWidget * widget, gpointer data){
	if(nickPage == NULL)
		return TRUE;
	nickPage->NickName(gtk_entry_get_text(GTK_ENTRY(NicknameEntry)));
	gtk_widget_hide(NameWindow);
	return TRUE;
}

gboolean on_nickname_clicked(GtkButton * button, gpointer data){
	BigPage * p = (BigPage *)data;
	nickPage = p;
	gtk_entry_set_text(GTK_ENTRY(NicknameEntry), p->GetName());
	gtk_widget_show_all(NameWindow);
	return TRUE;
}

// add saving log options here
void on_filesel_ok_clicked(){
	gtk_widget_hide(FileSelWindow);
	gchar * file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileSelWindow));

	if(BigPageList != NULL){
		int num = gtk_notebook_get_current_page(GTK_NOTEBOOK(LogNotebook));
		BigPage * p = (BigPage *)g_list_nth_data(BigPageList, num);
		if(p != NULL)
			p->SaveLog(file);
	}
	g_free(file);
}

void on_filesel_cancel_clicked(){
	gtk_widget_hide(FileSelWindow);
}

void on_filesel_clicked(GtkDialog * diag, gint respid, gpointer user_data){
	switch(respid){
	case GTK_RESPONSE_CANCEL:
		gtk_widget_hide(FileSelWindow);
		break;
	case GTK_RESPONSE_OK: {
		gtk_widget_hide(FileSelWindow);
		gchar * file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileSelWindow));

		if(BigPageList != NULL){
			int num = gtk_notebook_get_current_page(GTK_NOTEBOOK(LogNotebook));
			BigPage * p = (BigPage *)g_list_nth_data(BigPageList, num);
			if(p != NULL)
				p->SaveLog(file);
		}
		g_free(file);
	} break;
	default:
		break;
	}
}

void on_connwindow_ok_clicked(){
	g_thread_create(ConnRunner, NULL, FALSE, NULL);
}

void on_connwindow_cancel_clicked(GtkWidget * widget, gpointer data){
	gtk_widget_hide(ConnWindow);
}

gboolean on_entry_activate(GtkWidget * widget, gpointer data){
	BigPage * page = (BigPage *)data;
	page->Say(gtk_entry_get_text(GTK_ENTRY(widget)));
	gtk_entry_set_text(GTK_ENTRY(widget), "");
	return TRUE;
}

void on_whisper_clicked(GtkWidget * widget){
	gchar * room;
	GtkTreeIter iter;
	GtkTreeModel * tm = GTK_TREE_MODEL(UserList);

	GtkTreeSelection * select = gtk_tree_view_get_selection(GTK_TREE_VIEW(UserTreeView));
	if(gtk_tree_selection_get_selected(select, &tm, &iter) == FALSE){
		gtk_widget_set_sensitive(JumpUserButton, FALSE);
		return;
	}

	gtk_tree_model_get(tm, &iter, 2, (gpointer)&room, -1);
	if(room != 0){
		BigPage * p;
		gint idto;
		gtk_tree_model_get(tm, &iter, 4, &p, -1);
		gtk_tree_model_get(tm, &iter, 0, &idto, -1);
		p->Whisper(idto, gtk_entry_get_text(GTK_ENTRY(WhisperEntry)));
	}

	gtk_entry_set_text(GTK_ENTRY(WhisperEntry), "");
	return;
}

void on_connect_activate(){
	gtk_widget_show(ConnWindow);
	gtk_entry_set_text(GTK_ENTRY(ConnectName), gtk_entry_get_text(GTK_ENTRY(DefaultName)));
	gtk_entry_set_text(GTK_ENTRY(ConnectAddr), gtk_entry_get_text(GTK_ENTRY(HomePalace)));
	gtk_entry_set_text(GTK_ENTRY(ConnectPort), gtk_entry_get_text(GTK_ENTRY(HomePort)));
}

void on_disconnect_activate(){
	int num = gtk_notebook_get_current_page(GTK_NOTEBOOK(Notebook));
	BigPage * p = (BigPage *)g_list_nth_data(BigPageList, num);
	if(num == 0){
		p->Disconnect();
		LastPage = p;
		BlankPage = TRUE;
		gtk_widget_set_sensitive(DissMenu, FALSE);
		gtk_widget_set_sensitive(UserListMenu, FALSE);
		gtk_widget_set_sensitive(RoomListMenu, FALSE);
		gtk_widget_set_sensitive(ExpressionMenu, FALSE);
		return;
	} 
	gtk_notebook_remove_page(GTK_NOTEBOOK(Notebook), num);
	gtk_notebook_remove_page(GTK_NOTEBOOK(LogNotebook), num);
	
	BigPageList = g_list_remove(BigPageList, p);
	sync_tabs();

	//gtk_widget_destroy(page);
	delete p;

	return;
}

void on_log_window_delete_event(){
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(LogMenu), FALSE);
}

void on_user_list_delete_event(){
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(UserListMenu), FALSE);
	gtk_widget_set_sensitive(JumpUserButton, FALSE);
	UserWindowvisible = FALSE;
	on_user_list_activate();
}

void on_room_list_delete_event(){
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(RoomListMenu), FALSE);
	gtk_widget_set_sensitive(JumpRoomButton, FALSE);
	RoomWindowvisible = FALSE;
	on_room_list_activate();
}

void on_log_window_activate(){
	myLogShown = !myLogShown;
	if(myLogChoice == LogChoice_Window){
		if(myLogShown)	gtk_widget_show(LogWindow);
		else		gtk_widget_hide(LogWindow);
	} else 
		gtk_window_set_resizable(GTK_WINDOW(MainWindow), (myLogShown ? TRUE : FALSE));	
	
	if(myLogShown)	gtk_widget_show(LogNotebook);
	else		gtk_widget_hide(LogNotebook);
}

BigPage * ExpressionPage = NULL;
void on_expression_activate(){
	myExpressionWindowShown = !myExpressionWindowShown;
	ExpressionPage = (BigPage *)g_list_nth_data(BigPageList,
						gtk_notebook_get_current_page(GTK_NOTEBOOK(Notebook)));
	
	if(myExpressionWindowShown){
		gtk_widget_show(ExpressionWindow);
		gtk_option_menu_set_history(GTK_OPTION_MENU(ColorMenu), ExpressionPage->GetColor());
		gtk_option_menu_set_history(GTK_OPTION_MENU(FaceMenu), ExpressionPage->GetFace());
	} else
		gtk_widget_hide(ExpressionWindow);
}

void on_room_list_activate(){
	RoomWindowvisible = !RoomWindowvisible;
	if(RoomWindowvisible == TRUE)
		gtk_widget_hide(RoomWindow);
	else {
		refresh_roomlists();
		gtk_widget_show(RoomWindow);
	}
}
void on_user_list_activate(){
	UserWindowvisible = !UserWindowvisible;
	if(UserWindowvisible == TRUE)
		gtk_widget_hide(UserWindow);
	else {
		refresh_userlists();
		gtk_widget_show(UserWindow);
	}
}

gboolean on_expression_delete_event(){
	gtk_widget_hide(ExpressionWindow);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ExpressionMenu), FALSE);

	gint	color = gtk_option_menu_get_history(GTK_OPTION_MENU(ColorMenu)),
		face = gtk_option_menu_get_history(GTK_OPTION_MENU(FaceMenu)),
		props_time[9],
		props_crc[9];

	if(ExpressionPage != NULL)
		ExpressionPage->NewLook(color, face, 0, props_time, props_crc);
	
	return TRUE;
}

void on_prefwindow_delete_event(){
	gtk_widget_hide(PrefWindow);

	myAutoSwitch = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CheckAutoSwitch));
	myShowTabbedLog = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CheckTabbedLog));
	myShowTabs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CheckTabs));
	myShowSend = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CheckShowSend));
	int newer = gtk_option_menu_get_history(GTK_OPTION_MENU(OptionLogPos));

	for(GList * i = g_list_first(BigPageList); i != NULL; i = g_list_next(i))
		((BigPage *)i->data)->ShowSendButton(myShowSend);

	if(BlankPage)
		LastPage->ShowSendButton(myShowSend);

	adjust_log(newer);
	sync_tabs();

	// write out preferences here
}


gboolean on_checktabbedlog_toggled(GtkWidget * widget, gboolean togged, gpointer data){
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CheckTabbedLog)) == TRUE){
		gtk_widget_set_sensitive(CheckAutoSwitch, TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(CheckAutoSwitch), TRUE);
		gtk_widget_set_sensitive(CheckAutoSwitch, FALSE);
	}
	return TRUE;
}

void on_pref_activate(){
	gtk_widget_show(PrefWindow);
}

void on_about_activate(){
	gtk_widget_show(AboutWindow);
}

void on_savelog_activate(){
	gtk_widget_show(FileSelWindow);
}

void on_clearlog_activate(){
	int num = gtk_notebook_get_current_page(GTK_NOTEBOOK(LogNotebook));
	BigPage * p = (BigPage *)g_list_nth_data(BigPageList, num);
	p->ClearLog();
	if(BlankPage == TRUE){
		gtk_widget_set_sensitive(SaveLogMenu, FALSE);
		gtk_widget_set_sensitive(ClearLogMenu, FALSE);
	}
}

void on_fontselect_cancel(){
	gtk_widget_hide(FontWindow);
}

void on_fontselect_redraw(){
	GdkGC	*gc = gdk_gc_new(FontDrawArea->window);
	gint	width, height;

	PangoContext * context = gtk_widget_create_pango_context(FontDrawArea);
	PangoLayout * layout = pango_layout_new(context);
	// select from preferences dialog
	PangoFontDescription * fontdesc = pango_font_description_from_string(myFont);
	pango_layout_set_font_description(layout, fontdesc);
	pango_layout_set_text(layout, myFont, -1);
	pango_layout_get_pixel_size(layout, &width, &height);
	gdk_draw_layout(FontDrawArea->window, gc, 0, 5, layout);
	
	pango_font_description_free(fontdesc);
	g_object_unref(layout);
	g_object_unref(context);
	g_object_unref(gc);
}

void on_fontselect_okay(){
	gtk_widget_hide(FontWindow);
	myFont = g_strdup(gtk_font_selection_dialog_get_font_name(
				GTK_FONT_SELECTION_DIALOG(FontWindow)));
	gtk_widget_queue_draw_area(FontDrawArea, 0, 0, 500, 500);
}

void on_fontselect_clicked(){
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(FontWindow), myFont);
	gint result = gtk_dialog_run (GTK_DIALOG (FontWindow));

	switch (result) {
	case GTK_RESPONSE_OK:
	case GTK_RESPONSE_APPLY:
		on_fontselect_okay();
		break;
	default:
		on_fontselect_cancel();
		break;
	}
}

gboolean on_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data){
	int x = (int)event->x;
	int y = (int)event->y;
	((BigPage *)data)->Move(x, y);
	return TRUE;
}

gboolean on_prop_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){

	//PropRedraw();
	GdkGC * gc = gdk_gc_new(widget->window);
	gdk_draw_drawable(widget->window, gc, PropBackBuffer,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height );
	g_object_unref(gc);

	return TRUE;
}

gboolean on_preview_expose_event(GtkWidget * widget, GdkEventExpose *event, gpointer data){

	GdkGC * gc = gdk_gc_new(widget->window);
	gdk_draw_drawable(widget->window, gc, PreviewBuffer,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height );
	g_object_unref(gc);

	return TRUE;
}

gboolean on_image_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){

	((BigPage *)data)->Redraw();
	GdkGC * gc = gdk_gc_new(widget->window);
	gdk_draw_drawable(widget->window, gc, BackBuffer,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height );
	g_object_unref(gc);

	return TRUE;
}

int main(int argc, char **argv){
	GladeXML * xml;
	gchar * file;

//	if(!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();
	gtk_init (&argc, &argv);
	ConfigPath = g_build_filename(g_get_home_dir(), ".linpal", NULL);
	SettingsFile = g_build_filename(ConfigPath, "settings.xml", NULL);
	ProgramPath = g_path_get_dirname(argv[0]);
	DefaultBGFile = g_build_filename(ProgramPath, "default.png", NULL);

	g_print("Run Program Path:   %s\n", ProgramPath);
	g_print("Local Storage Path: %s\n", ConfigPath);
	g_print("Settings File:      %s\n", SettingsFile);
	g_print("Default Background: %s\n", DefaultBGFile);
    
	mkdir(ConfigPath, 0700);
	gchar * PropPath = g_build_filename(ConfigPath, "props", NULL);
	mkdir(PropPath, 0700);
	g_free(PropPath);

    	gchar * gladefile = g_build_filename(ProgramPath, "linpal.glade", NULL);
	xml = glade_xml_new(gladefile, NULL, NULL);
	g_free(gladefile);

	MainWindow = glade_xml_get_widget(xml, "mainwindow");
	VBoxMain = glade_xml_get_widget(xml, "vboxmain");
	TableMain = glade_xml_get_widget(xml, "tablemain");
	LogWindow = glade_xml_get_widget(xml, "logwindow");
	FileSelWindow =  (GtkWidget *)gtk_file_chooser_dialog_new("Save As", GTK_WINDOW(MainWindow),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_OK,
						NULL);
	//glade_xml_get_widget(xml, "fileselwindow");
	ConnWindow = glade_xml_get_widget(xml, "connwindow");
	UserWindow = glade_xml_get_widget(xml, "userwindow");
	RoomWindow = glade_xml_get_widget(xml, "roomwindow");
	AboutWindow = glade_xml_get_widget(xml, "aboutwindow");
	ExpressionWindow = glade_xml_get_widget(xml, "expressionwindow");
	PrefWindow = glade_xml_get_widget(xml, "prefwindow");
	NameWindow = glade_xml_get_widget(xml, "namewindow");
	FontWindow = glade_xml_get_widget(xml, "fontselectionwindow");
	HomePalace = glade_xml_get_widget(xml, "entryhomepalace");
	PreviewArea = glade_xml_get_widget(xml, "drawingareapreview");
	HomePort = glade_xml_get_widget(xml, "entryhomeport");
	FontDrawArea = glade_xml_get_widget(xml, "fontdrawingarea");
	DefaultName = glade_xml_get_widget(xml, "defaultnameentry");
	NicknameEntry = glade_xml_get_widget(xml, "nicknameentry");
	ConnectAddr = glade_xml_get_widget(xml, "serverentry");
	ConnectPort = glade_xml_get_widget(xml, "portentry");
	ConnectName = glade_xml_get_widget(xml, "connectnameentry");
	LogNotebook = glade_xml_get_widget(xml, "lognotebook");
	Notebook = glade_xml_get_widget(xml, "notebook");
	DissMenu = glade_xml_get_widget(xml, "disconnect");
	RoomListMenu = glade_xml_get_widget(xml, "room_list1");
	UserListMenu = glade_xml_get_widget(xml, "user_list1");
	ExpressionMenu = glade_xml_get_widget(xml, "expression_menu");
	SaveLogMenu = glade_xml_get_widget(xml, "savelog");
	ClearLogMenu = glade_xml_get_widget(xml, "clearlog");
	LogMenu = glade_xml_get_widget(xml, "logwindowmenu");
	ColorMenu = glade_xml_get_widget(xml, "optionmenucolor");
	FaceMenu = glade_xml_get_widget(xml, "optionmenuface");
	WhisperEntry = glade_xml_get_widget(xml, "whisperentry");
	WhisperLabel = glade_xml_get_widget(xml, "whisperlabel");
	WhisperButton = glade_xml_get_widget(xml, "whisperbutton");
	CheckTabs = glade_xml_get_widget(xml, "checkenabletabs");
	CheckAutoSwitch = glade_xml_get_widget(xml, "checkautoswitch");
	CheckTabbedLog = glade_xml_get_widget(xml, "checktabbedlog");
	CheckShowSend = glade_xml_get_widget(xml, "checkshowsend");
	OptionLogPos = glade_xml_get_widget(xml, "optionlog");
	OptionBrowser = glade_xml_get_widget(xml, "optionbrowser");
	PropDrawArea = glade_xml_get_widget(xml, "propdrawarea");
	BrowserEntry = glade_xml_get_widget(xml, "browserentry");
	JumpRoomButton = glade_xml_get_widget(xml, "jumproom");
	JumpUserButton = glade_xml_get_widget(xml, "jumpuser");
	RoomTreeView = glade_xml_get_widget(xml, "treeviewroom");
	UserTreeView = glade_xml_get_widget(xml, "treeviewuser");
	WhisperUserButton = glade_xml_get_widget(xml, "whisperuser");

	glade_xml_signal_connect(xml, "on_mainwindow_delete_event", on_mainwindow_delete_event);
	//glade_xml_signal_connect(xml, "on_logwindow_delete_event", on_log_window_delete_event);
	glade_xml_signal_connect(xml, "on_roomwindow_delete_event", on_room_list_delete_event);
	glade_xml_signal_connect(xml, "on_userwindow_delete_event", on_user_list_delete_event);
	glade_xml_signal_connect(xml, "on_prefwindow_delete_event", on_prefwindow_delete_event);
	glade_xml_signal_connect(xml, "on_namewindow_delete_event", on_nickname_close);
	glade_xml_signal_connect(xml, "on_expression_delete_event",G_CALLBACK(on_expression_delete_event));
	glade_xml_signal_connect(xml, "on_nickname_cancel_clicked", on_nickname_close);
	glade_xml_signal_connect(xml, "on_nickname_ok_clicked", G_CALLBACK(on_nickname_okay));
	glade_xml_signal_connect(xml, "on_nicknameentry_activate", G_CALLBACK(on_nickname_okay));
	glade_xml_signal_connect(xml, "on_closepref_clicked", on_prefwindow_delete_event);
	glade_xml_signal_connect(xml, "on_pref_activate", on_pref_activate);
	glade_xml_signal_connect(xml, "on_savelog_activate", on_savelog_activate);
	glade_xml_signal_connect(xml, "on_clearlog_activate", on_clearlog_activate);
	glade_xml_signal_connect(xml, "on_about_delete_event", G_CALLBACK(gtk_widget_hide_on_delete));
	glade_xml_signal_connect(xml, "on_quit_activate", on_mainwindow_delete_event);
	glade_xml_signal_connect(xml, "on_connect_activate", on_connect_activate);
	glade_xml_signal_connect(xml, "on_disconnect_activate", on_disconnect_activate);
	glade_xml_signal_connect(xml, "on_log_window_activate", on_log_window_activate);
	glade_xml_signal_connect(xml, "on_expression_activate", on_expression_activate);
	glade_xml_signal_connect(xml, "on_about_activate", on_about_activate);
	glade_xml_signal_connect(xml, "on_connwindow_delete_event", G_CALLBACK(on_connwindow_cancel_clicked));
	glade_xml_signal_connect(xml, "on_connwindow_cancel_clicked", G_CALLBACK(on_connwindow_cancel_clicked));
	glade_xml_signal_connect(xml, "on_connwindow_ok_clicked", on_connwindow_ok_clicked);
	glade_xml_signal_connect(xml, "on_connwindow_activate", on_connwindow_ok_clicked);
	glade_xml_signal_connect(xml, "on_user_list_activate", on_user_list_activate);
	glade_xml_signal_connect(xml, "on_room_list_activate", on_room_list_activate);
//	glade_xml_signal_connect(xml, "on_filesel_ok_clicked", on_filesel_ok_clicked);
//	glade_xml_signal_connect(xml, "on_filesel_cancel_clicked", on_filesel_cancel_clicked);
	glade_xml_signal_connect(xml, "on_fontselect_clicked", on_fontselect_clicked);
	glade_xml_signal_connect(xml, "on_fontselect_okay", on_fontselect_okay);
	glade_xml_signal_connect(xml, "on_fontselect_apply", on_fontselect_okay);
	glade_xml_signal_connect(xml, "on_fontselect_cancel", on_fontselect_cancel);
	glade_xml_signal_connect(xml, "on_fontselect_delete_event", on_fontselect_cancel);
	glade_xml_signal_connect(xml, "on_fontselect_expose", on_fontselect_redraw);
	glade_xml_signal_connect(xml, "on_change_page", on_change_page);
	glade_xml_signal_connect(xml, "on_checktabbedlog_toggled", G_CALLBACK(on_checktabbedlog_toggled));
	glade_xml_signal_connect(xml, "on_browser_changed", G_CALLBACK(on_browser_changed));
	glade_xml_signal_connect(xml, "on_homepage_clicked", G_CALLBACK(on_homepage_clicked));
	glade_xml_signal_connect(xml, "on_face_changed", (update_preview));
	glade_xml_signal_connect(xml, "on_color_changed", (update_preview));
	glade_xml_signal_connect(xml, "on_refreshusers_clicked", refresh_userlists);
	glade_xml_signal_connect(xml, "on_refreshrooms_clicked", refresh_roomlists);
	glade_xml_signal_connect(xml, "on_expressionclose_clicked",G_CALLBACK(on_expression_delete_event));
	glade_xml_signal_connect(xml, "on_prop_expose_event", G_CALLBACK(on_prop_expose_event));
	glade_xml_signal_connect(xml, "on_preview_expose_event", G_CALLBACK(on_preview_expose_event));
	glade_xml_signal_connect(xml, "on_room_cursor", G_CALLBACK(on_room_cursor));
	glade_xml_signal_connect(xml, "on_room_clicked", G_CALLBACK(on_room_clicked));
	glade_xml_signal_connect(xml, "on_user_cursor", G_CALLBACK(on_user_cursor));
	glade_xml_signal_connect(xml, "on_userjump_clicked", G_CALLBACK(on_userjump_clicked));
	glade_xml_signal_connect(xml, "on_whisper_clicked", G_CALLBACK(on_whisper_clicked));
	glade_xml_signal_connect(xml, "on_whisper_activate", G_CALLBACK(on_whisper_clicked));

	g_signal_connect(FileSelWindow, "response", G_CALLBACK(on_filesel_clicked), NULL);
	g_signal_connect(FileSelWindow, "delete-event", G_CALLBACK(on_filesel_cancel_clicked), NULL);

	// ID, Palace, Name, Room, BigPage
	UserList = gtk_tree_store_new(5,	G_TYPE_INT, G_TYPE_STRING,
						G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);
	// ID, Flags, Palace, Name, Population, BigPage
	RoomList = gtk_tree_store_new(6,	G_TYPE_INT, G_TYPE_INT,
						G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);

	gtk_tree_view_set_model(GTK_TREE_VIEW(UserTreeView), GTK_TREE_MODEL(UserList));
	gtk_tree_view_set_model(GTK_TREE_VIEW(RoomTreeView), GTK_TREE_MODEL(RoomList));

	GtkCellRenderer * text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn * col = gtk_tree_view_column_new_with_attributes("Palace", text,
						"text", 1,
						NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(UserTreeView), col);
	col = gtk_tree_view_column_new_with_attributes("User", text,
						"text", 2,
						NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(UserTreeView), col);


	col = gtk_tree_view_column_new_with_attributes("Palace", text,
						"text", 2,
						NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(RoomTreeView), col);
	col = gtk_tree_view_column_new_with_attributes("Room", text,
						"text", 3,
						NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(RoomTreeView), col);
	col = gtk_tree_view_column_new_with_attributes("Population", text,
						"text", 4,
						NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(RoomTreeView), col);

	gtk_widget_show_all(UserTreeView);
	gtk_widget_show_all(RoomTreeView);

	BackBuffer = gdk_pixmap_new(GDK_DRAWABLE(MainWindow->window), PalaceWidth, PalaceHeight, -1);
	PreviewBuffer = gdk_pixmap_new(GDK_DRAWABLE(MainWindow->window), PropSize * 3, PropSize * 3, -1);
	if(PreviewBuffer == NULL)
		g_printf("Error making preview buffer!\n");

	g_object_ref(G_OBJECT(LogNotebook));

	GtkWidget * colors = gtk_menu_new();

	for(int i = 0; i < 16; i++){
		gchar *little;
		little = g_strdup_printf("%d.png", i);
    		file = g_build_filename(ProgramPath, "colors", little, NULL);
		g_free(little);
		ColorPixbuf[i] = gdk_pixbuf_new_from_file(file, NULL);
		g_free(file);

		little = g_strdup_printf("%d_small.png", i);
    		file = g_build_filename(ProgramPath, "colors", little, NULL);
		g_free(little);
		GtkWidget * mi = gtk_menu_item_new();
		gtk_container_add(GTK_CONTAINER(mi), gtk_image_new_from_file(file));
		g_free(file);
		gtk_menu_shell_append(GTK_MENU_SHELL(colors), mi);
	}
	gtk_widget_show_all(colors);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(ColorMenu), colors);

	GtkWidget * faces = gtk_menu_new();
	for(int i = 0; i < 13; i++){
		gchar *little;
		little = g_strdup_printf("%d.png", i);
    		file = g_build_filename(ProgramPath, "faces", little, NULL);
		g_free(little);
		FacePixbuf[i] = gdk_pixbuf_new_from_file(file, NULL);
		g_free(file);

		little = g_strdup_printf("%d_small.png", i);
    		file = g_build_filename(ProgramPath, "faces", little, NULL);
		g_free(little);
		GtkWidget * mi = gtk_menu_item_new();
		gtk_container_add(GTK_CONTAINER(mi), gtk_image_new_from_file(file));
		g_free(file);
		gtk_menu_shell_append(GTK_MENU_SHELL(faces), mi);
	}
	gtk_widget_show_all(faces);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(FaceMenu), faces);

    	file = g_build_filename(ProgramPath, "colors", "shadow.png", NULL);
	ShadowPixbuf = gdk_pixbuf_new_from_file(file, NULL);
	g_free(file);

    	file = g_build_filename(ProgramPath, "trans.png", NULL);
	TransPixbuf = gdk_pixbuf_new_from_file(file, NULL);
	g_free(file);

    	file = g_build_filename(ProgramPath, "world.svg", NULL); //"world48.png", NULL);
	IconPixbufWorld = gdk_pixbuf_new_from_file(file, NULL);
	g_free(file);

	update_preview();

	WindowIconList = g_list_append(WindowIconList, IconPixbufWorld);
	gtk_window_set_default_icon_list(WindowIconList);
	g_list_free(WindowIconList);

	// Loaded configuration values
	// if faild, here are defaults
	myFont = g_strdup("Sans 8");
	myShowTabs = FALSE;
	myShowSend = TRUE;
	myShowTabbedLog = FALSE;
	myAutoSwitch = TRUE;
	myLogShown = TRUE;
	myBrowserChoice = 0;
	myLogChoice = LogChoice_Bottom;
	// save config here

	// action on loaded values
	if(myShowSend == FALSE)
		gtk_widget_hide(glade_xml_get_widget(xml, "sendbutton"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(CheckTabbedLog), myShowTabbedLog);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(CheckAutoSwitch), myAutoSwitch);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(CheckTabs), myShowTabs);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(CheckShowSend), myShowSend);
	on_log_window_activate();
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(LogMenu), !myLogShown);
	gtk_option_menu_set_history(GTK_OPTION_MENU(OptionLogPos), myLogChoice);
	gtk_option_menu_set_history(GTK_OPTION_MENU(OptionBrowser), myBrowserChoice);
	sync_tabs();

	Tooltips = gtk_tooltips_new();

	gdk_threads_enter();
	LastPage = new BigPage(Notebook, LogNotebook);
	LastPage->Disconnect();
	gtk_notebook_remove_page(GTK_NOTEBOOK(Notebook), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(LogNotebook), 0);

	BlankPage = TRUE;
	gtk_main();
	gdk_threads_leave(); 
	return 0;
}

