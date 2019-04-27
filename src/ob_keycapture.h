/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_KEYCAPTURE.H:  contains the Keycapture-Object
  Author: Chris Veigl

  This Object sends keycodes of pressed keys 

  
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

class KEYCAPTUREOBJ : public BASE_CL
{

	public:

	int mode;
	int findcode, replacecode;

	KEYCAPTUREOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void session_reset(void);

	void session_start(void);

	void work(void);

	~KEYCAPTUREOBJ();

	friend LRESULT CALLBACK KeyCaptureDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
