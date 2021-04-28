/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  OB_MOUSE.CPP:  contains the Mouse-Object
  Author: Chris Veigl

  This Objects proveds a control for the Mouse position and clicking functions.
  the values for x/y-positions and clicks are received by the input ports.

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.


-----------------------------------------------------------------------------*/

#define _WIN32_WINNT 0x0500
#define OEMRESOURCE

#include "brainBay.h"
#include "ob_mouse.h"
//#include <winuser.h>

MOUSEOBJ * mo;
int test=0;


HINSTANCE hinst;                 // handle to current instance  
HCURSOR hCurs1, hCurs2,hCurs3;  // cursor handles 
 
// Yin-shaped cursor AND mask 
 
BYTE ANDmaskCursor[] = 
{ 
    0xFF, 0xFC, 0x3F, 0xFF,   // line 1 
    0xFF, 0xC0, 0x1F, 0xFF,   // line 2 
    0xFF, 0x00, 0x3F, 0xFF,   // line 3 
    0xFE, 0x00, 0xFF, 0xFF,   // line 4 
 
    0xF7, 0x01, 0xFF, 0xFF,   // line 5 
    0xF0, 0x03, 0xFF, 0xFF,   // line 6 
    0xF0, 0x03, 0xFF, 0xFF,   // line 7 
    0xE0, 0x07, 0xFF, 0xFF,   // line 8 
 


 //  0x00, 0x00, 0x00, 0x0F,   // line 5 
 //   0x00, 0x00, 0x00, 0x0F,   // line 6 
 //   0x00, 0x00, 0x00, 0x0F,   // line 7 
 //   0x00, 0x00, 0x00, 0x00,   // line 8 
 


    0xC0, 0x07, 0xFF, 0xFF,   // line 9 
    0xC0, 0x0F, 0xFF, 0xFF,   // line 10 
    0x80, 0x0F, 0xFF, 0xFF,   // line 11 
    0x80, 0x0F, 0xFF, 0xFF,   // line 12 
 
    0x80, 0x07, 0xFF, 0xFF,   // line 13 
    0x00, 0x07, 0xFF, 0xFF,   // line 14 
    0x00, 0x03, 0xFF, 0xFF,   // line 15 
    0x00, 0x00, 0xFF, 0xFF,   // line 16 
 
    0x00, 0x00, 0x7F, 0xFF,   // line 17 
    0x00, 0x00, 0x1F, 0xFF,   // line 18 
    0x00, 0x00, 0x0F, 0xFF,   // line 19 
    0x80, 0x00, 0x0F, 0xFF,   // line 20 
 
    0x80, 0x00, 0x07, 0xFF,   // line 21 
    0x80, 0x00, 0x07, 0xFF,   // line 22 
    0xC0, 0x00, 0x07, 0xFF,   // line 23 
    0xC0, 0x00, 0x0F, 0xFF,   // line 24 
 
    0xE0, 0x00, 0x0F, 0xFF,   // line 25 
    0xF0, 0x00, 0x1F, 0xFF,   // line 26 
    0xF0, 0x00, 0x1F, 0xFF,   // line 27 
    0xF8, 0x00, 0x3F, 0xFF,   // line 28 
 
    0xFE, 0x00, 0x7F, 0xFF,   // line 29 
    0xFF, 0x00, 0xFF, 0xFF,   // line 30 
    0xFF, 0xC3, 0xFF, 0xFF,   // line 31 
    0xFF, 0xFF, 0xFF, 0xFF    // line 32 
};
 
// Yin-shaped cursor XOR mask 
 
