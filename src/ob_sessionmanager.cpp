/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
  
  MODULE: OB_SESSIONMANAGER.CPP:  contains functions for the Threshold-Generator-Object

  The Threshold-Object has its own window, it uses GDI-drawings 
  Min- and Max- values can be selected, also a pre-gain. 
  With the additional 'only rising' and 'only falling' - properties, segments of 
  a signal can be passed / filtered out.
  draw_meter: draws the meter and shows threshold- and actual values
  apply_threshold: stores the current setting of the toolbox-window
  ThresholdDlgHandler: processes the events for the Threshold-toolbox window
  MeterWndHandler: processes the events for the Meter - drawing window

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_sessionmanager.h"
#include <wingdi.h> 

#define KEY_UP 38
#define KEY_DOWN 40
#define KEY_LEFT 37
#define KEY_RIGHT 39
#define KEY_ENTER 13


int find_bmpfiles(char * reportname, int select, char * targetfilename) {
	char reportpath[256];
	strcpy(reportpath,GLOBAL.resourcepath); 
	strcat(reportpath,"REPORTS\\");
	strcat(reportpath,reportname);
	strcat(reportpath,"*.bmp");
	// printf("folder: %s\n",reportpath);

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
		while ((*tmp!=0) && (*tmp!='\r') && (*tmp!='\n'))
		{
			szdata[pos++]=*tmp;
			tmp++;
		}
		while ((*tmp=='\r') || (*tmp=='\n')) tmp++;
		szdata[pos]=0;

		char *tmp2=szdata;
		int pos2=0;
		while ((*tmp2) && (*tmp2!=',')) {
		   st->sessionname[st->menuitems][pos2++]=*tmp2;
		   tmp2++;
		}
		st->sessionname[st->menuitems][pos2]=0;

		if (*tmp2) {
			tmp2++; pos2=0;
			while ((*tmp2) && (*tmp2!=',')) {
			   st->sessionpath[st->menuitems][pos2++]=*tmp2;
			   tmp2++;
			}
		}
		st->sessionpath[st->menuitems][pos2]=0;

		if (*tmp2) {
			tmp2++; pos2=0;
			while ((*tmp2) && (*tmp2!=',')) {
			   st->sessionreport[st->menuitems][pos2++]=*tmp2;
			   tmp2++;
			}
		}
		st->sessionreport[st->menuitems][pos2]=0;
		st->maxreportitems[st->menuitems] = find_bmpfiles(st->sessionreport[st->menuitems], -1, NULL);
		st->menuitems++;
	}
}


