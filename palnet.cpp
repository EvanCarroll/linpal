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

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "palnet.h"
PalNet::PalNet(PalCallback * usersupplied) {

	UserData = usersupplied;
	id = x = y = port = 0;
	memset(name, 0, MaxNameLen);
	server = NULL;
	connected = false;
	highfirst = false;
	dkey = 0xa2c2a;	// different for different clients?
	
	if(dkey == 0)
		dkey = 1;
	for(int i = 0; i < 512; i++){
		int quo = dkey / 0x1f31d;
		int rem = dkey % 0x1f31d;
		int k = (16807 * rem - 2836 * quo) ;
		if( k > 0)
			dkey = k;
		else
			dkey = k + 0x7fffffff;
		lut[i] = (int)
				(((double)dkey / (double)2147483647) * (double)256);
	}
	dkey = 1;
}

PalNet::~PalNet(){
	Disconnect();
	shutdown(sock, 2);
	close(sock);

	if(server)
		delete[] server;
}

void PalNet::SetCallback(PalCallback * data){
	UserData = data;
}


fd_set read_fg, write_fg;
struct timeval waitd;

#define SANITY(x) if((x) == -1){ Disconnect(); return false; }
bool PalNet::Connect(const char * server, int port, const char * name){
	if(server == NULL || name == NULL)
		return false;

	Disconnect();

	strncpy(this->name, name, MaxNameLen - 1);
	this->server = new char[strlen(server)];
	memcpy(this->server, server, strlen(server));

	struct hostent *h;
	if ((h=gethostbyname(server)) == NULL) {  // get the host info
		Disconnect();
		return false;
        }

        struct sockaddr_in dest_addr;
        SANITY((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
        memset(&(dest_addr.sin_zero), '\0', 8);

	// connect
	SANITY(connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)))

	connected = true;
	return true;
}

const char * PalNet::Disconnect(){
	if(!connected)
		return "no server\0";
	
	// bye message
	WriteInt(0x62796520);
	WriteInt(0);
	WriteInt(id);

	Flush();

	connected = false; 
	return server;
}
void PalNet::Say(const char * msg){
	if(!connected || msg == NULL)
		return;
	
	char * buff = new char[strlen(msg)];
	memcpy(buff, msg, strlen(msg));

	int len = strlen(msg);
	Encrypt((unsigned char *)buff, len);
	WriteInt(0x78746c6b);
	WriteInt(len + 3); // strlen + 0, + 2 bytes for short
	WriteInt(id);
	WriteShort(len + 3); // crypted msg len is the same as above?
	WriteBytes((unsigned char *)buff, len);
	WriteByte(0); 
	Flush();
	delete[] buff;
}


void PalNet::Wear(int face, int color, int count, int prop_time[9], int prop_crc[9]){
//	printf("here\n");
/*
	WriteInt(1970500163);
//	WriteInt(1131574133);
	WriteInt(2);
	WriteInt(id);
	WriteShort(color);
	
	WriteInt(1970500166);
	WriteInt(2);
	WriteInt(id);
	WriteShort(face);
*/
	WriteInt(1970500164);
	WriteInt(8 + (count * 8));
	WriteInt(id);
	WriteShort(face); // ???
	WriteShort(color); // ???
	WriteInt(count);
	for(int i = 0; i < count; i++){
		WriteInt(prop_time[i]);
		WriteInt(prop_crc[i]);
	}
	Flush();
}

void PalNet::Whisper(int idto, const char * msg){
	if(!connected || msg == NULL)
		return;

	char * buff = new char[strlen(msg)];
	memcpy(buff, msg, strlen(msg));

	int len = strlen(msg);
	Encrypt((unsigned char *)buff, len);
	WriteInt(0x78776973);
	WriteInt(len + 7); // strlen + 0, + 2 bytes for short, + 4 bytes for id
	WriteInt(id);
	WriteInt(idto);
	WriteShort(len + 3); // crypted msg len is the same as above?
	WriteBytes((unsigned char *)buff, len);
	WriteByte(0); 
	Flush();
	delete[] buff;
}

