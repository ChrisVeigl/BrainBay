/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_KEYSTRIKE.H:  contains the Keystrike-Object
  Author: Chris Veigl

  This Object can generate a selectable Keyboard Sequence 
  (Keyup and Keydown Messages for various Keys)

  
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/

#include "brainBay.h"

class KEYSTRIKEOBJ : public BASE_CL
{
		
	public:
	float input,oldinput;
	int numkeys, actkey;
	char keylist[50][50];
	
	KEYSTRIKEOBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~KEYSTRIKEOBJ();
};
