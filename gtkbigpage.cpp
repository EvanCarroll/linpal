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
#include <glib.h>
#include <glib/gprintf.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gtkbigpage.h"
#include "gtkmain.h"

// WTF functions ==============================
pPalClient LocateClient(BigPage * p, int id){
	for(list<pPalClient>::iterator iter = p->clientelle.begin(); iter != p->clientelle.end(); iter++)
		if((*iter)->getID() == id)
			return (*iter);
	return NULL;
}

BigPage::BigPage(GtkWidget * Notebook, GtkWidget * LogNotebook){
	this->Notebook = Notebook;
	this->LogNotebook = LogNotebook;
	population = 0;
	palace = 0;
	dlQueue = NULL;
	DLinProgress = FALSE;

	GtkWidget * vbox = gtk_vbox_new(FALSE, 5);

	drawable = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawable, PalaceWidth, PalaceHeight);
	gtk_box_pack_start(GTK_BOX(vbox), drawable, FALSE, FALSE, 0);

	textentry = gtk_entry_new_with_max_length(245);

	nickname = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(nickname), GTK_RELIEF_NONE);
	gtk_widget_set_sensitive(nickname, FALSE);

	sendbutton = gtk_button_new();
	//gtk_widget_set_size_request(GTK_WIDGET(sendbutton), 16, 16);
	gtk_container_add(	GTK_CONTAINER(sendbutton),
				gtk_image_new_from_stock(GTK_STOCK_JUMP_TO,
				GTK_ICON_SIZE_BUTTON));
	gtk_button_set_relief(GTK_BUTTON(sendbutton), GTK_RELIEF_NONE);
	gtk_widget_set_sensitive(sendbutton, FALSE);

	GtkWidget * textbox = gtk_hbox_new(FALSE, 5);

	gtk_box_pack_start(GTK_BOX(textbox), nickname, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(textbox), textentry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(textbox), sendbutton, FALSE, FALSE, 0);

	gtk_tooltips_set_tip(Tooltips, nickname, "Change your current name",
				"You can change your current name by clicking here and entering a new one in the dialog." );
	gtk_tooltips_set_tip(Tooltips, textentry, "Write message",
				"Once you are done typing what you wish to say here, press Enter or click the send button.");
	gtk_tooltips_set_tip(Tooltips, sendbutton, "Send",
				"Once you have written a message, send it by clicking here or pressing the Enter button.");

	gtk_box_pack_start(GTK_BOX(vbox), textbox, TRUE, TRUE, 0);

	GtkWidget * sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), sep, TRUE, TRUE, 0);

	GtkWidget * hbox = gtk_hbox_new(FALSE, 5);

	roomlabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), roomlabel, TRUE, TRUE, 0);

	gtk_tooltips_set_tip(Tooltips, roomlabel, "Current room name", "Current room name");

	progressbar = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(hbox), progressbar, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	page = vbox;

	logpage = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget * tv = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tv), FALSE);
	
	gtk_container_add(GTK_CONTAINER(logpage), tv);
	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(logpage),	GTK_POLICY_NEVER,
									GTK_POLICY_ALWAYS);
	
	logbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
	GtkTextTag * tt = gtk_text_buffer_create_tag(logbuffer, "blue_foreground",
					"foreground", "blue", NULL);
	gtk_text_buffer_create_tag(logbuffer, "gray_foreground",
					"foreground", "gray", NULL);
	gtk_text_buffer_create_tag(logbuffer, "underline", "underline",
					PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(logbuffer, "italic", "style",
					PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(logbuffer, "name", "weight",
					PANGO_WEIGHT_BOLD, NULL);
	g_signal_connect(G_OBJECT(tt), "event", G_CALLBACK(on_link_clicked), this);

	logadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(logpage));

	gtk_widget_show_all(logpage);

	//gtk_widget_set_sensitive(close, FALSE);
	gtk_notebook_append_page(GTK_NOTEBOOK(Notebook), page, NULL);//top);
	gtk_notebook_append_page(GTK_NOTEBOOK(LogNotebook), logpage, NULL);//"A Palace");

	Title("Connecting...");
	bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);

	gtk_widget_add_events(drawable, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(drawable, "expose_event", G_CALLBACK(on_image_expose_event), this);
	g_signal_connect(textentry, "activate", G_CALLBACK(on_entry_activate), this);
	g_signal_connect(sendbutton, "clicked", G_CALLBACK(on_send_clicked), this);
	g_signal_connect(nickname, "clicked", G_CALLBACK(on_nickname_clicked), this);
	g_signal_connect(drawable, "button_press_event", G_CALLBACK(on_drawingarea_button_press_event), this);
//	g_signal_connect(page, "destroy", G_CALLBACK(destroyed), this);

	gtk_widget_show_all(vbox);

	if(!myShowSend)
		gtk_widget_hide(sendbutton);

	Redraw();

	palace = new PalNet((PalCallback *)this);
	bubbles = new GtkBubble(this);

	killThreads = FALSE;
	killMutex = g_mutex_new();
	endDLMutex = g_mutex_new();
	endBubbleMutex = g_mutex_new();
	netThread = bubbleThread = dlThread = NULL;
	servername = roomname = bgfilename = mediaurl = downloadfile = name = 0;

}

