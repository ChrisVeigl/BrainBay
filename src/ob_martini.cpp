/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_MARTINI.CPP:  contains functions for the MARTINI-Generator-Object
  Author: Chris Veigl

  The MARTINI-Object provides a Brainwave controlled Martini mixing game.
  Two Relais control the power supply for fwo liquid-pumps that provide
  Gin and Vermouth. The realais are connected to the Monolith EEG port D
  and can be switched by the Com-Writer element.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_martini.h"
#include <wingdi.h> 
#include <vfw.h>	 


#define RUNTIME 90    //seconds
#define PUMP1TIME 700   // max time for vermouth
#define PUMP2TIME 700   // max time for gin
#define PUMP1GAME 600   // max time for vermouth
#define PUMP2GAME 600   // max time for gin


HWND m_audio=NULL;
HWND m_audio2=NULL;
int oldstate=-1;
int actsound=0;

void draw_MARTINI(MARTINIOBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	BITMAP bm;

	char szdata[256];
	RECT rect;
	HPEN actpen;
	// int ballx,bally;
	int racketx,rackets,level;
	HDC hdcMem;
	HBITMAP hbmOld;
    float tmp;
	
	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);

	if (st->state != oldstate) st->redraw=1;

	if (st->redraw==1)
	{
		SelectObject (hdc, DRAW.pen_white);		
   		SelectObject (hdc, DRAW.brush_white);
		Rectangle(hdc,0,0,rect.right,rect.bottom);

	   	
		switch (st->state)
		{
			case 0:
				hdcMem = CreateCompatibleDC(hdc);
				hbmOld = (HBITMAP) SelectObject(hdcMem, st->g_hbm1);
				GetObject(st->g_hbm1, sizeof(bm), &bm);
				break;
			case 1:
				hdcMem = CreateCompatibleDC(hdc);
				hbmOld = (HBITMAP) SelectObject(hdcMem, st->g_hbm2);
				GetObject(st->g_hbm2, sizeof(bm), &bm);
				break;
			case 2:
				hdcMem = CreateCompatibleDC(hdc);
				hbmOld = (HBITMAP) SelectObject(hdcMem, st->g_hbm3);
				GetObject(st->g_hbm3, sizeof(bm), &bm);
				break;
		}

		
		BitBlt(hdc, 0,0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);

		
		if (st->state==1)
		{
			SelectObject (hdc, DRAW.brush_blue);
			Rectangle(hdc,rect.right-205,350,rect.right-45,370);
   	
			SelectObject (hdc, DRAW.brush_yellow);
			Rectangle(hdc,rect.right-200,355,rect.right-50,365);
		}

		oldstate=st->state;
	}


	switch (st->state)
	{

	  case 0:
		    SelectObject(hdc, DRAW.mediumFont);
		    SetTextColor(hdc,RGB(0,0,150));
			sprintf(szdata, " The Mindreading Martini Maker");
			ExtTextOut( hdc, 60,40, 0, &rect,szdata, strlen(szdata), NULL ) ;

			SetTextColor(hdc,RGB(100,0,0));
			sprintf(szdata, "get Ready :");
			ExtTextOut( hdc, rect.right-280,200, 0, &rect,szdata, strlen(szdata), NULL ) ;
			sprintf(szdata, "input Brainwaves ...");
			ExtTextOut( hdc, rect.right-280,230, 0, &rect,szdata, strlen(szdata), NULL ) ;

			SetTextColor(hdc,RGB(190,100,0));

			switch ((int)st->preset_min)
			{
				case 2:	sprintf(szdata, "Preset-Selection : Sweet     "); break;
				case 4:	sprintf(szdata, "Preset-Selection : Normal    "); break;
				case 6:	sprintf(szdata, "Preset-Selection : Dry       "); break;
			}
			ExtTextOut( hdc, rect.right-340,300, 0, &rect,szdata, strlen(szdata), NULL ) ;

	  break;

	 case 1:
		racketx=(int) (rect.right-200);
		rackets=(int) (rect.right-200+(int)((float)st->time*150/(float)st->gametime));

   		SelectObject (hdc, DRAW.pen_white);		

   		SelectObject (hdc, DRAW.brush_blue);
		Rectangle(hdc,racketx,355,rackets,365);


	    actpen = CreatePen (PS_SOLID,1,PALETTERGB(90,90,155));

		SelectObject (hdc, actpen);

		tmp=(float)st->sampletime/256.0f/(float)st->gametime;
		level=223-(int)(tmp*80);
		if (level<140) level=140;
			MoveToEx(hdc,115-(int)(tmp*71), level,NULL);	
			LineTo(hdc,155+(int)(tmp*67), level);
	//	Rectangle(hdc,118,223,152,level);

	
		SelectObject(hdc, DRAW.mediumFont);
		SetTextColor(hdc,RGB(255,0,0));

		SelectObject (hdc, DRAW.pen_red);	
	

		SetTextColor(hdc,RGB(0,0,150));
	
		sprintf(szdata, "ALPHA: %.2f     ",st->alpha);
		ExtTextOut( hdc, rect.right-280,90, 0, &rect,szdata, strlen(szdata), NULL ) ;

		sprintf(szdata, "BETA : %.2f     ",st->beta);
		ExtTextOut( hdc, rect.right-280,120, 0, &rect,szdata, strlen(szdata), NULL ) ;
	
		SetTextColor(hdc,RGB(190,100,0));

		sprintf(szdata, "Vermouth:Gin = 1:%.2f                    ",st->ratio);
		ExtTextOut( hdc, rect.right-320,160, 0, &rect,szdata, strlen(szdata), NULL ) ;


		SetTextColor(hdc,RGB(0,100,0));
/*
		sprintf(szdata, "read %d seconds. ",st->time);
		ExtTextOut( hdc, rect.right-220,300, 0, &rect,szdata, strlen(szdata), NULL ) ;
*/
		DeleteObject(actpen);
	  break;

    
	  case 2:
		    SelectObject(hdc, DRAW.mediumFont);
		    SetTextColor(hdc,RGB(150,150,150));
			SetBkColor(hdc,RGB(0,0,0));
			sprintf(szdata, "Processing Drink ...");
			ExtTextOut( hdc, 50,50, 0, &rect,szdata, strlen(szdata), NULL ) ;
		    SetTextColor(hdc,RGB(220,180,150));

			sprintf(szdata, "Activate Vermouth %.2f seconds",(float)st->pump1/256.0f);
			ExtTextOut( hdc, 90,80, 0, &rect,szdata, strlen(szdata), NULL ) ;
			sprintf(szdata, "Activate Gin %.2f seconds",(float)st->pump2/256.0f);
			ExtTextOut( hdc, 90,110, 0, &rect,szdata, strlen(szdata), NULL ) ;
	  break;

	}

	
    
	EndPaint( st->displayWnd, &ps );
	st->redraw=1;

}