BYTE XORmaskCursor[] = 
{ 
    0x00, 0x00, 0x00, 0x00,   // line 1 
    0x00, 0x03, 0xC0, 0x00,   // line 2 
    0x00, 0x3F, 0x00, 0x00,   // line 3 
    0x00, 0xFE, 0x00, 0x00,   // line 4 
 
    0x0E, 0xFC, 0x00, 0x00,   // line 5 
    0x07, 0xF8, 0x00, 0x00,   // line 6 
    0x07, 0xF8, 0x00, 0x00,   // line 7 
    0x0F, 0xF0, 0x00, 0x00,   // line 8 
 
    0x1F, 0xF0, 0x00, 0x00,   // line 9 
    0x1F, 0xE0, 0x00, 0x00,   // line 10 
    0x3F, 0xE0, 0x00, 0x00,   // line 11 
    0x3F, 0xE0, 0x00, 0x00,   // line 12 
 
    0x3F, 0xF0, 0x00, 0x00,   // line 13 
    0x7F, 0xF0, 0x00, 0x00,   // line 14 
    0x7F, 0xF8, 0x00, 0x00,   // line 15 
    0x7F, 0xFC, 0x00, 0x00,   // line 16 
 
    0x7F, 0xFF, 0x00, 0x00,   // line 17 
    0x7F, 0xFF, 0x80, 0x00,   // line 18 
    0x7F, 0xFF, 0xE0, 0x00,   // line 19 
    0x3F, 0xFF, 0xE0, 0x00,   // line 20 
 
    0x3F, 0xC7, 0xF0, 0x00,   // line 21 
    0x3F, 0x83, 0xF0, 0x00,   // line 22 
    0x1F, 0x83, 0xF0, 0x00,   // line 23 
    0x1F, 0x83, 0xE0, 0x00,   // line 24 
 
    0x0F, 0xC7, 0xE0, 0x00,   // line 25 
    0x07, 0xFF, 0xC0, 0x00,   // line 26 
    0x07, 0xFF, 0xC0, 0x00,   // line 27 
    0x01, 0xFF, 0x80, 0x00,   // line 28 
 
    0x00, 0xFF, 0x00, 0x00,   // line 29 
    0x00, 0x3C, 0x00, 0x00,   // line 30 
    0x00, 0x00, 0x00, 0x00,   // line 31 
    0x00, 0x00, 0x00, 0x00    // line 32 
};
 


LRESULT CALLBACK ClickselectDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{

    if (mo==NULL) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SetDlgItemText(hDlg, IDC_NEXTCLICK,"");
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_SETDOUBLE: 
				mo->setdouble=TRUE;
				SetDlgItemText(hDlg, IDC_NEXTCLICK,"   ------------------");

				break;
			case IDC_SETRIGHT: 
				mo->setright=TRUE;
				SetDlgItemText(hDlg, IDC_NEXTCLICK,"                          ------------------");

				break;
			case IDC_SETDRAG: 
				mo->setdrag=TRUE;
				SetDlgItemText(hDlg, IDC_NEXTCLICK,"                                                 -----------------");
				break;

			}
			return TRUE;
//		case WM_SIZE:
//		case WM_MOVE:  update_toolbox_position(hDlg);
//		break;
		return TRUE;  
	}
    return FALSE;
}