void PalNet::Navigate(int room){
	WriteInt(0x6e617652);
	WriteInt(2);
	WriteInt(id);
	WriteShort(room);
	Flush();
}

void PalNet::Move(int x, int y){
	if(!connected || x < 0 || y < 0 || x > 512 || y > 384)
		return;
	
	this->x = x;
	this->y = y;

	WriteInt(1967943523);
	WriteInt(4);
	WriteInt(id);
	WriteShort(y);
	WriteShort(x);
	Flush();

	// not bounced by server
	UserData->UserMove(id, this->x, this->y);
}

void PalNet::Name(const char * name){
	if(!connected || name == NULL)
		return;

	strncpy(this->name, name, MaxNameLen - 1);
	
	WriteInt(0x7573724e);
	WriteInt(strlen(name) + 1);
	WriteInt(id);
	WriteByte(strlen(name));
	WriteBytes((unsigned char *)name, strlen(name));
	Flush();

}

void PalNet::RequestUserList(){
	WriteInt(0x754c7374);
	WriteInt(0);
	WriteInt(id);
	Flush();
}

void PalNet::RequestRoomList(){
	WriteInt(0x724c7374);
	WriteInt(0);
	WriteInt(id);
	Flush();
}
void PalNet::RequestFile(const char * file){
	WriteInt(0x7146696c); // "qFil"
	WriteInt(strlen(file) + 1);
	WriteInt(id);
	WriteByte(strlen(file));
	WriteBytes((unsigned char *)file, strlen(file));
	Flush();
}

/* class c1
*/
void PalNet::RequestAsset(int a, int b){
	WriteInt(0x71417374);
	WriteInt(12);
	WriteInt(this->id);
	WriteInt(0x50726F70); // "Prop"
	WriteInt(a);
	WriteInt(b);
	Flush();
}

int thefd;
void * PalNetRunner(void * palnet){
	PalNet * pn = (PalNet *)palnet;
	int	msgid,
		size,
		p;

	while(true){
		if(!pn)
			return NULL;
		if(!pn->connected)
			return NULL;

		unsigned char * buf = new unsigned char[12];
		int	ret = 0,
			sofar = 0;

		while(true){
			ret = recv(pn->sock, &buf[sofar], 12 - sofar, 0);
			if(ret <= 0){
				pn->Disconnect();
				return NULL;
			}
			sofar += ret;
			if(sofar == 12)
				break;
		}
		for(int i = 0; i < sofar; i++)
			pn->recvQueue.push_back(buf[i]);

		delete[] buf;

		msgid = pn->ReadInt();
		size = pn->ReadInt();
		p = pn->ReadInt();

		sofar = 0;
		if(size > 0){
			buf = new unsigned char[size];
			while(true){
				ret = recv(pn->sock, &buf[sofar], size - sofar, 0);
				if(ret <= 0){
					pn->Disconnect();
					return NULL;
				}
				sofar += ret;
				if(sofar == size)
					break;
			}
			for(int i = 0; i < sofar; i++)
				pn->recvQueue.push_back(buf[i]);

			delete[] buf;
		}

		switch(msgid){
		// Correct squence of events for logging in start ===============
		
		// message tiyid, byte order on server
		case 1886610802: printf("Untested server version.\n");
 		case 1920559476: pn->highfirst = true;
				 p =	((p & 0xFF) << 24) |
				 	((p & 0xFF00) << 8) |
				 	((p & 0xFF0000) >> 8) |
					((p & 0xFF000000) >> 24);
				 pn->RLogOn(size, p);break;

		case 1919251312: 
 		case 1953069426: pn->highfirst = false;
				 pn->RLogOn(size, p);break;

		// alternate logon reply
		case 1919250482: pn->RAltLogon(size, p); break;
		
		// server version
		case 1986359923: pn->RVersion(size, p); break;

		// receive server info
		case 1936289382: pn->RServerInfo(size, p); break;
		
		// user status
		case 1968403553: pn->RUserStatus(size, p); break;

		// user logged on and max
		case 1819240224: pn->RUserLog(size, p); break;

		// got http server location
		case 1213486160: pn->RMediaAddr(size, p); break;
		
		// the following four are whenever you change rooms as well as login
		case 1919905645: pn->RRoomDesc(size, p); break;
		case 1919971955: pn->RUserList(size, p); break;
		case 1701733490: pn->RRoomDescend(size, p); break;
		case 1852863091: pn->RUserNew(size, p); break;

		// pinged
		case 1885957735: pn->RPing(size, p); break;

		// talk
		case 2020895851: pn->RChat(size, p); break;

		// receive whisper
		case 2021091699: pn->RWhisperD(size, p); break;

		// whisper/chat?
		case 1952541803:
		case 2003331443: pn->RWhisper(size, p); break;

		// movement
		case 1967943523: pn->RMove(size, p); break;

		// user color
		case 1970500163: pn->RColor(size, p); break;

		// user face
		case 1970500166: pn->RFace(size, p); break;

		// user desc (prop)
		case 1970500164: pn->RDesc(size, p); break;

		// user prop
		case 1970500176: pn->RProp(size, p); break;

		// user rename
		case 1970500174: pn->RRename(size, p); break;

 		// person leaving?
		case 1652122912: pn->RLogoff(size, p); break;

		// connection died?
		case 1685026670: pn->RError(size, p); break;

		// incoming file
		case 1933994348: pn->RrFile(size, p); break;

		// asset incoming
		case 1933669236: pn->RrAsset(size, p); break;

		// user exit room
		case 1701868147: pn->RUserExit(size, p); break;

		// got reply of all roooms
		case 1917612916: pn->RAllRooms(size, p); break;

		// got reply of all users
		case 1967944564: pn->RAllUsers(size, p); break;

		default:
			printf("Unsupported message: %d\n", msgid);
			pn->recvQueue.clear();
			break;
		}
		pn->recvQueue.clear();
	}
	return NULL;
}

