/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SHADOW.H:  contains the SHADOW-Object
  Author: Chris Veigl

  This Object allows to occlude a selectable area of the screen

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"



class SHADOWOBJ : public BASE_CL
{

  public: 
	float input,oldinput;
	int top,left,bottom,right;
	int locked;
	COLORREF bkcolor;

    SHADOWOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
    ~SHADOWOBJ();
    
  
};
