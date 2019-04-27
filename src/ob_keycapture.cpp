/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
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
	findcode=32;
	replacecode=1;

}
	
void KEYCAPTUREOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_KEYCAPTUREBOX, ghWndStatusbox, (DLGPROC)KeyCaptureDlgHandler));
}

void KEYCAPTUREOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("mode",P_INT,&mode);
    load_property("findcode",P_INT,&findcode);
    load_property("replacecode",P_INT,&replacecode);
}

void KEYCAPTUREOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"mode",P_INT,&mode);
    save_property(hFile,"findcode",P_INT,&findcode);
    save_property(hFile,"replacecode",P_INT,&replacecode);
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
	if (mode==0)
		pass_values(0, GLOBAL.pressed_key);
	else if (mode==1) {
		if (GLOBAL.pressed_key == findcode)
			pass_values(0, replacecode);
		else
			pass_values(0, 0);
	}

	if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
	{
		SetDlgItemInt(hDlg, IDC_ACTKEY, GLOBAL.pressed_key, 0); 
	}

}


KEYCAPTUREOBJ::~KEYCAPTUREOBJ() {}

LRESULT CALLBACK KeyCaptureDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char tmp[50];
	KEYCAPTUREOBJ * st;
	
	st = (KEYCAPTUREOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_KEYCAPTURE)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:
				SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "output all keycode values" ) ;
				SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "detect one keycode and output custom value" ) ;
				SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_SETCURSEL, st->mode, 0L ) ;
				SetDlgItemInt(hDlg, IDC_ACTKEY, 0, 0);
				SetDlgItemInt(hDlg, IDC_FINDCODE, st->findcode, 0);
				SetDlgItemInt(hDlg, IDC_REPLACECODE, st->replacecode, 0);
				break;	

		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_MODECOMBO:
	 				 if (HIWORD(wParam)==CBN_SELCHANGE)
					 	st->mode=(SendMessage(GetDlgItem(hDlg, IDC_MODECOMBO), CB_GETCURSEL , 0, 0));
					break;
				case IDC_FINDCODE:
					st->findcode=GetDlgItemInt(hDlg,IDC_FINDCODE,0,0);
					break;
				case IDC_REPLACECODE:
					st->replacecode=GetDlgItemInt(hDlg,IDC_REPLACECODE,0,0);
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




