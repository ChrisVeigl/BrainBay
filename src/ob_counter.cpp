/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COUNTER.CPP:  contains functions for the COUTNER-Object
  Author: Chris Veigl

  This Object outputs a counter value at it's port and displays the value in a
  seperate Window. It can count transitions of the input to or from INVALID_VALUE
  or use the input directly (without counting)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_counter.h"
#include <wingdi.h> 
 
void draw_counter(COUNTEROBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[30];
	RECT rect;
//	int  act,width,height,bottom,y1,y2;
    HBRUSH actbrush;	
	
	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);

	SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc,st->fontcolor);

    SelectObject(hdc, st->font);

	actbrush=CreateSolidBrush(st->bkcolor);
	SelectObject (hdc, actbrush);		
	FillRect(hdc, &rect, actbrush);

	if (st->countervalue == INVALID_VALUE) sprintf(szdata, "INV");
	else if (st->timeformat) {
		int hours,minutes,seconds;
		seconds=((int)st->countervalue);
		hours=(int)(seconds/3600);
		minutes=(int)(seconds/60) % 60;
		seconds=seconds%60;
		if (hours)
			wsprintf(szdata,"%d:%d:%d",hours,minutes,seconds);
		else
			wsprintf(szdata,"%d:%d",minutes,seconds);

	}
	else if (st->integer) {
		if (st->countervalue>0)
			wsprintf(szdata, "%d",(int)(st->countervalue+0.5f));
		else
			wsprintf(szdata, "%d",(int)(st->countervalue-0.5f));
	}
	else {
		switch(st->digits) {
			case 0: sprintf(szdata, "%.0f",st->countervalue); break;
			case 1: sprintf(szdata, "%.1f",st->countervalue); break;
			case 2: sprintf(szdata, "%.2f",st->countervalue); break;
			case 3: sprintf(szdata, "%.3f",st->countervalue); break;
			case 4: sprintf(szdata, "%.4f",st->countervalue); break;
		}		
	}

	DrawText(hdc, szdata, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	DeleteObject(actbrush);
	EndPaint( st->displayWnd, &ps );
}










LRESULT CALLBACK CounterDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	COUNTEROBJ * st;
	
	st = (COUNTEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_COUNTER)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=4; lpsi.nMax=300;
				SetScrollInfo(GetDlgItem(hDlg,IDC_FONTSIZEBAR),SB_CTL,&lpsi, TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_FONTSIZEBAR), SB_CTL,st->fontsize, TRUE);
				SetDlgItemInt(hDlg, IDC_FONTSIZE, st->fontsize, FALSE);
				lpsi.nMin=0; lpsi.nMax=4;
				SetScrollInfo(GetDlgItem(hDlg,IDC_DIGITSBAR),SB_CTL,&lpsi, TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_DIGITSBAR), SB_CTL,st->digits, TRUE);
				SetDlgItemInt(hDlg, IDC_DIGITS, st->digits, FALSE);

				SetDlgItemText(hDlg, IDC_CAPTION, st->wndcaption);

				SetDlgItemInt(hDlg, IDC_RESETVALUE, (int)st->resetvalue,TRUE);
				switch (st->mode) 
				{
					case 0: CheckDlgButton(hDlg, IDC_COUNTFT,TRUE); break;
					case 1: CheckDlgButton(hDlg, IDC_COUNTTF,TRUE); break;
					case 2: CheckDlgButton(hDlg, IDC_COUNTIV,TRUE); break;
					case 3: CheckDlgButton(hDlg, IDC_COUNTFREQ,TRUE); break;
				}
				CheckDlgButton(hDlg, IDC_SHOWCOUNTER,st->showcounter);
				CheckDlgButton(hDlg, IDC_INTEGER,st->integer);
				CheckDlgButton(hDlg, IDC_TIMEFORMAT,st->timeformat);
				
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_COUNTFT: st->mode=0; break;
			case IDC_COUNTTF: st->mode=1; break;
			case IDC_COUNTIV: st->mode=2; break;
			case IDC_COUNTFREQ: st->mode=3; break;

			case IDC_RESETCOUNTER:  st->countervalue=st->resetvalue; break;
			case IDC_RESETVALUE:  st->resetvalue=(float)GetDlgItemInt(hDlg, IDC_RESETVALUE,NULL, 1); break;
			
			case IDC_FONTCOLOR:
				st->fontcolor=select_color(hDlg,st->fontcolor);
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_BKCOLOR:
				st->bkcolor=select_color(hDlg,st->bkcolor);
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_CAPTION:
				GetDlgItemText(hDlg,IDC_CAPTION,st->wndcaption,50);
				SetWindowText(st->displayWnd,st->wndcaption);
				break;
			
			case IDC_INTEGER:
				  st->integer=IsDlgButtonChecked(hDlg,IDC_INTEGER);
				  InvalidateRect(st->displayWnd,NULL,TRUE);
				  break;

			case IDC_TIMEFORMAT:
				  st->timeformat=IsDlgButtonChecked(hDlg,IDC_TIMEFORMAT);
				  InvalidateRect(st->displayWnd,NULL,TRUE);
				  break;

			case IDC_SHOWCOUNTER:
				{  int i;
				   i=IsDlgButtonChecked(hDlg,IDC_SHOWCOUNTER);
				   if ((st->showcounter)&&(!i)&&(st->displayWnd))  { DestroyWindow(st->displayWnd); st->displayWnd=NULL; }
				   if ((!st->showcounter)&&(i)) 
				   {  
					   if(!(st->displayWnd=CreateWindow("Counter_Class", "Counter", WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME,st->left, st->top, st->right-st->left, st->bottom-st->top, ghWndMain, NULL, hInst, NULL)))
							report_error("can't create Counter Window");
					   else { SetForegroundWindow(st->displayWnd); ShowWindow( st->displayWnd, TRUE ); UpdateWindow( st->displayWnd ); }
				   }
				   st->showcounter=i;
				}
				break;

			}
			return TRUE;
			
		case WM_HSCROLL:
			if (lParam == (long) GetDlgItem(hDlg,IDC_FONTSIZEBAR))  
			{
				int nNewPos=get_scrollpos(wParam,lParam);
				SetDlgItemInt(hDlg, IDC_FONTSIZE, nNewPos, TRUE);
                st->fontsize=nNewPos;
  				if (st->font) DeleteObject(st->font);
				if (!(st->font = CreateFont(-MulDiv(st->fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
					report_error("Font creation failed!");

				InvalidateRect(st->displayWnd, NULL, TRUE);
			} 
			if (lParam == (long) GetDlgItem(hDlg,IDC_DIGITSBAR))  
			{
				int nNewPos=get_scrollpos(wParam,lParam);
				SetDlgItemInt(hDlg, IDC_DIGITS, nNewPos, TRUE);
                st->digits=nNewPos;
				InvalidateRect(st->displayWnd, NULL, TRUE);
			} 
			break;

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_FONTCOLOR),st->fontcolor);
			color_button(GetDlgItem(hDlg,IDC_BKCOLOR),st->bkcolor);
		break;
	}
    return FALSE;
}