BigPage::~BigPage(){
	if(palace)
		if(palace->Connected())
			Disconnect();

	gint i = gtk_notebook_page_num(GTK_NOTEBOOK(Notebook), page);
	if(i != -1)
		gtk_notebook_remove_page(GTK_NOTEBOOK(Notebook), i);
	
	i = gtk_notebook_page_num(GTK_NOTEBOOK(LogNotebook), logpage);
	if(i != -1)
		gtk_notebook_remove_page(GTK_NOTEBOOK(LogNotebook), i);

	for(GList * i = g_list_first(dlQueue); i != NULL; i = g_list_next(i))
		g_free(i->data);

	g_list_free(dlQueue);

	g_free(servername);
	g_free(roomname);
	g_free(bgfilename);
	g_free(mediaurl);
	g_free(downloadfile);
	g_free(name);

	if(palace != 0){
		delete palace;
		palace = 0;
	}

	delete bubbles;
}

gboolean BigPage::Connect(const gchar * server, int port, const gchar * name){
	g_free(servername);
	servername = g_strdup(server);
	g_free(this->name);
	this->name = g_strdup(name);
	gdk_threads_enter();
	LogSystem("Connecting as %s to %s:%d.", this->name, servername, port);
	gdk_threads_leave();

	bool ret = palace->Connect(servername, port, this->name);
	if(ret){
		killThreads = FALSE;
		netThread = g_thread_create(PalNetRunner, palace, TRUE, NULL);
		dlThread = g_thread_create(DLWatcher, this, TRUE, NULL);
		bubbleThread = g_thread_create(BubbleWatcher, this, TRUE, NULL);
		if(netThread == NULL || dlThread == NULL || bubbleThread == NULL){
			killThreads = TRUE;
			gdk_threads_enter();
			palace->Disconnect();
			if(netThread)	g_thread_join(netThread);
			if(dlThread) g_thread_join(dlThread);
			if(bubbleThread) g_thread_join(bubbleThread);
			ret = false;
			LogSystem("Failed to create threads for %s:%d.", servername, port);
		} else {
			gdk_threads_enter();
			LogSystem("Connected to %s:%d.", servername, port);
			gtk_button_set_label(GTK_BUTTON(nickname), this->name);
			gtk_widget_set_sensitive(nickname, TRUE);
			gtk_widget_set_sensitive(sendbutton, TRUE);
			gtk_widget_set_sensitive(close, TRUE);
		}
	} else {
		gdk_threads_enter();
		LogSystem("Failed to reach %s:%d.", servername, port);
		Title("Not Connected");
		if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(Notebook)) == 1)
			gtk_widget_set_sensitive(close, FALSE);
		gtk_button_set_label(GTK_BUTTON(nickname), "");
	}
	gdk_threads_leave();
	return ret;
}

void BigPage::NickName(const gchar * newname){
	gchar * good = g_convert_with_fallback(newname, strlen(newname),
						"cp1252", "UTF-8", ".", NULL, NULL, NULL);
	if(good == NULL)
		return;
	palace->Name(good);
	g_free(good);
	gtk_button_set_label(GTK_BUTTON(nickname), newname);
}

const gchar * BigPage::GetName(){
	if(palace != 0)
		return palace->GetName();
	return NULL;
}

void BigPage::Whisper(gint to, const gchar *wrote){
	if(strcmp(wrote, "") == 0)
		return;
	// send chat message here, maybe make a duplicate
	gchar * good = g_convert_with_fallback(wrote, strlen(wrote),
						"cp1252", "UTF-8", ".", NULL, NULL, NULL);
	if(good){
		if(palace != 0)
			palace->Whisper(to, good);
		g_free(good);
	}
}

void BigPage::Move(gint x, gint y){
	if(palace != 0)
		palace->Move(x, y);
}

void BigPage::RequestUserList(){
	if(palace != 0)
		palace->RequestUserList();
}

void BigPage::RequestRoomList(){
	if(palace != 0)
		palace->RequestRoomList();
}

void BigPage::Say(const gchar *wrote){
	if(strcmp(wrote, "") == 0)
		return;
	// send chat message here, maybe make a duplicate
	gchar * good = g_convert_with_fallback(wrote, strlen(wrote),
						"cp1252", "UTF-8", ".", NULL, NULL, NULL);
	if(good){
		if(palace != 0)
			palace->Say(good);
		g_free(good);
	}
}

void BigPage::Navigate(gint room){
	clientelle.clear();
	palace->Navigate(room);
}