void PalNet::Encrypt(unsigned char * buffer, int len){
	int a = 0;
	int i = 0;
	for(int j = len - 1; j >= 0; j--){
		unsigned char b = buffer[j];
		buffer[j] = (unsigned char)(b ^ lut[i++] ^ a);
		a = (unsigned char)(buffer[j] ^ lut[i++]);
	}
}

void PalNet::Decrypt(unsigned char * buffer, int len){
	unsigned char a = 0;
	int i = 0;
	for(int j = len - 1; j >= 0; j--){
		unsigned char b = buffer[j];
		buffer[j] = (unsigned char)(b ^ lut[i++] ^ a);
		a = (unsigned char)(b ^ lut[i++]);
	}
}

inline int PalNet::ReadInt(){
	if(!connected)
		return -1;
	int a = ReadByte();
	int b = ReadByte();
	int c = ReadByte();
	int d = ReadByte();
	if(highfirst)
		return	(a << 24) | (b << 16) | (c << 8) | d;

	return	(d << 24) | (c << 16) | (b << 8) | a;
}
inline int PalNet::ReadShort(){
	if(!connected)
		return -1;
	int a = ReadByte();
	int b = ReadByte();
	if(highfirst)
		return	(a << 8) | b;

	return	(b << 8) | a;
}

inline int PalNet::ReadByte(){
	if(recvQueue.size() == 0){
		printf("Read extra byte!\n");
		return 0;
	}
	int val = recvQueue.front();
	recvQueue.pop_front();
	return val;
}

inline void PalNet::ReadFully(unsigned char * buffer, int len){
	while(len-- > 0)
		*buffer++ = (unsigned char)(ReadByte() & 0xFF);
}

void PalNet::WriteInt(int d){
	if(highfirst){
		WriteByte((d >> 24) & 0xFF);
		WriteByte((d >> 16) & 0xFF);
		WriteByte((d >> 8) & 0xFF);
		WriteByte((d) & 0xFF);
	} else {
		WriteByte((d) & 0xFF);
		WriteByte((d >> 8) & 0xFF);
		WriteByte((d >> 16) & 0xFF);
		WriteByte((d >> 24) & 0xFF);
	}
}

