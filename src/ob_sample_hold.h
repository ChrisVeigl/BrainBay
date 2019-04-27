/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SAMPLE_HOLD.h:  contains the Sample-and-hold-Object


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"

class SAMPLE_HOLDOBJ : public BASE_CL
{

	public:

	float hold,act, trigger, old_value;
	int mode;
	float resetvalue;

	SAMPLE_HOLDOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void session_reset(void);

	void session_start(void);

	void work(void);

	~SAMPLE_HOLDOBJ();

	friend LRESULT CALLBACK Sample_HoldDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
