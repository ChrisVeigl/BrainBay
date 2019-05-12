/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_NOT.CPP
  Authors: Jeremy Wilkerson, Chris Veigl


  This Object performs the NOT-operation on it's two Input-Values and presents the
  result at the output-port. FALSE it represented by the constant INVALID_VALUE, TRUE
  is represented by the constand TRUE_VALUE (def: 512.0f )

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_not.h"

LRESULT CALLBACK NotDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	NOTOBJ * st;
	char temp[200];
	
	st = (NOTOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_NOT)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				CheckDlgButton(hDlg, IDC_BINARY, st->binary);
				SetDlgItemInt(hDlg,IDC_BITS,st->bits,0);
				SetDlgItemInt(hDlg,IDC_TRUE_VALUE,(int)st->numericTrueValue,1);
				SetDlgItemInt(hDlg,IDC_FALSE_VALUE,(int)st->numericFalseValue,1);

				SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "True-Value (512)" ) ;
				SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "NumericValue (below)" ) ;
				SendDlgItemMessage( hDlg, IDC_TRUECOMBO, CB_SETCURSEL, (WPARAM) (st->trueMode), 0L ) ;

				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "INVALID_VALUE (-32767)" ) ;
				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "NumericValue (below)") ;
				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "No Output" ) ;
				SendDlgItemMessage( hDlg, IDC_FALSECOMBO, CB_SETCURSEL, (WPARAM) (st->falseMode), 0L ) ;

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
				case IDC_BITS:
					st->bits=GetDlgItemInt(hDlg,IDC_BITS,NULL,0);
                    break;
				case IDC_TRUECOMBO:
					st->trueMode=SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_GETCURSEL, 0, 0 ) ;
					break;
				case IDC_FALSECOMBO:
					st->falseMode=SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_GETCURSEL, 0, 0 ) ;
					break;
				case IDC_TRUE_VALUE:
					GetDlgItemText(hDlg, IDC_TRUE_VALUE, temp,sizeof(temp));
					st->numericTrueValue=(float)atoi(temp);
                    break;
				case IDC_FALSE_VALUE:
					GetDlgItemText(hDlg, IDC_FALSE_VALUE, temp,sizeof(temp));
					st->numericFalseValue=(float)atoi(temp);
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


NOTOBJ::NOTOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
	input = INVALID_VALUE;
	binary=0;
	bits=127;
	numericTrueValue=1.0;
	numericFalseValue=0.0;
	trueMode=0;
	falseMode=0;
}
	
void NOTOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_NOTBOX, ghWndStatusbox, (DLGPROC)NotDlgHandler));
}


void NOTOBJ::load(HANDLE hFile) 
{
	int tmp;
	load_object_basics(this);
	load_property("binary",P_INT,&binary);
	load_property("bits",P_INT,&bits);
	load_property("trueMode",P_INT,&trueMode);
	load_property("falseMode",P_INT,&falseMode);
    tmp=1;
	load_property("trueValue",P_INT,&tmp);
	numericTrueValue=tmp;
    tmp=0;
	load_property("falseValue",P_INT,&tmp);
	numericFalseValue=tmp;
}

void NOTOBJ::save(HANDLE hFile) 
{
	int tmp;
	save_object_basics(hFile, this);
	save_property(hFile,"binary",P_INT,&binary);
	save_property(hFile,"bits",P_INT,&bits);
	save_property(hFile,"trueMode",P_INT,&trueMode);
	save_property(hFile,"falseMode",P_INT,&falseMode);
	tmp=numericTrueValue;
	save_property(hFile,"trueValue",P_INT,&tmp);
	tmp=numericFalseValue;
	save_property(hFile,"falseValue",P_INT,&tmp);
}
	
void NOTOBJ::incoming_data(int port, float value)
{
	if (port == 0)
		input = value;
}
	
void NOTOBJ::work(void)
{
	float value;

	if (!binary)
	{
		float actTrueValue=0,actFalseValue=0;

		switch (trueMode) {
			case 0: actTrueValue=TRUE_VALUE; break;
			case 1: actTrueValue=numericTrueValue; break;
		}

		switch (falseMode) {
			case 0: actFalseValue=INVALID_VALUE; break;
			case 1: actFalseValue=numericFalseValue; break;
		}

		if (input == INVALID_VALUE)
			pass_values(0, actTrueValue);
		else if (falseMode != 2) pass_values(0, actFalseValue);

	}
	else
	{
		value=(float) (  ((int)input) ^ bits );
		pass_values(0, value);
	}

}
	
NOTOBJ::~NOTOBJ() {}