void PalNet::WriteShort(int d){
	if(highfirst){
		WriteByte((d >> 8) & 0xFF);
		WriteByte((d) & 0xFF);
	} else {
		WriteByte((d) & 0xFF);
		WriteByte((d >> 8) & 0xFF);
	}
}

void PalNet::WriteByte(unsigned char d){
	sendQueue.push_back(d);
}

void PalNet::Flush(){
	int ret = 0;
	int sofar = 0;
	int max = sendQueue.size();

	if(sendQueue.size() == 0)
		return;

	unsigned char * buf = new unsigned char[max];
	while(sendQueue.size() > 0){
		buf[sofar++] = sendQueue.front();
		sendQueue.pop_front();
	}

	sofar = 0;
	while(sofar < max){
		ret = send(sock, &buf[sofar], max - sofar, 0);
		if(ret == -1){
			printf("Send Error\n");
			perror("send");
			Disconnect();
			break;
		} else 
			sofar += ret;
	}
	delete[] buf;
}

void PalNet::WriteBytes(const unsigned char * d, int cnt){
	for(int i = 0; i < cnt; i++)
		WriteByte(d[i]);
}

// 'regi'
void PalNet::RLogOn(int a, int b){
	// = a; a is validation
	id = b;

	// bk.c(eb) writes a,b,c, no flush
	WriteInt(1919248233);
	WriteInt(128);
	WriteInt(id); // client id/room number?
	// working from id
	WriteInt(0x5905f923);// b[0]
	WriteInt(0xcf07309c);// b[1]

	WriteByte(strlen(name));
	WriteBytes((unsigned char *)name, strlen(name)); //? name  or super.a?

	int i = 64 - (1 + strlen(name));
	if (i < 0) 	// padding???
		i = 0;
	for(; i > 0; i--) 
		WriteByte(0);
	
	/*
	WriteInt(5);	// 5 
	*/
	WriteInt(0x80000004);

	//WriteInt(0);	// unset or d?
	WriteInt(0xf5dc385e);
	
        //WriteInt(0); // e?
	WriteInt(0xc144c580);
	
        //WriteInt(0); // f?
	WriteInt(0x00002a30);
	
        //WriteInt(0); // g?
	WriteInt(0x00021df9);
	
        //WriteInt(0); // h?
	WriteInt(0x00002a30);
        
	//WriteShort(1); // i? room id?
	WriteShort(0); // i? room id?

/*	// version
        WriteBytes((unsigned char *)"J2.0", 4); 
	WriteByte(0);
	WriteByte(0); */
	WriteBytes((unsigned char *)"350211", 6);
	
        WriteInt(0);

        //WriteInt(0);
        WriteInt(1);

        //WriteInt(0);
        WriteInt(0x00000111);

        //WriteInt(0);
        WriteInt(1);

        //WriteInt(0);
        WriteInt(1);

        WriteInt(0);
	Flush();
}

// not fully implemented
// a bounced logon message?
void PalNet::RAltLogon(int a, int b){
	// orig logon id seems to be bullshit?
	//id = b;
//	FILE * fp = fopen("altlogonrx.hex", "w+");
	for(int i = 0; i < a; i++)
		ReadByte();
//		fputc(ReadByte(), fp);	// unknown server params
//	fclose(fp);
}


void PalNet::RVersion(int a, int b){
	version = b;
}

void PalNet::RServerInfo(int a, int b){
	int unknown = ReadInt();
	int size = ReadByte();
	memset(serverName, 0, MaxNameLen);
	ReadFully((unsigned char *)serverName, size);
	// pass server name back to gui
	UserData->ServerInfo(serverName, 0, unknown);
}

// not fully implemented
void PalNet::RUserStatus(int a, int b){
	// a is length? b is client id
	unsigned char *bigbuff = new unsigned char[a + 1];
	ReadFully((unsigned char *)bigbuff, a);
	bigbuff[a] = 0;
	delete[] bigbuff;
}

