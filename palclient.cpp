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

#include "palclient.h"

PalClient::PalClient(int id, const char * name, int x, int y, int face, int color){
	strcpy(this->name, name);
	this->id = id;
	this->x = x;
	this->y = y;
	this->color = color;
	this->face = face;
	lastSpoke = 0.0;
}

void PalClient::Move(int x, int y){
	this->x = x;
	this->y = y;
}

void PalClient::Rename(const char * name){
	strcpy(this->name, name);
}

void PalClient::Color(int c){
	color = c;
}

void PalClient::Face(int f){
	face = f;
}