LRESULT CALLBACK CounterWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	COUNTEROBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_COUNTER)
		{	st=(COUNTEROBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

	if (st==NULL) return DefWindowProc( hWnd, message, wParam, lParam );
	
	switch( message ) 
	{	case WM_DESTROY:
		 break;
		case WM_KEYDOWN:
			  SendMessage(ghWndMain, message,wParam,lParam);
			break;
		case WM_MOUSEACTIVATE:
		  close_toolbox();
		  actobject=st;
		  SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
		  InvalidateRect(ghWndDesign,NULL,TRUE);
			break;
		case WM_SIZE: 
		case WM_MOVE:
			{
  			  WINDOWPLACEMENT  wndpl;
			  GetWindowPlacement(st->displayWnd, &wndpl);

			  if (GLOBAL.locksession) {
				  wndpl.rcNormalPosition.top=st->top;
				  wndpl.rcNormalPosition.left=st->left;
				  wndpl.rcNormalPosition.right=st->right;
				  wndpl.rcNormalPosition.bottom=st->bottom;
				  SetWindowPlacement(st->displayWnd, &wndpl);
 				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE)&~WS_SIZEBOX);
			  }
			  else {
				  st->top=wndpl.rcNormalPosition.top;
				  st->left=wndpl.rcNormalPosition.left;
				  st->right=wndpl.rcNormalPosition.right;
				  st->bottom=wndpl.rcNormalPosition.bottom;
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE) | WS_SIZEBOX);
			  }
			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
		case WM_PAINT:
			draw_counter(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}






//
//  Object Implementation
//


