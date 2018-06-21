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

	Palette Array from Jim Bumgardener
*/

#include <glib.h>
#include <stdio.h>

#include "gtkmain.h"
#include "gtkasset.h"

// Palette Code from Jim Bumgardener
unsigned long gMMPal[] = // 0x00BBGGRR
{0x00ffffff, 0x00ffffcc, 0x00ffff99, 0x00ffff66, 0x00ffff33, 0x00ffff00, 0x00ffdfff, 0x00ffdfcc,
 0x00ffdf99, 0x00ffdf66, 0x00ffdf33, 0x00ffdf00, 0x00ffbfff, 0x00ffbfcc, 0x00ffbf99, 0x00ffbf66,
 0x00ffbf33, 0x00ffbf00, 0x00ff9fff, 0x00ff9fcc, 0x00ff9f99, 0x00ff9f66, 0x00ff9f33, 0x00ff9f00,
 0x00ff7fff, 0x00ff7fcc, 0x00ff7f99, 0x00ff7f66, 0x00ff7f33, 0x00ff7f00, 0x00ff5fff, 0x00ff5fcc,
 0x00ff5f99, 0x00ff5f66, 0x00ff5f33, 0x00ff5f00, 0x00ff3fff, 0x00ff3fcc, 0x00ff3f99, 0x00ff3f66,
 0x00ff3f33, 0x00ff3f00, 0x00ff1fff, 0x00ff1fcc, 0x00ff1f99, 0x00ff1f66, 0x00ff1f33, 0x00ff1f00,
 0x00ff00ff, 0x00ff00cc, 0x00ff0099, 0x00ff0066, 0x00ff0033, 0x00ff0000, 0x00eeeeee, 0x00dddddd,
 0x00cccccc, 0x00bbbbbb, 0x00aaffff, 0x00aaffcc, 0x00aaff99, 0x00aaff66, 0x00aaff33, 0x00aaff00,
 0x00aadfff, 0x00aadfcc, 0x00aadf99, 0x00aadf66, 0x00aadf33, 0x00aadf00, 0x00aabfff, 0x00aabfcc,
 0x00aabf99, 0x00aabf66, 0x00aabf33, 0x00aabf00, 0x00aaaaaa, 0x00aa9fff, 0x00aa9fcc, 0x00aa9f99,
 0x00aa9f66, 0x00aa9f33, 0x00aa9f00, 0x00aa7fff, 0x00aa7fcc, 0x00aa7f99, 0x00aa7f66, 0x00aa7f33,
 0x00aa7f00, 0x00aa5fff, 0x00aa5fcc, 0x00aa5f99, 0x00aa5f66, 0x00aa5f33, 0x00aa5f00, 0x00aa3fff,
 0x00aa3fcc, 0x00aa3f99, 0x00aa3f66, 0x00aa3f33, 0x00aa3f00, 0x00aa1fff, 0x00aa1fcc, 0x00aa1f99,
 0x00aa1f66, 0x00aa1f33, 0x00aa1f00, 0x00aa00ff, 0x00aa00cc, 0x00aa0099, 0x00aa0066, 0x00aa0033,
 0x00aa0000, 0x00999999, 0x00888888, 0x00777777, 0x00666666, 0x0055ffff, 0x0055ffcc, 0x0055ff99,
 0x0055ff66, 0x0055ff33, 0x0055ff00, 0x0055dfff, 0x0055dfcc, 0x0055df99, 0x0055df66, 0x0055df33,
 0x0055df00, 0x0055bfff, 0x0055bfcc, 0x0055bf99, 0x0055bf66, 0x0055bf33, 0x0055bf00, 0x00559fff,
 0x00559fcc, 0x00559f99, 0x00559f66, 0x00559f33, 0x00559f00, 0x00557fff, 0x00557fcc, 0x00557f99,
 0x00557f66, 0x00557f33, 0x00557f00, 0x00555fff, 0x00555fcc, 0x00555f99, 0x00555f66, 0x00555f33,
 0x00555f00, 0x00555555, 0x00553fff, 0x00553fcc, 0x00553f99, 0x00553f66, 0x00553f33, 0x00553f00,
 0x00551fff, 0x00551fcc, 0x00551f99, 0x00551f66, 0x00551f33, 0x00551f00, 0x005500ff, 0x005500cc,
 0x00550099, 0x00550066, 0x00550033, 0x00550000, 0x00444444, 0x00333333, 0x00222222, 0x00111111,
 0x0000ffff, 0x0000ffcc, 0x0000ff99, 0x0000ff66, 0x0000ff33, 0x0000ff00, 0x0000dfff, 0x0000dfcc,
 0x0000df99, 0x0000df66, 0x0000df33, 0x0000df00, 0x0000bfff, 0x0000bfcc, 0x0000bf99, 0x0000bf66,
 0x0000bf33, 0x0000bf00, 0x00009fff, 0x00009fcc, 0x00009f99, 0x00009f66, 0x00009f33, 0x00009f00,
 0x00007fff, 0x00007fcc, 0x00007f99, 0x00007f66, 0x00007f33, 0x00007f00, 0x00005fff, 0x00005fcc,
 0x00005f99, 0x00005f66, 0x00005f33, 0x00005f00, 0x00003fff, 0x00003fcc, 0x00003f99, 0x00003f66,
 0x00003f33, 0x00003f00, 0x00001fff, 0x00001fcc, 0x00001f99, 0x00001f66, 0x00001f33, 0x00001f00,
 0x000000ff, 0x000000cc, 0x00000099, 0x00000066, 0x00000033, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
 0x00f0f0f0, 0x00e0e0e0, 0x00d0d0d0, 0x00c0c0c0, 0x00b0b0b0, 0x00a0a0a0, 0x00808080, 0x00707070,
 0x00606060, 0x00505050, 0x00404040, 0x00303030, 0x00202020, 0x00101010, 0x00080808, 0x00000000,};
