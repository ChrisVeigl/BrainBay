/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_ERPDETECT.H:  declarations for the ERP-Detector-Object
  Author: Chris Veigl

  The ERP-Detector - Object can record a signal and average a given number of trials
  (recording phase). Then, the signal stream is compared to this pattern and the 
  similarity (%) to the pattern is presented to the object's output port (detection phase)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"

#define ERPBUFLEN 1024 
#define MAXCHN 6 

LRESULT CALLBACK ErpdetectboxDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class ERPDETECTOBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;
	  
  public:

	float input[MAXCHN];	
	float ringbuf[MAXCHN][ERPBUFLEN];
	float epochbuf[MAXCHN][ERPBUFLEN];
	int bufpos,bufstart;
	int mode,trigger,recpos;
	int epochs, current, length, prestim;
	int method;
	int channels, actchn;
	char erpfile[256];

	ERPDETECTOBJ(int num);
	void update_inports(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~ERPDETECTOBJ();

   
};
