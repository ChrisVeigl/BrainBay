/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_BUTTON.CPP:  contains functions for the Button-Object

  The Button-Object has its own window, it uses GDI-drawings 

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_button.h"
#include <wingdi.h> 



void draw_button(BUTTONOBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[256];
	RECT rect;
	HBRUSH bkbrush;
	HBITMAP hBitmap = NULL;
	BITMAP          bitmap;
	HDC             hdcMem;
	HGDIOBJ         oldBitmap;

	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);

    bkbrush=CreateSolidBrush(st->bkcolor);
	if (st->redraw) FillRect(hdc, &rect,bkbrush);
    
   	SelectObject (hdc, bkbrush);

	if (strlen(st->buttonpath)>0) {
		char bitmappath[256];
		strcpy(bitmappath,GLOBAL.resourcepath);
	    strcat(bitmappath,"\\GRAPHICS\\");
		strcat(bitmappath,st->buttonpath);

		hBitmap = (HBITMAP)LoadImage(hInst, bitmappath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (hBitmap) {
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);
			GetObject(hBitmap, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

	   	//	TransparentBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
       //                   hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, st->transcolor); // RGB(0,255,0));

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
		}
	}

	DeleteObject(bkbrush);
	st->redraw=0;
	EndPaint( st->displayWnd, &ps );
}

void update_border (HWND displayWnd, int showborder)
{
	if (GLOBAL.locksession) {
		 if (showborder)
		    SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)& ~(WS_SIZEBOX) | (WS_CAPTION));
		 else 
 		    SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)& ~(WS_SIZEBOX+WS_CAPTION+WS_THICKFRAME));
	} else {
		 if (showborder)
			  SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | (WS_SIZEBOX+WS_CAPTION));
		 else 
 		    SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)& ~(WS_CAPTION+WS_THICKFRAME) | WS_SIZEBOX);
	}
}

LRESULT CALLBACK ButtonDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	char temp[100];
	BUTTONOBJ * st;
	int x;
	
	st = (BUTTONOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_BUTTON)) return(FALSE);
					
	switch( message )
	{
		case WM_INITDIALOG:
			{
				SetDlgItemInt(hDlg, IDC_VALUE1, st->value1,TRUE);
				SetDlgItemInt(hDlg, IDC_VALUE2, st->value2,TRUE);

				SetDlgItemText(hDlg, IDC_BUTTONPATH, st->buttonpath);
				SetDlgItemText(hDlg, IDC_BUTTONCAPTION, st->buttoncaption);

				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Play Session");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Stop Session");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "End Session");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Output Value1 if pressed, else Value2");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Output Value1 if pressed, else INVALID_VALUE");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Toggle Value1 and Value2");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Send Value2 for 1 second");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Display Device Settings");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Display Application Settings");
				SendDlgItemMessage( hDlg, IDC_FUNCTIONCOMBO, CB_SETCURSEL, (WPARAM) (st->buttonfunction), 0L ) ;
				CheckDlgButton(hDlg, IDC_DISPLAYBORDER, st->displayborder);

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_TRANSCOL:
				st->transcolor=select_color(hDlg,st->transcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_BKCOL:
				st->bkcolor=select_color(hDlg,st->bkcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_BUTTONPATH:
				GetDlgItemText(hDlg,IDC_BUTTONPATH,st->buttonpath,256); 
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;

			case IDC_BUTTONCAPTION:
				GetDlgItemText(hDlg, IDC_BUTTONCAPTION, st->buttoncaption, 80);
				SetWindowText(st->displayWnd,st->buttoncaption);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;

			case IDC_VALUE1:
				st->value1=GetDlgItemInt(hDlg,IDC_VALUE1,NULL,TRUE); 
				break;
			case IDC_VALUE2:
				st->value2=GetDlgItemInt(hDlg,IDC_VALUE2,NULL,TRUE); 
				break;
			case IDC_FUNCTIONCOMBO:
				if (HIWORD(wParam)==CBN_SELCHANGE)
				{
					st->buttonfunction=SendDlgItemMessage(hDlg, IDC_FUNCTIONCOMBO, CB_GETCURSEL, 0, 0 );
				}
				break;
			case IDC_DISPLAYBORDER:
				 st->displayborder=IsDlgButtonChecked(hDlg,IDC_DISPLAYBORDER);
				 update_border(st->displayWnd,st->displayborder);
  				 st->redraw=1;
  				 InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			}
			return TRUE;

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_TRANSCOL),st->transcolor);
			color_button(GetDlgItem(hDlg,IDC_BKCOL),st->bkcolor);
			break;
	}
    return FALSE;
}


