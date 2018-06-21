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

#ifndef PALCLIENT_H
#define PALCLIENT_H

class PalClient {
public:
	PalClient(int id, const char * name, int x, int y, int face, int color);
	void Color(int c);
	void Face(int f);
	void Move(int x, int y);
	void Rename(const char * name);
	double getLastSpoke(){ return lastSpoke; }
	const char * getName(){ return name; }
	int getID(){ return id; }
	int getX(){ return x; }
	int getY(){ return y; }
	int getFace(){ return face; }
	int getColor(){ return color; }

	int	prop_time[9],
		prop_crc[9],
		prop_count;

private:
	friend int ChatTimeout(void * param);

	double	lastSpoke;
	int 	id,
		color,
		face,
		x,
		y;
	char 	name[128];
};
typedef PalClient * pPalClient;


#endif