LRESULT CALLBACK MouseDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{

	MOUSEOBJ * st;
	
	st = (MOUSEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MOUSE)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;		
				lpsi.nMin=1; lpsi.nMax=1000;
				
				SetScrollInfo(GetDlgItem(hDlg,IDC_DWELLTIMEBAR),SB_CTL,&lpsi,TRUE);
				
				lpsi.nMin=1; lpsi.nMax=100;
				SetScrollInfo(GetDlgItem(hDlg,IDC_DWELLRADIUSBAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_RESETRADIUSBAR),SB_CTL,&lpsi,TRUE);
				
				SetScrollPos(GetDlgItem(hDlg,IDC_DWELLTIMEBAR), SB_CTL,st->dwelltime,TRUE);
				SetDlgItemInt(hDlg, IDC_DWELLTIME,st->dwelltime,1);
				
				SetScrollPos(GetDlgItem(hDlg,IDC_DWELLRADIUSBAR), SB_CTL,st->dwellradius,TRUE);
				SetDlgItemInt(hDlg, IDC_DWELLRADIUS,st->dwellradius,1);

				SetScrollPos(GetDlgItem(hDlg,IDC_RESETRADIUSBAR), SB_CTL,st->resetradius,TRUE);
				SetDlgItemInt(hDlg, IDC_RESETRADIUS,st->resetradius,1);


				CheckDlgButton(hDlg, IDC_XINC, st->xinc);
				CheckDlgButton(hDlg, IDC_YINC, st->yinc);
				CheckDlgButton(hDlg, IDC_BYPASS, st->bypass);
				CheckDlgButton(hDlg, IDC_BYPASS_POS, st->bypass_pos);
				CheckDlgButton(hDlg, IDC_SELACTIVE, st->clickselect);
				CheckDlgButton(hDlg, IDC_ENABLE_DWELLING, st->enable_dwelling);
				CheckDlgButton(hDlg, IDC_AUTODETECT, st->autodetect);

				SetDlgItemInt(hDlg, IDC_XMAX,st->xmax,0);
				SetDlgItemInt(hDlg, IDC_YMAX,st->ymax,0);

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;

//	SetSystemCursor(LoadCursorFromFile("C:\\windows\\cursors\\appstar2.ani"), 0);
//	SetCursor(LoadCursorFromFile("C:\\windows\\cursors\\3dsmove.cur"));

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_XINC: 
				st->xinc=IsDlgButtonChecked(hDlg, IDC_XINC);
				break;
			case IDC_YINC: 
				st->yinc=IsDlgButtonChecked(hDlg, IDC_YINC);
				break;
			case IDC_XMAX: 
				st->xmax=GetDlgItemInt(hDlg, IDC_XMAX, NULL, 0);
				break;
			case IDC_YMAX: 
				st->ymax=GetDlgItemInt(hDlg, IDC_YMAX, NULL, 0);
				break;
			case IDC_BYPASS: 
				st->bypass=IsDlgButtonChecked(hDlg, IDC_BYPASS);				
				break;
			case IDC_BYPASS_POS: 
				st->bypass_pos=IsDlgButtonChecked(hDlg, IDC_BYPASS_POS);
				break;

			case IDC_ENABLE_DWELLING: 
				st->enable_dwelling=IsDlgButtonChecked(hDlg, IDC_ENABLE_DWELLING);
				break;
			case IDC_AUTODETECT: 
				st->autodetect=IsDlgButtonChecked(hDlg, IDC_AUTODETECT);
				if (st->autodetect)
				{
					st->xmax=GetSystemMetrics(SM_CXSCREEN);
					SetDlgItemInt(hDlg, IDC_XMAX, st->xmax, 0);
					st->ymax=GetSystemMetrics(SM_CYSCREEN);
					SetDlgItemInt(hDlg, IDC_YMAX, st->ymax, 0);
				}
				break;
			case IDC_SELACTIVE: 
				st->clickselect=IsDlgButtonChecked(hDlg, IDC_SELACTIVE);
				if (st->clickselect)
				{
					st->hWndClick=CreateDialog(hInst, (LPCTSTR)IDD_CLICKSELECTOR, NULL, (DLGPROC)ClickselectDlgHandler);
					SetWindowPos(st->hWndClick, HWND_TOP, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
					ShowWindow(st->hWndClick,TRUE);
					
				} else if (st->hWndClick) SendMessage(st->hWndClick,WM_CLOSE,0,0); 
				break;

			}
			return TRUE;
		case WM_HSCROLL:
		{
			int nNewPos; 
			nNewPos=get_scrollpos(wParam, lParam);
		    {
			  if (lParam == (long) GetDlgItem(hDlg,IDC_DWELLTIMEBAR))  
			  {   
				  
				  SetDlgItemInt(hDlg, IDC_DWELLTIME,nNewPos,1);
			      st->dwelltime=nNewPos;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_DWELLRADIUSBAR))  
			  {   
				  
				  SetDlgItemInt(hDlg, IDC_DWELLRADIUS,nNewPos,1);
			      st->dwellradius=nNewPos;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_RESETRADIUSBAR))  
			  {   
				  
				  SetDlgItemInt(hDlg, IDC_RESETRADIUS,nNewPos,1);
			      st->resetradius=nNewPos;
			  }
		  
			}
		
		} break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;  
	}
    return FALSE;
}



MOUSEOBJ::MOUSEOBJ(int num) : BASE_CL()
{
	inports = 8;
	strcpy(in_ports[0].in_name,"xPos");
	strcpy(in_ports[1].in_name,"yPos");
	strcpy(in_ports[2].in_name,"left-Clk");  
	strcpy(in_ports[3].in_name,"right-Clk");
	strcpy(in_ports[4].in_name,"drag click");
	strcpy(in_ports[5].in_name,"double click");
	strcpy(in_ports[6].in_name,"dwell enable");
	strcpy(in_ports[7].in_name,"mouse enable");

	outports = 2;
	strcpy(out_ports[0].out_name,"click done");
	strcpy(out_ports[1].out_name,"dwell time");

	width=125;

	counter=0; updatepos=4;

	dwelltime=1; dwellradius=7; resetradius=10; dwellcount=0; disable_dwell=FALSE;
	dwxmax=0;dwxmin=10000;dwymax=0;dwymin=10000; minmaxreset=0;

	xmax=1024; ymax=768; bypass=0; bypass_pos=0; autodetect=0; enable_dwelling=0;

	xpos=(float)(xmax/2); ypos=(float)(ymax/2); 
	lbutton=INVALID_VALUE; rbutton=INVALID_VALUE;dragbutton=INVALID_VALUE;doublebutton=INVALID_VALUE;
	r_clicked=0;l_clicked=0;drag_active=0;double_clicked=0;
	xinc=1;yinc=1;time_to_release_lbutton=0;
	setdouble=FALSE;setright=FALSE;setdrag=FALSE;
	clickselect=FALSE;
	hWndClick=NULL;mo=this;

    hCurs1=LoadCursor(NULL,IDC_ARROW);
	strcpy(cursorfile,GLOBAL.resourcepath);
	strcat(cursorfile,"\\CURSORS\\dwell_arrow1.cur");
	cursorindex=strstr(cursorfile,"1.cur");

}
	
