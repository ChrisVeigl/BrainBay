/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  OB_MOUSE.H:  contains the Mouse-Object
  Author: Chris Veigl

  This Objects proveds a control for the Mouse position and clicking functions.
  the values for x/y-positions and clicks are received by the input ports.

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.


-----------------------------------------------------------------------------*/

#include "brainBay.h"

class MOUSEOBJ : public BASE_CL
{
	protected: 
		float i;
		
	public:
	float xpos, ypos;
	int   xinc, yinc, dwelltime, dwellradius, resetradius, xmax, ymax, disable_dwell ;
	int bypass,bypass_pos, counter,updatepos,autodetect,enable_dwelling, minmaxreset;
	int lbutton, rbutton, lbuttond, dragbutton, doublebutton;
	int r_clicked, l_clicked, double_clicked, drag_active;
	int setdouble,setright,setdrag,clickselect;
	int dwxmin,dwxmax,dwymin,dwymax,dwellcount;
	int time_to_release_lbutton;
	char cursorfile[255];
	char * cursorindex;
	HWND hWndClick;
	
	MOUSEOBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~MOUSEOBJ();
};
