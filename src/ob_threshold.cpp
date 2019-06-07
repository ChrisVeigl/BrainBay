/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_THRESHOLD.CPP:  contains functions for the Threshold-Generator-Object

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
#include "ob_threshold.h"
#include <wingdi.h> 
 
void draw_meter(THRESHOLDOBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[20];
	RECT rect;
	int  act,width,mid,height,bottom,y1,y2;
	HBRUSH actbrush,bkbrush;
	HPEN bkpen;
	float min, max;

	
	GetClientRect(st->displayWnd, &rect);
	//width=rect.right>>1;
	width=(int)((float)rect.right/200.0f*st->barsize);
	mid=rect.right>>1;
    	
	min=st->in_ports[0].in_min;
	max=st->in_ports[0].in_max;

	hdc = BeginPaint (st->displayWnd, &ps);
	bottom=rect.bottom-15;
	height=(bottom-rect.top-20);

    act=bottom-(int)(size_value(min,max,st->current_value,0.0f,(float)height,0));
	y1 =bottom-(int)(size_value(min,max,(float)st->lower_limit,0.0f,(float)height,0));
	y2 =bottom-(int)(size_value(min,max,(float)st->upper_limit,0.0f,(float)height,0));

	if (act>bottom) act=bottom;
	if (act<bottom-height) act=bottom-height;

	actbrush=CreateSolidBrush(st->color);
    bkbrush=CreateSolidBrush(st->bkcolor);
	bkpen =CreatePen (PS_SOLID,1,st->bkcolor);
	if (st->redraw) FillRect(hdc, &rect,bkbrush);
    
   	SelectObject (hdc, bkpen);		
   	SelectObject (hdc, bkbrush);
	if (st->adapt_lower_limit)
	{
		Rectangle(hdc,15,st->old_y1,50,st->old_y1+15);
		MoveToEx(hdc,5, st->old_y1,NULL);	
		LineTo(hdc,rect.right, st->old_y1);
	}

	if (st->adapt_upper_limit)
	{
		Rectangle(hdc,15,st->old_y2,50,st->old_y2+15);
		MoveToEx(hdc,5, st->old_y2,NULL);	
		LineTo(hdc,rect.right, st->old_y2);
	}

	if ((st->last_value<act)&&(!st->redraw)) 
	{
			SelectObject (hdc, bkbrush);		
			Rectangle(hdc,mid-width,st->last_value,mid+width,act);
	}
	else
	{
			SelectObject (hdc, actbrush);
			Rectangle(hdc,mid-width,bottom,mid+width,act);
	}

	if ((st->fontsize>0))
	{
		RECT txtpos;int x;
		SelectObject(hdc, st->font);
		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, st->fontbkcolor);
		SetTextColor(hdc,st->fontcolor);

		//wsprintf(szdata, "%d   ",(int)(st->current_value+0.5f));
		//ExtTextOut( hdc, rect.right-40,10, 0, &rect,szdata, strlen(szdata), NULL ) ;

		txtpos.left=5;txtpos.right=50;
		txtpos.top=0;txtpos.bottom=0;
		if ((st->baseline) && (st->firstadapt)) 
			sprintf(szdata, "  ");
		else sprintf(szdata, " %.2f ",st->lower_limit);
		DrawText(hdc, szdata, -1, &txtpos, DT_CALCRECT);
		x=txtpos.bottom;
		txtpos.top=y1-x;txtpos.bottom=txtpos.top+x;
		DrawText(hdc, szdata, -1, &txtpos, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	

		txtpos.left=5;txtpos.right=50;
		txtpos.top=0;txtpos.bottom=0;
		if ((st->baseline) && (st->firstadapt)) 
			sprintf(szdata, " get baseline ");
		else sprintf(szdata, " %.2f ",st->upper_limit);
		DrawText(hdc, szdata, -1, &txtpos, DT_CALCRECT);
//		txtpos.top=y2+1;txtpos.bottom+=y2+1;

		x=txtpos.bottom;
		txtpos.top=y2-x;txtpos.bottom=txtpos.top+x;


		DrawText(hdc, szdata, -1, &txtpos, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		
	}
	
	SelectObject (hdc, DRAW.pen_ltblue);		
	MoveToEx(hdc,5, y1,NULL);	
	LineTo(hdc,rect.right, y1);
	st->old_y1=y1;

	MoveToEx(hdc,5, y2,NULL);	
	LineTo(hdc,rect.right, y2);
	st->old_y2=y2;

	DeleteObject(actbrush);
	DeleteObject(bkbrush);
	DeleteObject(bkpen);
	st->last_value=act;
	st->redraw=0;
	EndPaint( st->displayWnd, &ps );
}



	

void apply_threshold(HWND hDlg, THRESHOLDOBJ * st)
{
  static int updating=FALSE;


  if (!updating)
  { 
	updating=TRUE;
	
	if (st->interval_len != GetDlgItemInt(hDlg, IDC_AVGINTERVAL,NULL,0)) {
		st->interval_len= GetDlgItemInt(hDlg, IDC_AVGINTERVAL,NULL,0);
		st->clear_averagers();
	}
	st->signal_gain= GetDlgItemInt(hDlg, IDC_AVGGAIN,NULL,TRUE);
	st->adapt_lower_limit=GetDlgItemInt(hDlg, IDC_ADAPT_LOWERLIMIT,NULL,TRUE);
	st->adapt_upper_limit=GetDlgItemInt(hDlg, IDC_ADAPT_UPPERLIMIT,NULL,TRUE);
	st->adapt_interval=GetDlgItemInt(hDlg, IDC_ADAPTINTERVAL,NULL,TRUE);
	st->barsize=GetDlgItemInt(hDlg, IDC_BARSIZE,NULL,TRUE);
	st->fontsize=GetDlgItemInt(hDlg, IDC_FONTSIZE,NULL,TRUE);
	if (st->font) DeleteObject(st->font);
	if (!(st->font = CreateFont(-MulDiv(st->fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
		report_error("Font creation failed!");

	st->play_interval=0;
	GetDlgItemText(hDlg, IDC_METERCAPTION, st->wndcaption, 50);
	SetWindowText(st->displayWnd,st->wndcaption);
	st->op=IsDlgButtonChecked(hDlg,IDC_AND);
	updating=FALSE;
  }

}




	
LRESULT CALLBACK ThresholdDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	char temp[100];
	THRESHOLDOBJ * st;
	int x;
	
	st = (THRESHOLDOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_THRESHOLD)) return(FALSE);

	for (x=0;x<GLOBAL.objects;x++) if (objects[x]==actobject) 
	{ char tmp[10]; wsprintf(tmp,"%d",x); 
	  //SendDlgItemMessage(ghWndStatusbox,IDC_LISTE,LB_ADDSTRING, 0, (LPARAM)(tmp)); 
	}
					

	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				SendDlgItemMessage(hDlg, IDC_UPPERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Not used") ;
				SendDlgItemMessage(hDlg, IDC_UPPERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Percent of min/max range" ) ;
				SendDlgItemMessage(hDlg, IDC_UPPERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Quantile of samples" ) ;
				SendDlgItemMessage(hDlg, IDC_UPPERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Percent of average value" ) ;
				SendDlgItemMessage( hDlg, IDC_UPPERMODE_COMBO, CB_SETCURSEL, (WPARAM) (st->adapt_upper_mode), 0L ) ;

				SendDlgItemMessage(hDlg, IDC_LOWERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Not used") ;
				SendDlgItemMessage(hDlg, IDC_LOWERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Percent of min/max range" ) ;
				SendDlgItemMessage(hDlg, IDC_LOWERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Quantile of samples" ) ;
				SendDlgItemMessage(hDlg, IDC_LOWERMODE_COMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Percent of average value" ) ;
				SendDlgItemMessage( hDlg, IDC_LOWERMODE_COMBO, CB_SETCURSEL, (WPARAM) (st->adapt_lower_mode), 0L ) ;

				SetDlgItemInt(hDlg,IDC_TRUE_VALUE,(int)st->numericTrueValue,1);
				SetDlgItemInt(hDlg,IDC_FALSE_VALUE,(int)st->numericFalseValue,1);
				SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Input-Value") ;
				SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "NumericValue (below)" ) ;
				SendDlgItemMessage( hDlg, IDC_TRUECOMBO, CB_SETCURSEL, (WPARAM) (st->trueMode), 0L ) ;

				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "INVALID_VALUE (-32767)" ) ;
				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "NumericValue (below)") ;
				SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "No Output" ) ;
				SendDlgItemMessage( hDlg, IDC_FALSECOMBO, CB_SETCURSEL, (WPARAM) (st->falseMode), 0L ) ;
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE; // |SIF_POS;
				
				lpsi.nMin=1; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_AVGINTERVALBAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_ADAPTINTERVALBAR),SB_CTL,&lpsi,TRUE);

				lpsi.nMin=-1000; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_AVGGAINBAR),SB_CTL,&lpsi,TRUE);

				lpsi.nMin=0; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_AVGFROMBAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_AVGTOBAR),SB_CTL,&lpsi,TRUE);

				lpsi.nMin=0; lpsi.nMax=100;
				SetScrollInfo(GetDlgItem(hDlg,IDC_BARSIZEBAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_FONTSIZEBAR),SB_CTL,&lpsi,TRUE);

				init=TRUE;
				SetDlgItemInt(hDlg, IDC_AVGINTERVAL, st->interval_len,TRUE);
				SetDlgItemInt(hDlg, IDC_ADAPTINTERVAL, st->adapt_interval,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_AVGINTERVALBAR), SB_CTL,st->interval_len,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_ADAPTINTERVALBAR), SB_CTL,st->adapt_interval,TRUE);

				SetDlgItemInt(hDlg, IDC_AVGGAIN, st->signal_gain,1);
				SetScrollPos(GetDlgItem(hDlg,IDC_AVGGAINBAR), SB_CTL,st->signal_gain,TRUE);

				SetDlgItemInt(hDlg, IDC_BARSIZE, st->barsize,0);
				SetScrollPos(GetDlgItem(hDlg,IDC_BARSIZEBAR), SB_CTL,st->barsize,TRUE);
				SetDlgItemInt(hDlg, IDC_FONTSIZE, st->fontsize,0);
				SetScrollPos(GetDlgItem(hDlg,IDC_FONTSIZEBAR), SB_CTL,st->fontsize,TRUE);

				sprintf(temp,"%.2f",st->lower_limit);
				SetDlgItemText(hDlg, IDC_AVGFROM, temp);
				SetScrollPos(GetDlgItem(hDlg,IDC_AVGFROMBAR), SB_CTL,(int) size_value(st->in_ports[0].in_min,st->in_ports[0].in_max, st->lower_limit ,0.0f,1000.0f,0),TRUE);

				sprintf(temp,"%.2f",st->upper_limit);
				SetDlgItemText(hDlg, IDC_AVGTO, temp);
				SetScrollPos(GetDlgItem(hDlg,IDC_AVGTOBAR), SB_CTL,(int) size_value(st->in_ports[0].in_min,st->in_ports[0].in_max, st->upper_limit ,0.0f,1000.0f,0),TRUE);

				SetDlgItemInt(hDlg, IDC_ADAPT_LOWERLIMIT, st->adapt_lower_limit,TRUE);
				SetDlgItemInt(hDlg, IDC_ADAPT_UPPERLIMIT, st->adapt_upper_limit,TRUE);

				SetDlgItemText(hDlg, IDC_METERCAPTION, st->wndcaption);

				CheckDlgButton(hDlg, IDC_AND,st->op);
				CheckDlgButton(hDlg, IDC_OR,!st->op);
				CheckDlgButton(hDlg, IDC_SHOWMETER,st->showmeter);
				CheckDlgButton(hDlg, IDC_RISING,st->rising);
				CheckDlgButton(hDlg, IDC_FALLING,st->falling);
				// CheckDlgButton(hDlg, IDC_USE_MEDIAN,st->usemedian);
				CheckDlgButton(hDlg, IDC_BASELINE,st->baseline);
				if (st->baseline) SetDlgItemText(hDlg,IDC_INTERVALUNIT,"Seconds");
				else SetDlgItemText(hDlg,IDC_INTERVALUNIT,"Samples");

				init=FALSE;
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_AND: st->op=TRUE; break;
			case IDC_OR:  st->op=FALSE; break;
			case IDC_RISING: st->rising=IsDlgButtonChecked(hDlg,IDC_RISING); break;
			case IDC_FALLING: st->falling=IsDlgButtonChecked(hDlg,IDC_FALLING); break;
			// case IDC_USE_MEDIAN: st->usemedian=IsDlgButtonChecked(hDlg,IDC_USE_MEDIAN); break;
			case IDC_BASELINE: st->baseline=IsDlgButtonChecked(hDlg,IDC_BASELINE); 
				if (st->baseline) SetDlgItemText(hDlg,IDC_INTERVALUNIT,"Seconds");
				else SetDlgItemText(hDlg,IDC_INTERVALUNIT,"Samples");
				break;
			case IDC_UPPERMODE_COMBO:
				st->adapt_upper_mode=SendDlgItemMessage(hDlg, IDC_UPPERMODE_COMBO, CB_GETCURSEL, 0, 0 ) ;
				break;
			case IDC_LOWERMODE_COMBO:
				st->adapt_lower_mode=SendDlgItemMessage(hDlg, IDC_LOWERMODE_COMBO, CB_GETCURSEL, 0, 0 ) ;
				break;
			case IDC_TRUECOMBO:
				st->trueMode=SendDlgItemMessage(hDlg, IDC_TRUECOMBO, CB_GETCURSEL, 0, 0 ) ;
				break;
			case IDC_FALSECOMBO:
				st->falseMode=SendDlgItemMessage(hDlg, IDC_FALSECOMBO, CB_GETCURSEL, 0, 0 ) ;
				break;
			case IDC_TRUE_VALUE:
				GetDlgItemText(hDlg, IDC_TRUE_VALUE, temp,sizeof(temp));
				st->numericTrueValue=(float)atoi(temp);
                break;
			case IDC_FALSE_VALUE:
				GetDlgItemText(hDlg, IDC_FALSE_VALUE, temp,sizeof(temp));
				st->numericFalseValue=(float)atoi(temp);
                break;
			case IDC_SELECTCOLOR:
				st->color=select_color(hDlg,st->color);
				st->redraw=1;
				InvalidateRect(hDlg,NULL,FALSE);
				break;
			case IDC_BKCOLOR:
				st->bkcolor=select_color(hDlg,st->bkcolor);
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

			case IDC_AVGFROM:
				GetDlgItemText(hDlg, IDC_AVGFROM,temp,sizeof(temp));
				if (!init)	
					 st->lower_limit=(float)atof(temp);
				break;
			case IDC_AVGTO:
				GetDlgItemText(hDlg, IDC_AVGTO,temp,sizeof(temp));
				if (!init) 
					st->upper_limit=(float)atof(temp);
				break;
			case IDC_AVGINTERVAL:
			case IDC_ADAPTINTERVAL:
			case IDC_AVGGAIN:
			case IDC_ADAPT_LOWERLIMIT:
			case IDC_ADAPT_UPPERLIMIT:
			case IDC_METERCAPTION:
				 if (!init) 
					 apply_threshold(hDlg, st);
					break;

			
			case IDC_SHOWMETER:
				{  int i;
				   i=IsDlgButtonChecked(hDlg,IDC_SHOWMETER);
				   if ((st->showmeter)&&(!i)&&(st->displayWnd))  { DestroyWindow(st->displayWnd); st->displayWnd=NULL; }
				   if ((!st->showmeter)&&(i)) 
				   {  
					   if(!(st->displayWnd=CreateWindow("Meter_Class", st->wndcaption, WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,st->left, st->top, st->right-st->left, st->bottom-st->top, ghWndMain, NULL, hInst, NULL)))
							report_error("can't create Meter Window");
					   else { st->last_value=0; ShowWindow( st->displayWnd, TRUE ); UpdateWindow( st->displayWnd ); }
				   }
				   st->showmeter=i;
				}
				break;

			}
			return TRUE;
			
		case WM_HSCROLL:
		{
			int nNewPos; float x;

			nNewPos=get_scrollpos(wParam,lParam);

			if (lParam == (long) GetDlgItem(hDlg,IDC_AVGINTERVALBAR))  SetDlgItemInt(hDlg, IDC_AVGINTERVAL,nNewPos,0);
			if (lParam == (long) GetDlgItem(hDlg,IDC_ADAPTINTERVALBAR))  SetDlgItemInt(hDlg, IDC_ADAPTINTERVAL,nNewPos,0);
			if (lParam == (long) GetDlgItem(hDlg,IDC_AVGGAINBAR))  SetDlgItemInt(hDlg, IDC_AVGGAIN,nNewPos,TRUE);
			if (lParam == (long) GetDlgItem(hDlg,IDC_BARSIZEBAR))  SetDlgItemInt(hDlg, IDC_BARSIZE,nNewPos,TRUE);
			if (lParam == (long) GetDlgItem(hDlg,IDC_FONTSIZEBAR))  SetDlgItemInt(hDlg, IDC_FONTSIZE,nNewPos,TRUE);
			if (lParam == (long) GetDlgItem(hDlg,IDC_AVGFROMBAR))
			{  
				  x=size_value(0.0f,1000.0f,(float)nNewPos,st->in_ports[0].in_min,st->in_ports[0].in_max,0);
				  sprintf(temp,"%.2f",x);
				  SetDlgItemText(hDlg, IDC_AVGFROM, temp);

			}
			if (lParam == (long) GetDlgItem(hDlg,IDC_AVGTOBAR))  
			{  
				  x=size_value(0.0f,1000.0f,(float)nNewPos,st->in_ports[0].in_min,st->in_ports[0].in_max,0);
			   	  sprintf(temp,"%.2f",x);
				  SetDlgItemText(hDlg, IDC_AVGTO, temp);
			}
			apply_threshold(hDlg,st);
			if (st->displayWnd) {st->last_value=INVALID_VALUE; st->redraw=1; InvalidateRect(st->displayWnd,NULL,TRUE);}
		
		}	break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_SELECTCOLOR),st->color);
			color_button(GetDlgItem(hDlg,IDC_BKCOLOR),st->bkcolor);
			color_button(GetDlgItem(hDlg,IDC_FONTCOL),st->fontcolor);
			color_button(GetDlgItem(hDlg,IDC_FONTBKCOL),st->fontbkcolor);
		break;
	}
    return FALSE;
}


