/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_GANGLION.H:  contains the interface to the OpenBCI Ganglion device
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#include <iostream>
#include "SDL_net.h"

#define TCP_OK 200
#define TCP_BAD_REQUEST 400
#define TCP_ERROR 0


using namespace std;


class GANGLIONOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;

  public: 
	char  archivefile[256];
	char  device[100];
	HANDLE filehandle;
	int  filemode;
	long filelength;

    GANGLIONOBJ(int num);

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
