/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_TCP_SENDER.H:  contains functions for the Neuro Server data transmission
  Based on SDL_net-Code by Jeremy Wilkerson

  This object can connect to a running neuroserver and transmit a stream of signals
  in the EDF-format. the signals are captured from the object's input ports

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
#define s_writebuflength 8192
#define s_readbuflength 128

#define sockettimeout 50


class TCP_SENDEROBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];
	int ppos;
  
public: 

	IPaddress ip; 
	TCPsocket sock;
	SDLNet_SocketSet set;

	char readbuf[s_readbuflength];
	char writebuf[s_writebuflength];

	struct EDFHEADERStruct header;
	struct CHANNELStruct channel[MAX_EEG_CHANNELS];
	char   edfinfos[8192];
	char   host[101];
	int    streamnum;
	int    state;
	int    packetcount;
	int    packetnum;
	int    syncloss;
	LONGLONG timestamp;


    TCP_SENDEROBJ(int num);
	void update_inports(void);
	void incoming_data(int, float);
	int  connect();
	void get_captions(void);
	void work(void);
	int  read_tcp(char * readbuf, int size);
	int  start_sending(void);
	void close_tcp(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~TCP_SENDEROBJ();

};