// end code from Jim Bumgardener



GtkAssetMgr::GtkAssetMgr(){
	loaded = NULL;
	count = 0;
}

GtkAssetMgr::~GtkAssetMgr(){
	// clear loaded data
	for(GList * first = g_list_first(loaded); first != NULL; first = g_list_next(first)){
		AssetNode * n = (AssetNode * )(first->data);
		g_free(n->data);
		delete n;
	}
	g_list_free(loaded);
}

void myfree(guchar * pixels, gpointer data){
	g_free(pixels);
}

unsigned short SwapShort(unsigned short val){
	return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
}

unsigned long SwapLong(unsigned long val){
	return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | 
		((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

void GtkAssetMgr::NewAsset(gint a, gint b, gint len, const gpointer data){
	for(GList * first = g_list_first(loaded); first != NULL; first = g_list_next(first)){
		AssetNode * n = (AssetNode * )(first->data);
		// don't do duplicates
		if(n->a == a && n->b == b)
			return;
	}
	AssetNode * anewnode = new AssetNode();
	anewnode->a = a;
	anewnode->b = b;
	gchar *ab;
	ab = g_strdup_printf("%x-%x.png", a, b);
	gchar * local = g_build_filename(ConfigPath, "props", ab, NULL);
	g_free(ab);
	guchar * transdata = (guchar *)g_malloc(44 * 44 * 4);

	// construct pixel data
	unsigned char * writer = (unsigned char *)transdata;

	PropHeaderPtr ph = (PropHeaderPtr)data;
	memcpy(&(anewnode->ph), ph, 12);
	if (ph->height < 1 || ph->height > 64) {
		anewnode->ph.height = SwapShort(anewnode->ph.height);
		anewnode->ph.width = SwapShort(anewnode->ph.width);
		anewnode->ph.hOffset = SwapShort(anewnode->ph.hOffset);
		anewnode->ph.vOffset = SwapShort(anewnode->ph.vOffset);
		anewnode->ph.scriptOffset = SwapShort(anewnode->ph.scriptOffset);
		anewnode->ph.flags = SwapShort(anewnode->ph.flags);
	}
	unsigned char * rlestuff = (unsigned char *)data;
	unsigned char alpha = (anewnode->ph.flags & PF_PropGhostFlag ? 127 : 255);
	rlestuff += 12;

	for(int y = 0; y < anewnode->ph.height; y++){
		int x = anewnode->ph.width;
		while(x > 0){
			unsigned char	count = *rlestuff++;
			unsigned char	upperc = count >> 4,
					lowerc = count & 0xF;
			x -= upperc + lowerc;
			if(x < 0)
				goto END;

			for(int skip = 0; skip < upperc * 4; skip++)
				*writer++ = 0;
			while(lowerc--){

				int color = *(rlestuff++);
				int lookedup = gMMPal[color];

				//red green blue alpha
				writer[0] = lookedup & 0xFF;
				writer[1] = (lookedup >> 8) & 0xFF;
				writer[2] = (lookedup >> 16) & 0xFF;
				writer[3] = alpha;
				writer += 4;
			}
		}
	}
END:
	// save and add to list
	GdkPixbuf * saver = gdk_pixbuf_new_from_data(transdata, GDK_COLORSPACE_RGB, TRUE,
							8, 44, 44, 44 * 4, myfree, NULL);
	gdk_pixbuf_save(saver, local, "png", NULL, NULL);
	anewnode->data = saver;
	g_free(local);
	ab = g_strdup_printf("%x-%x.header", a, b);
	local = g_build_filename(ConfigPath, "props", ab, NULL);
	g_free(ab);
	FILE * fp = fopen(local, "w+");
	if(fp){
		fwrite(&anewnode->ph, 12, 1, fp);
		fclose(fp);
	}

	loaded = g_list_prepend(loaded, anewnode);
	count++;
}

GdkPixbuf * GtkAssetMgr::Asset(gint a, gint b){
	if(count > 1500){
		//delete nodes;
		// should really switch over to a faster method...
		for(GList * i = g_list_first(loaded); i != NULL; i = g_list_next(i)){
			AssetNode * n = (AssetNode * )(i->data);
			g_free(n->data);
			delete n;
		}
		g_list_free(loaded);
		loaded = NULL;
	}

	for(GList * first = g_list_first(loaded); first != NULL; first = g_list_next(first)){
		AssetNode * n = (AssetNode * )(first->data);
		if(n->a == a && n->b == b)
			return n->data;
	}
	AssetNode * anewnode = new AssetNode();
	anewnode->a = a;
	anewnode->b = b;
	gchar *ab;
	ab = g_strdup_printf("%x-%x.png", a, b);
	gchar * local = g_build_filename(ConfigPath, "props", ab, NULL);
	g_free(ab);
	anewnode->data = gdk_pixbuf_new_from_file(local, NULL);
	g_free(local);
	ab = g_strdup_printf("%x-%x.header", a, b);
	local = g_build_filename(ConfigPath, "props", ab, NULL);
	g_free(ab);
	FILE * fp = fopen(local, "r+");
	if(fp){
		fread(&(anewnode->ph), 12, 1, fp);
		fclose(fp);
	}
	g_free(local);

	if(anewnode->data == NULL){
		delete anewnode;
		return NULL;
	}
	loaded = g_list_prepend(loaded, anewnode);
	count++;
	return anewnode->data;
}

PropHeader * GtkAssetMgr::Header(gint a, gint b){
	for(GList * first = g_list_first(loaded); first != NULL; first = g_list_next(first)){
		AssetNode * n = (AssetNode * )(first->data);
		if(n->a == a && n->b == b)
			return &n->ph;
	}
	return NULL;
}

