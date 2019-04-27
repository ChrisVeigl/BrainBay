/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_TRANSLATE.H:  contains the TRANSLATE-Object


  This object is not finished yet. It shall provide mapping for signals
  Now, it can do only gaining and shifting of the signal.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define MAX_TRANSLATIONPOINTS 40

class TRANSLATEOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
  
  public: 
	float input;
	float map[1024];
	int   points;
	int   pointx[MAX_TRANSLATIONPOINTS];
	float pointy[MAX_TRANSLATIONPOINTS];
	int   setpoint,actmousex,actmousey;

	
	TRANSLATEOBJ(int num);
	void calculate_map(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~TRANSLATEOBJ();


};
