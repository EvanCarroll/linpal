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

#ifndef GTKASSET_H
#define GTKASSET_H

#include <gtk/gtk.h>

// Palette Code from Jim Bumgardener
extern unsigned long gMMPal[];

typedef struct {
	short width,height,hOffset,vOffset;
	short scriptOffset; // 6/22/95 unused
	short flags;   // 6/22/95
} PropHeader,*PropHeaderPtr;


#define PF_PropFaceFlag		0x02 // Prop is a face - don't show regular faceprop
#define PF_PropGhostFlag	0x04 // Prop is transparent
#define PF_PropRareFlag		0x08 // Prop is rare - don't copy, delete fromfaves when dropped
#define PF_Animate		0x10 // 4/24/96 JAB
#define PF_Palindrome		0x20 // 4/24/96 JAB
// End code from jim bumgardener



class AssetNode {
public:
	GdkPixbuf * data;
	gint a, b;
	PropHeader ph;
};

class GtkAssetMgr {
public:
	GtkAssetMgr();
	~GtkAssetMgr();
	void NewAsset(gint a, gint b, gint len, const gpointer data);
	GdkPixbuf * Asset(gint a, gint b);
	PropHeader *Header(gint a, gint b);
	GList * Head() { return loaded; };

private:
	int	count;
	GList	*loaded;
};



#endif
