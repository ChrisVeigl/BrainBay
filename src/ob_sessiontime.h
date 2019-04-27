/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_SESSIONTIME.H  declarations for the Sessiontime-Object
  Author:  Chris Veigl


  This Object outputs the sessiontime and can halt a running session 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"

class SESSIONTIMEOBJ : public BASE_CL
{
	public:
	int stopwhenfinish;
	int loadnextconfig;
	int sessiontime;
	int countdown;
	char nextconfigname[100];
	long count;

	SESSIONTIMEOBJ(int num);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);	
	void make_dialog(void);
	void session_start(void); 
	void session_reset(void);
	void work(void);
	~SESSIONTIMEOBJ();
};