// class c2
void PalNet::RUserLog(int a, int b){
	int pop = ReadInt();
	UserData->Population(pop);
}

// receive media address
void PalNet::RMediaAddr(int a, int b){
	char *url = new char[a + 1];
	ReadFully((unsigned char *)url, a);
	url[a] = 0;
	UserData->MediaAddr(url);
}

// not fully implemented
void PalNet::RRoomDesc(int a, int b){
	char	* name,
		* bgfile,
		* desc;

	ReadInt();	// 276
	ReadInt();	// 0
	ReadShort();	// 86 lc
	int word0 = ReadShort(); // word0, 2 
	int word1 = ReadShort(); // word1, 24, 0xc
	ReadShort(); // word2, 0
	ReadShort(); // word3, 0
	ReadShort(); // f, 1
	ReadShort(); // g, 208 , off 0x14-5
	ReadInt(); // hi, 0
	ReadInt(); // jk, 0
	ReadInt(); // lm, 0
	ReadInt(); // n0, 0
	int word4 = ReadShort(); // 236, word4

	desc = new char[word4];
	ReadFully((unsigned char *)desc, word4);

	for(int sofar = 0; sofar < (a - word4 - 40); sofar++)
		ReadByte();

	int len = desc[word0];

	name = new char[len + 1];
	for(int i = 0; i < len; i++)
		name[i] = desc[word0 + 1 + i];
	name[len] = 0;

	int len2 = desc[word1];
	bgfile = new char[len2+1];
	for(int i = 0; i < len2; i++)
		bgfile[i] = desc[word1 + 1 + i];
	bgfile[len2] = 0;


	UserData->RoomDesc(name, bgfile);

	delete[] name;
	delete[] desc;
	delete[] bgfile;
}

void PalNet::RUserList(int a, int b){
	// b is size
	for(int i = 0; i < b; i++){
		int theirid = ReadInt();
		int y = ReadShort();
		int x = ReadShort();
		int j[9], k[9];
		int i1 = 0;
		do {	j[i1] = ReadInt();
			k[i1] = ReadInt();
		} while (++i1 < 9); // props
		ReadShort(); // room
		int face = ReadShort(); // face
		int color = ReadShort(); // color
		ReadShort(); // 0?
		ReadShort(); // 0?
		int propnum = ReadShort(); // number of props
		if(propnum < 9)
			j[propnum] = k[propnum] = 0;
		unsigned char theirname[32];
		memset(theirname, 0, 32);
		ReadFully(theirname, 32);
		theirname[theirname[0] + 1] = 0;
		for(int k = 1; k < 32; k++)
			theirname[k-1] = theirname[k];
		// add client call back here
		UserData->UserEnter(theirid, (char *)theirname, x, y, face, color);
		UserData->UserProp(theirid, j, k);

	}
}

void PalNet::RRoomDescend(int a, int b){
}

void PalNet::RUserNew(int a, int b){
	int theirid = ReadInt();
	int y = ReadShort();
	int x = ReadShort();
	int j[9], k[9];
	int i1 = 0;
	do {	j[i1] = ReadInt();
		k[i1] = ReadInt();
	} while (++i1 < 9); // props
	ReadShort(); //room
	int face = ReadShort();
	int color = ReadShort();
	ReadShort(); // zero?
	ReadShort(); // zero?
	int propnum = ReadShort();
	if(propnum < 9)
		j[propnum] = k[propnum] = 0;
	unsigned char theirname[32];
	memset(theirname, 0, 32);
	ReadFully(theirname, 32);
	theirname[theirname[0] + 1] = 0;
	for(int k = 1; k < 32; k++)
		theirname[k-1] = theirname[k];
	// add client enter call back here
	UserData->UserEnter(theirid, (char *)theirname, x, y, face, color);
	UserData->UserProp(theirid, j, k);
}

void PalNet::RPing(int a, int b){
	if(b != id)
		return;

	WriteInt(0x706f6e67);
	WriteInt(0);
	WriteInt(b);
	Flush();
}

