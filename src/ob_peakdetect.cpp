/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_PEAKDETECT.CPP
  Author:  Chris Veigl


  This Object outputs the tops and/or valleys of a signal

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_peakdetect.h"


LRESULT CALLBACK PeakDetectDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	PEAKDETECTOBJ * st;
	
	st = (PEAKDETECTOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_PEAKDETECT)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:

				switch (st->mode)
				{
				case 0:
					CheckDlgButton(hDlg,IDC_TOPS,TRUE);
					CheckDlgButton(hDlg,IDC_VALLEYS,FALSE);
					CheckDlgButton(hDlg,IDC_BOTH,FALSE);
					break;
				case 1:
					CheckDlgButton(hDlg,IDC_TOPS,FALSE);
					CheckDlgButton(hDlg,IDC_VALLEYS,TRUE);
					CheckDlgButton(hDlg,IDC_BOTH,FALSE);
					break;
				case 2:
					CheckDlgButton(hDlg,IDC_TOPS,FALSE);
					CheckDlgButton(hDlg,IDC_VALLEYS,FALSE);
					CheckDlgButton(hDlg,IDC_BOTH,TRUE);
					break;		
				}
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_TOPS:  st->mode=0;break;
				case IDC_VALLEYS:  st->mode=1;break;
				case IDC_BOTH:  st->mode=2;break;
				
			}
			break;
			
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}


PEAKDETECTOBJ::PEAKDETECTOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	width=70;
    strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
	mode =0; dir=0; olddir=0; peak=0;
	oldval=0;
    
}
	
void PEAKDETECTOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_PEAKDETECTBOX, ghWndStatusbox, (DLGPROC)PeakDetectDlgHandler));
}

void PEAKDETECTOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("mode",P_INT,&mode);
}

void PEAKDETECTOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"mode",P_INT,&mode);
}
	
void PEAKDETECTOBJ::incoming_data(int port, float value)
{
	input=value;	
}
	
void PEAKDETECTOBJ::work(void)
{
	if (input!=INVALID_VALUE)
	{  
		if (input > oldval) dir=1;
		else if (input< oldval) dir=-1;

		if (dir!=olddir)
		{
			if ((mode==0)&&(dir==-1)) { peak=oldval; }
			if ((mode==1)&&(dir==1))  { peak=oldval; }
			if (mode==2)  { peak=oldval; }
			olddir=dir;
		}
		oldval=input;
		pass_values(0, peak);
	}
	else pass_values(0, INVALID_VALUE);
}


PEAKDETECTOBJ::~PEAKDETECTOBJ() {}

