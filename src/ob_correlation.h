/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_CORRELATION.H:  declarations for the Correlation-Object
  Author: Jeremy Wilkerson

  The Correlation between two input-streams is calculated and presented 
  at the output port.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define NUMSAMPLES 1001

class CORRELATIONOBJ : public BASE_CL
{
	protected:
        float accum1, accum2, in1,in2;
		float samples1[NUMSAMPLES];
        float samples2[NUMSAMPLES];
        int interval;
        int writepos1, writepos2;
        int added1, added2;

	public:

	CORRELATIONOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~CORRELATIONOBJ();

	friend LRESULT CALLBACK CorrDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    private:
    
    void change_interval(int newinterval);
};
