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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "palbubble.h"
#include "gtkmain.h"

PalBubble::PalBubble(){
	rootBubble = 0;
}

PalBubble::~PalBubble(){
	while(rootBubble != 0){
		BubbleNode * tmp = rootBubble;
		rootBubble = rootBubble->next;
		delete tmp;
	}
}

void PalBubble::AddBubble(double time, int x, int y, bool whisper, const char * msg){
	BubbleNode * i = rootBubble;
	if(i == 0){
		rootBubble = i = new BubbleNode(
					((msg[0] == ':' || msg[0] == '^' || msg[0] == '!') ?
					(PalBubble::Type)msg[0] : PalBubble::Normal),
					time, x, y, whisper,
					((msg[0] == ':' || msg[0] == '^' || msg[0] == '!') ?
						&msg[1] : &msg[0]),
					0);
	} else {
		while(i->next != 0)
			i = i->next;

		i->next = new BubbleNode(
					((msg[0] == ':' || msg[0] == '^' || msg[0] == '!') ?
					(PalBubble::Type)msg[0] : PalBubble::Normal),
					time, x, y, whisper,
					((msg[0] == ':' || msg[0] == '^' || msg[0] == '!') ?
						&msg[1] : &msg[0]),
					i);
	}
}

/* Call this with another thread.
 * When true is returned, the bubbles need to be re-rendered.
 */
bool PalBubble::UpdateTimer(double curtime){
	bool changed = false;

	BubbleNode * i = rootBubble;
	while(i != 0){
		if(i->visible == false){
			BubbleNode * j = rootBubble;
			bool	allowed = true,
				leftside = false;
			int	jtop, jleft, jbottom, jright,
				itop, ileft, iright, ibottom;
			CalcDim(i);

			itop = i->srcy;
			ileft = i->srcx + 44;
			ileft = max(ileft, 0);
			iright = ileft + i->width;
			itop = min(PalaceHeight - i->height, itop);
			itop = max(itop, 0);
			ibottom = itop + i->height;

			if(iright > PalaceWidth){
				leftside = TRUE;
				ileft = i->srcx - (i->width + 44);
				ileft = max(ileft, 0);
				iright = ileft + i->width;
			}

			while(j != 0){

				if(j->visible == false){
					j = j->next;
					continue;
				}

				jtop = j->y;
				jleft = j->x;
				jright = jleft + j->width;
				jbottom = jtop + j->height;
			
				if(	jbottom >= itop && jtop <= ibottom && 
					jright >= ileft && jleft <= iright){
					leftside = TRUE;
					ileft = i->srcx - (i->width + 44);
					ileft = max(ileft, 0);
					iright = ileft + i->width;
					if(	jbottom >= itop && jtop <= ibottom && 
						jright >= ileft && jleft <= iright ){
						allowed = false;
						break;
					}
				}

				j = j->next;
			}
			if(allowed){
				i->visible = true;
				changed = true;
				i->x = ileft;
				i->y = itop;
				i->starttime = curtime;
			}

		} else {
			if((double)(curtime - i->starttime) >
				10.0 + (0.1 * (double)strlen(i->msg))){
				BubbleNode * tmp = i;
				i = i->next;
				if(tmp == rootBubble)
					rootBubble = i;
				delete tmp;
				changed = true;
				continue;
			}
		}
		i = i->next;
	}

	return changed;
}

PalBubble::BubbleNode::BubbleNode(Type t, double time, int x, int y,\
					bool whisper, const char * msg, BubbleNode * last){
	type = t;
	starttime = time;
	this->whisper = whisper;
	this->srcx = x;
	this->srcy = y;
	this->msg = strdup(msg);
	visible = false;
	next = 0;
	prev = last;
}

PalBubble::BubbleNode::~BubbleNode(){
	if(msg != NULL)
		free(msg);
	if(prev != 0)
		prev->next = next;
	if(next != 0)
		next->prev = prev;
}