// class b
void PalNet::RChat(int a, int b){
	//a is msg chksum + 1, idfrom is b
	int len = ReadShort();
	char *msg = new char[len + 1];
	ReadFully((unsigned char *)msg, len - 2);	// should get the zero term
	Decrypt((unsigned char *)msg, len - 3);
	msg[len] = 0;
	// chat call back
	UserData->UserSay(b, msg);
	delete [] msg;
}

void PalNet::RWhisperD(int a, int b){
	//a is msg chksum + 1, idfrom is b
	int len = ReadShort();
	char *msg = new char[len + 1];
	ReadFully((unsigned char *)msg, len - 2);	// should get the zero term
	Decrypt((unsigned char *)msg, len - 3);
	msg[len] = 0;
	// chat call back
	UserData->RecvWhisper(b, msg);
	delete [] msg;
}


// class v
void PalNet::RWhisper(int a, int b){
	// a is len, b is idto
	char *msg = new char[a + 1];
	ReadFully((unsigned char *)msg, a);	// should grab the zero
	msg[a] = 0;
	// system msg
	UserData->RecvWhisper(b, msg);
	delete[] msg;
}

void PalNet::RMove(int a, int b){
	// a is four, b is idwho
	int y = ReadShort();
	int x = ReadShort();
	// callback
	UserData->UserMove(b, x, y);
}

void PalNet::RColor(int a, int b){
	UserData->UserColor(b, (PalCallback::Color)ReadShort());
}

// class de
void PalNet::RFace(int a, int b){
	UserData->UserFace(b, (PalCallback::Face)ReadShort());
}

// class d4
void PalNet::RDesc(int a, int b){
	//ReadShort(); // la - face?
	UserData->UserFace(b, (PalCallback::Face)ReadShort());
	//ReadShort(); // lb - color?
	UserData->UserColor(b, (PalCallback::Color)ReadShort());
	int lc = ReadInt(); // cg.cg?

	int	d[9],
		e[9];

	for(int i = 0; i < 9; i++){
		if(i >= lc){
			d[i] = 0;
			e[i] = 0;
			continue;
		}
		d[i] = ReadInt();
		e[i] = ReadInt();
	}
	UserData->UserProp(b, d, e);
}

// class x
void PalNet::RProp(int a, int b){
	int lc = ReadInt(); // cg.cg?

	int	d[9],
		e[9];

	for(int i = 0; i < 9; i++){
		if(i >= lc){
			d[i] = 0;
			e[i] = 0;
			continue;
		}
		d[i] = ReadInt();
		e[i] = ReadInt();
	}
	UserData->UserProp(b, d, e);
}

void PalNet::RRename(int a, int b){
	int len = ReadByte();
	char *name = new char[len + 1];

	ReadFully((unsigned char *)name, len);
	name[len] = 0;

	UserData->UserRename(b, name);

	delete[] name;
}

void PalNet::RLogoff(int a, int b){
	// a is four, b is leaving id
	UserData->Population(ReadInt());
	UserData->UserLeave(b);
}

void PalNet::RError(int a, int b){
	Disconnect();
}
/*
typedef struct {
	LONG	        transactionID;	// Made unique by server 
	LONG            blockSize;
	short           blockNbr;
	short           nbrBlocks;
	union {
		struct {
			LONG    size;
			Str63   name;
			char    data[1];
		} firstBlockRec;
	
		struct {
			char    data[1];
		} nextBlockRec;
	} varBlock;

} FileBlockHeader, *FileBlockPtr;

*/
PalNet::FileDownloadNode::FileDownloadNode(char * name, int transID, int size){
	strncpy(this->name, name, 63);
	this->transID = transID;
	totalSize = size;
	data = new unsigned char [totalSize];
	sofar = 0;
	next = prev = 0;
	nextChunk = 0;
}

PalNet::FileDownloadNode::~FileDownloadNode(){
	if(prev != 0)
		prev->next = next;
	if(next != 0)
		next->prev = prev;
	delete[] data;
}