void BigPage::SaveLog(const gchar * file){
	FILE * fp = fopen(file, "w+");
	if(!fp){
		g_print("Error saving log to file %s\n", file);
		return;
	}

	GtkTextIter istart, iend;
	gtk_text_buffer_get_start_iter(logbuffer, &istart);
	gtk_text_buffer_get_end_iter(logbuffer, &iend);
	gchar * text = gtk_text_buffer_get_text(logbuffer, &istart, &iend, FALSE);
	fwrite(text, strlen(text), 1, fp);
	fclose(fp);
}

void BigPage::ClearLog(){
	gtk_text_buffer_set_text(logbuffer, "", 0);
}

void BigPage::Progress(gdouble fraction){
	if(fraction <= 0.0 || fraction >= 1.0){
		gtk_widget_set_sensitive(progressbar, FALSE);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.001);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), "");
		return;
	} else
		gtk_widget_set_sensitive(progressbar, TRUE);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), fraction);
	gchar * good = g_convert_with_fallback(bgfilename, strlen(bgfilename), "UTF-8",
						"cp1252", ".", NULL, NULL, NULL);
	if(good){
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), good);
		g_free(good);
	}
}

void BigPage::Redraw(){
	if(PageIndex() != gtk_notebook_get_current_page(GTK_NOTEBOOK(Notebook)))
		return;

	int width, height,
		x, y;
	ColorWhite.red = 65535;
	ColorWhite.green = 65535;
	ColorWhite.blue = 65535;
	ColorWhite.pixel = 0xFFFFFFFF;
	ColorBlack.red = 0;
	ColorBlack.green = 0;
	ColorBlack.blue = 0;
	ColorBlack.pixel = 0;
	gchar *	good;
	GdkGC	*gc = gdk_gc_new(BackBuffer);

	PangoContext * context = gtk_widget_create_pango_context(drawable);
	PangoLayout * layout = pango_layout_new(context);
	// select from preferences dialog
	PangoFontDescription * fontdesc = pango_font_description_from_string(myFont);
	pango_layout_set_font_description(layout, fontdesc);
	
	// render background
	gdk_draw_pixbuf(BackBuffer, gc, bgpixbuf,
			0, 0, 0, 0, PalaceWidth, PalaceHeight,
			GDK_RGB_DITHER_NONE, 0, 0);

	// do people
	for(list<pPalClient>::iterator iter = clientelle.begin(); iter != clientelle.end(); iter++){
		// draw props
		gboolean drawhead = TRUE;
		y = (*iter)->getY() - 22;
		x = (*iter)->getX() - 22;

		for(int i = 0; i < 9; i++){
			if((*iter)->prop_time[i] == 0)
				break;

			PropHeader * ph = PropMgr.Header((*iter)->prop_time[i], (*iter)->prop_crc[i]);
			if(ph == NULL)
				break;
			if((ph->flags & PF_PropFaceFlag) != 0){
				drawhead = FALSE;
				break;
			}
		}
		if(drawhead == TRUE){
			gdk_draw_pixbuf(BackBuffer, gc, ShadowPixbuf,
					0, 0, x, y, PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
			gdk_draw_pixbuf(BackBuffer, gc, ColorPixbuf[(*iter)->getColor()],
					0, 0, x, y, PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
			gdk_draw_pixbuf(BackBuffer, gc, FacePixbuf[(*iter)->getFace()],
					0, 0, x, y, PropSize, PropSize,
					GDK_RGB_DITHER_NONE, 0, 0);
		}

		for(int i = 0; i < 9; i++){
			if((*iter)->prop_time[i] == 0)
				break;

			GdkPixbuf * prop = PropMgr.Asset((*iter)->prop_time[i], (*iter)->prop_crc[i]);
			PropHeader * ph = PropMgr.Header((*iter)->prop_time[i], (*iter)->prop_crc[i]);
			if(ph != NULL && prop != NULL){
				x = ((*iter)->getX() + ph->hOffset) - 22;
				y = ((*iter)->getY() + ph->vOffset) - 22;
				if(prop != NULL)
					gdk_draw_pixbuf(BackBuffer, gc, prop, 0, 0, x, y,
							PropSize, PropSize,
							GDK_RGB_DITHER_NONE, 0, 0);
			} else
				break;
		}

	}

	for(list<pPalClient>::iterator iter = clientelle.begin(); iter != clientelle.end(); iter++){
		// draw name
		pango_layout_set_width(layout, -1);
		good = g_convert_with_fallback((*iter)->getName(), strlen((*iter)->getName()), "utf-8",
					"cp1252", ".", NULL, NULL, NULL);

		pango_layout_set_text(layout, (good != NULL ? good : (*iter)->getName()), -1);

		gdk_gc_set_foreground(gc, &ColorBlack);
		pango_layout_get_pixel_size(layout, &width, &height);
		y = (*iter)->getY() + 16;
		x = (*iter)->getX() - (width / 2);
		y = min(PalaceHeight - height, y);
		x = max(0, x);
		x = min(PalaceWidth - width, x);
		gdk_draw_rectangle(BackBuffer, gc, TRUE, x - 1, y - 1, width + 2, height + 2); 
		gdk_draw_arc(BackBuffer, gc, TRUE, x - ((BubbleBorder/2) + BubbleBorder), y - 1,
				BubbleBorder * 3, height + 2, 5760, 5760 * 2);

		gdk_draw_arc(BackBuffer, gc, TRUE, x + width - ((BubbleBorder/2) + BubbleBorder),
				y - 1,
				BubbleBorder * 3, height + 2, 5760 * 3, 5760 * 2);

		gdk_gc_set_foreground(gc, &ColorWhite);
		gdk_draw_layout(BackBuffer, gc, x, y, layout);
		if(good)
			g_free(good);

	}

	pango_font_description_free(fontdesc);
	g_object_unref(layout);
	g_object_unref(context);
	g_object_unref(gc);

	// text
	bubbles->Render();

	// TODO be more effecient here
//	gtk_widget_queue_draw_area(drawable, 0, 0, PalaceWidth, PalaceHeight);
	gc = gdk_gc_new(drawable->window);
	gdk_draw_drawable(drawable->window, gc, BackBuffer,
				0, 0, 0, 0, PalaceWidth, PalaceHeight);
	g_object_unref(gc);
}

gint BigPage::PageIndex(){
	return gtk_notebook_page_num(GTK_NOTEBOOK(Notebook), page);
}

void BigPage::NewLook(int color, int face, int count, int prop_time[9], int prop_crc[9]){
	// not bounced
	this->face = (Face)face;
	this->color = (Color)color;

	pPalClient pc = LocateClient(this, palace->ID());
	if(pc != NULL){
		pc->Face(face);
		pc->Color(color);
		Redraw();
	}

	palace->Wear(face, color, count, prop_time, prop_crc);
}

void BigPage::Title(const gchar * text, ...){
	gchar * full;
	va_list ap;
	va_start(ap, text);
	full = g_strdup_vprintf(text, ap);
	va_end(ap);
	gchar * good = g_convert_with_fallback(full, strlen(full), "UTF-8",
						"cp1252", ".", NULL, NULL, NULL);
	g_free(full);
	if(good == NULL)
		return;

	GtkWidget * top = gtk_hbox_new(FALSE, 5);
	close = gtk_button_new();
	gtk_widget_set_size_request(GTK_WIDGET(close), 16, 16);
	gtk_container_add(	GTK_CONTAINER(close),
				gtk_image_new_from_stock(GTK_STOCK_CLOSE,
				GTK_ICON_SIZE_MENU));
	gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);

	g_signal_connect(G_OBJECT(close), "clicked",
					 G_CALLBACK(close_tab), this);

	gtk_box_pack_start(GTK_BOX(top), gtk_label_new(good), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(top), GTK_WIDGET(close), FALSE, FALSE, 0);

	gtk_notebook_set_menu_label_text(GTK_NOTEBOOK(LogNotebook), logpage, good);
	gtk_notebook_set_menu_label_text(GTK_NOTEBOOK(Notebook), page, good);
	gtk_widget_show_all(top);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(Notebook), page, top);
	gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(LogNotebook), logpage, good);
	g_free(good);
}

