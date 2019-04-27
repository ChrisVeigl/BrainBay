/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_BALLGAME.CPP:  contains functions for the BALLGAME-Generator-Object
  Author: Chris Veigl

  The BALLGAME-Object provides an Arkanoid-like Ballgame.
  The position of the bar is controlled by the input-port value.
  the drawing in done in a seperate window using GDI-functions

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_ballgame.h"
#include <wingdi.h> 
 
void draw_ballgame(BALLGAMEOBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[40];
	RECT rect;
	HBRUSH actbrush;
	int ballx,bally,racketx,rackets;

	
	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);


	ballx=(int) ((rect.right/100.0f)*st->xpos);
	bally=(int) ((rect.bottom/100.0f)*st->ypos);
	racketx=(int) ((rect.right/100.0f)*st->rpos);
	rackets=(int) ((rect.right/100.0f)*st->racket);

	actbrush=CreateSolidBrush(RGB(180,0,0));
    
   	SelectObject (hdc, DRAW.pen_white);		
   	SelectObject (hdc, DRAW.brush_white);
	Rectangle(hdc,0,0,rect.right,rect.bottom);

    SelectObject(hdc, DRAW.mediumFont);
	SetTextColor(hdc,RGB(255,0,0));

	SelectObject (hdc, DRAW.pen_red);	
	
    sprintf(szdata, "Points: %d                    Best: %d",st->points,st->best);
    ExtTextOut( hdc, rect.left+10,10, 0, &rect,szdata, strlen(szdata), NULL ) ;

   	SelectObject (hdc, DRAW.brush_orange);
	Rectangle(hdc,ballx,bally,ballx+20,bally+20);

   	SelectObject (hdc, DRAW.brush_blue);
	Rectangle(hdc,racketx,rect.bottom-20,racketx+rackets,rect.bottom);


	DeleteObject(actbrush);
	EndPaint( st->displayWnd, &ps );
}



	
LRESULT CALLBACK BALLGAMEDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	BALLGAMEOBJ * st;
	
	st = (BALLGAMEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_BALLGAME)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE; // |SIF_POS;
				
				lpsi.nMin=1; lpsi.nMax=100;
				SetScrollInfo(GetDlgItem(hDlg,IDC_SPEEDBAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_RACKETBAR),SB_CTL,&lpsi,TRUE);
				SetDlgItemInt(hDlg, IDC_SPEED,st->speed,0);
				SetDlgItemInt(hDlg, IDC_RACKET,st->racket,0);
				SetScrollPos(GetDlgItem(hDlg,IDC_SPEEDBAR), SB_CTL,st->speed,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_RACKETBAR), SB_CTL,st->racket,TRUE);

				CheckDlgButton(hDlg, IDC_MIDDLE, st->reset_middle);

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 
				case IDC_RESETBEST: 
						st->best=0;
					break;
				case IDC_MIDDLE: 
						st->reset_middle=IsDlgButtonChecked(hDlg,IDC_MIDDLE);
					break;
			}
			return TRUE;
			
		case WM_HSCROLL:
		{
			int nNewPos;

			nNewPos=get_scrollpos(wParam,lParam);

			if (lParam == (long) GetDlgItem(hDlg,IDC_SPEEDBAR))  { SetDlgItemInt(hDlg, IDC_SPEED,nNewPos,0); st->speed=nNewPos;}
			if (lParam == (long) GetDlgItem(hDlg,IDC_RACKETBAR)) {  SetDlgItemInt(hDlg, IDC_RACKET,nNewPos,0);st->racket=nNewPos;}
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


LRESULT CALLBACK BallgameWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	BALLGAMEOBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_BALLGAME)
		{	st=(BALLGAMEOBJ *)objects[t];
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
			}
			InvalidateRect(hWnd,NULL,TRUE);
			break;
		case WM_PAINT:
			draw_ballgame(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}






//
//  Object Implementation
//


