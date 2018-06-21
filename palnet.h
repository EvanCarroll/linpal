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


#ifndef PALNET_H
#define PALNET_H

#include <list>
#include "palcallback.h"

using namespace std;


class PalNet {
public:
	PalNet(PalCallback * usersupplied);

	~PalNet();

	void SetCallback(PalCallback * data);
	bool Connect(const char * server, int port, const char * name);
	const char * Disconnect();
	void Say(const char * msg);
	void Whisper(int idto, const char * msg); // specific person to talk to
	void Move(int x, int y);
	void Name(const char * name);
	void Navigate(int id);
	void Wear(int face, int color, int count, int prop_time[9], int prop_crc[9]);
	void RequestUserList();
	void RequestRoomList();
	void RequestFile(const char * file);
	void RequestAsset(int a, int b);
	bool Connected() { return connected; }
	int  ID() { return id; }
	const char * GetName(){ return name; }

	static const int MaxNameLen = 256;

private:
	friend void * PalNetRunner(void * palnet);

	// Network messages
	void RLogOn(int a, int b);	// tiyid byte order 
	void RAltLogon(int a, int b);	// 1919250482
	void RVersion(int a, int b);	// 1986359923
	void RServerInfo(int a, int b);	// 1936289382
	void RUserStatus(int a, int b);	// 1968403553
	void RUserLog(int a, int b);	// 6c6f6720 'log '
	void RMediaAddr(int a, int b);  // 1213486160 'HTTP'
	void RRoomDesc(int a, int b);	// 1919905645
	void RUserList(int a, int b);	// 1919971955
	void RRoomDescend(int a, int b);// 1701733490
	void RUserNew(int a, int b);	// 1852863091
	void RPing(int a, int b);	// 1885957735
	void RChat(int a, int b);	// 'xtlk' - encoded
	void RWhisperD(int a, int b);	// 'xwis' - encoded
	void RWhisper(int a, int b);
	void RMove(int a, int b);
	void RColor(int a, int b);	// 1970500163
	void RFace(int a, int b);	// 1970500166
	void RDesc(int a, int b);	// 1970500164 ?
	void RProp(int a, int b);	// 1970500176 ?
	void RRename(int a, int b);	// 1970500174
	void RLogoff(int a, int b);	// 1652122912
	void RError(int a, int b);
	void RrFile(int a, int b);	// 0x7346696C 'sFil' response to qFil
	void RrAsset(int a, int b);	// 0x73417374
	void RUserExit(int a, int b);	// 0x65707273
	void RAllRooms(int a, int b);	// 0x724C7374 'rLst'
	void RAllUsers(int a, int b);	// 0x754C7374 'uLst'

	unsigned char * ReadBytes();
	int  ReadByte();
	int  ReadShort();
	int  ReadInt();
	long ReadLong();
	void ReadFully(unsigned char * buffer, int len);

	void WriteBytes(const unsigned char * d, int cnt);
	void WriteByte(unsigned char d);
	void WriteShort(int d);
	void WriteInt(int d);
	void WriteLong(long d);
	void Flush();

	void Decrypt(unsigned char * buffer, int len);
	void Encrypt(unsigned char * buffer, int len);

	class FileDownloadNode {
	public:
		FileDownloadNode(char * name, int transID, int size);
		~FileDownloadNode();

		void	AddChunk(int size, unsigned char * data);
		int	GetID(){ return transID; }
		FileDownloadNode * GetNext(){ return next; }

		FileDownloadNode * next,
				 * prev;
		unsigned char * data;
		char	name[64];
		int	transID,
			sofar,
			nextChunk,
			totalSize;
	} * rootDownload;

	FileDownloadNode * LocateDownload(int transID);

	PalCallback *	UserData;

	list<unsigned char>	sendQueue,
				recvQueue;
	bool	connected,
		highfirst; 	// member a of ouput in ph
	char 	name[MaxNameLen],
		serverName[MaxNameLen],
		*server;
	int	id,
		room,
		x,
		y,
		dkey,
		lut[512],
		version,
		port,
		sock;
};

void * PalNetRunner(void * palnet);

#endif