COUNTEROBJ::COUNTEROBJ(int num) : BASE_CL()
	  {

		outports = 1;
		inports = 2;
		width=65;
		strcpy(in_ports[0].in_name,"in");
		strcpy(in_ports[1].in_name,"reset");
		strcpy(out_ports[0].out_name,"value");

		top=50;left=200; right=400; bottom=300;
		countervalue=0; fontsize=25;
		fontcolor=255; bkcolor=0;
		strcpy (wndcaption,"Counter");
		mode=0;
		digits=2;
		resetvalue=0;
		timeformat=0;
		scount=0;
		showcounter=TRUE;

	//	color=RGB(0,0,100);

		if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
			report_error("Font creation failed!");


		if(!(displayWnd=CreateWindow("Counter_Class", wndcaption, WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		    report_error("can't create Counter Window");

		else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }
		InvalidateRect(displayWnd, NULL, TRUE);
	  }
	  
	  void COUNTEROBJ::session_reset(void) 
	  {  
			countervalue=resetvalue;
	  }

  	  void COUNTEROBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_COUNTERBOX, ghWndStatusbox, (DLGPROC)CounterDlgHandler));
	  }
	  void COUNTEROBJ::load(HANDLE hFile) 
	  {
		  float temp;
		  load_object_basics(this);
		  load_property("mode",P_INT,&mode);
		  load_property("coutnervalue",P_FLOAT,&countervalue);
		  load_property("resetvalue",P_FLOAT,&resetvalue);
		  load_property("showcounter",P_INT,&showcounter);
		  load_property("fontsize",P_INT,&fontsize);
		  if (font) DeleteObject(font);
		  if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
				report_error("Font creation failed!");

		  load_property("fontcolor",P_FLOAT,&temp);
		  fontcolor=(COLORREF)temp;
		  load_property("bkcolor",P_FLOAT,&temp);
		  bkcolor=(COLORREF)temp;
		  if ((bkcolor==0)&&(fontcolor==0)) fontcolor=255;
		  load_property("top",P_INT,&top);
		  load_property("left",P_INT,&left);
		  load_property("right",P_INT,&right);
		  load_property("bottom",P_INT,&bottom);
		  load_property("integer",P_INT,&integer);
		  load_property("wndcaption",P_STRING,wndcaption);
		  load_property("digits",P_INT,&digits);
		  load_property("timeformat",P_INT,&timeformat);

		if (!showcounter)
		{
			if (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
		}
		else 
		{ 
			MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 
		    SetWindowText(displayWnd,wndcaption);

			if (GLOBAL.locksession) {
 				SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				//SetWindowLong(displayWnd, GWL_STYLE, 0);
			} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
			InvalidateRect (displayWnd, NULL, TRUE);
		}
	  }
		
	  void COUNTEROBJ::save(HANDLE hFile) 
	  {	  
		  float temp;
	 	  save_object_basics(hFile, this);
		  save_property(hFile,"mode",P_INT,&mode);
		  save_property(hFile,"coutnervalue",P_FLOAT,&countervalue);
		  save_property(hFile,"resetvalue",P_FLOAT,&resetvalue);
		  save_property(hFile,"showcounter",P_INT,&showcounter);

		  save_property(hFile,"fontsize",P_INT,&fontsize);
		  temp=(float)fontcolor;;
		  save_property(hFile,"fontcolor",P_FLOAT,&temp);
		  temp=(float)bkcolor;;
		  save_property(hFile,"bkcolor",P_FLOAT,&temp);
		  		  
		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);
		  save_property(hFile,"integer",P_INT,&integer);
		  save_property(hFile,"wndcaption",P_STRING,wndcaption);
		  save_property(hFile,"digits",P_INT,&digits);	  
		  save_property(hFile,"timeformat",P_INT,&timeformat);
	  }


  	  void COUNTEROBJ::update_inports(void)
      {
		  InvalidateRect(displayWnd,NULL,TRUE);
	  }

	  void COUNTEROBJ::incoming_data(int port, float value)
      {
		if (port==0) { oldinput=input; input=value; }
		// if (port==1) reset=value;
		if (port==1) { if (value!=INVALID_VALUE) countervalue=resetvalue; }

		  scount++;
		  switch (mode)
		  {
			case 0:  if ((oldinput==INVALID_VALUE) && (input!=INVALID_VALUE))
						countervalue++;
					break;
		    case 1:  if ((oldinput!=INVALID_VALUE) && (input==INVALID_VALUE))
						countervalue++;
					break;
			case 2:  countervalue=input;
					break;
			case 3:  if ((oldinput==INVALID_VALUE) && (input!=INVALID_VALUE))
					 {
						 //period=TIMING.acttime-prev_time;
						 //prev_time=TIMING.acttime;
						 //countervalue=(float)TIMING.pcfreq/(float)period;
						 countervalue = (float)PACKETSPERSECOND/(float)scount;
						 scount=0;
					 }
					 //if (period*2<TIMING.acttime-prev_time) countervalue=0;
					break;
		  }

        
      }
        

	  void COUNTEROBJ::work(void) 
	  {

		  pass_values(0,countervalue);
		
		  if ((displayWnd)&&(!TIMING.draw_update)) 
		  {
		    InvalidateRect(displayWnd,NULL,FALSE);
		  }
	  }

      

COUNTEROBJ::~COUNTEROBJ()
	  {
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
	  }  