void PalNet::FileDownloadNode::AddChunk(int size, unsigned char * data){
	for(int i = 0; i < size; i++)
		this->data[sofar++] = data[i];
	nextChunk++;
}

PalNet::FileDownloadNode * PalNet::LocateDownload(int transID){
	FileDownloadNode * i = rootDownload;
	while(true){
		if(i == 0)
			return 0;
		else if (i->GetID() == transID)
			return i;

		i = i->GetNext();
	}
}

void PalNet::RrFile(int a, int b){
	FileDownloadNode * n;
	int transid = ReadInt();
	int chunksize = ReadInt();
	int chunknum = ReadShort();
	int totalchunks = ReadShort();

	if(chunknum == 0){
		int totalsize = ReadInt();
		if(totalsize < chunksize){
			printf("Unexpected extra large first chunck size!");
			return;
		}
		int namelen = ReadByte();
		unsigned char * name = new unsigned char[namelen + 1];
		ReadFully(name, namelen);
		name[namelen] = 0;
		n = new FileDownloadNode((char *)name, transid, totalsize);
		n->prev = rootDownload;
		if(rootDownload == 0)
			rootDownload = n;
		else
			rootDownload->next = n;

		for(int i = namelen; i < 63; i++)
			ReadByte();	// skip off 64 bytes from beginning of size of name

		delete[] name;

		UserData->RecvFileStart(transid, n->name);
	} else {
		n = LocateDownload(transid);
		if(n == 0)
			return;

		if(n->nextChunk != chunknum){
			UserData->RecvFileAbort(transid, n->name);
			delete n;
			return;
		}
	}

	unsigned char * data = new unsigned char[chunksize];
	ReadFully(data, chunksize);
	n->AddChunk(chunksize, data);
	delete[] data;

	UserData->RecvFileChunk(transid, n->name, n->sofar, n->totalSize);
	
	if(chunknum == totalchunks - 1){
		UserData->RecvFileEnd(transid, n->name, n->totalSize, (void *)n->data);
		delete n;
	}
}

void PalNet::RrAsset(int a, int b){
	int bigtype = ReadInt();
	if(bigtype != 1349676912) // "Prop"
		printf("Unknown/unhandled asset %d\n", bigtype);
	int id_a = ReadInt();
	int id_b = ReadInt();
	int len = a - 12;
	len -= 13 * 4;
	unsigned char * data = new unsigned char [len + 1];
	ReadFully(data, 13 * 4);// unknown, part of it is name after 20 bytes
	ReadFully(data, len);
	UserData->RecvAsset(id_a, id_b, len, data);
	delete[] data;
}

void PalNet::RUserExit(int a, int b){
	UserData->UserLeave(b);
}

// class y- a bunch of bp's
void PalNet::RAllRooms(int a, int b){
	RoomListNode * root, * present, * last;
	root = new RoomListNode();
		
	last = root;

	for(int i = 0; i < b; i++){
		present = new RoomListNode();
		last->next = present; 
		present->roomID = ReadInt();
		present->flags = ReadShort();
		present->population = ReadShort();
		int len = ReadByte();
		len = (len + (4 - (len & 3))) - 1;
		present->name = new char[len +1];
		ReadFully((unsigned char *)present->name, len);
		present->name[len] = 0;
		last = present;
		//printf("%d %d %s\n", roomid, flags, name);
	}

	UserData->RecvAllRooms(root->next);
	delete root;
}

// class d6 - a bunch of bt's
void PalNet::RAllUsers(int a, int b){
	UserListNode * root, * present, * last;

	root = new UserListNode();

	last = root;

	for(int i = 0; i < b; i++){
		present = new UserListNode();
		last->next = present; 
		present->userID = ReadInt();
		ReadShort(); // unknown
		present->room = ReadShort();
		int len = ReadByte();
		len = (len + (4 - (len & 3))) - 1;
		present->name = new char[len +1];
		ReadFully((unsigned char *)present->name, len);
		present->name[len] = 0;
		last = present;
		//printf("%d %d %s\n", roomid, flags, name);
	}

	UserData->RecvAllUsers(root->next);
	delete root;
}

