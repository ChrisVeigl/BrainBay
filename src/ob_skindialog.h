/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SKINDIALOG.H:  defclarations for the Skinned User Dialog

  based on the SkinStyle- Win32 Skinning Library 
  by John Roark <jroark@cs.usfca.edu>
  http://www.codeproject.com/dialog/skinstyle.asp
 
   This object callows to import a user-draw dialog from a skin.ini-file,
   to define button- and text-fields, and to process the dialog-events.
   the states of the buttons are presented at the object's output ports.

   Due to a Modification of the Skinstyle-Library, two buttons and one text-field
   can be combined to a Slider-Element: the output-value can be increased and
   decreased with the two buttons. 
   
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "Skinstyle\SkinObject.h"
#include "Skinstyle\SkinDialog.h"
#include "Skinstyle\SkinButton.h"
#include "Skinstyle\TransLabel.h"
#include "Skinstyle\DynArray.h"
#include "Skinstyle\KToolTip.h"


struct buttoninfoStruct {
	int num;
	char name[20];
	int x1,x2,y1,y2;
	char caption[40];
    int slidernum;
	int buttontype;
} ;

struct sliderinfoStruct {
	int num;
	char caption[20];
	char font[20];
	int fontsize;
	int fontcolor;
	int x1,x2,y1,y2;
	int min,max,set,type;
} ;

class SKINDIALOGOBJ : public BASE_CL
{
	public:
    int resetbutton[30],setbutton[30],setslider[30];
	int sliders[30],buttons[30];
	int num_sliders,num_buttons;
	int skin_top,skin_left,skin_bottom,skin_right;
	int dlgheight,dlgwidth;
	int buttoninfos,sliderinfos;
	int actbuttoninfo,actsliderinfo,actmousex, actmousey;
	char skinfile[256];
	char bmpfilename[256];
	struct buttoninfoStruct buttoninfo[30];
	struct sliderinfoStruct sliderinfo[30];

	SKINDIALOGOBJ(int num);

	int parse_skinfile(void);
	int save_skinfile(void);
	int apply_skin(void);
	int update_sliders(void);
	int update_buttons(void);
	int sliderport(int);
	int buttonport(int);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~SKINDIALOGOBJ();

	friend LRESULT CALLBACK SkinDialogDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
