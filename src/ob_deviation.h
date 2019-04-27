/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_DEVIATION.H:  declarations for the Deviation-Object
  Author: Chris Veigl

  This Object outputs the Standard Deviation and Mean of n 
  Samples captured from it's input-port

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"

#define NUMSAMPLES 1001

class DEVIATIONOBJ : public BASE_CL
{
	protected:
		float meanaccu,devaccu;
		float samples[NUMSAMPLES];
		float squares[NUMSAMPLES];
		float mean,deviation;
        int writepos, added;

	public:
		int interval; 

	DEVIATIONOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	void change_interval(int newinterval);

	~DEVIATIONOBJ();

	friend LRESULT CALLBACK DEVIATIONDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
       
    
};