void MOUSEOBJ::make_dialog(void) 
{
  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MOUSEBOX, ghWndStatusbox, (DLGPROC)MouseDlgHandler));

}

void MOUSEOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("xmax",P_INT,&xmax);
    load_property("ymax",P_INT,&ymax);
    load_property("xinc",P_INT,&xinc);
    load_property("yinc",P_INT,&yinc);
	load_property("dwelltime",P_INT,&dwelltime);
	load_property("enable_dwelling",P_INT,&enable_dwelling);
    load_property("dwellradius",P_INT,&dwellradius);
	load_property("resetradius",P_INT,&resetradius);
    load_property("bypass",P_INT,&bypass);
    load_property("bypass_pos",P_INT,&bypass_pos);
    load_property("clickselect",P_INT,&clickselect);
	if (clickselect)
	{
		hWndClick=CreateDialog(hInst, (LPCTSTR)IDD_CLICKSELECTOR, NULL, (DLGPROC)ClickselectDlgHandler);
		SetWindowPos(hWndClick, HWND_TOP, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
		ShowWindow(hWndClick,TRUE);
	}

	load_property("autodetect",P_INT,&autodetect);
	if (autodetect)
	{  
		xmax=GetSystemMetrics(SM_CXSCREEN);
		ymax=GetSystemMetrics(SM_CYSCREEN);
	}

	strcpy(in_ports[2].in_name,"left-Clk");  
	strcpy(in_ports[3].in_name,"right-Clk");
	strcpy(in_ports[4].in_name,"drag click");
	strcpy(in_ports[5].in_name,"double click");
	strcpy(in_ports[6].in_name,"dwell enable");
	strcpy(in_ports[7].in_name,"mouse enable");
	strcpy(out_ports[0].out_name,"click done");
	strcpy(out_ports[1].out_name,"dwell time");

	width=125;


}

void MOUSEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"xmax",P_INT,&xmax);
    save_property(hFile,"ymax",P_INT,&ymax);
    save_property(hFile,"xinc",P_INT,&xinc);
    save_property(hFile,"yinc",P_INT,&yinc);
	save_property(hFile,"dwelltime",P_INT,&dwelltime);
	save_property(hFile,"enable_dwelling",P_INT,&enable_dwelling);
    save_property(hFile,"dwellradius",P_INT,&dwellradius);
	save_property(hFile,"resetradius",P_INT,&resetradius);
    save_property(hFile,"bypass",P_INT,&bypass);
    save_property(hFile,"bypass_pos",P_INT,&bypass_pos);
    save_property(hFile,"clickselect",P_INT,&clickselect);
	save_property(hFile,"autodetect",P_INT,&autodetect);
    
}
	
void MOUSEOBJ::incoming_data(int port, float value)
{
	switch (port) 
	{
		case 0: if (!xinc) xpos = value; 
			else { if (fabs(value)>0.001) xpos+= value; }//if (fabs(value)>0.8) xpos+= value; }
			  //  if (xpos<0) xpos=0; if(xpos*3.76f>(float)xmax) xpos=(float)xmax/3.76f;
			  if (xpos<0) xpos=0; if(xpos>(float)xmax) xpos=(float)xmax;
			  break;
		case 1: if (!yinc) ypos = value; 
			else { if (fabs(value)>0.001) ypos+= value;} // if (fabs(value)>0.8) ypos+= value; }
				//if (ypos<0) ypos=0; if(ypos*1.56f>(float)ymax) ypos=(float) ymax/1.56f; 
			   if (ypos<0) ypos=0; if(ypos>(float)ymax) ypos=(float) ymax;
			  break;

		case 2: lbutton = (int)value; 
			  break;

		case 3: if (value==1.0f) { setright=TRUE; setdouble=FALSE; setdrag=FALSE; } 
			    else { rbutton = (int)value; setright=FALSE;} //setright=FALSE; setdouble=FALSE; setdrag=FALSE; } 
				break;

		case 4: if (value==1.0f) { setdrag=TRUE; setdouble=FALSE; setright=FALSE; }
			else { dragbutton = (int)value; setdrag=FALSE;} // setright=FALSE; setdouble=FALSE; setdrag=FALSE; } 
				break;

		case 5: if (value==1.0f) { setdouble=TRUE; setdrag=FALSE; setright=FALSE; }
			else { doublebutton = (int)value;setdouble=FALSE;} // setright=FALSE; setdouble=FALSE; setdrag=FALSE; } 
				break;
		case 6: if (value==INVALID_VALUE) enable_dwelling=0; else enable_dwelling=1;
				break;

		case 7: if (value==INVALID_VALUE) bypass=1; else bypass=0;
				break;

	}
}