BALLGAMEOBJ::BALLGAMEOBJ(int num) : BASE_CL()
	  {
		outports = 1;
		inports = 1;
		strcpy(in_ports[0].in_name,"in");

	    out_ports[0].get_range=-1;
		strcpy(out_ports[0].out_name,"Fx");
		strcpy(out_ports[0].out_dim,"none");
		strcpy(out_ports[0].out_desc,"Ballgame-Sounds");
	    out_ports[0].out_max=127;
        out_ports[0].out_min=0;
		reset_middle=0;
		
		speed=50;racket=50;
		xpos=50.0f;ypos=50.0f;xspeed=0.0f; yspeed=0.2f;
		rpos=50.0f;
		state=0;
		input=0.0f;
		adjust_r= 120.0f-racket/2;
		points=0;best=0;
		left=500;right=800;top=50;bottom=400;

        
		if(!(displayWnd=CreateWindow("Ballgame_Class", "Ballgame", WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		    report_error("can't create Ballgame Window");
		else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }
		InvalidateRect(displayWnd, NULL, TRUE);
	  }
	  void BALLGAMEOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_BALLGAMEBOX, ghWndStatusbox, (DLGPROC)BALLGAMEDlgHandler));
	  }
	  void BALLGAMEOBJ::load(HANDLE hFile) 
	  {
		  load_object_basics(this);
		  load_property("speed",P_INT,&speed);
		  load_property("racket",P_INT,&racket);
		  load_property("best",P_INT,&best);
		  load_property("middle",P_INT,&reset_middle);
		  load_property("top",P_INT,&top);
		  load_property("left",P_INT,&left);
		  load_property("right",P_INT,&right);
		  load_property("bottom",P_INT,&bottom);
		  MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
	  }
		
	  void BALLGAMEOBJ::save(HANDLE hFile) 
	  {	  
	 	  save_object_basics(hFile, this);
		  save_property(hFile,"speed",P_INT,&speed);
		  save_property(hFile,"racket",P_INT,&racket);
		  save_property(hFile,"best",P_INT,&best);
		  save_property(hFile,"middle",P_INT,&reset_middle);
  		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);

	  }


	  void BALLGAMEOBJ::incoming_data(int port, float value)
      {
        input = size_value(in_ports[0].in_min,in_ports[0].in_max,value,0.0f,100.0f,0);
      }
        

	  void BALLGAMEOBJ::work(void) 
	  {
		if (GLOBAL.fly) return;
		if (state==0)
		{
			rpos=input+adjust_r;
			xpos+=xspeed*speed/50.0f;
			if ((xpos>100)||(xpos<0)) {xspeed=-xspeed; xpos+=xspeed*speed/50.0f;
			pass_values(0,30);}
			ypos+=yspeed*speed/50.0f;if (ypos<0) {yspeed=-yspeed; ypos+=yspeed*speed/50.0f;
			pass_values(0,30);}
			if (ypos>93)
			{
				if ((xpos>(rpos-5))&&(xpos<rpos+racket))
				{
					yspeed=-yspeed; 
//					xspeed=((rand()%50)-25)/100.0f;
					xspeed=(xpos-(rpos+racket/2))/50.0f;
					if (xspeed>0.3f) xspeed=0.3f;if (xspeed<-0.3f) xspeed=-0.3f;
					xspeed+=((rand()%50)-25)/500.0f;
					ypos+=yspeed*speed/50.0f;
					points++;
					if (points>best) { best=points; pass_values(0,100); }
					else pass_values(0,70);
				}
				else
				{
					state=1;points=0;
					pass_values(0,20);
				}
			}
		}
		if (state>0)
		{
			state++; 
			if (state==512) 
			{ 
			   state=0; xpos=50; ypos=50; 
			   xspeed=0; yspeed=0.2f; 
			   if (reset_middle) adjust_r= -input+50-racket/2;
			}
		}

		if ((displayWnd)&&(!TIMING.draw_update)) 
		{
		  InvalidateRect(displayWnd,NULL,FALSE);
		}
	  }


BALLGAMEOBJ::~BALLGAMEOBJ()
	  {
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
	  }  

