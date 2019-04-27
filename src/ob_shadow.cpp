/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SHADOW.CPP:  contains functions for the SHADOW-Object
  Author: Chris Veigl

  This Object allows to occlude a selectable area of the screen

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_shadow.h"
#include <wingdi.h> 

void draw_shadow(SHADOWOBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[30];
	RECT rect;
    HBRUSH actbrush;	

	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);
	actbrush=CreateSolidBrush(st->bkcolor);
	SelectObject (hdc, actbrush);		
	FillRect(hdc, &rect, actbrush);
	DeleteObject(actbrush);
	EndPaint( st->displayWnd, &ps );
}



	
LRESULT CALLBACK ShadowDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	SHADOWOBJ * st;
	int sty=0;
	
	st = (SHADOWOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_SHADOW)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_BKCOLOR:
				st->bkcolor=select_color(hDlg,st->bkcolor);
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_EDIT_WINDOW:
				SetWindowLong(st->displayWnd, GWL_STYLE, WS_VISIBLE | WS_THICKFRAME);
				sty= GetWindowLong(st->displayWnd, GWL_EXSTYLE);
				sty &= ~(WS_EX_LAYERED | WS_EX_TRANSPARENT);
				SetWindowLong(st->displayWnd, GWL_EXSTYLE,sty);
				st->locked=0;
				break;
			case IDC_LOCK_WINDOW:
				SetWindowLong(st->displayWnd, GWL_STYLE, WS_VISIBLE);
				SetWindowLong(st->displayWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT ); 
				SetLayeredWindowAttributes(st->displayWnd, 0, 127, LWA_ALPHA);
				st->locked=1;
				break;
			}
			return TRUE;
			

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_BKCOLOR),st->bkcolor);
		break;
	}
    return FALSE;
}


LRESULT CALLBACK ShadowWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	SHADOWOBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_SHADOW)
		{	st=(SHADOWOBJ *)objects[t];
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
				st->top=wndpl.rcNormalPosition.top;
				st->left=wndpl.rcNormalPosition.left;
				st->right=wndpl.rcNormalPosition.right;
				st->bottom=wndpl.rcNormalPosition.bottom;
			    InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
		case WM_PAINT:
			draw_shadow(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}



//
//  Object Implementation
//


SHADOWOBJ::SHADOWOBJ(int num) : BASE_CL()
	  {

		outports = 0;
		inports = 1;
		width=65;
		strcpy(in_ports[0].in_name,"in");

		top=50;left=200; right=400; bottom=300;
		bkcolor=0; locked=0;
        
		if(!(displayWnd=CreateWindow("Shadow_Class", "shadow-window", WS_VISIBLE | WS_THICKFRAME,
			left, top, right-left, bottom-top, NULL, NULL, hInst, NULL)))
		    report_error("can't create Shadow Window");

		else {
			//SetLayeredWindowAttributes(displayWnd, RGB(255,255,255), 0, LWA_COLORKEY);
			SetLayeredWindowAttributes(displayWnd, 0, 127, LWA_ALPHA);
			SetWindowPos(displayWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow( displayWnd, TRUE );  
			UpdateWindow( displayWnd ); 
			InvalidateRect(displayWnd, NULL, TRUE);
		}
	  }

  	  void SHADOWOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SHADOWBOX, ghWndStatusbox, (DLGPROC)ShadowDlgHandler));
	  }
	  void SHADOWOBJ::load(HANDLE hFile) 
	  {
		  float temp;
		  load_object_basics(this);
		  load_property("bkcolor",P_FLOAT,&temp);
		  bkcolor=(COLORREF)temp;
		  load_property("top",P_INT,&top);
		  load_property("left",P_INT,&left);
		  load_property("right",P_INT,&right);
		  load_property("bottom",P_INT,&bottom);
		  load_property("locked",P_INT,&locked);

		  SetWindowPos(displayWnd, HWND_TOPMOST, left,top,right-left,bottom-top, 0);

		  if (locked) {
				SetWindowLong(displayWnd, GWL_STYLE, WS_VISIBLE);
				SetWindowLong(displayWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT ); 
				SetLayeredWindowAttributes(displayWnd, 0, 127, LWA_ALPHA);
		  }
		  else {
				SetWindowLong(displayWnd, GWL_STYLE, WS_VISIBLE | WS_THICKFRAME);
				int sty= GetWindowLong(displayWnd, GWL_EXSTYLE);
				sty &= ~(WS_EX_LAYERED | WS_EX_TRANSPARENT);
				SetWindowLong(displayWnd, GWL_EXSTYLE,sty);
		  }

			//MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 
			//SetWindowText(displayWnd,wndcaption);
			/*
			if (GLOBAL.locksession) {
 				SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				//SetWindowLong(displayWnd, GWL_STYLE, 0);
			} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
			*/
			InvalidateRect (displayWnd, NULL, TRUE);
	  }
		
	  void SHADOWOBJ::save(HANDLE hFile) 
	  {	  
		  float temp;
	 	  save_object_basics(hFile, this);
  		  temp=(float)bkcolor;
		  save_property(hFile,"bkcolor",P_FLOAT,&temp);  
		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);
		  save_property(hFile,"locked",P_INT,&locked);
	  }


	  void SHADOWOBJ::incoming_data(int port, float value)
      {
		if (value != INVALID_VALUE) {
		   input=255.0f-size_value(in_ports[0].in_min,in_ports[0].in_max,value,0.0f,255.0f,1);
		}
      }
        

	  void SHADOWOBJ::work(void) 
	  {		
		  if ((displayWnd)&&(!TIMING.draw_update)) 
		  {
	     	SetLayeredWindowAttributes(displayWnd, 0, input, LWA_ALPHA);
			InvalidateRect(displayWnd,NULL,FALSE);
		  }
	  }
      

SHADOWOBJ::~SHADOWOBJ()
{
	if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
}  