void mouse_input(DWORD flags, LONG x, LONG y)
{

	INPUT    Input={0};

	Input.type      = INPUT_MOUSE;
	Input.mi.dwFlags  = flags;

	Input.mi.dx=x;
	Input.mi.dy=y;
	::SendInput(1,&Input,sizeof(Input));
}

	
void MOUSEOBJ::work(void)
{
    POINT p;
	int reset=0;

	if (GLOBAL.fly) return;

	if (time_to_release_lbutton) 	{
		time_to_release_lbutton--;
		if (!time_to_release_lbutton){
			mouse_input(MOUSEEVENTF_LEFTUP,0,0);
			lbutton=INVALID_VALUE;
		}
	}

	counter++; if (counter>updatepos) counter=0;
	if ((!counter) && (!bypass))  	
	{
		 GetCursorPos(&p);

		if (xpos!= INVALID_VALUE) p.x = (int)xpos;
		if (ypos!= INVALID_VALUE) p.y = (int)ypos;

		if (!bypass_pos)
		  mouse_input(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|0x4000,(LONG)xpos * 70,(LONG)ypos * 85);
						
		if (enable_dwelling) 
		{
			if (p.x < dwxmin) dwxmin=p.x;
		    if (p.x > dwxmax) dwxmax=p.x;
		    if (p.y < dwymin) dwymin=p.y;
		    if (p.y > dwymax) dwymax=p.y;
		    
			if ( (dwxmax-dwxmin<dwellradius) && (dwymax-dwymin<dwellradius) && !disable_dwell)
			  dwellcount++; else dwellcount=0;

			pass_values(1,(float)dwellcount); 
	        
			if ( (dwxmax-dwxmin>resetradius) || (dwymax-dwymin>resetradius)) disable_dwell=FALSE;
		
			if (minmaxreset++ >= dwelltime)
			{  dwxmax=0;dwxmin=10000;dwymax=0;dwymin=10000;minmaxreset=0; }

			int cpos=(int)((float)dwellcount*6.0f/(float)dwelltime);
			if (cpos>5) cpos=5;
			*cursorindex='1'+cpos;
//			SetSystemCursor(LoadCursor(hInst,( LPCTSTR) MAKEINTRESOURCE(IDC_CURSOR1+cpos)), OCR_NORMAL);

		    if (dwellcount>=dwelltime)
			{
			      dwellcount=0;
			  	  lbutton=TRUE;
				  disable_dwell=TRUE;
				  dwxmax=0;dwxmin=10000;dwymax=0;dwymin=10000;			
			}
		}

		if ((lbutton != INVALID_VALUE) && (!l_clicked))	 
		{ 
			if (enable_dwelling) lbutton=INVALID_VALUE;
			l_clicked=1;
			updatepos=4;

			if (setdouble)
			{ doublebutton=TRUE; setdouble=FALSE; reset=1;}
			else if (setright)
			{ rbutton=TRUE; setright=FALSE; reset=1;}
			else if (setdrag)
			{ dragbutton=TRUE; setdrag=FALSE;reset=1; }
			else
			{
				if (drag_active) {
					drag_active=0;
					mouse_input(MOUSEEVENTF_LEFTUP,0,0);
				}
				else {
					mouse_input(MOUSEEVENTF_LEFTDOWN,0,0);
					mouse_input(MOUSEEVENTF_LEFTUP,0,0);
				}
			}
		}
		else if ((lbutton == INVALID_VALUE) && (l_clicked))	l_clicked=0;

		if ((rbutton != INVALID_VALUE) && (!r_clicked))	{ 
			if (enable_dwelling) rbutton=INVALID_VALUE; 
			r_clicked=1;
			mouse_input(MOUSEEVENTF_RIGHTDOWN,0,0);
			mouse_input(MOUSEEVENTF_RIGHTUP,0,0);
		}
		else if ((rbutton == INVALID_VALUE) && (r_clicked))	r_clicked=0;

		if ((doublebutton != INVALID_VALUE) && (!double_clicked)) { 
			if (enable_dwelling) doublebutton=INVALID_VALUE; 
			double_clicked=1;
			mouse_input(MOUSEEVENTF_LEFTDOWN,0,0);
			mouse_input(MOUSEEVENTF_LEFTUP,0,0);
			mouse_input(MOUSEEVENTF_LEFTDOWN,0,0);
			mouse_input(MOUSEEVENTF_LEFTUP,0,0);
		}
		else if ((doublebutton == INVALID_VALUE) && (double_clicked)) double_clicked=0;

		if ((dragbutton != INVALID_VALUE) && (!drag_active))
		{ 
			drag_active=1; 
			updatepos=6;
			mouse_input(MOUSEEVENTF_LEFTDOWN,0,0);
			if (clickselect) dragbutton=INVALID_VALUE;
		}


	} 

	if (!TIMING.dialog_update)
	{
		if (hDlg=ghWndToolbox)
		{
			SetDlgItemInt(hDlg,IDC_ACT_X,(int)(xpos),FALSE); //*3.76f
			SetDlgItemInt(hDlg,IDC_ACT_Y,(int)(ypos),FALSE); // *1.56f
		}

		if (hWndClick)
		{
			if(setdouble)
				SetDlgItemText(hWndClick, IDC_NEXTCLICK,"   ------------------");
			else if (setright)
					SetDlgItemText(hWndClick, IDC_NEXTCLICK,"                          ------------------");
			else if (setdrag)
					SetDlgItemText(hWndClick, IDC_NEXTCLICK,"                                                 -----------------");
			else SetDlgItemText(hWndClick, IDC_NEXTCLICK," ");
		}
	}

	if (reset) pass_values(0,INVALID_VALUE);
}
	