void BigPage::Disconnect(){
	g_mutex_lock(killMutex);
	killThreads = TRUE;
	g_mutex_unlock(killMutex);

	if(palace != 0)
		palace->Disconnect();

	if(netThread) g_thread_join(netThread);
	if(dlThread) g_thread_join(dlThread);
	if(bubbleThread) g_thread_join(bubbleThread);

	bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);
	clientelle.clear();
	g_free(roomname);
	roomname = g_strdup("");
	gtk_label_set_text(GTK_LABEL(roomlabel), "");
	Title("Not Connected");
	LogSystem("Disconnected.");
	gtk_widget_set_sensitive(sendbutton, FALSE);
	gtk_widget_set_sensitive(textentry, FALSE);
	gtk_widget_set_sensitive(close, FALSE);
	gtk_widget_hide(nickname);
	Redraw();
}


// =================================== USER SUPPLIED FUNCTIONS
void BigPage::UserColor(int who, Color color){
	pPalClient pc = LocateClient(this, who);
	if(pc == NULL)
		return;

	pc->Color(color);

	gdk_threads_enter();
	Redraw();
	gdk_threads_leave();
}

void BigPage::UserFace(int who, Face face){
	pPalClient pc = LocateClient(this, who);
	if(pc == NULL)
		return;

	pc->Face(face);

	gdk_threads_enter();
	Redraw();
	gdk_threads_leave();
}

