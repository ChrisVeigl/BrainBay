/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
  
  OB_KEYCAPTURE.CPP:  contains the Keycapture-Object
  Author: Chris Veigl

  This Object sends keycodes of pressed keys 

  
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_keycapture.h"

KEYCAPTUREOBJ::KEYCAPTUREOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 0;
	width=75;
	strcpy(out_ports[0].out_name,"key");

	mode=0;

}
	
void KEYCAPTUREOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_KEYCAPTUREBOX, ghWndStatusbox, (DLGPROC)KeyCaptureDlgHandler));
}

void KEYCAPTUREOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("mode",P_INT,&mode);
}

void KEYCAPTUREOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"mode",P_INT,&mode);
}
	

void KEYCAPTUREOBJ::session_start(void)
{
	// mode=0;
}

void KEYCAPTUREOBJ::session_reset(void)
{

}

  	
void KEYCAPTUREOBJ::incoming_data(int port, float value)
{

}


void KEYCAPTUREOBJ::work(void)
{
	pass_values(0, GLOBAL.pressed_key);
}


KEYCAPTUREOBJ::~KEYCAPTUREOBJ() {}

LRESULT CALLBACK KeyCaptureDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char tmp[50];
	KEYCAPTUREOBJ * st;
	
	st = (KEYCAPTUREOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_SAMPLE_HOLD)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemInt(hDlg, IDC_MODE, st->mode, 0);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_MODE:
					GetDlgItemInt(hDlg,IDC_MODE,&st->mode,NULL);
					break;

			}
			return TRUE;
			break;
		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}




