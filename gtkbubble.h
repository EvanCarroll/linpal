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

#ifndef GTKBUBBLE_H
#define GTKBUBBLE_H

#include "palbubble.h"
#include "gtkbigpage.h"

class BigPage;

class GtkBubble : public PalBubble {
public:
	GtkBubble(BigPage * owner);
	virtual ~GtkBubble();

	void Render();
	void CalcDim(BubbleNode * n);

private:
	BigPage * owner;
};

#endif
