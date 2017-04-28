/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
  
  OB_GANGLION.H:  contains the interface to the OpenBCI Ganglion device
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"


#include <iostream>
#include "SDL_net.h"

#define TCP_OK 200
#define TCP_BAD_REQUEST 400
#define TCP_ERROR 0


using namespace std;

#define defaulthost "localhost"
#define GANGLIONHUB_PORT 10996
#define s_writebuflength 1024
#define s_readbuflength 8192

#define sockettimeout 50


class GANGLIONOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];

  public: 
	IPaddress ip; 
	TCPsocket sock;
	SDLNet_SocketSet set;

	int  state;
	char readbuf[s_readbuflength];
	char writebuf[s_writebuflength];


	char  archivefile[256];
	char  device[100];
	HANDLE filehandle;
	int  filemode;
	long filelength;

    GANGLIONOBJ(int num);

	int  connect();
	int  read_tcp(char * readbuf, int size);
	int  sendstring_tcp(char * buf);
	void close_tcp(void);

	void update_channelinfo(void);
	void session_reset(void);
	void session_start(void);
	void session_stop (void);
	long session_length (void);
	void session_pos (long pos);
	void work(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~GANGLIONOBJ();
  
};
