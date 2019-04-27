/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_DEBOUNCE.H:  declarations for the Debounce-Object
  Author: Chris Veigl

  This Object filters sudden changes to INVALID_VALUE. the number of considered
  samples can be selected.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"


class DEBOUNCEOBJ : public BASE_CL
{
	protected:
        float out,old_value;
        int dtime;
		int count_time;
		int active;

	public:

	DEBOUNCEOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~DEBOUNCEOBJ();

	friend LRESULT CALLBACK DebounceDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
