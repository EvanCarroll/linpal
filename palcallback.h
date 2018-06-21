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

#ifndef PALCALLBACK_H
#define PALCALLBACK_H

class UserListNode {
public:
	UserListNode(){
		next = 0;
		name = 0;
	}
	~UserListNode(){
		if(next != 0)
			delete next;
		if(name != 0)
			delete[] name;
	}

	UserListNode * next;
	char *	name;
	int	room,
		userID;
};

class RoomListNode {
public:
	RoomListNode(){
		next = 0;
		name = 0;
		population = 0;
	}
	~RoomListNode(){
		if(next != 0)
			delete next;
		if(name != 0)
			delete[] name;
	}

	RoomListNode * next;
	char *	name;
	int	flags,
		population,
		roomID;
};

class PalCallback {
public:
	typedef enum {
		Red = 0,
		Orange,
		Light_Yellow,
		Yellow_Green,
		Medium_Green,
		Dark_Green,
		Blue_Green,
		Turqoise,
		Light_Blue,
		Medium_Blue,
		Dark_Blue,
		Navy_Blue,
		Purple,
		Light_Purple,
		Magenta,
		Pink_Red
	} Color;

	typedef enum {
		Sleepy = 0,
		Happy,
		Down,
		Open,
		Wink_Left,
		Nothing,
		Wink_Right,
		Lean_Left,
		Up,
		Lean_Right,
		Unhappy,
		Dead,
		Mad
	} Face;

	virtual ~PalCallback() = 0; 
	virtual void ServerInfo(const char * title, int loc, int max) = 0;
	virtual void MediaAddr(const char * url) = 0;
	virtual void RoomDesc(const char * title, const char * bgfile) = 0;
	virtual void UserSay(int idfrom, const char * text) = 0;
	virtual void SystemSay(int idto, const char * text) = 0;
	virtual void RecvWhisper(int idto, const char * text) = 0;
	virtual void UserMove(int idwho, int x, int y) = 0;
	virtual void UserRename(int idwho, const char * newname) = 0;
	virtual void ConnectionClosed(int reason, const char * msg) = 0;
	virtual void UserEnter(int idwho, const char * name, int x, int y, int face, int color) = 0;
	virtual void UserLeave(int idwho) = 0;
	virtual void Population(int pop) = 0;
	virtual void UserProp(int idwho, int a[9], int b[9]) = 0;
	virtual void UserFace(int idwho, Face face) = 0;
	virtual void UserColor(int idwho, Color color) = 0;
	virtual void RecvAsset(int a, int b, int len, const void * buff) = 0;
	virtual void RecvAllRooms(RoomListNode * node) = 0;
	virtual void RecvAllUsers(UserListNode * node) = 0;
	virtual void RecvFileStart(int transid, const char * name) = 0;
	virtual void RecvFileAbort(int transid, const char * name) = 0;
	virtual void RecvFileChunk(int transid, const char * name, int sofar, int total) = 0;
	virtual void RecvFileEnd(int transid, const char * name, int size, const void * buff) = 0;
};

#endif