void BigPage::LogSystem(const gchar * text, ...){
	gchar *full;
	// threads enter here
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(logbuffer, &iter);

	GTimeVal tv;
	g_get_current_time(&tv);
	GDate	*date = g_date_new();
	//g_date_set_time(date, tv.tv_sec);//time(NULL));
	//g_date_strftime(full, 1024, "(%I:%M:%S %p)", date);
	full = (gchar *)g_malloc0(1024);
	strftime(full, 1024, "(%I:%M:%S %p)", localtime(&tv.tv_sec));
	g_date_free(date);
	gtk_text_buffer_insert_with_tags_by_name(logbuffer, &iter, full, -1, "gray_foreground", NULL);
	gtk_text_buffer_insert(logbuffer, &iter, " ", -1);
	g_free(full);

	va_list ap;
	va_start(ap, text);
	full = g_strdup_vprintf(text, ap);
	va_end(ap);

	gchar * fullgood = g_convert_with_fallback(full, strlen(full), "UTF-8", "cp1252",
							".", NULL, NULL, NULL);
	g_free(full);

	gtk_text_buffer_insert_with_tags_by_name(logbuffer, &iter, fullgood, -1, "gray_foreground", NULL);
	g_free(fullgood);

	gtk_text_buffer_insert(logbuffer, &iter, "\n", 1);

	if( 	gtk_notebook_page_num(GTK_NOTEBOOK(LogNotebook), logpage)	==
		gtk_notebook_get_current_page(GTK_NOTEBOOK(LogNotebook))	)
		gtk_adjustment_set_value(logadjustment, logadjustment->upper);
}

void BigPage::LogSelf(gboolean whisper, const gchar * from, const gchar * text, ...){
	gchar *full;
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(logbuffer, &iter);

	GTimeVal tv;
	g_get_current_time(&tv);
	GDate	*date = g_date_new();
	//g_date_set_time(date, tv.tv_sec);//time(NULL));
	//g_date_strftime(full, 1024, "(%I:%M:%S %p)", date);
	full = (gchar *)g_malloc0(1024);
	strftime(full, 1024, "(%I:%M:%S %p)", localtime(&tv.tv_sec));
	g_date_free(date);
	gtk_text_buffer_insert(logbuffer, &iter, full, -1);
	gtk_text_buffer_insert(logbuffer, &iter, " ", -1);
	g_free(full);

	va_list ap;
	va_start(ap, text);
	full = g_strdup_vprintf(text, ap);
	va_end(ap);
	// threads enter here
	gchar	*url,
		*c = full,
		*d;

	// convert
	gchar * fromgood = g_convert_with_fallback(from, strlen(from), "utf-8",
						"cp1252", ".", NULL, NULL, NULL);
	gtk_text_buffer_insert_with_tags_by_name(logbuffer, &iter, fromgood, -1,
							"name", (whisper == TRUE ? "italic" : NULL), NULL);
	if(fromgood)
		g_free(fromgood);

	while(*c != 0){
		gboolean isurl = FALSE;
		if(	(c[0] == 'h' &&
			 c[1] == 't' &&
			 c[2] == 't' &&
			 c[3] == 'p' ) ||
			(c[0] == 'w' &&
			 c[1] == 'w' &&
			 c[2] == 'w' )
			 ){
			isurl = TRUE;
		}


		gchar * start = c;
		while(*c != 0 && *c != ' ' && *c != '\t')
			c++;


		url = (gchar *)g_malloc0((ulong)(c - start) + 1);
		d = url;
		c = start;

		while(*c != 0 && *c != ' ' && *c != '\t')
			*d++ = *c++;
		*d++ = *c;
		*d = 0;
		gchar * good = g_convert_with_fallback(url, strlen(url), "UTF-8",
							"cp1252", ".", NULL, NULL, NULL);
		g_free(url);
		if(isurl == TRUE){
			g_signal_connect(textentry, "activate", G_CALLBACK(on_entry_activate), this);
			gtk_text_buffer_insert_with_tags_by_name(logbuffer,
				&iter, good, -1, "blue_foreground", "underline",
				(whisper == TRUE ? "italic" : NULL), NULL);
		} else 
			gtk_text_buffer_insert_with_tags_by_name(logbuffer,
				&iter, good, -1, (whisper == TRUE ? "italic" : NULL), NULL);

		g_free(good);

		if(*c != 0)
			c++;
	}
	g_free(full);

	gtk_text_buffer_insert(logbuffer, &iter, "\n", 1);

	if( 	gtk_notebook_page_num(GTK_NOTEBOOK(LogNotebook), logpage)	==
		gtk_notebook_get_current_page(GTK_NOTEBOOK(LogNotebook))	)
		gtk_adjustment_set_value(logadjustment, logadjustment->upper);
}

void BigPage::Send(){
	on_entry_activate(textentry, this);
}

void BigPage::ShowSendButton(gboolean yes){
	if(yes == TRUE)
		gtk_widget_show_all(sendbutton);
	else
		gtk_widget_hide(sendbutton);
}

void BigPage::UpdateRoom(){
	gchar	*full,
		*full2,
		*good;

	// sometimes, this info isn't read yet
	if(roomname == 0)
		return;

	full = g_strdup_printf(" (%d/%d)", clientelle.size(), population);
	full2 = g_strconcat(roomname, full, NULL);

	good = g_convert_with_fallback(full2, strlen(full2), "UTF-8",
					"cp1252", "?", NULL, NULL, NULL);

	g_free(full);
	g_free(full2);

	if(good){
		gdk_threads_enter();
		gtk_label_set_text(GTK_LABEL(roomlabel), good);
		gdk_threads_leave();
		g_free(good);
	}
}