LRESULT CALLBACK MeterWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	THRESHOLDOBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_THRESHOLD)
		{	st=(THRESHOLDOBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

	if (st==NULL) return DefWindowProc( hWnd, message, wParam, lParam );
	
	switch( message ) 
	{	case WM_DESTROY:
		 break;
		case WM_KEYDOWN:
			// printf("keydown: %ld, %ld\n",lParam,wParam);
		    switch(wParam) {
				case KEY_UP:
				  // step=st->gain/50; 
				  //if (step<1) step=1;
  				  st->lower_limit+=1;
	   			  InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
				case KEY_DOWN:
				  // step=st->gain/50; 
				  // if (step<1) step=1;
  				  st->lower_limit-=1;
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
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE) | WS_SIZEBOX);
			  }
			  InvalidateRect(hWnd,NULL,TRUE);
			}
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


THRESHOLDOBJ::THRESHOLDOBJ(int num) : BASE_CL()
	  {
		outports = 3;
		inports = 1;
		width=65;
		strcpy(in_ports[0].in_name,"in");
		strcpy(out_ports[0].out_name,"out");
		strcpy(out_ports[1].out_name,"lower");
		strcpy(out_ports[2].out_name,"upper");

		play_interval=0;lower_limit=1; upper_limit=512;	signal_gain=100;
		range_min=0; range_max=0;
		strcpy (wndcaption,"Meter");
		numericTrueValue=1;
		numericFalseValue=0;
		trueMode=0;
		falseMode=0;

		last_value=0;
		for (accupos=0;accupos<ACCULEN;accupos++) accu[accupos]=0; 
		accupos=0;redraw=1;firstadapt=1;
		interval_len=1; op=TRUE;
		threshold_avg_sum=0;baseline=0;
		showmeter=1;rising=0;falling=0; adapt_lower_limit=0;adapt_upper_limit=0;adapt_interval=200;
		adapt_upper_mode=THRESHOLD_ADAPTMODE_NONE;
		adapt_lower_mode=THRESHOLD_ADAPTMODE_NONE;
		input=0;current_value=0;
		fontsize=10; barsize=30;
		left=510;right=550;top=20;bottom=400;
		color=RGB(0,0,100);bkcolor=RGB(255,255,255);fontcolor=0;fontbkcolor=RGB(255,255,255);
        
        clear_averagers();

		if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
			report_error("Font creation failed!");

        
		if(!(displayWnd=CreateWindow("Meter_Class", wndcaption, WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		    report_error("can't create Meter Window");
		else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }
		InvalidateRect(displayWnd, NULL, TRUE);
	  }


	void THRESHOLDOBJ::session_start(void)
	{
		clear_averagers();
	}
	
	void THRESHOLDOBJ::session_reset(void)
	{
		firstadapt=1;
		clear_averagers();
	}
	void THRESHOLDOBJ::session_pos(long pos)
	{
		clear_averagers();
	}

	  void THRESHOLDOBJ::make_dialog(void)
	  {
	 		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_THRESHOLDBOX, ghWndStatusbox, (DLGPROC)ThresholdDlgHandler));
	  }
	  void THRESHOLDOBJ::load(HANDLE hFile) 
	  {
		float temp;
		load_object_basics(this);
		load_property("play_interval",P_INT,&play_interval);
		load_property("interval_len",P_INT,&interval_len);
		load_property("gain",P_INT,&signal_gain);
		load_property("from-input",P_FLOAT,&lower_limit);
		load_property("to-input",P_FLOAT,&upper_limit);
		load_property("and/or",P_INT,&op);
		load_property("show-meter",P_INT,&showmeter);
		load_property("only-rising",P_INT,&rising);
		load_property("only-falling",P_INT,&falling);
		load_property("baseline",P_INT,&baseline);
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
		load_property("top",P_INT,&top);
		load_property("left",P_INT,&left);
		load_property("right",P_INT,&right);
		load_property("bottom",P_INT,&bottom);

		if (! load_property("adapt_lower_limit",P_INT,&adapt_lower_limit))
		load_property("bigadapt",P_INT,&adapt_lower_limit);     // legacy support

		if (!load_property("adapt_upper_limit",P_INT,&adapt_upper_limit))
		load_property("smalladapt",P_INT,&adapt_upper_limit);  // legacy support

		load_property("adapt_upper_mode",P_INT,&adapt_upper_mode);
		if (!load_property("adapt_lower_mode",P_INT,&adapt_lower_mode))
		{   //legacy support
			int  usemedian;
			load_property("usemedian",P_INT,&usemedian);
			if (adapt_upper_limit==0) adapt_upper_mode=THRESHOLD_ADAPTMODE_NONE;
			else {
				adapt_upper_limit=100-adapt_upper_limit;    //  upper quantile was given in % from top value in earlier versions ... 
				if (adapt_upper_limit<1) adapt_upper_limit=1;
				adapt_upper_mode= usemedian ? THRESHOLD_ADAPTMODE_QUANTILE : THRESHOLD_ADAPTMODE_AVERAGE;
			}

			if (adapt_lower_limit==0) adapt_lower_mode=THRESHOLD_ADAPTMODE_NONE;
			else adapt_lower_mode= usemedian ? THRESHOLD_ADAPTMODE_QUANTILE : THRESHOLD_ADAPTMODE_AVERAGE;
		}

		load_property("adaptinterval",P_INT,&adapt_interval);
		load_property("fontsize",P_INT,&fontsize);
		if (fontsize)
		{
			if (font) DeleteObject(font);
		    if (!(font = CreateFont(-MulDiv(fontsize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
					report_error("Font creation failed!");
		}
		load_property("barsize",P_INT,&barsize);
		load_property("wndcaption",P_STRING,wndcaption);

		int tmp;
		load_property("trueMode",P_INT,&trueMode);
		load_property("falseMode",P_INT,&falseMode);
		tmp=1;
		load_property("numericTrueValue",P_INT,&tmp);
		numericTrueValue=tmp;
		tmp=0;
		load_property("numericFalseValue",P_INT,&tmp);
		numericFalseValue=tmp;

		if (!showmeter)
		{
			if (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
		}
		else 
		{ 
			MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE); 

			if (GLOBAL.locksession) {
	 			SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				//SetWindowLong(displayWnd, GWL_STYLE, 0);
			} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
		    SetWindowText(displayWnd,wndcaption);
			InvalidateRect (displayWnd, NULL, TRUE);
		}
		session_reset();
		redraw=1;
	  }
		
	  void THRESHOLDOBJ::save(HANDLE hFile) 
	  {	  
		float temp;
	 	save_object_basics(hFile, this);
		save_property(hFile,"play_interval",P_INT,&play_interval);
		save_property(hFile,"interval_len",P_INT,&interval_len);
		save_property(hFile,"gain",P_INT,&signal_gain);
		save_property(hFile,"from-input",P_FLOAT,&lower_limit);
		save_property(hFile,"to-input",P_FLOAT,&upper_limit);
		save_property(hFile,"and/or",P_INT,&op);
		save_property(hFile,"show-meter",P_INT,&showmeter);
		save_property(hFile,"only-rising",P_INT,&rising);
		save_property(hFile,"only-falling",P_INT,&falling);
		//save_property(hFile,"usemedian",P_INT,&usemedian);
		save_property(hFile,"baseline",P_INT,&baseline);
		temp=(float)color;
		save_property(hFile,"color",P_FLOAT,&temp);
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
		save_property(hFile,"adapt_lower_limit",P_INT,&adapt_lower_limit);
		save_property(hFile,"adapt_upper_limit",P_INT,&adapt_upper_limit);
		save_property(hFile,"adapt_lower_mode",P_INT,&adapt_lower_mode);
		save_property(hFile,"adapt_upper_mode",P_INT,&adapt_upper_mode);
		save_property(hFile,"adaptinterval",P_INT,&adapt_interval);
		save_property(hFile,"barsize",P_INT,&barsize);
		save_property(hFile,"fontsize",P_INT,&fontsize);
		save_property(hFile,"wndcaption",P_STRING,wndcaption);

		int tmp;
		save_property(hFile,"trueMode",P_INT,&trueMode);
		save_property(hFile,"falseMode",P_INT,&falseMode);
		tmp=numericTrueValue;
		save_property(hFile,"numericTrueValue",P_INT,&tmp);
		tmp=numericFalseValue;
		save_property(hFile,"numericFalseValue",P_INT,&tmp);
	  }


  	  void THRESHOLDOBJ::update_inports(void)
      {
		  InvalidateRect(displayWnd,NULL,TRUE);
	  }

	  void THRESHOLDOBJ::incoming_data(int port, float value)
      {
  		  input=value;
      }

	  void THRESHOLDOBJ::work(void) 
	  {
		// signal preparation: gain
	    float gain_value=(float)((input)*signal_gain/100.0);

		// signal preparation: averaging
		accupos++;
		if (accupos>=interval_len) accupos=0; 
		interval_sum-=accu[accupos];
		accu[accupos]=gain_value;
		interval_sum+=gain_value;
		current_value=interval_sum/interval_len;
		//printf("gain_value=%d, sum=%d, interval=%d, current_val=%d\n",(int)gain_value,(int)interval_sum,interval_len,(int)current_value);


		// update of adaptive threshold values (percentage and quantile)
		float compare_value=current_value;  // was: gain_value ... changed to averaged signal!
		threshold_avg_sum+=compare_value;
		int bucket_index=(int)size_value(in_ports[0].in_min,in_ports[0].in_max,compare_value,0,1024,1);
		buckets[bucket_index]++;

		if (adapt_num==0) { 
			range_min=current_value; 
			range_max=current_value;
		}
		else {
			if (current_value>range_max) range_max=current_value;
			if (current_value<range_min) range_min=current_value;
		}

        // extend interval from samples to seconds if baseline measurement
		long interval=adapt_interval;
		if (baseline) interval*=PACKETSPERSECOND;

		adapt_num++;
        if (adapt_num >= interval)   // enough samples: perform adaptive threshold calculation
        {
			int emptyBuckets;
			if ((adapt_lower_mode) || (adapt_upper_mode)) redraw=1;
			if ((adapt_lower_mode==THRESHOLD_ADAPTMODE_QUANTILE) || (adapt_upper_mode==THRESHOLD_ADAPTMODE_QUANTILE)) 
				emptyBuckets=1;
			else emptyBuckets=0;

			if ((!baseline) || (firstadapt)) {
				switch (adapt_lower_mode) {
					case THRESHOLD_ADAPTMODE_NONE:
						break;
					case THRESHOLD_ADAPTMODE_RANGE:
						lower_limit=size_value(0,100,adapt_lower_limit,range_min,range_max,0);
						break;
					case THRESHOLD_ADAPTMODE_QUANTILE:
						lower_limit=get_quantile(adapt_num * adapt_lower_limit / 100.0f);
						break;
					case THRESHOLD_ADAPTMODE_AVERAGE:
						lower_limit = threshold_avg_sum/interval*adapt_lower_limit/100.0f;
						break;
				}
				switch (adapt_upper_mode) {
					case THRESHOLD_ADAPTMODE_NONE:
						break;
					case THRESHOLD_ADAPTMODE_RANGE:
						upper_limit=size_value(0,100,adapt_upper_limit,range_min,range_max,0);
						break;
					case THRESHOLD_ADAPTMODE_QUANTILE:
						// TBD: check if compatibility break (quantile counted from top ?)
						upper_limit=get_quantile(adapt_num * adapt_upper_limit / 100.0f);
						break;
					case THRESHOLD_ADAPTMODE_AVERAGE:
						upper_limit = threshold_avg_sum/interval*adapt_upper_limit/100.0f;
						break;
				}
			}
			adapt_num=0;
			firstadapt=0;
			threshold_avg_sum=0;
			if (emptyBuckets) empty_buckets();
		}
		
		float output_value=current_value;
		if (rising&&(last_risingTest!=INVALID_VALUE)&&(last_risingTest>=current_value)) output_value=INVALID_VALUE;
		if (falling&&(last_risingTest!=INVALID_VALUE)&&(last_risingTest<=current_value)) output_value=INVALID_VALUE;
		if (op&&((current_value<lower_limit)||(current_value>upper_limit)))  output_value=INVALID_VALUE;
		if ((!op)&&((current_value<lower_limit)&&(current_value>upper_limit)))  output_value=INVALID_VALUE;

		if (baseline && firstadapt) output_value=INVALID_VALUE;

		if (output_value != INVALID_VALUE) {
			if (trueMode==0) pass_values(0,output_value);
			else if (trueMode==1) pass_values(0,numericTrueValue);
		} else {
			if (falseMode==0) pass_values(0,output_value);
			else if (falseMode==1) pass_values(0,numericFalseValue);
		}

		pass_values(1,lower_limit);
		pass_values(2,upper_limit);

		last_risingTest=current_value;
		
		if ((hDlg==ghWndToolbox)&&(!TIMING.dialog_update))
		{ 
			  char temp[100];

			  sprintf(temp,"%.2f",lower_limit);
			  SetDlgItemText(hDlg, IDC_AVGFROM, temp);
			  sprintf(temp,"%.2f",upper_limit);
			  SetDlgItemText(hDlg, IDC_AVGTO, temp);

			  if (adapt_upper_mode) SetScrollPos(GetDlgItem(hDlg,IDC_AVGTOBAR), SB_CTL,(int) size_value(in_ports[0].in_min,in_ports[0].in_max, upper_limit,0.0f,1000.0f,0),TRUE);
			  if (adapt_lower_mode) SetScrollPos(GetDlgItem(hDlg,IDC_AVGFROMBAR), SB_CTL,(int) size_value(in_ports[0].in_min,in_ports[0].in_max, lower_limit ,0.0f,1000.0f,0),TRUE);			  
		}

		if ((displayWnd)&&(!TIMING.draw_update)&&(!GLOBAL.fly))  InvalidateRect(displayWnd,NULL,FALSE);
	  
	  }

	  float THRESHOLDOBJ::get_quantile(int number_of_values) 
	  {
		int i, sum;
		for (i = 0, sum = 0; (i < 1024) && (sum < number_of_values); i++)
		//for (i = 1024, sum = 0; (i > 0) && (sum < number_or_values); i--)
		{
            sum += buckets[i];
		}
		printf("sum=%d, i=%d\n",sum,i);
		return(size_value(0.0f,1024.0f,(float)i,in_ports[0].in_min,in_ports[0].in_max,0));
	  }

      void THRESHOLDOBJ::empty_buckets()
      {
            for (int i = 0; i <= 1024; i++)	buckets[i] = 0;
            adapt_num = 0;
      }
      void THRESHOLDOBJ::clear_averagers()
      {
			for (int i=0;i<1000;i++) accu[i]=0;
			accupos=0;
			threshold_avg_sum=0;
			interval_sum=0;
  		    last_risingTest=INVALID_VALUE;
			empty_buckets();
	  }

THRESHOLDOBJ::~THRESHOLDOBJ()
	  {
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
	  }  
