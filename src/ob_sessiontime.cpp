/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_SESSIONTIME.CPP
  Author:  Chris Veigl


  This Object can halt a running session

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_sessiontime.h"


LRESULT CALLBACK SessiontimeDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	SESSIONTIMEOBJ * st;
	
	st = (SESSIONTIMEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_SESSIONTIME)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemInt(hDlg, IDC_SESSIONTIME, st->sessiontime, 0) ;
				SetDlgItemText(hDlg, IDC_NEXTCONFIGNAME, st->nextconfigname) ;
				CheckDlgButton(hDlg, IDC_STOPWHENFINISH,st->stopwhenfinish);
				CheckDlgButton(hDlg, IDC_LOADNEXTCONFIG,st->loadnextconfig);
				CheckDlgButton(hDlg, IDC_COUNTDOWN,st->countdown);
				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			
			switch (LOWORD(wParam)) 
			{ 
			case IDC_STOPWHENFINISH:
				st->stopwhenfinish=IsDlgButtonChecked(hDlg,IDC_STOPWHENFINISH);
				break;
			case IDC_LOADNEXTCONFIG:
				st->loadnextconfig=IsDlgButtonChecked(hDlg,IDC_LOADNEXTCONFIG);
				break;
			case IDC_NEXTCONFIGNAME:
				GetDlgItemText(hDlg, IDC_NEXTCONFIGNAME, st->nextconfigname, 100);
				break;
			case IDC_SESSIONTIME:
				st->sessiontime=GetDlgItemInt(hDlg,IDC_SESSIONTIME,NULL,0);
				break;
			case IDC_COUNTDOWN:
				st->countdown=IsDlgButtonChecked(hDlg,IDC_COUNTDOWN);
				break;
			}
			return TRUE;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
} 


SESSIONTIMEOBJ::SESSIONTIMEOBJ(int num) : BASE_CL()
{
	int t;
	outports = 1;
	inports = 1;
	width=95;
	count=0;
	countdown=1;
	stopwhenfinish=true;
	loadnextconfig=false;
	sessiontime=120;
	sprintf(in_ports[0].in_name,"stop");
	sprintf(out_ports[0].out_name,"time");
	strcpy(nextconfigname,"test.con");
}
	
void SESSIONTIMEOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("sessiontime",P_INT,&sessiontime);
	load_property("stopwhenfinish",P_INT,&stopwhenfinish);
	load_property("nextconfigname",P_STRING,nextconfigname);
	load_property("loadnextconfig",P_INT,&loadnextconfig);
	load_property("countdown",P_INT,&countdown);
}

void SESSIONTIMEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"sessiontime",P_INT,&sessiontime);
	save_property(hFile,"stopwhenfinish",P_INT,&stopwhenfinish);
	save_property(hFile,"nextconfigname",P_STRING,nextconfigname);
	save_property(hFile,"loadnextconfig",P_INT,&loadnextconfig);
	save_property(hFile,"countdown",P_INT,&countdown);
}

void SESSIONTIMEOBJ::make_dialog(void) 
{  
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SESSIONTIMEBOX, ghWndStatusbox, (DLGPROC)SessiontimeDlgHandler)); 
}

void SESSIONTIMEOBJ::session_start(void)
{
	//count=0;
}

	
void SESSIONTIMEOBJ::incoming_data(int port, float value)
{
	if ((value != INVALID_VALUE) && (stopwhenfinish))
			SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);

}
	
void SESSIONTIMEOBJ::session_reset(void)
{
	count=0;
}

void SESSIONTIMEOBJ::work(void)
{
	count++;
	int seconds=count/PACKETSPERSECOND;
	if (countdown) 
		pass_values(0,(float)(sessiontime-seconds));
	else
		pass_values(0,(float)seconds);
	if ((seconds>sessiontime) && (stopwhenfinish)) {
		SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
		for (int t=0;t<GLOBAL.objects;t++) objects[t]->session_end();
		if (loadnextconfig) {
			strcpy(GLOBAL.nextconfigname, nextconfigname);
			SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,1);
		}
	}
}

SESSIONTIMEOBJ::~SESSIONTIMEOBJ() {}

