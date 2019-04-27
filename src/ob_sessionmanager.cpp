/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SESSIONMANAGER.CPP:  contains functions for the Sessionmanager-Object

  The Sessionmanager-Object has its own window, it uses GDI-drawings 

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_sessionmanager.h"
#include <wingdi.h> 

#define NAVI_WIDTH 200
#define NAVI_HEIGHT 200
#define NAVI_SELECTDISTANCE 40

int find_bmpfiles(char * reportname, int select, char * targetfilename) {
	char reportpath[256];
	strcpy(reportpath,GLOBAL.resourcepath); 
	strcat(reportpath,"REPORTS\\");
	strcat(reportpath,reportname);
	strcat(reportpath,"*.bmp");
	// printf("folder: %s\n",reportpath);
	if (!reportname) return 0;
	if (!strlen(reportname)) return 0;

	HANDLE hFind;
	WIN32_FIND_DATA data;
	int count =0;

	hFind = FindFirstFile(reportpath, &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// printf("found: %s\n", data.cFileName);
			if (count==select)
				strcpy (targetfilename, data.cFileName);
			count++;
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return(count);
}

void parse_menuitems(SESSIONMANAGEROBJ* st)
{
	int pos=0;
	char szdata[256];
	char * tmp=st->sessionlist;

	st->menuitems=0;
	while (*tmp) {
		pos=0;

		// read a whole line
		while ((*tmp!=0) && (*tmp!='\r') && (*tmp!='\n'))
		{
			szdata[pos++]=*tmp;
			tmp++;
		}
		while ((*tmp!=0) && ((*tmp=='\r') || (*tmp=='\n'))) tmp++;
		szdata[pos]=0;

		// get sessionname
		char *tmp2=szdata;
		int pos2=0;
		while ((*tmp2) && (*tmp2!='#')) {
		   st->sessionname[st->menuitems][pos2++]=*tmp2;
		   tmp2++;
		}
		st->sessionname[st->menuitems][pos2]=0;

		pos2=0;
		// get sessionpath
		if (*tmp2) {
			tmp2++; 
			while ((*tmp2) && (*tmp2!='#')) {
			   st->sessionpath[st->menuitems][pos2++]=*tmp2;
			   tmp2++;
			}
		}
		st->sessionpath[st->menuitems][pos2]=0;

		pos2=0;
		// get sessionreport
		if (*tmp2) {
			tmp2++;
			while ((*tmp2) && (*tmp2!='#')) {
			   st->sessionreport[st->menuitems][pos2++]=*tmp2;
			   tmp2++;
			}
		}
		st->sessionreport[st->menuitems][pos2]=0;

		// get all bitmap files for the sessionreport
		st->maxreportitems[st->menuitems] = find_bmpfiles(st->sessionreport[st->menuitems], -1, NULL);
		st->menuitems++;
	}
}


void draw_sessionmanager(SESSIONMANAGEROBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[256];
	RECT rect;
	HBRUSH actbrush,bkbrush;
	HPEN bkpen;
	HBITMAP hBitmap = NULL;
	BITMAP          bitmap;
	HDC             hdcMem;
	HGDIOBJ         oldBitmap;

	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);

	actbrush=CreateSolidBrush(st->selectcolor);
    bkbrush=CreateSolidBrush(st->bkcolor);
	bkpen =CreatePen (PS_SOLID,1,st->bkcolor);
	if (st->redraw) FillRect(hdc, &rect,bkbrush);
    
   	SelectObject (hdc, bkpen);		
   	SelectObject (hdc, bkbrush);

	if (strlen(st->logopath)>0) {
		char bitmappath[256];
		strcpy(bitmappath,GLOBAL.resourcepath); 
		strcat(bitmappath,st->logopath);

		hBitmap = (HBITMAP)LoadImage(hInst, bitmappath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (hBitmap) {
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);
			GetObject(hBitmap, sizeof(bitmap), &bitmap);
			BitBlt(hdc, st->logo_x, st->logo_y, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
		}
	}

	if (st->displaynavigation) {
		char bitmappath[256];
		strcpy(bitmappath,GLOBAL.resourcepath); 
		if (strlen(st->actreport)>0)
			strcat(bitmappath,"\\GRAPHICS\\navigation-buttons_reports.bmp");
		else
			strcat(bitmappath,"\\GRAPHICS\\navigation-buttons.bmp");

		hBitmap = (HBITMAP)LoadImage(hInst, bitmappath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (hBitmap) {
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);
			GetObject(hBitmap, sizeof(bitmap), &bitmap);

			// NAVI_X = rect.right/3*2 + 30 ;
			TransparentBlt(hdc, st->navcross_x, st->navcross_y, NAVI_WIDTH, NAVI_HEIGHT,
                          hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, RGB(0,255,0));

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
		}
	}

	if (strlen(st->actreport)>0) {
		char reportpath[256];
		strcpy(reportpath,GLOBAL.resourcepath); 
		strcat(reportpath,"REPORTS\\");
		strcat(reportpath,st->actreport);

		hBitmap = (HBITMAP)LoadImage(hInst, reportpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, hBitmap);
		GetObject(hBitmap, sizeof(bitmap), &bitmap);
		// BitBlt(hdc, 0, rect.bottom/2, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

		//float g= st->bitmapsize/100.0f;
		float g= (float)(rect.right-20)/(float)bitmap.bmWidth;
		StretchBlt(hdc, 10, rect.bottom-10-(float)bitmap.bmHeight*g, (float)bitmap.bmWidth*g, (float)bitmap.bmHeight*g,
                         hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);

		SelectObject(hdc, st->smallfont);
		SetTextColor(hdc,st->selectcolor);
		SetBkColor(hdc, st->bkcolor);
		char reportcaption[256];
		char reportdate[50];
		if (strlen(st->actreport)>20)
		{
			strncpy (reportdate,st->actreport+strlen(st->actreport)-23,10);
			reportdate[10]=0;
		}
		else strcpy (reportdate,"n.a.");
		wsprintf(reportcaption,"Results for Session %d (%s):",st->actreportitem+1, reportdate);
		//wsprintf(reportcaption,"Results for Session %d:",st->actreportitem+1);
		ExtTextOut(hdc, 10, rect.bottom-35-(float)bitmap.bmHeight*g, 0, &rect,reportcaption, strlen(reportcaption), NULL );
	}
 	
	SelectObject(hdc, st->font);
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, st->fontbkcolor);
	SetTextColor(hdc,st->fontcolor);
	char actname[100];
	for (int i=0; i<st->menuitems; i++) {
		if (i==st->actmenuitem) {
			SetBkColor(hdc, st->selectcolor);
			if (st->maxreportitems[i]) 
				wsprintf(actname,"%s (%d/%d)",st->sessionname[i],st->actreportitem+1, st->maxreportitems[i]);
			else
				wsprintf(actname,"%s",st->sessionname[i]);
		} else {
			SetBkColor(hdc, st->fontbkcolor);
			if (st->maxreportitems[i]) 
				wsprintf(actname,"%s (%d)",st->sessionname[i],st->maxreportitems[i]);
			else
				wsprintf(actname,"%s",st->sessionname[i]);
		}
		ExtTextOut(hdc, st->menu_x,st->menu_y+i*2*(st->fontsize+3), 0, &rect,actname, strlen(actname), NULL );
	}
		
	DeleteObject(actbrush);
	DeleteObject(bkbrush);
	DeleteObject(bkpen);
	st->redraw=0;
	EndPaint( st->displayWnd, &ps );
}