void start_martinigame (	MARTINIOBJ * st)
{
	char tmpfile[256];
	char tmpname[50];

	strcpy(tmpfile,GLOBAL.resourcepath);
	actsound++;
	wsprintf(tmpname,"martini\\Track%d.wav",actsound%3+1);
	strcat(tmpfile,tmpname);

	if (m_audio) {	MCIWndStop(m_audio); 	MCIWndDestroy(m_audio); }
	if (m_audio2) {	MCIWndStop(m_audio2); 	MCIWndDestroy(m_audio2); }

	m_audio = MCIWndCreate(ghWndMain, hInst,WS_THICKFRAME|MCIWNDF_NOMENU|MCIWNDF_NOPLAYBAR|MCIWNDF_NOERRORDLG,tmpfile);
	strcpy(tmpfile,GLOBAL.resourcepath);
	strcat(tmpfile,"martini\\fin.wav");
	m_audio2 = MCIWndCreate(ghWndMain, hInst,WS_THICKFRAME|MCIWNDF_NOMENU|MCIWNDF_NOPLAYBAR|MCIWNDF_NOERRORDLG,tmpfile);
	MCIWndPlay(m_audio);
						
	srand( TIMING.packetcounter);
	st->sampletime=0;
	st->state=1;
	st->time=0;
	st->cnt1=0;
	st->alpha=0;
	st->beta=0;
	st->baseline=st->input1/st->input2;

	st->redraw=1;
	st->redrawcnt=0;
	InvalidateRect(st->displayWnd,NULL,FALSE);	
	InvalidateRect(st->displayWnd,NULL,FALSE);	
	InvalidateRect(st->displayWnd,NULL,FALSE);
}
	
