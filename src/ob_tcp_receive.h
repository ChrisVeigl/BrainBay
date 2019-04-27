/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_TCP_RECEIVE.H:  declarations for the Neuro Server data reception
  Based on SDL_net-Code by Jeremy Wilkerson

  This object can connect to a running neuroserver and receive a stream of signals
  in the EDF-format. the signals are presented to the object's output ports

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include <iostream>
#include "SDL_net.h"

#define TCP_OK 200
#define TCP_BAD_REQUEST 400
#define TCP_ERROR 0


using namespace std;

#define defaulthost "localhost"
#define PORT 8336
#define writebuflength 128
#define readbuflength 8192
#define watchbuflength 14000

#define sockettimeout 50


class TCP_RECEIVEOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];
	int  state,ppos;
	char packet[500];
  
public: 
	IPaddress ip; 
	TCPsocket sock;
	SDLNet_SocketSet set;

	char edfheader[readbuflength];
	char watchbuf[watchbuflength];
	char writebuf[100];

	struct EDFHEADERStruct header;
	struct CHANNELStruct channel[MAX_EEG_CHANNELS];
	char   host[101];
	int    streamnum;
	int    packetcount;
	int    bufstart,bufend;
	int    watching;
	int    packetnum;
	int    syncloss;
	LONGLONG timestamp;


    TCP_RECEIVEOBJ(int num);
	int  connect();
	void get_captions(void);
	void work(void);
	int  read_tcp(char * readbuf, int size);
	int  watch_tcp(char * readbuf, int size);
	int  start_watching(void);
	void unwatch(void);
	void clear_buffer(void);
	void reset(void);
	void session_reset(void);
	void session_start(void);
	void session_stop(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~TCP_RECEIVEOBJ();

};
