/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_MAGNITUDE.H:  contains the MAGNITUDE-Object
  Authors: Jim Peters,Chris Veigl

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/



#include "brainBay.h"


//  from OB_MAGNITUDE.CPP :
LRESULT CALLBACK MagnitudeDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class MAGNITUDEOBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;
	  
  public:

	float input;
	
	int filtertype;
	float center;
	float wid;
	int order;
	int gain;
	
    FidFilter *lp1filt;
    FidFunc *lp1funcp;
	FidRun *lp1run;
	void * lp1fbuf;

	FidFilter *lp2filt;
    FidFunc *lp2funcp;
	FidRun *lp2run;
	void * lp2fbuf;

	MAGNITUDEOBJ(int num);
	void make_dialog(void);
	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~MAGNITUDEOBJ();

 };