void BigPage::Download(const gchar * file){
	dlQueue = g_list_append(dlQueue, g_strdup(file));
}

void BigPage::ServerInfo(const char * server, int loc, int max){
	gdk_threads_enter();
	g_free(servername);
	servername = g_strdup(server);
	Title(server);
	gdk_threads_leave();
	UpdateRoom();
}

void BigPage::MediaAddr(const char * url){
	g_free(mediaurl);
	mediaurl = g_strdup(url);
}

void BigPage::RoomDesc(const char * title, const char * bgfile){
	g_free(roomname);
	roomname = g_strdup(title);
	UpdateRoom();
	Download(bgfile);
}

void BigPage::UserSay(int idfrom, const char * text){
	pPalClient pc = LocateClient(this, idfrom);
	if(pc == NULL)
		return;

	bubbles->AddBubble((double)time(NULL), pc->getX(), pc->getY(), false, text);

	gdk_threads_enter();
	LogSelf(FALSE, pc->getName(), ":\t%s", text);
	Redraw();
	gdk_threads_leave();
}

void BigPage::SystemSay(int idto, const char * text){
	gdk_threads_enter();
	LogSystem(text);
	gdk_threads_leave();
}

void BigPage::RecvWhisper(int idto, const char * text){
	pPalClient pc = LocateClient(this, idto);

	if(pc != NULL)
		bubbles->AddBubble((double)time(NULL), pc->getX(), pc->getY(), true, text);
	else
		bubbles->AddBubble((double)time(NULL), 0, 0, true, text);

	gdk_threads_enter();
	LogSelf(TRUE, (pc == NULL ? "***" : pc->getName()), ":\t%s", text);
	Redraw();
	gdk_threads_leave();
}

void BigPage::UserMove(int idwho, int x, int y){
	pPalClient pc = LocateClient(this, idwho);
	if(pc == NULL)
		return;

	pc->Move(x, y);
	
	// move is not bounced
	if(idwho != palace->ID())
		gdk_threads_enter();
	Redraw();
	if(idwho != palace->ID())
		gdk_threads_leave();
}

void BigPage::UserRename(int idwho, const char * newname){
	pPalClient pc = LocateClient(this, idwho);
	if(pc == NULL)
		return;

	pc->Rename(newname);

	gdk_threads_enter();
	Redraw();
	gdk_threads_leave();
}

void BigPage::ConnectionClosed(int why, const char * msg){

	gdk_threads_enter();
	LogSystem("Connection closed remotely: %s.", msg);
	gtk_widget_set_sensitive(sendbutton, FALSE);
	gdk_threads_leave();
	UpdateRoom();
	gdk_threads_enter();
	Disconnect();
	gdk_threads_leave();
}

void BigPage::UserEnter(int idwho, const char * name, int x, int y, int face, int color){

	if(idwho == palace->ID()){
		this->color = (Color)color;
		this->face = (Face)face;
	}

	clientelle.push_back(new PalClient(idwho, name, x, y, face, color));

	gdk_threads_enter();
	LogSystem("%s has entered.", name);
	Redraw();
	gdk_threads_leave();
	UpdateRoom();
}

void BigPage::Population(int pop){
	population = pop;
	UpdateRoom();
}

void BigPage::UserLeave(int idwho){
	pPalClient pc = LocateClient(this, idwho);
	if(pc == NULL)
		return;
	
	gdk_threads_enter();
	LogSystem("%s has left.", pc->getName());
	for(list<pPalClient>::iterator iter = clientelle.begin(); iter != clientelle.end(); iter++)
		if((*iter)->getID() == idwho){
			pPalClient cli = *iter;
			delete cli;
			clientelle.erase(iter);
			break;
		}
	Redraw();
	gdk_threads_leave();
	UpdateRoom();
}

void BigPage::UserProp(int idwho, int a[9], int b[9]){
	// if asset isn't found, then request, for now, request like a mofo
	pPalClient pc = LocateClient(this, idwho);
	if(pc == NULL)
		return;

	for(int pos = 0; pos < 9; pos++){
		if(PropMgr.Asset(a[pos], b[pos]) == NULL && a[pos] != 0 && b[pos] != 0)
			palace->RequestAsset(a[pos], b[pos]);
		
		pc->prop_time[pos] = a[pos];
		pc->prop_crc[pos] = b[pos];
	}
	gdk_threads_enter();
	Redraw();
	gdk_threads_leave();
}

void BigPage::RecvAsset(int a, int b, int len, const void * data){
	PropMgr.NewAsset(a, b, len, (gpointer)data);
	gdk_threads_enter();
	Redraw();
	gdk_threads_leave();
}

