/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
  
  OB_NEUROBIT.H:  contains the interface to the Neurobit OPTIMA device
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"




class NEUROBITOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;

  public: 
	int  test;
    char  archivefile[256];
	char  device[100];
	HANDLE filehandle;
	int  filemode;

    NEUROBITOBJ(int num);
    void update_channelinfo(void);
	void save_devctx(void);
	void load_devctx(void);
	void session_reset(void);
	void session_start(void);
	void session_stop (void);
	void work(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~NEUROBITOBJ();
  
};
