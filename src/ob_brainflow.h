/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_BRAINFLOW.H:  contains the interface to the Brainflow dll for device integration
  
-----------------------------------------------------------------------------*/

#if _MSC_VER >= 1900


#include "brainBay.h"
#include <vector>

// process for receiving chunks, called in timer.cpp
void process_brainflow(void);


class BRAINFLOWOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;

  public: 
    char  archivefile[256];
	int  board_selection,board_id,channels;
	int show_position, show_extrachannels;
	char ipaddress[40], macaddress[40], serialport[40];
	int  timeout;
	char bfConfigString[250];
	int  channelMap[MAX_PORTS];
	int bf_channels;
	int syncChannel;

	int ipport;
	HANDLE filehandle;
	int  filemode;
	long filelength;
	int sync;

    BRAINFLOWOBJ(int num);
	int add_channels(std::vector <int> channelList, char * type);
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
    ~BRAINFLOWOBJ();
  
};

#endif