LRESULT CALLBACK ButtonWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	BUTTONOBJ * st;
	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_BUTTON)
		{	st=(BUTTONOBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

	if (st==NULL) return DefWindowProc( hWnd, message, wParam, lParam );
	
	switch( message ) 
	{	case WM_DESTROY:
		 break;
		case WM_MOUSEACTIVATE:
   	      st->redraw=1;
		  close_toolbox();
		  actobject=st;
		  SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
		  InvalidateRect(ghWndDesign,NULL,TRUE);
		break;

		case WM_LBUTTONDOWN:
			{  int actx,acty,minpoint,i;
			   float actdist,mindist;
			   actx=(int)LOWORD(lParam);
			   acty=(int)HIWORD(lParam);
			   printf("button pressed in wnd %ld at: %ld, %ld\n",(long) hWnd, (long) actx, (long)acty);
			   // if (distance (actx, acty, NAVI_X+170, NAVI_Y+33) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_BACKSPACE,0); 
			   switch (st->buttonfunction) {
				   case BUTTONFUNCTION_PLAYSESSION:
						SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RUNSESSION,0);
					break;
				   case BUTTONFUNCTION_STOPSESSION:
						SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
					break;
				   case BUTTONFUNCTION_ENDSESSION:
						SendMessage(ghWndStatusbox,WM_COMMAND,IDC_ENDSESSION,0);
					break;
				   case BUTTONFUNCTION_VAL1VAL2:
				   case BUTTONFUNCTION_VAL1INV:
						st->state=STATE_PRESSED;
					break;
				   case BUTTONFUNCTION_TOGGLEVAL:
						st->togglecount=!(st->togglecount);
					break;
				   case BUTTONFUNCTION_TOGGLE1SEC:
						st->state=STATE_PRESSED;
						st->togglecount=1;
					break;
				   case BUTTONFUNCTION_DEVSETTINGS:
						SendMessage(ghWndMain,WM_COMMAND,IDM_DEVICESETTINGS,0);
					break;
				   case BUTTONFUNCTION_APPSETTINGS:
						SendMessage(ghWndMain,WM_COMMAND,IDM_SETTINGS,0);
					break;
			   }
			}
			break;

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			//mmi->ptMaxSize.x = 10;
			//mmi->ptMaxSize.y = 10;
			mmi->ptMinTrackSize.x = 5;
			mmi->ptMinTrackSize.y = 5;
			//mmi->ptMaxTrackSize.x = 10;
			//mmi->ptMaxTrackSize.y = 10;
			return 0;
		}
		case WM_SIZE: 
		case WM_MOVE:
			{
  			  WINDOWPLACEMENT  wndpl;
			  GetWindowPlacement(st->displayWnd, &wndpl);
  	 	      st->redraw=TRUE;

			  if (GLOBAL.locksession) {
				  wndpl.rcNormalPosition.top=st->top;
				  wndpl.rcNormalPosition.left=st->left;
				  wndpl.rcNormalPosition.right=st->right;
				  wndpl.rcNormalPosition.bottom=st->bottom;
				  SetWindowPlacement(st->displayWnd, &wndpl);


				  //if (st->displayborder) SetWindowLong(st->displayWnd, GWL_STYLE, (WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME )&~WS_SIZEBOX);
				  // else SetWindowLong(st->displayWnd, GWL_STYLE, 0);
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				 // if (st->displayborder) SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				 // else SetWindowLong(st->displayWnd, GWL_STYLE, 0);
			  }
			  else {
				  st->top=wndpl.rcNormalPosition.top;
				  st->left=wndpl.rcNormalPosition.left;
				  st->right=wndpl.rcNormalPosition.right;
				  st->bottom=wndpl.rcNormalPosition.bottom;
				  st->redraw=TRUE; 
				  st->redraw=TRUE;
				  // SetWindowLong(st->displayWnd, GWL_STYLE, (WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME | WS_SIZEBOX));
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE) | WS_SIZEBOX);
			  }
			  update_border(st->displayWnd,st->displayborder);

			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;

		case WM_ERASEBKGND:
			st->redraw=1;
			return 0;

		case WM_PAINT:
			draw_button(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}