MOUSEOBJ::~MOUSEOBJ() 
{
	if (hWndClick) SendMessage(hWndClick,WM_CLOSE,0,0);
//	DestroyCursor(hCurs3);
//    SetSystemCursor(hCurs1,OCR_NORMAL);
}


/*
	// Alternative or deprecated ways to control the cursor:

	hCurs=CopyCursor(GetCursor());
	SetSystemCursor(hCurs,OCR_NORMAL);
	OCR_APPSTARTING, OCR_HAND, OCR_SIZEALL
	OCR_CROSS, OCR_HELP, OCR_IBEAM, OCR_NO,	OCR_UP,	OCR_WAIT

	// Create a custom cursor at run time: 
 	hCurs = CreateCursor( hinst,   // app. instance 
				 19,                // horizontal position of hot spot 
				 2,                 // vertical position of hot spot 
				 32,                // cursor width 
				 32,                // cursor height 
				 ANDmaskCursor,     // AND mask 
				 XORmaskCursor );   // XOR mask 
	
	// Change the cursor for window class represented by hwnd. 
    // SetClassLong(ghWndMain,    // window handle 
    // GCL_HCURSOR,      // change cursor 
    // (LONG) hCurs3);   // new cursor 

	//  Animated Cursor: 

		GetWindowsDirectory(MyDir,255);
		tmpstr=strcat(MyDir,"\\Cursors\\dinosaur.ani");
		hCurs=LoadCursorFromFile(tmpstr);


 	// deprecated cursor control API calls:
     SetCursorPos(p.x,p.y);
	 mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
	 mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
	
       [DllImport("User32.dll", SetLastError=true)]
       public static extern int SendInput(int nInputs, ref INPUT pInputs, int cbSize);

       public struct INPUT 
       { 
              public int type; 
              public MOUSEINPUT mi; 
       }

        public struct MOUSEINPUT 
       {
              public int dx;
              public int dy;
              public int mouseData;
              public int dwFlags;
              public int time;
              public int dwExtraInfo;
       }

       // Call the API
       int resSendInput;
       resSendInput = SendInput(1, ref input, Marshal.SizeOf(input));
       if (resSendInput == 0 || Marshal.GetLastWin32Error() != 0)
              System.Diagnostics.Debug.WriteLine(Marshal.GetLastWin32Error());

*/
