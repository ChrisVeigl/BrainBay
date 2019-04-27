/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COMPARE.CPP:  contains functions for the Comparator-Object
  Author: Chris Veigl

  This Object compares two inputs (if they are greater, less or equal)
  If the condition is met, the output is copied from input port 1,
  if not, the output is set to INVALID_VALUE


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_compare.h"

LRESULT CALLBACK COMPAREDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	COMPAREOBJ * st;
	char tmp[20];
	
	st = (COMPAREOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_COMPARE)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				CheckDlgButton(hDlg, IDC_RET_INVALID, st->out_invalid);
				sprintf(tmp,"%.2f",st->out_value);
				SetDlgItemText(hDlg, IDC_RET_VALUE, tmp);
				switch (st->method)
				{
					case 0: CheckDlgButton(hDlg, IDC_GREATER, TRUE); break;
					case 1: CheckDlgButton(hDlg, IDC_GREATEREQUAL, TRUE); break;
					case 2: CheckDlgButton(hDlg, IDC_LESS, TRUE); break;
					case 3: CheckDlgButton(hDlg, IDC_LESSEQUAL, TRUE); break;
					case 4: CheckDlgButton(hDlg, IDC_EQUAL, TRUE); break;
				}
			break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 

			case IDC_GREATER: st->method=0; strcpy(st->tag, "A>B"); InvalidateRect(ghWndDesign,NULL,TRUE);break;
			case IDC_GREATEREQUAL: st->method=1; strcpy(st->tag, "A>=B");InvalidateRect(ghWndDesign,NULL,TRUE);break;
			case IDC_LESS: st->method=2;strcpy(st->tag, "A<B"); InvalidateRect(ghWndDesign,NULL,TRUE);break;
			case IDC_LESSEQUAL: st->method=3; strcpy(st->tag, "A<=B");InvalidateRect(ghWndDesign,NULL,TRUE);break;
			case IDC_EQUAL: st->method=4; strcpy(st->tag, "A=B");InvalidateRect(ghWndDesign,NULL,TRUE);break;
			case IDC_RET_INVALID: st->out_invalid=IsDlgButtonChecked(hDlg,IDC_RET_INVALID);break;
			case IDC_RET_VALUE:
					GetDlgItemText(hDlg,IDC_RET_VALUE,tmp,20);
					sscanf(tmp,"%f",&st->out_value);
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

COMPAREOBJ::COMPAREOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
    out_invalid=true;
	out_value=0;
    strcpy(in_ports[0].in_name,"A");
    strcpy(in_ports[1].in_name,"B");
	strcpy(out_ports[0].out_name,"out");
    method = 0; strcpy(tag, "A>B");
}
	
void COMPAREOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_COMPAREBOX, ghWndStatusbox, (DLGPROC)COMPAREDlgHandler));
}

void COMPAREOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("method",P_INT,&method);
   load_property("output_inv",P_INT,&out_invalid);
   load_property("output_value",P_FLOAT,&out_value);
}

void COMPAREOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"method",P_INT,&method);
    save_property(hFile,"output_inv",P_INT,&out_invalid);
    save_property(hFile,"output_value",P_FLOAT,&out_value);
}
	
void COMPAREOBJ::incoming_data(int port, float value)
{
	switch (port)
	{
		case 0: inputA=value; break;
		case 1: inputB=value; break;
	}
}
	
void COMPAREOBJ::work(void)
{
    float c;
	
//	if (inputA==INVALID_VALUE) inputA=-INVALID_VALUE;
//	if (inputB==INVALID_VALUE) inputB=-INVALID_VALUE;

	if ((inputB==INVALID_VALUE) || (inputA==INVALID_VALUE)) c=INVALID_VALUE;
	else
	switch (method)
	{
		case 0: if (inputA>inputB) c=inputA; else c=INVALID_VALUE; break;
		case 1: if (inputA>=inputB) c=inputA; else c=INVALID_VALUE; break;
		case 2: if (inputA<inputB) c=inputA; else c=INVALID_VALUE; break;
		case 3: if (inputA<=inputB) c=inputA; else c=INVALID_VALUE; break;
		case 4: if (inputA==inputB) c=inputA; else c=INVALID_VALUE; break;
	}
	if ((c==INVALID_VALUE) && (!out_invalid))
		pass_values(0, out_value);
	else 
		pass_values(0, c);
}


COMPAREOBJ::~COMPAREOBJ() {}