void draw_meter(SESSIONMANAGEROBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[256];
	RECT rect;
	HBRUSH actbrush,bkbrush;
	HPEN bkpen;

	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);

	actbrush=CreateSolidBrush(st->color);
    bkbrush=CreateSolidBrush(st->bkcolor);
	bkpen =CreatePen (PS_SOLID,1,st->bkcolor);
	if (st->redraw) FillRect(hdc, &rect,bkbrush);
    
   	SelectObject (hdc, bkpen);		
   	SelectObject (hdc, bkbrush);

	if (strlen(st->actreport)>0) {
		HBITMAP hBitmap = NULL;
		BITMAP          bitmap;
		HDC             hdcMem;
		HGDIOBJ         oldBitmap;
		char reportpath[256];
		strcpy(reportpath,GLOBAL.resourcepath); 
		strcat(reportpath,"REPORTS\\");
		strcat(reportpath,st->actreport);

		hBitmap = (HBITMAP)LoadImage(hInst, reportpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, hBitmap);
		GetObject(hBitmap, sizeof(bitmap), &bitmap);
		// BitBlt(hdc, 0, rect.bottom/2, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
		StretchBlt(hdc, 0, rect.bottom/2, rect.right, rect.bottom/2,
                         hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
	}
 	
	SelectObject(hdc, st->font);
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, st->fontbkcolor);
	SetTextColor(hdc,st->fontcolor);
	char actname[100];
	for (int i=0; i<st->menuitems; i++) {
		if (i==st->actmenuitem) {
			SetBkColor(hdc, st->fontbkcolor);
			if (st->maxreportitems[i]) 
				wsprintf(actname,"%s (%d/%d)",st->sessionname[i],st->actreportitem+1, st->maxreportitems[i]);
			else
				wsprintf(actname,"%s",st->sessionname[i]);
		} else {
			SetBkColor(hdc, st->color);
			if (st->maxreportitems[i]) 
				wsprintf(actname,"%s (%d)",st->sessionname[i],st->maxreportitems[i]);
			else
				wsprintf(actname,"%s",st->sessionname[i]);
		}
		ExtTextOut(hdc, rect.left+40,25+i*(40+st->fontsize), 0, &rect,actname, strlen(actname), NULL );
	}
		
	DeleteObject(actbrush);
	DeleteObject(bkbrush);
	DeleteObject(bkpen);
	st->redraw=0;
	EndPaint( st->displayWnd, &ps );
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
				SetDlgItemText(hDlg, IDC_WNDCAPTION, st->wndcaption);
				SetDlgItemText(hDlg, IDC_SESSIONLIST, st->sessionlist);
				CheckDlgButton(hDlg, IDC_DISPLAYREPORTS, st->displayreports);
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
				st->color=select_color(hDlg,st->color);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				break;
			case IDC_FONTCOL:
				st->fontcolor=select_color(hDlg,st->fontcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				break;
			case IDC_FONTBKCOL:
				st->fontbkcolor=select_color(hDlg,st->fontbkcolor);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				break;
			case IDC_SESSIONLIST:
				GetDlgItemText(hDlg,IDC_SESSIONLIST,st->sessionlist,4096); 
				parse_menuitems(st);
				break;
			case IDC_WNDCAPTION:
				GetDlgItemText(hDlg,IDC_WNDCAPTION,st->wndcaption,80); 
				SetWindowText(st->displayWnd,st->wndcaption);
				break;
			case IDC_DISPLAYREPORTS:
				 st->displayreports=  IsDlgButtonChecked(hDlg,IDC_DISPLAYREPORTS);
  				 st->redraw=1;
				break;
			}
			return TRUE;
			
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_SELECTCOLOR),st->color);
			color_button(GetDlgItem(hDlg,IDC_FONTCOL),st->fontcolor);
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
					  st->actreportitem=0;
					  st->actreport[0]=0;
				  }
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				case KEY_DOWN:
				  if (st->actmenuitem<st->menuitems-1) {
					  st->actmenuitem++;
 					  st->actreportitem=0; 
					  st->actreport[0]=0;
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
		case WM_SIZE: 
		case WM_MOVE:
			{
			WINDOWPLACEMENT  wndpl;
			GetWindowPlacement(st->displayWnd, &wndpl);
			st->top=wndpl.rcNormalPosition.top;
			st->left=wndpl.rcNormalPosition.left;
			st->right=wndpl.rcNormalPosition.right;
			st->bottom=wndpl.rcNormalPosition.bottom;
			}
			st->redraw=1;
			InvalidateRect(hWnd,NULL,TRUE);
			break;

		case WM_ERASEBKGND:
			st->redraw=1;
			return 0;

		case WM_PAINT:
			draw_meter(st);
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
	strcpy (sessionlist,"Testsession1,test,smr-graph\r\nTestsession2,opi_test,opi-graph");
	displayreports=1;
	redraw=1;
	fontsize=18;
	left=10;right=550;top=20;bottom=400;

	actmenuitem=0;
	actreportitem=0;
	actreport[0]=0;
	parse_menuitems(this);

	color=RGB(100,200,100);
	bkcolor=RGB(55,55,55);
	fontcolor=RGB(50,50,200);
	fontbkcolor=RGB(255,255,255);

	if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
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
	/*
	load_property("color",P_FLOAT,&temp);
	color=(COLORREF)temp;
	load_property("bkcol",P_FLOAT,&temp);
	bkcolor=(COLORREF)temp;
	if (bkcolor==color) bkcolor=RGB(255,255,255);
	temp=0;
	load_property("fontcol",P_FLOAT,&temp);
	fontcolor=(COLORREF)temp;
	temp=RGB(255,255,255);
	load_property("fontbkcol",P_FLOAT,&temp);
	fontbkcolor=(COLORREF)temp;
	*/
	load_property("top",P_INT,&top);
	load_property("left",P_INT,&left);
	load_property("right",P_INT,&right);
	load_property("bottom",P_INT,&bottom);
/*	load_property("fontsize",P_INT,&fontsize);
	if (fontsize)
	{
		if (font) DeleteObject(font);
		if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
			report_error("Font creation failed!");
	} */
	load_property("wndcaption",P_STRING,wndcaption);
	load_property("sessionlist",P_STRING,sessionlist);
	char * tmp=sessionlist;
	while (*tmp) { if (*tmp==';') *tmp='\n'; if (*tmp==':') *tmp='\r'; tmp++; } 
	load_property("displayreports",P_INT,&displayreports);

	parse_menuitems(this);
	MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 
	SetWindowText(displayWnd,wndcaption);
	redraw=1;
}
		
void SESSIONMANAGEROBJ::save(HANDLE hFile) 
{	  
	float temp;
	save_object_basics(hFile, this);
	/*
	temp=(float)color;
	save_property(hFile,"color",P_FLOAT,&temp);
	temp=(float)bkcolor;
	save_property(hFile,"bkcol",P_FLOAT,&temp);
	temp=(float)fontcolor;
	save_property(hFile,"fontcol",P_FLOAT,&temp);
	temp=(float)fontbkcolor;
	save_property(hFile,"fontbkcol",P_FLOAT,&temp);
	*/
	save_property(hFile,"top",P_INT,&top);
	save_property(hFile,"left",P_INT,&left);
	save_property(hFile,"right",P_INT,&right);
	save_property(hFile,"bottom",P_INT,&bottom);
	save_property(hFile,"fontsize",P_INT,&fontsize);
	save_property(hFile,"wndcaption",P_STRING,wndcaption);

	char * tmp=sessionlist;
	while (*tmp) { if (*tmp=='\n') *tmp=';'; if (*tmp=='\r') *tmp=':'; tmp++; } 
	save_property(hFile,"sessionlist",P_STRING,sessionlist);
	tmp=sessionlist;
	while (*tmp) { if (*tmp==';') *tmp='\n'; if (*tmp==':') *tmp='\r'; tmp++; } 

	save_property(hFile,"displayreports",P_INT,&displayreports);
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