LRESULT CALLBACK MartiniDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	MARTINIOBJ * st;
	char tmpfile[256];
	char tmpname[50];
	
	st = (MARTINIOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MARTINI)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;

				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				
				lpsi.nMin=30; lpsi.nMax=600;
				SetScrollInfo(GetDlgItem(hDlg,IDC_GAMETIMEBAR),SB_CTL,&lpsi,TRUE);
				
				SetScrollPos(GetDlgItem(hDlg,IDC_GAMETIMEBAR), SB_CTL,st->gametime,TRUE);
				SetDlgItemInt(hDlg,IDC_GAMETIME, st->gametime, FALSE);
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 
				case IDC_SWEET: 
					st->preset_min=2;
					st->preset_max=4;
					break;
				case IDC_NORMAL: 
					st->preset_min=4;
					st->preset_max=6;
					break;
				case IDC_DRY:
					st->preset_min=6;
					st->preset_max=10;
					break;
				case IDC_QUICKIE: 
					st->ratio=(st->preset_min+st->preset_max)/2;
					st->state=1;
					st->time=st->gametime-1;
					st->cnt1=256;
					break;
				case IDC_PUMP1:
					if ((!st->pump1cnt) && (!st->pump2cnt)) st->pump1cnt=PUMP1TIME;
					break;
				case IDC_PUMP2:
					if ((!st->pump1cnt) && (!st->pump2cnt)) st->pump2cnt=PUMP2TIME;
					break;
				case IDC_START: 
					start_martinigame(st);
					break;

			}
			return TRUE;

		case WM_HSCROLL:
		{
			int nNewPos;

			nNewPos=get_scrollpos(wParam,lParam);

			if (lParam == (long) GetDlgItem(hDlg,IDC_GAMETIMEBAR))  { SetDlgItemInt(hDlg, IDC_GAMETIME,nNewPos,0); st->gametime=nNewPos;}
			InvalidateRect(st->displayWnd,NULL,TRUE);
		
		}	break; 
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
//			color_button(GetDlgItem(hDlg,IDC_SELECTCOLOR),st->color);
		break;
	}
    return FALSE;
}

int mmouse_x=0;
int mmouse_y=0;

