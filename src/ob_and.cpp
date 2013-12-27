/* -----------------------------------------------------------------------------

  BrainBay  -  Version 1.9, GPL 2003-2014

  MODULE:  OB_AND.CPP
  Authors: Jeremy Wilkerson, Chris Veigl


  This Object performs the AND-operation on it's two Input-Values and presents the
  result at the output-port. FALSE it represented by the constant INVALID_VALUE, TRUE
  is represented by the constand TRUE_VALUE (def: 512.0f )

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/
  
#include "brainBay.h"
#include "ob_and.h"



LRESULT CALLBACK AndDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	ANDOBJ * st;
	
	st = (ANDOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_AND)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				CheckDlgButton(hDlg, IDC_BINARY, st->binary);
				CheckDlgButton(hDlg, IDC_ONE, st->output_one);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_BINARY:
					st->binary=IsDlgButtonChecked(hDlg,IDC_BINARY);
                    break;
				case IDC_ONE:
					st->output_one=IsDlgButtonChecked(hDlg,IDC_ONE);
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


ANDOBJ::ANDOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
	strcpy(out_ports[0].out_name,"out");
	input1 = INVALID_VALUE;
	input2 = INVALID_VALUE;
	binary=0;
	output_one=0;
}

	
void ANDOBJ::make_dialog(void) 
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ANDBOX, ghWndStatusbox, (DLGPROC)AndDlgHandler));
}

void ANDOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("binary",P_INT,&binary);
	load_property("one",P_INT,&output_one);
}

void ANDOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"binary",P_INT,&binary);
	save_property(hFile,"one",P_INT,&output_one);
}
	
void ANDOBJ::incoming_data(int port, float value)
{
	if (port == 0)
		input1 = value;
	else if (port == 1)
		input2 = value;
}
	
void ANDOBJ::work(void)
{
	float value = TRUE_VALUE;
	if (!binary)
	{
		if ((input1 == INVALID_VALUE) || (input2 == INVALID_VALUE))
			value = INVALID_VALUE;
	}
	else
	{
		value=(float) ( ((int)input1) & ((int) input2) );
		if ((output_one) && (value!=0)) value=1;
	}
	pass_values(0, value);
}
	
ANDOBJ::~ANDOBJ() {}
