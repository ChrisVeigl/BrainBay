/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_EEG.H:  contains the EEG-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"


//  from OB_EEG.CPP :

LRESULT CALLBACK EEGDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class EEGOBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;
	int i;
	  
  public:
	int scrolling;
    unsigned char	  readstate;
	unsigned int 	  extract_pos;
    unsigned char     number;
	unsigned char     old_number;
	unsigned char     switches;
	unsigned char     aux;
	unsigned int      buffer[MAX_EEG_CHANNELS*2];
	unsigned int      chnmatrix;
	int				  resolution;


	EEGOBJ(int num);
	void make_dialog(void);
	void session_start(void);
	void session_stop(void);
	void session_reset(void);
	void session_pos(long pos);
	long session_length(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void work(void);
	~EEGOBJ();
	
	
};
