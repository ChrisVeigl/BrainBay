/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_SESSIONMANAGER.H:  contains the SESSIONMANAGER-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define ACCULEN 1000




class SESSIONMANAGEROBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;

  public: 
	int displaynavigation;
	int menuitems;
	int actmenuitem;
	int actreportitem;
	char wndcaption[80];
	char sessionlist[4096];
	char sessionname[20][100];
	char sessionpath[20][256];
	char sessionreport[20][256];
	char logopath[256];
	int  maxreportitems[20];
	char actreport[256];
	int bitmapsize;
	int menu_x,menu_y,navcross_x,navcross_y,logo_x,logo_y;

	HFONT font, smallfont;
	COLORREF selectcolor, bkcolor, fontcolor, fontbkcolor;
	int fontsize;
	int top,left,right,bottom;
	int redraw;

    SESSIONMANAGEROBJ(int num);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
    ~SESSIONMANAGEROBJ();
    
};