void BigPage::RecvAllRooms(RoomListNode * node){
	GtkTreeIter iter, child;
	gdk_threads_enter();
	gtk_tree_store_append(GTK_TREE_STORE(RoomList), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(RoomList), &iter, 2, servername, -1);
	gtk_tree_store_set(GTK_TREE_STORE(RoomList), &iter, 3, 0, -1);

	int count = 0;
	while(node != 0){
		count += node->population;
		gtk_tree_store_append(GTK_TREE_STORE(RoomList), &child, &iter); 
		gchar * gname = g_convert_with_fallback(node->name, strlen(node->name), "UTF-8",
						"cp1252", ".", NULL, NULL, NULL);
		gtk_tree_store_set(GTK_TREE_STORE(RoomList), &child, 0, node->roomID, -1);
		gtk_tree_store_set(GTK_TREE_STORE(RoomList), &child, 1, node->flags, -1);
		gtk_tree_store_set(GTK_TREE_STORE(RoomList), &child, 3, gname, -1);
		gtk_tree_store_set(GTK_TREE_STORE(RoomList), &child, 4, node->population, -1);
		gtk_tree_store_set(GTK_TREE_STORE(RoomList), &child, 5, this, -1);
		g_free(gname);
		node = node->next;
	}
	gtk_tree_store_set(GTK_TREE_STORE(RoomList), &iter, 4, count, -1);

	gdk_threads_leave();
}

void BigPage::RecvAllUsers(UserListNode * node){
	GtkTreeIter iter, child;
	gdk_threads_enter();
	gtk_tree_store_append(GTK_TREE_STORE(UserList), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(UserList), &iter, 1, servername, -1);
	gtk_tree_store_set(GTK_TREE_STORE(UserList), &iter, 2, 0, -1);

	int count = 0;
	while(node != 0){
		count++;
		gtk_tree_store_append(GTK_TREE_STORE(UserList), &child, &iter); 
		gchar * gname = g_convert_with_fallback(node->name, strlen(node->name), "UTF-8",
						"cp1252", ".", NULL, NULL, NULL);
		gtk_tree_store_set(GTK_TREE_STORE(UserList), &child, 0, node->userID, -1);
		gtk_tree_store_set(GTK_TREE_STORE(UserList), &child, 2, gname, -1);
		gtk_tree_store_set(GTK_TREE_STORE(UserList), &child, 3, node->room, -1);
		gtk_tree_store_set(GTK_TREE_STORE(UserList), &child, 4, this, -1);
		g_free(gname);
		node = node->next;
	}
	gdk_threads_leave();
}

void BigPage::RecvFileStart(int transid, const char * name){
	gdk_threads_enter();
	LogSystem("Start receiving file %s.", name);
	gdk_threads_leave();
}
void BigPage::RecvFileAbort(int transid, const char * name){
	gdk_threads_enter();
	LogSystem("Error receiving file %s.", name);
	Progress(-1.0);
	gdk_threads_leave();
	DLinProgress = FALSE;
}
void BigPage::RecvFileChunk(int transid, const char * name, int sofar, int total){
	gdk_threads_enter();
	Progress((gdouble)((gdouble)sofar / (gdouble)total));
	gdk_threads_leave();
}
void BigPage::RecvFileEnd(int transid, const char * name, int size, const void * buff){
	gchar	*local;

	local = g_build_filename(ConfigPath, servername, name, NULL);
	FILE * fp = fopen(local, "w+");
	if(fp){
		fwrite(buff, size, 1, fp);
		fclose(fp);
	}

	gdk_threads_enter();
	LogSystem("End receiving file %s.", name);
	Progress(-1.0);
	bgpixbuf = gdk_pixbuf_new_from_file(local, NULL);
	if(!GDK_IS_PIXBUF(bgpixbuf)){
		g_print("Received file (%s) doesn't appear to be valid!\n", local);
		LogSystem("Received file corrupt!");
		bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);
	}
	Redraw();
	gdk_threads_leave();
	g_free(local);
	DLinProgress = FALSE;
}

size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream){
	return fwrite(ptr, size, nmemb, (FILE*)stream);
}

int progress_func(BigPage * bp, double t, double n, double ut, double un){
	if(bp->curlsize > 0 && bp->curlsize == (int)t){
		gdk_threads_enter();
		bp->LogSystem("Download aborted, file loaded locally.");
		bp->Progress(-1.0);
		gdk_threads_leave();
		return 1;
	}
	if(t <= 0)
		bp->Progress(0.0);
	bp->Progress((gdouble)(n / t));
	return 0;
}

gpointer BubbleWatcher(gpointer param){
	BigPage * UserData = (BigPage *) param;
	while(true){
		g_mutex_lock(UserData->killMutex);
		if(UserData->killThreads == TRUE){
			g_mutex_unlock(UserData->killMutex);
			break;
		}
		if(UserData->palace->Connected() == FALSE){
			g_mutex_unlock(UserData->killMutex);
			break;
		}
		if(UserData->bubbles->UpdateTimer((double)time(NULL))){
			gdk_threads_enter();
			UserData->Redraw();
			gdk_threads_leave();
		}
		g_mutex_unlock(UserData->killMutex);

		sleep(1);
		g_thread_yield();
	}
	return NULL;
}

