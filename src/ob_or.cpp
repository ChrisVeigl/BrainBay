/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_OR.CPP
  Authors: Jeremy Wilkerson, Chris Veigl


  This Object performs the OR-operation on it's two Input-Values and presents the
  result at the output-port. FALSE it represented by the constant INVALID_VALUE, TRUE
  is represented by the constand TRUE_VALUE (def: 512.0f )

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_or.h"


LRESULT CALLBACK OrDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	OROBJ * st;
	char temp[200];
	
	st = (OROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_OR)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				CheckDlgButton(hDlg, IDC_BINARY, st->binary);
				SetDlgItemInt(hDlg,IDC_TRUE_VALUE,(int)st->trueValue,1);
				SetDlgItemInt(hDlg,IDC_FALSE_VALUE,(int)st->falseValue,1);

				SendDlgItemMessage(hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Send True-Value for true and False-Value for false" ) ;
				SendDlgItemMessage(hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Send Input1 for true and False-Value for false" ) ;
				SendDlgItemMessage(hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Send Input1 for true and nothing for false" ) ;
				SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_SETCURSEL, (WPARAM) (st->mode), 0L ) ;
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

				case IDC_MODECOMBO:
					st->mode=SendDlgItemMessage(hDlg, IDC_MODECOMBO, CB_GETCURSEL, 0, 0 ) ;
				case IDC_TRUE_VALUE:
					GetDlgItemText(hDlg, IDC_TRUE_VALUE, temp,sizeof(temp));
					st->trueValue=(float)atoi(temp);
                    break;
				case IDC_FALSE_VALUE:
					GetDlgItemText(hDlg, IDC_FALSE_VALUE, temp,sizeof(temp));
					st->falseValue=(float)atoi(temp);
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




OROBJ::OROBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
	strcpy(out_ports[0].out_name,"out");
	input1 = INVALID_VALUE;
	input2 = INVALID_VALUE;
	binary=0;
	trueValue=512.0;
	falseValue=INVALID_VALUE;
	mode=0;
}
	
void OROBJ::make_dialog(void) 
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ORBOX, ghWndStatusbox, (DLGPROC)OrDlgHandler));
}


void OROBJ::load(HANDLE hFile) 
{
	int tmp;
	load_object_basics(this);
	load_property("binary",P_INT,&binary);
	load_property("mode",P_INT,&mode);
	load_property("trueValue",P_INT,&tmp);
	trueValue=tmp;
	load_property("falseValue",P_INT,&tmp);
	falseValue=tmp;
}

void OROBJ::save(HANDLE hFile) 
{
	int tmp;
	save_object_basics(hFile, this);
	save_property(hFile,"binary",P_INT,&binary);
	save_property(hFile,"mode",P_INT,&mode);
	tmp=trueValue;
	save_property(hFile,"trueValue",P_INT,&tmp);
	tmp=falseValue;
	save_property(hFile,"falseValue",P_INT,&tmp);
}
	
void OROBJ::incoming_data(int port, float value)
{
	if (port == 0)
		input1 = value;
	else if (port == 1)
		input2 = value;
}
	
void OROBJ::work(void)
{
	float value;
	
	if (!binary)
	{
		switch (mode) {
			case 0:
				if ((input1 != INVALID_VALUE) || (input2 != INVALID_VALUE))
					pass_values(0, trueValue);
				else pass_values(0, falseValue);
				break;
			case 1:
				if ((input1 != INVALID_VALUE) || (input2 != INVALID_VALUE))
					pass_values(0, input1);
				else pass_values(0, falseValue);
				break;
			case 2:
				if ((input1 != INVALID_VALUE) || (input2 != INVALID_VALUE)) 
					pass_values(0, input1);
				break;
		}
	}
	else
	{
		value=(float) ( ((int)input1) | ((int) input2) );
		pass_values(0, value);
	}
}
	
OROBJ::~OROBJ() {}
