/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_DISPLAYVECTOR.CPP  declarations for the DisplayVector-Object
  Author:  Raymond Khoa

  This Object shows the value or values (in the case of array_data_ports
  coming to the module's inport (for testing purposes)

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_displayvector.h"
#include <iostream>

using namespace std;

DISPLAYVECTOROBJ::DISPLAYVECTOROBJ(int num) : BASE_CL()
{
	inports = 1;
	outports = 0;
	width=95;
	in_ports[0].in_type = MFLOAT;
	strcpy(value_str,"");
}
	
void DISPLAYVECTOROBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_DISPLAYVECTOR, ghWndStatusbox, (DLGPROC)DisplayVectorDlgHandler));
}

void DISPLAYVECTOROBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
}

void DISPLAYVECTOROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
}
	
void DISPLAYVECTOROBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE) sprintf(value_str,"%.4f ",value);
	else strcpy(value_str,"INVALID_VALUE");
}
void DISPLAYVECTOROBJ::incoming_data(int port, float *value, int count)
{
	if (count>0){
		strcpy(value_str,"");
   		for (int i=0;i<count;i++)
			sprintf(value_str,"%s%.4f ",value_str,value[i]);
	}
	else
		strcpy(value_str,"NO_VALUES");
}

void DISPLAYVECTOROBJ::work(void)
{
	SetDlgItemText(hDlg,IDC_VALUES,value_str);
}

DISPLAYVECTOROBJ::~DISPLAYVECTOROBJ() {
}

LRESULT CALLBACK DisplayVectorDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	DISPLAYVECTOROBJ * st;
	
	st = (DISPLAYVECTOROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_DISPLAYVECTOR)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
			break;		
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_VALUES:
					break;
			}
			return TRUE;
			break;
		case WM_HSCROLL:			
		
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}
