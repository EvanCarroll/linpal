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

#ifndef BIGPAGE_H
#define BIGPAGE_H

#include <gtk/gtk.h>

#include "palnet.h"
#include "palclient.h"
#include "palcallback.h"

#include "gtkasset.h"
#include "gtkbubble.h"

class GtkBubble;

gpointer MyGet(gpointer local);
gpointer BubbleWatcher(gpointer local);
gpointer DLWatcher(gpointer local);

class BigPage : PalCallback{
public:
	BigPage(GtkWidget * Notebook, GtkWidget * LogNotebook);
	virtual ~BigPage();
	void Disconnect();
	void Say(const gchar * wrote);
	void Whisper(gint to, const gchar * wrote);
	void Move(gint x, gint y);
	void RequestRoomList();
	void RequestUserList();
	void NickName(const gchar * newname);
	void Navigate(gint room);
	void Redraw();
	void NewLook(int color, int face, int count, int prop_time[9], int prop_crc[9]);
	void ShowSendButton(gboolean yes);
	void SaveLog(const gchar * file);
	void Send();
	void ClearLog();
	void Progress(gdouble fraction);
	void LogSystem(const gchar * text, ...);
	void LogSelf(gboolean whisper, const gchar * from, const gchar * text, ...);
	gint PageIndex();
	gboolean Connect(const gchar * server, int port, const gchar * name); // blocking
	const gchar * GetName();
	Color GetColor(){ return color; }
	Face  GetFace(){ return face; }

	// netcallback supplied functions ====================
	void ServerInfo(const char * title, int loc, int max);
	void MediaAddr(const char * url);
	void RoomDesc(const char * title, const char * bgfile);
	void UserSay(int idfrom, const char * text);
	void SystemSay(int idto, const char * text);
	void RecvWhisper(int idto, const char * text);
	void UserMove(int idwho, int x, int y);
	void UserRename(int idwho, const char * newname);
	void ConnectionClosed(int reason, const char * msg);
	void UserEnter(int idwho, const char * name, int x, int y, int face, int color);
	void UserLeave(int idwho);
	void Population(int pop);
	void UserProp(int idwho, int a[9], int b[9]);
	void UserFace(int idwho, Face type);
	void UserColor(int idwho, Color type);
	void RecvAsset(int a, int b, int len, const void * data);
	void RecvAllRooms(RoomListNode * node);
	void RecvAllUsers(UserListNode * node);
	void RecvFileStart(int transid, const char * name);
	void RecvFileAbort(int transid, const char * name);
	void RecvFileChunk(int transid, const char * name, int sofar, int total);
	void RecvFileEnd(int transid, const char * name, int size, const void * buff);

private:
	friend pPalClient LocateClient(BigPage * p, int id);
	friend int progress_func(BigPage * bp, double t, double n, double ut, double un);
	friend gpointer MyGet(gpointer);
	friend gpointer BubbleWatcher(gpointer);
	friend gpointer DLWatcher(gpointer);
	friend class GtkBubble;

	void Title(const gchar * text, ...);
	void UpdateRoom();
	void Download(const gchar * file);

	Color	color;
	Face	face;
	PalNet *palace;
	list<pPalClient>	clientelle;
	GtkBubble*	bubbles;
	gint		population,
			curlsize;
	gboolean	DLinProgress,
			killThreads;
	GThread		*netThread,
			*bubbleThread,
			*dlThread;
	GMutex		*killMutex,
			*endBubbleMutex,
			*endDLMutex;
	GList		*dlQueue;
	GtkAdjustment*	logadjustment;
	GtkWidget *	textentry,
		*	progressbar,
		*	close,
		*	drawable,
		*	page,
		*	logpage,
		*	LogNotebook,
		*	Notebook,
		*	sendbutton,
		*	nickname,
		*	roomlabel;
	GtkTextBuffer*	logbuffer;
	GdkPixbuf *	bgpixbuf;
	gchar		*servername,
			*roomname,
			*bgfilename,
			*mediaurl,
			*downloadfile,
			*name;
};

#endif