gpointer DLWatcher(gpointer param){
	BigPage * UserData = (BigPage *) param;
	while(true){
		g_mutex_lock(UserData->killMutex);
		if(UserData->killThreads == TRUE){
			g_mutex_unlock(UserData->killMutex);
			break;
		}

		if(UserData->palace->Connected() == FALSE){
			g_mutex_unlock(UserData->killMutex);
			break;
		}
		if(UserData->dlQueue != NULL && UserData->DLinProgress == FALSE){
			UserData->DLinProgress = TRUE;
			g_thread_create(MyGet, UserData, FALSE, NULL);
		}
		g_mutex_unlock(UserData->killMutex);
		sleep(1);
		g_thread_yield();
	}
	return NULL;
}

gpointer MyGet(gpointer param){
	BigPage * UserData = (BigPage *) param;
	gchar	*local,
		*file,
		*localpath,
		*localnew;
	CURL * c;
	int curlres;
	struct stat fileinfo;
	gchar * node = (gchar *)g_list_first(UserData->dlQueue)->data;

	g_free(UserData->bgfilename);
	UserData->bgfilename = g_strdup(node);
	UserData->dlQueue = g_list_remove(UserData->dlQueue, node);
	g_free(node);

	file = g_strconcat(UserData->mediaurl, UserData->bgfilename, NULL);
	local = g_build_filename(	ConfigPath, 
					UserData->servername, UserData->bgfilename, NULL);
	localnew = g_build_filename(g_get_tmp_dir(), UserData->bgfilename, NULL);

	localpath = g_path_get_dirname(local);
	mkdir(localpath, 0700);
	g_free(localpath);

	stat(local, &fileinfo);
	UserData->curlsize = fileinfo.st_size;

	// if diff file names, always get the new one, regardless of size
//	if(strcmp(UserData->bgfilename, UserData->downloadfile) != 0)
//		UserData->curlsize = -1;

	FILE * fp = fopen(localnew, "w+");

	if(!fp){
		g_print("Unable to open local temp file for writing:\n%s\n", localnew);
		fclose(fp);
		UserData->bgpixbuf = gdk_pixbuf_new_from_file(local, NULL);
		// tried to load local file
		if(!GDK_IS_PIXBUF(UserData->bgpixbuf) || curlres != 0)
			UserData->bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);
		UserData->DLinProgress = FALSE;
		return NULL;
	}

	c = curl_easy_init();

	// net failed
	if(c == 0){
		g_print("Unable to initialize curl libraries!\n");
		UserData->palace->RequestFile(UserData->bgfilename);
		g_print("Attempting to download through server.\n");
		fclose(fp);
		UserData->bgpixbuf = gdk_pixbuf_new_from_file(local, NULL);
		// tried to load local file
		if(!GDK_IS_PIXBUF(UserData->bgpixbuf) || curlres != 0)
			UserData->bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);
		return NULL;
	}

	gdk_threads_enter();
	UserData->LogSystem("Start downloading %s.", UserData->bgfilename);
	gdk_threads_leave();

 	curl_easy_setopt(c, CURLOPT_URL, file);
 	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(c, CURLOPT_PROGRESSDATA, UserData);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, fp);

	curlres = curl_easy_perform(c);

	fclose(fp);

	curl_easy_cleanup(c);
	if(curlres != 0 && curlres != CURLE_ABORTED_BY_CALLBACK){
		g_print("Unable to download file via web.\n");
		UserData->palace->RequestFile(UserData->bgfilename);
		g_print("Attempting to download through server.\n");
	} else while(curlres == 0){   // download succedded
		// move new file to regular file
		stat(localnew, &fileinfo);
		UserData->curlsize = fileinfo.st_size;

		FILE	*fpd = fopen(local, "w+"),
			*fps = fopen(localnew, "r");

		if(!fpd || !fps){
			g_print("Unable to open at least one file for moving data!\n%s\n%s\n",
				local, localnew);
			break;		
		}
		
		for(int a = 0; a < UserData->curlsize; a++)
			fputc(fgetc(fps), fpd);

		fclose(fpd);
		fclose(fps);

		printf("File downloaded and stored locally.\n");

		break;
	}

	gdk_threads_enter();
	UserData->LogSystem("Finished loading %s.", local);
	UserData->Progress(-1.0);
	UserData->bgpixbuf = gdk_pixbuf_new_from_file(local, NULL);
	if(!GDK_IS_PIXBUF(UserData->bgpixbuf)){
		g_print("Downloaded file (%s) doesn't appear to be valid!\n", local);
		UserData->LogSystem("Downloaded file corrupt!");
		UserData->bgpixbuf = gdk_pixbuf_new_from_file(DefaultBGFile, NULL);
	}
	UserData->Redraw();
	gdk_threads_leave();

	g_free(file);
	g_free(local);
	g_free(localnew);

	UserData->DLinProgress = FALSE;
	return NULL;
}


