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

#include "gtkbubble.h"
#include "gtkmain.h"

GtkBubble::GtkBubble(BigPage * owner){
	this->owner = owner;
}

GtkBubble::~GtkBubble(){

}

void GtkBubble::CalcDim(BubbleNode * n){
	gchar *	good;
	PangoContext * context = gtk_widget_create_pango_context(owner->drawable);
	PangoLayout * layout = pango_layout_new(context);
	PangoFontDescription * fontdesc = pango_font_description_from_string(myFont);
	pango_layout_set_font_description(layout, fontdesc);
	good = g_convert_with_fallback(n->msg, strlen(n->msg), "utf-8",
					"cp1252", ".", NULL, NULL, NULL);
	pango_layout_set_text(layout, (good != NULL ? good : n->msg), -1);
	g_free(good);

	pango_layout_set_width(layout, 1000 * 100);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
	pango_layout_get_pixel_size(layout, &n->width, &n->height);
	pango_font_description_free(fontdesc);
	g_object_unref(layout);
	g_object_unref(context);
}

void GtkBubble::Render(){
	gchar *	good;
	GdkGC	*gc = gdk_gc_new(BackBuffer);
	gboolean leftside = FALSE;

	PangoContext * context = gtk_widget_create_pango_context(owner->drawable);
	PangoLayout * layout = pango_layout_new(context);
	// select from preferences dialog
	PangoFontDescription * fontdesc = pango_font_description_from_string(myFont);
	PangoFontDescription * fontdesci = pango_font_description_from_string(myFont);
	pango_font_description_set_style(fontdesci, PANGO_STYLE_ITALIC);
	pango_layout_set_font_description(layout, fontdesc);

	for(BubbleNode * i = rootBubble; i != 0; i = i->next){
		if(!i->visible)
			continue;

		leftside = FALSE;

		good = g_convert_with_fallback(i->msg, strlen(i->msg), "utf-8",
					"cp1252", ".", NULL, NULL, NULL);

		if(i->whisper)
			pango_layout_set_font_description(layout, fontdesci);
		else
			pango_layout_set_font_description(layout, fontdesc);
		pango_layout_set_text(layout, (good != NULL ? good : i->msg), -1);

		pango_layout_set_width(layout, 1000 * 100);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
		if(good)
			g_free(good);
		gdk_gc_set_foreground(gc, &ColorWhite);
		gdk_gc_set_background(gc, &ColorWhite);
		leftside = ((i->x + i->width) < i->srcx);
		
		switch(i->type){
		case PalBubble::Normal:
			GdkPoint points[3];
			points[0].x = i->x + (leftside == TRUE ? i->width : 0);
			points[0].y = i->y;
			points[1].x = i->srcx;
			points[1].y = i->srcy + (PropSize/2);
			points[2].x = i->x + (leftside == TRUE ? i->width : 0);
			points[2].y = i->y + 10;

			gdk_draw_polygon(BackBuffer, gc, TRUE, points, 3);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y - BubbleBorder, i->width, BubbleBorder);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y + i->height, i->width, BubbleBorder);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x - BubbleBorder, i->y, BubbleBorder, i->height);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x + i->width, i->y, BubbleBorder, i->height);
	
			gdk_draw_arc(BackBuffer, gc, TRUE, i->x - BubbleBorder, i->y - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE, i->x - BubbleBorder,
					(i->y + i->height) - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760 * 2, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE,
					(i->x + i->width) - (BubbleBorder), i->y + i->height - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760 * 3, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE, (i->x + i->width) - BubbleBorder, i->y - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 0, 5760);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y, i->width, i->height); 

			break;

		case PalBubble::Thought:
			gdk_draw_arc(BackBuffer, gc, TRUE, i->srcx +
					(leftside != TRUE ? PropSize/2 : -PropSize/2), (i->srcy + i->y)/2,
					BubbleBorder, BubbleBorder, 0, 5760 * 4);
			gdk_draw_arc(BackBuffer, gc, TRUE,
				(leftside != TRUE ? i->x - (BubbleBorder * 2) : i->x + i->width),
				i->y, BubbleBorder * 2, BubbleBorder * 2, 0, 5760 * 4);

			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y - BubbleBorder, i->width, BubbleBorder);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y + i->height, i->width, BubbleBorder);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x - BubbleBorder, i->y, BubbleBorder, i->height);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x + i->width, i->y, BubbleBorder, i->height);
	
			gdk_draw_arc(BackBuffer, gc, TRUE, i->x - BubbleBorder, i->y - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE, i->x - BubbleBorder,
					(i->y + i->height) - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760 * 2, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE,
					(i->x + i->width) - (BubbleBorder), i->y + i->height - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 5760 * 3, 5760);
			gdk_draw_arc(BackBuffer, gc, TRUE, (i->x + i->width) - BubbleBorder, i->y - BubbleBorder,
					BubbleBorder * 2, BubbleBorder * 2, 0, 5760);
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y, i->width, i->height); 

			break;

		case PalBubble::Exclamation:
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x, i->y, i->width, i->height); 
			break;

		case PalBubble::Blocking:
			gdk_draw_rectangle(BackBuffer, gc, TRUE,
						i->x - BubbleBorder, i->y - BubbleBorder,
						i->width + (BubbleBorder * 2),
						i->height + (BubbleBorder * 2)); 
			break;
		}

		gdk_gc_set_foreground(gc, &ColorBlack);
		gdk_gc_set_background(gc, &ColorWhite);
		gdk_draw_layout(BackBuffer, gc, i->x, i->y, layout);
	}

	pango_font_description_free(fontdesc);
	pango_font_description_free(fontdesci);
	g_object_unref(layout);
	g_object_unref(context);
	g_object_unref(gc);
}

