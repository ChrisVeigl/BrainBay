/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_EMOTIV.H:  contains the interface to the 
          Emotiv Epoc neuroheadset. 
  Author: Dominik Koller

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/
#include "brainBay.h"
#include "emotiv\\my_edk.h"
#include "emotiv\\my_EmoStateDLL.h"
#include "emotiv\\my_edkErrorCode.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

void process_emotiv(void);

class EMOTIVOBJ : public BASE_CL
{

public:
	char archivefile[256];
	HANDLE filehandle;
	int  filemode;
	int state;

	EMOTIVOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void updateHeadsetStatus(void);
	void work(void);
	
	void session_reset(void);
	void session_start(void);
	void session_stop (void);
	~EMOTIVOBJ();
};
	