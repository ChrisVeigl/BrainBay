/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COUNTER.H:  contains the COUNTER-Object
  Author: Chris Veigl

  This Object outputs a counter value at it's port and displays the value in a 
  seperate Window. It can count transitions of the input to or from INVALID_VALUE
  or use the input directly (without counting)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"



class COUNTEROBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;
	LONGLONG period,prev_time;

  public: 
	float input,oldinput;
    HFONT font;
	float countervalue;
	float resetvalue;
	unsigned long scount;
	
	int  showcounter;
	int  fontsize;
	int  digits;
	int  timeformat;
	int  mode, integer;
	int  top,left,bottom,right;
	char wndcaption[50];
	COLORREF fontcolor,bkcolor;



    COUNTEROBJ(int num);
	void make_dialog(void);
	void session_reset(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void update_inports(void);
	void work(void);
    ~COUNTEROBJ();
    
  
};