int distance(int x1,int y1, int x2, int y2)
{
	return(sqrt ((float)((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2))));
}


LRESULT CALLBACK SessionmanagerDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	char temp[100];
	SESSIONMANAGEROBJ * st;
	int x;
	
	st = (SESSIONMANAGEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_SESSIONMANAGER)) return(FALSE);
					
	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE; // |SIF_POS;
				lpsi.nMin=0; lpsi.nMax=100;
				SetScrollInfo(GetDlgItem(hDlg,IDC_FONTSIZEBAR),SB_CTL,&lpsi,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_FONTSIZEBAR), SB_CTL,st->fontsize,TRUE);
				SetDlgItemInt(hDlg, IDC_FONTSIZE, st->fontsize,0);

				lpsi.nMin=0; lpsi.nMax=500;
				SetScrollInfo(GetDlgItem(hDlg,IDC_BITMAPSIZEBAR),SB_CTL,&lpsi,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_BITMAPSIZEBAR), SB_CTL,st->fontsize,TRUE);
				SetDlgItemInt(hDlg, IDC_BITMAPSIZE, st->bitmapsize,0);

				SetDlgItemText(hDlg, IDC_WINDOWCAPTION, st->wndcaption);
				SetDlgItemText(hDlg, IDC_LOGOPATH, st->logopath);
				SetDlgItemText(hDlg, IDC_SESSIONLIST, st->sessionlist);
				CheckDlgButton(hDlg, IDC_DISPLAYNAVIGATION, st->displaynavigation);

				SetDlgItemInt(hDlg, IDC_MENU_X, st->menu_x,0);
				SetDlgItemInt(hDlg, IDC_MENU_Y, st->menu_y,0);
				SetDlgItemInt(hDlg, IDC_NAVCROSS_X, st->navcross_x,0);
				SetDlgItemInt(hDlg, IDC_NAVCROSS_Y, st->navcross_y,0);
				SetDlgItemInt(hDlg, IDC_LOGO_X, st->logo_x,0);
				SetDlgItemInt(hDlg, IDC_LOGO_Y, st->logo_y,0);

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_SELECTCOLOR:
				st->selectcolor=select_color(hDlg,st->selectcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_FONTCOL:
				st->fontcolor=select_color(hDlg,st->fontcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_FONTBKCOL:
				st->fontbkcolor=select_color(hDlg,st->fontbkcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_BKCOLOR:
				st->bkcolor=select_color(hDlg,st->bkcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,FALSE);
				break;
			case IDC_SESSIONLIST:
				GetDlgItemText(hDlg,IDC_SESSIONLIST,st->sessionlist,4096); 
				parse_menuitems(st);
				st->redraw=1;
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_WINDOWCAPTION:
				GetDlgItemText(hDlg,IDC_WINDOWCAPTION,st->wndcaption,80); 
				SetWindowText(st->displayWnd,st->wndcaption);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_LOGOPATH:
				GetDlgItemText(hDlg,IDC_LOGOPATH,st->logopath,100); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_DISPLAYNAVIGATION:
				 st->displaynavigation=  IsDlgButtonChecked(hDlg,IDC_DISPLAYNAVIGATION);
  				 st->redraw=1;
  				 InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_MENU_X:
				st->menu_x=GetDlgItemInt(hDlg,IDC_MENU_X,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_MENU_Y:
				st->menu_y=GetDlgItemInt(hDlg,IDC_MENU_Y,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_NAVCROSS_X:
				st->navcross_x=GetDlgItemInt(hDlg,IDC_NAVCROSS_X,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_NAVCROSS_Y:
				st->navcross_y=GetDlgItemInt(hDlg,IDC_NAVCROSS_Y,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_LOGO_X:
				st->logo_x=GetDlgItemInt(hDlg,IDC_LOGO_X,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_LOGO_Y:
				st->logo_y=GetDlgItemInt(hDlg,IDC_LOGO_Y,NULL,0); 
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			}
			return TRUE;

		case WM_HSCROLL:
		{
			int nNewPos;
			nNewPos=get_scrollpos(wParam,lParam);
			if (lParam == (long) GetDlgItem(hDlg,IDC_FONTSIZEBAR))  {
				SetDlgItemInt(hDlg, IDC_FONTSIZE,nNewPos,TRUE);
				st->fontsize=nNewPos;
				if (st->font) DeleteObject(st->font);
				st->font = CreateFont(-MulDiv(st->fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
			}
			if (lParam == (long) GetDlgItem(hDlg,IDC_BITMAPSIZEBAR))  {
				SetDlgItemInt(hDlg, IDC_BITMAPSIZE,nNewPos,TRUE);
				st->bitmapsize=nNewPos;
			}
			if (st->displayWnd) {st->redraw=1; InvalidateRect(st->displayWnd,NULL,TRUE);}		
		}	break;

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_SELECTCOLOR),st->selectcolor);
			color_button(GetDlgItem(hDlg,IDC_FONTCOL),st->fontcolor);
			color_button(GetDlgItem(hDlg,IDC_BKCOLOR),st->bkcolor);
			color_button(GetDlgItem(hDlg,IDC_FONTBKCOL),st->fontbkcolor);
			break;
	}
    return FALSE;
}


LRESULT CALLBACK SessionManagerWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	SESSIONMANAGEROBJ * st;
	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_SESSIONMANAGER)
		{	st=(SESSIONMANAGEROBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

	if (st==NULL) return DefWindowProc( hWnd, message, wParam, lParam );
	
	switch( message ) 
	{	case WM_DESTROY:
		 break;
		case WM_KEYDOWN:
			// printf("key: %d",wParam);
		    switch(wParam) {
				case KEY_UP:
				  if (st->actmenuitem>0) {
					  st->actmenuitem--;
					  if (st->maxreportitems[st->actmenuitem]) {
							st->actreportitem=st->maxreportitems[st->actmenuitem]-1;
							find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
					  }
					  else { st->actreportitem=0; st->actreport[0]=0;}
				  }
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				case KEY_DOWN:
				  if (st->actmenuitem<st->menuitems-1) {
					  st->actmenuitem++;
					  if (st->maxreportitems[st->actmenuitem]) {
							st->actreportitem=st->maxreportitems[st->actmenuitem]-1;
							find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
					  }
					  else { st->actreportitem=0; st->actreport[0]=0;}
				  }
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				case KEY_LEFT:
					if (st->maxreportitems[st->actmenuitem]) {
							if (st->actreportitem>0) st->actreportitem--;
							find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
					}
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;

				case KEY_RIGHT:
					if (st->maxreportitems[st->actmenuitem]) {
							if (st->actreportitem<st->maxreportitems[st->actmenuitem]-1) 
								st->actreportitem++;
							find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
					}
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				case KEY_BACKSPACE:
 				  if (st->actmenuitem>0) {
					    find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
  						char tmptxt[500];
						wsprintf(tmptxt,"Do you really want to remove the report graph %s",st->actreport);
						if (MessageBox(NULL, tmptxt, "Confirm removal", MB_YESNO) == IDYES) 
						{
							char reportpath[256];
							strcpy(reportpath,GLOBAL.resourcepath); 
							strcat(reportpath,"REPORTS\\");
							strcat(reportpath,st->actreport);
							printf ("remove bitmap: %s !!\n",reportpath);
							DeleteFile(reportpath);
						}
						parse_menuitems(st);
						if (st->maxreportitems[st->actmenuitem]) {
							st->actreportitem=st->maxreportitems[st->actmenuitem]-1;
							find_bmpfiles(st->sessionreport[st->actmenuitem], st->actreportitem, st->actreport);
						}
						else { st->actreportitem=0; st->actreport[0]=0;}
						st->redraw=1;
						InvalidateRect(st->displayWnd,NULL,FALSE);
				  }
				break;


				case KEY_ENTER:
				  if (st->actmenuitem<st->menuitems) 
				  {
						char configfilename[MAX_PATH];
						close_toolbox();
						strcpy(configfilename,GLOBAL.resourcepath); 
						strcat(configfilename,"CONFIGURATIONS\\");
						strcat(configfilename,st->sessionpath[st->actmenuitem]);
						strcat(configfilename,".con");
						// printf("trying to load configfile: %s\n",configfilename);
						if (!load_configfile(configfilename)) 
							report_error("Could not load Config File");
						else sort_objects();					  
				  }
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				}
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

			   // printf("lbuttondown in wnd %ld at: %ld, %ld\n",hWnd, actx,acty);
			   if (distance (actx, acty, st->navcross_x+100, st->navcross_y+33)  < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_UP,0); 
			   if (distance (actx, acty, st->navcross_x+33,  st->navcross_y+100) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_LEFT,0); 
			   if (distance (actx, acty, st->navcross_x+100, st->navcross_y+170) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_DOWN,0); 
			   if (distance (actx, acty, st->navcross_x+170, st->navcross_y+100) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_RIGHT,0); 
			   if (distance (actx, acty, st->navcross_x+100, st->navcross_y+100) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_ENTER,0); 
			   if (distance (actx, acty, st->navcross_x+170, st->navcross_y+170) < NAVI_SELECTDISTANCE) SendMessage(hWnd, WM_KEYDOWN, KEY_BACKSPACE,0); 

			}
			break;

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
 				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE)&~WS_SIZEBOX);
			  }
			  else {
				  st->top=wndpl.rcNormalPosition.top;
				  st->left=wndpl.rcNormalPosition.left;
				  st->right=wndpl.rcNormalPosition.right;
				  st->bottom=wndpl.rcNormalPosition.bottom;
				  st->redraw=TRUE; 
				  st->redraw=TRUE;
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE) | WS_SIZEBOX);
			  }
			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;

		case WM_ERASEBKGND:
			st->redraw=1;
			return 0;

		case WM_PAINT:
			draw_sessionmanager(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}


//
//  Object Implementation
//


SESSIONMANAGEROBJ::SESSIONMANAGEROBJ(int num) : BASE_CL()
{
	outports = 0;
	inports = 0;
	width=115;
	height=50;
	// strcpy(in_ports[0].in_name,"in");
	// strcpy(out_ports[0].out_name,"out");

	strcpy (wndcaption,"Session Manager");
	strcpy (logopath,"");
	strcpy (sessionlist,"Testsession1#testconfig#testgraph1\r\nTestsession2#testconfig2#testgraph2");
	displaynavigation=1;
	redraw=1;
	fontsize=18;
	bitmapsize=100;
	left=10;right=550;top=20;bottom=400;
    menu_x=40; menu_y=40;
	navcross_x=750;navcross_y=140;
	logo_x=650,logo_y=50;

	actmenuitem=0;
	actreportitem=0;
	actreport[0]=0;
	parse_menuitems(this);

	selectcolor=RGB(100,200,100);
	bkcolor=RGB(55,55,55);
	fontcolor=RGB(50,50,200);
	fontbkcolor=RGB(255,255,255);

	if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
		report_error("Font creation failed!");
	if (!(smallfont = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
		report_error("Font creation failed!");

	if(!(displayWnd=CreateWindow("SessionManager_Class", wndcaption, WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		report_error("can't create SessionManager Window");
	else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }

	InvalidateRect(displayWnd, NULL, TRUE);
}

void SESSIONMANAGEROBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SESSIONMANAGERBOX, ghWndMain, (DLGPROC)SessionmanagerDlgHandler));
}
void SESSIONMANAGEROBJ::load(HANDLE hFile) 
{
	float temp;
	load_object_basics(this);

	load_property("selectcolor",P_FLOAT,&temp);
	selectcolor=(COLORREF)temp;
	load_property("bkcol",P_FLOAT,&temp);
	bkcolor=(COLORREF)temp;
	if (bkcolor==selectcolor) bkcolor=RGB(255,255,255);
	temp=0;bitmapsize=100;
	load_property("fontcol",P_FLOAT,&temp);
	fontcolor=(COLORREF)temp;
	temp=RGB(255,255,255);
	load_property("fontbkcol",P_FLOAT,&temp);
	fontbkcolor=(COLORREF)temp;
	
	load_property("top",P_INT,&top);
	load_property("left",P_INT,&left);
	load_property("right",P_INT,&right);
	load_property("bottom",P_INT,&bottom);
	load_property("fontsize",P_INT,&fontsize);
	if (fontsize)
	{
		if (font) DeleteObject(font);
		if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
			report_error("Font creation failed!");
	} 
	load_property("wndcaption",P_STRING,wndcaption);
	load_property("logopath",P_STRING,logopath);
	load_property("sessionlist",P_STRING,sessionlist);
	char * tmp=sessionlist;
	while (*tmp) { if (*tmp=='§') *tmp='\n'; if (*tmp=='~') *tmp='\r'; tmp++; } 
	load_property("displaynavigation",P_INT,&displaynavigation);
	load_property("bitmapsize",P_INT,&bitmapsize);
	load_property("menu_x",P_INT,&menu_x);
	load_property("menu_y",P_INT,&menu_y);
	load_property("navcross_x",P_INT,&navcross_x);
	load_property("navcross_y",P_INT,&navcross_y);
	load_property("logo_x",P_INT,&logo_x);
	load_property("logo_y",P_INT,&logo_y);

	parse_menuitems(this);
	if (maxreportitems[0]>0)
		actreportitem=maxreportitems[0]-1;
	else actreportitem=0;

	MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 
	if (GLOBAL.locksession) {
 		SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
		//SetWindowLong(displayWnd, GWL_STYLE, 0);
	} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
	InvalidateRect (displayWnd, NULL, TRUE);

	SetWindowText(displayWnd,wndcaption);
	redraw=1;
}
		
void SESSIONMANAGEROBJ::save(HANDLE hFile) 
{	  
	float temp;
	save_object_basics(hFile, this);
	
	temp=(float)selectcolor;
	save_property(hFile,"selectcolor",P_FLOAT,&temp);
	temp=(float)bkcolor;
	save_property(hFile,"bkcol",P_FLOAT,&temp);
	temp=(float)fontcolor;
	save_property(hFile,"fontcol",P_FLOAT,&temp);
	temp=(float)fontbkcolor;
	save_property(hFile,"fontbkcol",P_FLOAT,&temp);
	
	save_property(hFile,"top",P_INT,&top);
	save_property(hFile,"left",P_INT,&left);
	save_property(hFile,"right",P_INT,&right);
	save_property(hFile,"bottom",P_INT,&bottom);
	save_property(hFile,"fontsize",P_INT,&fontsize);
	save_property(hFile,"wndcaption",P_STRING,wndcaption);
	save_property(hFile,"logopath",P_STRING,logopath);

	char * tmp=sessionlist;
	while (*tmp) { if (*tmp=='\n') *tmp='§'; if (*tmp=='\r') *tmp='~'; tmp++; } 
	save_property(hFile,"sessionlist",P_STRING,sessionlist);
	tmp=sessionlist;
	while (*tmp) { if (*tmp=='§') *tmp='\n'; if (*tmp=='~') *tmp='\r'; tmp++; } 

	save_property(hFile,"displaynavigation",P_INT,&displaynavigation);
	save_property(hFile,"bitmapsize",P_INT,&bitmapsize);
	save_property(hFile,"menu_x",P_INT,&menu_x);
	save_property(hFile,"menu_y",P_INT,&menu_y);
	save_property(hFile,"navcross_x",P_INT,&navcross_x);
	save_property(hFile,"navcross_y",P_INT,&navcross_y);
	save_property(hFile,"logo_x",P_INT,&logo_x);
	save_property(hFile,"logo_y",P_INT,&logo_y);
}

void SESSIONMANAGEROBJ::incoming_data(int port, float value)
{
}
        
void SESSIONMANAGEROBJ::work(void) 
{
	if ((displayWnd)&&(!TIMING.draw_update)&&(!GLOBAL.fly)) 
		InvalidateRect(displayWnd,NULL,FALSE);	  
}
	  
SESSIONMANAGEROBJ::~SESSIONMANAGEROBJ()
{
	if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
}  