LRESULT CALLBACK MartiniWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	MARTINIOBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_MARTINI)
		{	st=(MARTINIOBJ *)objects[t];
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
		  st->redraw=1;
		  InvalidateRect(st->displayWnd,NULL,FALSE);
			break;
		case WM_MOUSEMOVE:
			mmouse_x=LOWORD(lParam);
			mmouse_y=HIWORD(lParam);
			break;
		case WM_LBUTTONDOWN:
			// char test[100];
			// sprintf(test,"x=%d  y=%d",mmouse_x,mmouse_y);
			// SetDlgItemText(ghWndStatusbox,IDC_STATUS,test);

			if ((mmouse_x>80) && (mmouse_x<300) && (mmouse_y>120) && (mmouse_y<270))
			{	// start  
				// report ("start");
				start_martinigame(st);
			}
			if ((mmouse_x>300) && (mmouse_x<390) && (mmouse_y>380) && (mmouse_y<430))
			{	// test gin  
				// report ("test gin");
				if ((!st->pump1cnt) && (!st->pump2cnt)) st->pump1cnt=PUMP1TIME;
			}
			if ((mmouse_x>440) && (mmouse_x<640) && (mmouse_y>380) && (mmouse_y<430))
			{	// test vermouth
				// report ("test vermouth");
				if ((!st->pump1cnt) && (!st->pump2cnt)) st->pump2cnt=PUMP2TIME;
			}
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
			InvalidateRect(hWnd,NULL,FALSE);
			break;
		case WM_PAINT:
			draw_MARTINI(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}






//
//  Object Implementation
//


MARTINIOBJ::MARTINIOBJ(int num) : BASE_CL()
	  {
		char tmpfile[256];
	
		outports = 1;
		inports = 2;
		strcpy(in_ports[0].in_name,"alpha");
		strcpy(in_ports[1].in_name,"beta");

	    out_ports[0].get_range=-1;
		strcpy(out_ports[0].out_name,"P1");
		strcpy(out_ports[0].out_dim,"none");
		strcpy(out_ports[0].out_desc,"Gin");
	    out_ports[0].out_max=127;
        out_ports[0].out_min=0;
		
		alpha=0;beta=0;
		time=0;cnt1=0;

		preset_min=4;
		preset_max=6;

		input1=0.0f; input2=0.0f;
		left=500;right=800;top=50;bottom=400;

		width=80;
		gametime=RUNTIME;

		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"martini\\");
		strcat(tmpfile,"martini1.bmp");
		g_hbm1 = (HBITMAP) LoadImage(NULL, tmpfile,IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
		if (!g_hbm1) report_error("could not open Martini-Bitmap File");

		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"martini\\");
		strcat(tmpfile,"martini2.bmp");
		g_hbm2 = (HBITMAP) LoadImage(NULL, tmpfile,IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
		if (!g_hbm1) report_error("could not open Martini-Bitmap File");

		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"martini\\");
		strcat(tmpfile,"martini3.bmp");
		g_hbm3 = (HBITMAP) LoadImage(NULL, tmpfile,IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
		if (!g_hbm1) report_error("could not open Martini-Bitmap File");

        
		if(!(displayWnd=CreateWindow("Martini_Class", "Martini", WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		    report_error("can't create MARTINI Window");
		else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }

		state=0; redraw=1;redrawcnt=0;
		pump1cnt=0;pump2cnt=0;sampletime=0;

		srand( TIMING.packetcounter);
		InvalidateRect(displayWnd, NULL, TRUE);
	  }
	  void MARTINIOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MARTINIBOX, ghWndStatusbox, (DLGPROC)MartiniDlgHandler));
	  }
	  void MARTINIOBJ::load(HANDLE hFile) 
	  {
		  load_object_basics(this);
		  load_property("top",P_INT,&top);
		  load_property("left",P_INT,&left);
		  load_property("right",P_INT,&right);		
		  load_property("bottom",P_INT,&bottom);
		  MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
	  }
		
	  void MARTINIOBJ::save(HANDLE hFile) 
	  {	  
	 	  save_object_basics(hFile, this);
  		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);

	  }


	  void MARTINIOBJ::incoming_data(int port, float value)
      {
		  if (port==0)
		  {
			  if (value!=INVALID_VALUE) input1 = value; else input1=0;

		  }

          if (port==1) 
		  {
			  if (value!=INVALID_VALUE) input2 = value; else input2=0;
		  }
	  }
        

	  void MARTINIOBJ::work(void) 
	  {

		if (GLOBAL.fly) return;

		if( redrawcnt++ > 50) {redrawcnt=0; redraw=0; InvalidateRect(displayWnd,NULL,FALSE); }

		if (pump1cnt)  
		{ 
			if (pump1cnt==PUMP1TIME) pass_values(0,16); 
			if (--pump1cnt==0) pass_values(0,0);
			return;
		}
		if (pump2cnt)  
		{ 
			if (pump2cnt==PUMP2TIME) pass_values(0,32); 
			if (--pump2cnt==0) pass_values(0,0);
			return;
		}

//		if (state==-1) {pass_values(0,0); state=0;}
//

		if (state==1)
		{
	        sampletime++;
			cnt1++;
			if (cnt1>256) 
			{
				cnt1=0;time++; 
//				if (time==1) { redraw=1; InvalidateRect(displayWnd,NULL,FALSE); }
				

				if ((time==gametime-7)&& m_audio) { MCIWndStop(m_audio);}
				if ((time==gametime-5)&& m_audio2) { MCIWndPlay(m_audio2);}

				if (time>=gametime) 
				{ 
					pump1=(int)((float)PUMP1GAME/(1.0f+ratio)); 
					if (pump1>PUMP1TIME) pump1=PUMP1TIME;
					pump2=(int)((float)PUMP2GAME/(1.0f+ratio)*ratio); 
					if (pump2>PUMP2TIME) pump2=PUMP2TIME;
					time=0; 

					pass_values(0,16+32);
					state=2; 

					redraw=1; InvalidateRect(displayWnd,NULL,FALSE);
				}
			}

			
			alpha+=input1;
			beta+=input2;
			ratio=(preset_min+preset_max)/2 + ((alpha/beta)-baseline ) * (preset_max-preset_min)/2;
			if (ratio<preset_min) ratio=preset_min;
			if (ratio>preset_max) ratio=preset_max;
				

		}
		if (state==2)
		{

			cnt1++;
			if (cnt1==pump1)
			{
				if (pump1<pump2) pass_values(0,32); else pass_values(0,0); 
			}

			if (cnt1==pump2)
			{
				if (pump2<pump1) pass_values(0,16); else pass_values(0,0);
			}
			
			if (cnt1>2200) 
			{	
				state=0;
				redraw=1; InvalidateRect(displayWnd,NULL,FALSE);

				cnt1=0;

				if (m_audio) { MCIWndStop(m_audio); MCIWndDestroy(m_audio); m_audio=NULL;}
				if (m_audio2) { MCIWndStop(m_audio2); MCIWndDestroy(m_audio2); m_audio2=NULL;}
			}

		}

		//if (!TIMING.draw_update) InvalidateRect(displayWnd,NULL,FALSE);


	  }


MARTINIOBJ::~MARTINIOBJ()
	  {
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
		if (g_hbm1) DeleteObject(g_hbm1); 
		if (g_hbm2) DeleteObject(g_hbm2); 
		if (g_hbm3) DeleteObject(g_hbm3); 
		if (m_audio) {	MCIWndStop(m_audio); 	MCIWndDestroy(m_audio); }

	  }  

