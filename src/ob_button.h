/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_BUTTON.H:  contains the BUTTON-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define STATE_IDLE 0
#define STATE_PRESSED 1

#define BUTTONFUNCTION_PLAYSESSION 0
#define BUTTONFUNCTION_STOPSESSION 1
#define BUTTONFUNCTION_ENDSESSION 2
#define BUTTONFUNCTION_VAL1VAL2 3
#define BUTTONFUNCTION_VAL1INV 4
#define BUTTONFUNCTION_TOGGLEVAL 5
#define BUTTONFUNCTION_TOGGLE1SEC 6
#define BUTTONFUNCTION_DEVSETTINGS 7
#define BUTTONFUNCTION_APPSETTINGS 8


class BUTTONOBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;

  public: 
	int displayborder;
	int buttonfunction;
	char buttonpath[256];
	char buttoncaption[80];
	int bitmapsize;
	int state;
	int togglecount;

	COLORREF transcolor, bkcolor;
	int top,left,right,bottom;
	int redraw;
	int value1, value2;

    BUTTONOBJ(int num);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
    ~BUTTONOBJ();
    
};