//
//  Object Implementation
//


BUTTONOBJ::BUTTONOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 0;
	width=85;
	height=50;
	
	strcpy(out_ports[0].out_name,"out");

	strcpy (buttonpath,"end.bmp");
	strcpy (buttoncaption,"Button");
	displayborder=1;
	redraw=1;
	bitmapsize=100;
	left=10;right=550;top=20;bottom=400;
	value1=1; value2=0;
	buttonfunction=0;
	state=0;
	togglecount=0;

    transcolor=RGB(0,255,0);
	bkcolor=RGB(55,55,55);
	if(!(displayWnd=CreateWindow("Button_Class", "Button" , WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		report_error("can't create Button Window");
	else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }

    update_border(displayWnd,displayborder);
	InvalidateRect(displayWnd, NULL, TRUE);
}

void BUTTONOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_BUTTONBOX, ghWndMain, (DLGPROC)ButtonDlgHandler));
}
void BUTTONOBJ::load(HANDLE hFile) 
{
	float temp;
	load_object_basics(this);

	load_property("transcolor",P_FLOAT,&temp);
	transcolor=(COLORREF)temp;
	load_property("bkcol",P_FLOAT,&temp);
	bkcolor=(COLORREF)temp;
	temp=0;bitmapsize=100;

	load_property("top",P_INT,&top);
	load_property("left",P_INT,&left);
	load_property("right",P_INT,&right);
	load_property("bottom",P_INT,&bottom);
	load_property("buttonpath",P_STRING,buttonpath);
	load_property("displayborder",P_INT,&displayborder);
	load_property("bitmapsize",P_INT,&bitmapsize);
	load_property("buttonfunction",P_INT,&buttonfunction);
	load_property("value1",P_INT,&value1);
	load_property("value2",P_INT,&value2);
	load_property("buttoncaption",P_STRING,buttoncaption);

	MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 
    update_border(displayWnd,displayborder);
	SetWindowText(displayWnd,buttoncaption);
	InvalidateRect (displayWnd, NULL, TRUE);
	redraw=1;
}
		
void BUTTONOBJ::save(HANDLE hFile) 
{	  
	float temp;
	save_object_basics(hFile, this);
	
	temp=(float)transcolor;
	save_property(hFile,"transcolor",P_FLOAT,&temp);
	temp=(float)bkcolor;
	save_property(hFile,"bkcol",P_FLOAT,&temp);
	
	save_property(hFile,"top",P_INT,&top);
	save_property(hFile,"left",P_INT,&left);
	save_property(hFile,"right",P_INT,&right);
	save_property(hFile,"bottom",P_INT,&bottom);
	save_property(hFile,"buttonpath",P_STRING,buttonpath);
	save_property(hFile,"displayborder",P_INT,&displayborder);
	save_property(hFile,"bitmapsize",P_INT,&bitmapsize);
	save_property(hFile,"buttonfunction",P_INT,&buttonfunction);
	save_property(hFile,"value1",P_INT,&value1);
	save_property(hFile,"value2",P_INT,&value2);
	save_property(hFile,"buttoncaption",P_STRING,buttoncaption);
}

void BUTTONOBJ::incoming_data(int port, float value)
{
}
        
void BUTTONOBJ::work(void) 
{
	switch (buttonfunction) {
		case BUTTONFUNCTION_VAL1VAL2:
			if (state==STATE_PRESSED) {
				pass_values(0,value1);
				state=STATE_IDLE;
			} else pass_values(0,value2);
		break;
		case BUTTONFUNCTION_VAL1INV:
			if (state==STATE_PRESSED) {
				pass_values(0,value1);
				state=STATE_IDLE;
			} else pass_values(0,INVALID_VALUE);
		break;
		case BUTTONFUNCTION_TOGGLEVAL:
			if (togglecount) 
				pass_values(0,value1);
			else 
  			    pass_values(0,value2);
		break;
		case BUTTONFUNCTION_TOGGLE1SEC:
			if ((togglecount) && (togglecount<PACKETSPERSECOND)) { 
				pass_values(0,value1);
				togglecount++;
				if (togglecount>=PACKETSPERSECOND)
					togglecount=0;
			}
			else {
  			    pass_values(0,value2);
			}
		break;
	}
}
	  
BUTTONOBJ::~BUTTONOBJ()
{
	if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
}  
