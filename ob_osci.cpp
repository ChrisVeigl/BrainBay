/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
  
  MODULE: OB_OSCI.CPP:  contains functions for the Oscilloscope-Object
  Author: Chris Veigl

  The OSCI-Object has its own window, it uses GDI-drawings. 
  draw_osci: draws all used channels of the oscilloscope
  OsciboxDlgHandler: processes the events for the OSCI-toolbox window
  OsciWndHandler: processes the events for the OSCI-drawing window

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_osci.h"


void draw_osci(OSCIOBJ * st)
{
	PAINTSTRUCT ps;
	POINT psav;
	HDC hdc;
	RECT rect,txtpos,clr;
	TCHAR szdata[50];
	float  pbuf[MAX_EEG_CHANNELS][LEN_PIXELBUFFER];
	float oscitime,sec_total;
	int t,i,d,x,y,top,np,count,space;
	int  half_chn_height,channel_mid,upper,lower;
	int setnew,bcount;
    int ypos;
	char tmp[20];
	struct INPORTStruct * act;
	

    hdc = BeginPaint (st->displayWnd, &ps);
	GetClientRect(st->displayWnd, &rect);
    top=(WORD) rect.top+2;

	SetBkColor(hdc,st->bkcol);
	count=st->inports-1;
	if (count<=1) count=1;

    if (st->showgrid) st->drawstart=50; else st->drawstart=35;
	if (st->showseconds)
	{
		 space=rect.right-st->drawstart;
		 sec_total= (float)space*st->timer/PACKETSPERSECOND;
		  
		 st->periods = (int)(sec_total/st->showseconds);
		 if (st->periods>0)  st->drawend= st->showseconds*st->periods*PACKETSPERSECOND/st->timer+st->drawstart;
		 else { st->showseconds=0; st->drawend=rect.right-30; }
	}
	else st->drawend=rect.right-30;

	if (!st->group)
	{
		ypos=(int)(rect.bottom/count/2);		
		half_chn_height=(int) ((rect.bottom-50)/count/2);
	}
	else 
	{ 
		ypos=(int)(rect.bottom/2);	
		half_chn_height=(int) ((rect.bottom-50)/2);
	}

    SelectObject(hdc, DRAW.scaleFont);
	if ((st->signal_pos>=st->drawend)||(st->redraw)||(st->signal_pos<st->drawstart)) 
	{      
  	   st->redraw=FALSE;
		
  	   FillRect(hdc, &rect, st->bkbrush);
	
	   if (st->group)
	   {
		   int actxpos=90;
		   HBRUSH actbrush;
		   for (i=0;i<count;i++)
		   {
			    SetTextColor(hdc,st->captcol);
			    SetBkMode(hdc,TRANSPARENT);

				actxpos=90+150*i;

				sprintf(szdata, "%s",st->in_ports[i].in_desc);
				txtpos.left=actxpos;txtpos.right=actxpos;txtpos.bottom=5;txtpos.top=5;
	   			DrawText(hdc, szdata, -1, &txtpos, DT_CALCRECT);
				// actxpos=txtpos.right+40;
			    DrawText(hdc, szdata, -1, &txtpos, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				
				actbrush=CreateSolidBrush(st->sigcol[i]);
				txtpos.left-=15; txtpos.right=txtpos.left+10;
				FillRect(hdc,&txtpos,actbrush);
				DeleteObject(actbrush);
		   }
	   }

	   for (i=0;i<count;i++)
	   {
		 SetTextColor(hdc,st->captcol);
	     st->prev_pixel[i]=-1;
		 channel_mid=(i*2+1)*ypos; 
		 upper=channel_mid-half_chn_height;lower=channel_mid+half_chn_height;
		 
		 if (!st->group)
		 {
			sprintf(szdata, "%d:%s",i+1,st->in_ports[i].in_desc);
			if ((st->showseconds) &&(i==0)) ExtTextOut( hdc, st->drawstart+3,upper+10, 0, &rect,szdata, strlen(szdata), NULL ) ;
			else ExtTextOut( hdc, st->drawstart+3,upper+3, 0, &rect,szdata, strlen(szdata), NULL ) ;
		 }


		 if (st->showgrid)
		 {
			SelectObject (hdc, st->gridpen);
			MoveToEx(hdc,st->drawstart-5, lower,NULL);	
			LineTo(hdc,st->drawend, lower);
			MoveToEx(hdc,st->drawstart-5, upper,NULL);	
			LineTo(hdc,st->drawend, upper);

			if (st->group) 
			{	act=&(st->in_ports[st->groupselect]);
				SetTextColor(hdc,st->sigcol[st->groupselect]);
			}
			else act=&(st->in_ports[i]);

			MoveToEx(hdc,st->drawstart, upper,NULL);	
			LineTo(hdc,st->drawstart, lower);
		    MoveToEx(hdc,st->drawstart-5, channel_mid+half_chn_height/2,NULL);	
		    LineTo(hdc,st->drawstart, channel_mid+half_chn_height/2);
		    MoveToEx(hdc,st->drawstart-5, channel_mid-half_chn_height/2,NULL);	
		    LineTo(hdc,st->drawstart, channel_mid-half_chn_height/2);
		    MoveToEx(hdc,st->drawstart-5, channel_mid,NULL);	
		    LineTo(hdc,st->drawstart+5, channel_mid);

			sprintf(szdata,"%.2f ",(act->in_max/(st->gain/100.0f)+act->in_min/(st->gain/100.0f))/2);
			if (strcmp(act->in_dim,"none")) strcat(szdata,act->in_dim); 
			ExtTextOut( hdc, 2,channel_mid-5, 0, &rect,szdata, strlen(szdata), NULL ) ;

			sprintf(szdata, "%.2f ",act->in_max / (st->gain/100.0f));if (strcmp(act->in_dim,"none")) strcat(szdata,act->in_dim); 
			ExtTextOut( hdc, 2,channel_mid-half_chn_height, 0, &rect,szdata, strlen(szdata), NULL ) ;
			sprintf(szdata, "%.2f ",act->in_min / (st->gain/100.0f));if (strcmp(act->in_dim,"none")) strcat(szdata,act->in_dim); 
			ExtTextOut( hdc, 2,channel_mid+half_chn_height-10, 0, &rect,szdata, strlen(szdata), NULL ) ;

		 }
		 if (st->showline)
		 {
			SelectObject (hdc, st->gridpen);
//			SelectObject (hdc, DRAW.pen_ltblue);
			MoveToEx(hdc,st->drawstart-5, channel_mid,NULL);	
			LineTo(hdc,st->drawend, channel_mid);
		 }
		 if (st->group) i=count;
	   }

	   if ((st->showseconds)&&(!st->gradual))
	   {
		  int actpos=st->drawstart;

		  if (st->periods>0)
		  {
		    SelectObject (hdc, st->captpen);
		    SetTextColor(hdc,st->captcol);
		    SetBkColor(hdc,st->bkcol);
		    SetBkMode(hdc,OPAQUE);
		  
		    if (st->signal_pos>=st->drawend)
			{ 
				st->laststamp+=st->periods*st->showseconds;
				st->signal_pos=0;
			}
		   
		    oscitime=st->laststamp;	
		    for (t=0;t<=st->periods;t++)
			{
				print_time(tmp,oscitime,1);		
				ExtTextOut(hdc, actpos-15,ypos-half_chn_height-12, 0, &rect,tmp, strlen(tmp), NULL ) ;		

				for (i=0;i<count;i++)          // Draw lines
				{
					channel_mid=ypos*(i*2+1);  
					MoveToEx(hdc,actpos, channel_mid-half_chn_height,NULL);	
					LineTo(hdc,actpos, channel_mid+half_chn_height);
					if (st->group) i=count;
				}
				oscitime+=st->showseconds;	
				actpos+=st->showseconds*PACKETSPERSECOND/st->timer;
			}
//	  	    st->drawend=st->showseconds*st->periods*PACKETSPERSECOND/st->timer+st->drawstart;
		  }
		  
	   }
	   

	   if((st->signal_pos<st->drawstart)||(st->signal_pos>=st->drawend)) 
		   st->signal_pos=st->drawstart;
	  // else  // repaint memory
	   {
		   for (i=0;i<count;i++)
		   {
		   
  			 if (!st->group) channel_mid=ypos*(i*2+1);  
			 else channel_mid=ypos;  
			 act=&(st->in_ports[i]);
			 SelectObject (hdc, st->drawpen[i]);

			 setnew=TRUE;
			 if (!st->gradual)
			   for (t=1;t<st->signal_pos-st->drawstart;t++)
			   {

				/*	if ((st->secpos>-1)&&(st->showseconds))
				{
					SelectObject (hdc, st->captpen);
					MoveToEx(hdc,x+st->secpos, channel_mid-half_chn_height,NULL);	
					LineTo(hdc,x+st->secpos, channel_mid+half_chn_height);
				} 
				*/
		
				d=st->mempos-t;
				while (d<0) d+=1024;

				if (act)
				y=channel_mid-(int)size_value (act->in_min,act->in_max,st->pixelmem[i][d]*st->gain/100.0f,(float)-half_chn_height,(float)half_chn_height,0);
				else y=channel_mid;

				if (y<top) y=top;
				if (st->pixelmem[i][d]!=INVALID_VALUE) 
				{ 
					if (setnew) { MoveToEx(hdc,st->signal_pos-t, y,NULL); setnew=FALSE;}
					LineTo(hdc,st->signal_pos-t, y); 
				} else setnew=TRUE;
			   }
			 else
		       for (t=1;t<st->drawend-st->drawstart;t++)
			   {

				/*	if ((st->secpos>-1)&&(st->showseconds))
				{
					SelectObject (hdc, st->captpen);
					MoveToEx(hdc,x+st->secpos, channel_mid-half_chn_height,NULL);	
					LineTo(hdc,x+st->secpos, channel_mid+half_chn_height);
				} 
				*/
		
				d=st->mempos-t;
				while (d<0) d+=1024;

				if (act)
				y=channel_mid-(int)size_value (act->in_min,act->in_max,st->pixelmem[i][d]*st->gain/100.0f,(float)-half_chn_height,(float)half_chn_height,0);
				else y=channel_mid;

				if (y<top) y=top;
				if (st->pixelmem[i][d]!=INVALID_VALUE) 
				{ 
					if (st->signal_pos-t+1>st->drawstart)
					{
					  if (setnew) { MoveToEx(hdc,st->signal_pos-t, y,NULL); setnew=FALSE;}
					  LineTo(hdc,st->signal_pos-t, y);
					}
					else
					{
					   if (st->signal_pos-t+1==st->drawstart) { setnew=true;bcount=1;}
					   if (setnew) { MoveToEx(hdc,st->drawend-bcount, y,NULL); 
					   setnew=FALSE;}
					   LineTo(hdc,st->drawend-bcount, y); bcount++;
					}

				} else setnew=TRUE;
			  }

		  }
	   }
	}

//	wsprintf(tmp,"%d,%d",st->drawend,st->signal_pos);
//   ExtTextOut(hdc, 0,0, 0, &rect,tmp, strlen(tmp), NULL ) ;

    np=st->newpixels;
	if (np>0)
	{
		int line_x=st->drawstart+st->mysec*PACKETSPERSECOND/st->timer;
		if (st->mysec>=st->showseconds*st->periods) st->mysec=0;
		st->inc_mysec=0;

	  st->newpixels=0;
	    for (i=0;i<count;i++)
	      for (t=0;t<np;t++)
		     pbuf[i][t]=st->pixelbuffer[i][t];


	  x=st->signal_pos;
	  for (i=0;i<count;i++)
	  {
		if (!st->group) channel_mid=ypos*(i*2+1);  
		else channel_mid=ypos;  
	    act=&(st->in_ports[i]);

		SelectObject (hdc, st->drawpen[i]);
		if (st->gradual)
		{
			if ((!st->group)||(i==0))
			{
				clr.left=x;
				clr.right=x+np;
				clr.top=channel_mid-half_chn_height+1;
				clr.bottom=channel_mid+half_chn_height;
	  			FillRect(hdc, &clr, st->bkbrush);

				if (st->showline)
				{
					SelectObject (hdc, st->gridpen);
					MoveToEx(hdc,x, channel_mid,NULL);	
					LineTo(hdc,x+np, channel_mid);
				}
			}
		}

		if (st->prev_pixel[i]>=0) { MoveToEx(hdc,x-1, st->prev_pixel[i],NULL);	setnew=FALSE;}
		else setnew=TRUE;

		for (t=0; t<np; t++)
		{ 
			if (act) y=channel_mid-(int)size_value (act->in_min,act->in_max,pbuf[i][t]*st->gain/100.0f,(float)-half_chn_height,(float)half_chn_height,0);
			else y=channel_mid;
			st->pixelmem[i][st->mempos+t]=pbuf[i][t];

			if ((st->gradual)&&(st->showseconds))
			{
				if (x+t==line_x)
				{
				    SelectObject (hdc, st->captpen);
					MoveToEx(hdc,line_x, channel_mid-half_chn_height,&psav);	
					LineTo(hdc,line_x, channel_mid+half_chn_height);
					st->inc_mysec=1;
					SelectObject (hdc, st->drawpen[i]);
					MoveToEx(hdc,psav.x, psav.y,NULL);	

					SelectObject (hdc, st->captpen);
					if (i==0)
					{
						SetTextColor(hdc,st->captcol);
						SetBkColor(hdc,st->bkcol);
						SetBkMode(hdc,OPAQUE);
		  
						print_time(tmp,(float)st->mysec_total,1);		
						ExtTextOut(hdc, line_x-15,channel_mid-half_chn_height-12, 0, &rect,tmp, strlen(tmp), NULL ) ;
					}
				}
			}

			if (y<top) y=top;
			if (pbuf[i][t]!=INVALID_VALUE) 
			{ 
				SelectObject (hdc, st->drawpen[i]);
				if (setnew) { MoveToEx(hdc,x+t, y,NULL); setnew=FALSE;}
				LineTo(hdc,x+t, y);
			} else setnew=TRUE;
		}
		if (setnew) st->prev_pixel[i]=-1; else st->prev_pixel[i]=y;

	  }
	  if (st->inc_mysec) {
		  st->mysec+=st->showseconds;
		  st->mysec_total+=st->showseconds;
	  }
	  st->mempos+=np; if (st->mempos>=1024) st->mempos=0;
	  
	}
	st->signal_pos+=np;
	EndPaint( st->displayWnd, &ps );
}




LRESULT CALLBACK OsciboxDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int actsig=0;
	int t;
	char temp[20];
	
	OSCIOBJ * st;
	
	st = (OSCIOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_OSCI)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE; // |SIF_POS;
				lpsi.nMin=1; lpsi.nMax=4000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_OSCISPEEDBAR),SB_CTL,&lpsi,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_OSCISPEEDBAR), SB_CTL,st->timer,TRUE);		
				SetDlgItemInt(hDlg, IDC_OSCISPEED, st->timer,0);

				lpsi.nMin=1; lpsi.nMax=5;
				SetScrollInfo(GetDlgItem(hDlg,IDC_SIZEBAR),SB_CTL,&lpsi,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_SIZEBAR), SB_CTL,st->sigsize[0],TRUE);		
				
				lpsi.nMin=1; lpsi.nMax=10000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_OSCIGAINBAR),SB_CTL,&lpsi,TRUE);
				SetScrollPos(GetDlgItem(hDlg,IDC_OSCIGAINBAR), SB_CTL,st->gain,TRUE);		
				SetDlgItemInt(hDlg, IDC_OSCIGAIN, st->gain,0);

				SetDlgItemInt(hDlg,IDC_SECONDS,st->showseconds,0);

				CheckDlgButton(hDlg,IDC_GROUP,st->group);
				CheckDlgButton(hDlg,IDC_GRID,st->showgrid);
				CheckDlgButton(hDlg,IDC_LINE,st->showline);
				CheckDlgButton(hDlg,IDC_WITHIN,st->within);
				CheckDlgButton(hDlg,IDC_GRADUAL,st->gradual);

				for (t=0;t<st->inports;t++)
				{
					sprintf(temp,"Chn %d",t+1);
					SendDlgItemMessage(hDlg, IDC_SIGCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) temp ) ;
				}
				SetDlgItemText(hDlg,IDC_SIGCOMBO,"Chn 1");

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_GROUP:  st->redraw=TRUE;st->group=IsDlgButtonChecked(hDlg, IDC_GROUP);
							InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_GRID:  st->redraw=TRUE;st->showgrid=IsDlgButtonChecked(hDlg, IDC_GRID);
							InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_LINE:  st->redraw=TRUE;st->showline=IsDlgButtonChecked(hDlg, IDC_LINE);
							InvalidateRect(st->displayWnd,NULL,TRUE);
				break;

			case IDC_SHOWSECONDS:
				if ((int)GetDlgItemInt(hDlg, IDC_SHOWSECONDS,NULL,1)!=st->showseconds)
				{
						st->showseconds=GetDlgItemInt(hDlg, IDC_SHOWSECONDS,NULL,0);
						st->signal_pos=0;
						st->timercount=0;
						st->newpixels=0;
						st->laststamp=(float)TIMING.packetcounter/(float)PACKETSPERSECOND;
						st->redraw=TRUE;
						InvalidateRect(st->displayWnd,NULL,TRUE);
				}
				break;
			case IDC_WITHIN:  st->within=IsDlgButtonChecked(hDlg, IDC_WITHIN);
							InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_GRADUAL:  st->gradual=IsDlgButtonChecked(hDlg, IDC_GRADUAL);
							InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			
			case IDC_SIGCOMBO:  
				actsig=SendDlgItemMessage(hDlg, IDC_SIGCOMBO, CB_GETCURSEL, 0, 0 ) ;
				SetScrollPos(GetDlgItem(hDlg,IDC_SIZEBAR), SB_CTL,st->sigsize[actsig],TRUE);		
				InvalidateRect(hDlg,NULL,FALSE);
				break;
 			case IDC_BKCOLOR:
				st->bkcol=select_color(hDlg,st->bkcol);
				DeleteObject(st->bkbrush);
				st->bkbrush=CreateSolidBrush(st->bkcol);
				st->redraw=TRUE;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
 			case IDC_SIGCOLOR:
				st->sigcol[actsig]=select_color(hDlg,st->sigcol[actsig]);
				DeleteObject(st->drawpen[actsig]);
				st->drawpen[actsig]=CreatePen(PS_SOLID,1,st->sigcol[actsig]);
				st->redraw=TRUE;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_GRIDCOLOR:
				st->gridcol=select_color(hDlg,st->gridcol);
				DeleteObject(st->gridpen);
				st->gridpen=CreatePen(PS_SOLID,1,st->gridcol);
				st->redraw=TRUE;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;
			case IDC_CAPTCOLOR:
				st->captcol=select_color(hDlg,st->captcol);
				DeleteObject(st->captpen);
				st->captpen=CreatePen(PS_SOLID,1,st->captcol);
				st->redraw=TRUE;
				InvalidateRect(hDlg,NULL,FALSE);
				InvalidateRect(st->displayWnd,NULL,TRUE);
				break;

			}
			break;
			
		case WM_HSCROLL:
		{
			int nNewPos; 
	
		    if ((nNewPos=get_scrollpos(wParam, lParam))>=0)
		    {
			  if (lParam == (long) GetDlgItem(hDlg,IDC_OSCISPEEDBAR))
			  {  
				  SetDlgItemInt(hDlg, IDC_OSCISPEED,nNewPos,0);
				  st->timer=nNewPos;
				  st->signal_pos=0;
				  st->timercount=0;
				  st->newpixels=0;
				  st->laststamp=(float)TIMING.packetcounter/(float)PACKETSPERSECOND;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_OSCIGAINBAR))
			  {  
				  SetDlgItemInt(hDlg, IDC_OSCIGAIN,nNewPos,0);
				  st->gain=nNewPos;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_SIZEBAR))
			  {  
				  //SetDlgItemInt(hDlg, IDC_SIZE,nNewPos,0);
				  st->sigsize[actsig]=nNewPos;
				  DeleteObject(st->drawpen[actsig]);
				  st->drawpen[actsig]=CreatePen(PS_SOLID,nNewPos,st->sigcol[actsig]);
				  
			  }
			  st->redraw=TRUE;
 			  InvalidateRect(st->displayWnd,NULL,TRUE);

			}
		
		} break;
		
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_BKCOLOR),st->bkcol);
			color_button(GetDlgItem(hDlg,IDC_SIGCOLOR),st->sigcol[actsig]);
			color_button(GetDlgItem(hDlg,IDC_GRIDCOLOR),st->gridcol);
			color_button(GetDlgItem(hDlg,IDC_CAPTCOLOR),st->captcol);
			break;
		case WM_SIZE:  
		case WM_MOVE: update_toolbox_position(hDlg); 
		break;

		return TRUE;
	}
    return FALSE;
}

LRESULT CALLBACK OsciWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	
	OSCIOBJ * st;
	
	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_OSCI)
		{	st=(OSCIOBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

    if (st==NULL)	return DefWindowProc( hWnd, message, wParam, lParam );

	
	switch( message ) 
	{	

		case WM_DESTROY:
		 break;
		case WM_KEYDOWN:
			  SendMessage(ghWndMain, message,wParam,lParam);
		break;

		case WM_MOUSEACTIVATE:
	 	    st->redraw=TRUE;
			InvalidateRect(hWnd,NULL,FALSE);
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
				  
			   mindist=10000;minpoint=-1;
				  
			   if (st->group)
			   {
				  for (i=0;i<st->inports-1;i++)
				  {
					  
				       actdist = ((90+i*150.0f)-actx)*((90+i*150.0f)-actx)
						   + (10.0f-acty)*(10.0f-acty);
				
					   if (actdist<mindist) { mindist=actdist; minpoint=i; }

				  }

				  if (mindist<300) 
				  {
					  st->groupselect=minpoint;
					  st->redraw=1;
				  }
			   }
			   if ((actx>st->drawstart)&&(actx<st->drawend)&&(acty>30))
			   {
				    long ppos= (long) st->laststamp*PACKETSPERSECOND 
									+ (actx-st->drawstart)*st->timer;

					int pos = get_sliderpos(ppos);
					if (ppos!=GLOBAL.session_end)
					{
						SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,pos);
						GLOBAL.session_start=ppos;
						update_statusinfo();
					}
			   }
			}
 	 	case WM_RBUTTONDOWN:
			{
			   int actx,acty;

			   actx=(int)LOWORD(lParam);
			   acty=(int)HIWORD(lParam);

			   if ((actx>st->drawstart)&&(actx<st->drawend)&&(acty>30))
			   {
				    long ppos= (long) st->laststamp*PACKETSPERSECOND 
									+ (actx-st->drawstart)*st->timer;

					int pos = get_sliderpos(ppos);
					if (ppos!=GLOBAL.session_start)
					{
						SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,pos);
						GLOBAL.session_end=ppos;
						update_statusinfo();
					}
			   }
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
			  st->redraw=TRUE;
			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
		case WM_ERASEBKGND:
			st->redraw=TRUE;
			return 0;

		case WM_PAINT:
			
			  draw_osci(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}




//
//  Object Implementation
//


OSCIOBJ::OSCIOBJ(int num) : BASE_CL()	
	  {
	    outports = 0;
		inports = 1;
		
		newpixels=0;signal_pos=0;drawstart=25;drawend=20000;
		groupselect=0;
		mysec=0; mysec_total=0;
		inc_mysec=0;


		for (t=0;t<MAX_EEG_CHANNELS;t++) input[t]=0.0f; //512.0;
		for (t=0;t<MAX_PORTS;t++) sprintf(in_ports[t].in_name,"Chn%d",t+1);
		timer=1;gain=100; group=0; mempos=0; gradual=0;
		timercount=0;
		laststamp=(float)TIMING.packetcounter/(float)PACKETSPERSECOND;
		showgrid=TRUE;showline=TRUE;showseconds=TRUE;within=TRUE;
		bkcol=RGB(255,255,255);
		bkbrush=CreateSolidBrush(bkcol);

		gridcol=RGB(0,0,100);
		gridpen=CreatePen(PS_SOLID,1,gridcol);

		captcol=RGB(100,100,150);
		captpen=CreatePen(PS_SOLID,1,captcol);
		redraw=TRUE;
		


		for (t=0;t<MAX_EEG_CHANNELS;t++) 
		{
			sigcol[t]=RGB(150,0,0); sigsize[t]=1;
			drawpen[t]=CreatePen(PS_SOLID,1,sigcol[t]);
		}
		left=30;right=800;top=210;bottom=450;
		if(!(displayWnd=CreateWindow("Osci_Class", "Oscilloscope", WS_CLIPSIBLINGS | WS_CAPTION  | WS_THICKFRAME | WS_CHILD ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL))) 
		    report_error("can't create Oscillocope Window");
		else {SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }
		InvalidateRect(displayWnd, NULL, TRUE);
	  }

	  void OSCIOBJ::update_inports(void)
	  {
		  int x; //,i;
		  inports=count_inports(this);
		
		  if (inports>MAX_EEG_CHANNELS) inports=MAX_EEG_CHANNELS;
		  for(x=inports;x<MAX_EEG_CHANNELS;x++)
			  in_ports[x].get_range=1;

		  for(x=0;x<inports;x++)
		  {
			 input[x]=0;inputcount[x]=0;
		  }

		  height=CON_START+inports*CON_HEIGHT+5;
		  // signal_pos=0;
	      if (displayWnd) InvalidateRect(displayWnd,NULL,FALSE);
 	      InvalidateRect(ghWndDesign,NULL,TRUE);
	  }

	  void OSCIOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_OSCIBOX, ghWndStatusbox, (DLGPROC)OsciboxDlgHandler));
	  }
	  void OSCIOBJ::load(HANDLE hFile) 
	  { 
		float temp;
		char pname[20];

		newpixels=0;inports=6;signal_pos=0;drawend=20000;
		for (t=0;t<MAX_EEG_CHANNELS;t++)
		{input[t]=0.0f; inputcount[t]=0;}
		timercount=0; laststamp=0;

		load_object_basics(this);
		load_property("grid",P_INT,&showgrid);
		load_property("line",P_INT,&showline);
		load_property("gain",P_INT,&gain);
		load_property("seconds",P_INT,&showseconds);
		load_property("top",P_INT,&top);
		load_property("left",P_INT,&left);
		load_property("right",P_INT,&right);
		load_property("bottom",P_INT,&bottom);
  	    load_property("drawinterval",P_INT,&timer);
		load_property("within",P_INT,&within);
  	    load_property("group",P_INT,&group);
  	    load_property("gradual",P_INT,&gradual);
  	    

		temp=RGB(255,255,255);
	    load_property("background",P_FLOAT,&temp);
		bkcol=(COLORREF)temp;
		DeleteObject(bkbrush);
		bkbrush=CreateSolidBrush(bkcol);

		temp=RGB(0,0,100);
	    load_property("gridcol",P_FLOAT,&temp);
		gridcol=(COLORREF)temp;
		DeleteObject(gridpen);
		gridpen=CreatePen(PS_SOLID,1,gridcol);

		temp=RGB(100,100,150);
	    load_property("captcol",P_FLOAT,&temp);
		captcol=(COLORREF)temp;
		DeleteObject(captpen);
		captpen=CreatePen(PS_SOLID,1,captcol);

		for (t=0;t<inports;t++)
		{ 
			sprintf(pname,"signal%d",t+1);
			temp=RGB(150,0,0);
	        load_property(pname,P_FLOAT,&temp);
			sprintf(pname,"sigsize%d",t+1);
	        load_property(pname,P_INT,&sigsize[t]);
			sigcol[t]=(COLORREF)temp;
			DeleteObject(drawpen[t]);
			drawpen[t]=CreatePen(PS_SOLID,sigsize[t],sigcol[t]);
		}
		height=CON_START+inports*CON_HEIGHT+5;
		MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
	  }
	
	  void OSCIOBJ::save(HANDLE hFile) 
	  {	  
		  float temp;
		  char pname[20];

		  save_object_basics(hFile, this);
		  save_property(hFile,"grid",P_INT,&showgrid);
		  save_property(hFile,"line",P_INT,&showline);
		  save_property(hFile,"gain",P_INT,&gain);
		  save_property(hFile,"seconds",P_INT,&showseconds);
		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);
		  save_property(hFile,"drawinterval",P_INT,&timer);
		  save_property(hFile,"within",P_INT,&within);
		  save_property(hFile,"group",P_INT,&group);
		  save_property(hFile,"gradual",P_INT,&gradual);
		  
		  temp=(float)bkcol;
		  save_property(hFile,"background",P_FLOAT,&temp);
		  temp=(float)gridcol;
		  save_property(hFile,"gridcol",P_FLOAT,&temp);
		  temp=(float)captcol;
		  save_property(hFile,"captcol",P_FLOAT,&temp);

		  for (t=0;t<inports;t++)
		  { 
			sprintf(pname,"signal%d",t+1);
			temp=(float)sigcol[t];
	        save_property(hFile,pname,P_FLOAT,&temp);
			sprintf(pname,"sigsize%d",t+1);
	        save_property(hFile,pname,P_INT,&sigsize[t]);

		  }

	  }

	  void OSCIOBJ::incoming_data(int port, float value) 
	  {
		  if (inputcount[port]==0)
			input[port] = value;
		  else 	input[port] += value;
	      inputcount[port]++;
	  }

	  void OSCIOBJ::session_reset(void) 
	  {
  			signal_pos=0;
			timercount=0;
			newpixels=0;
			laststamp=0;
			mysec=0; mysec_total=0;
			inc_mysec=0;
			for (t=0;t<MAX_EEG_CHANNELS;t++)
			{input[t]=0.0f; inputcount[t]=0;}
			InvalidateRect(displayWnd,NULL,TRUE);
	  }

	  void OSCIOBJ::session_pos(long pos) 
	  {
  			signal_pos=0;
			timercount=0;
			newpixels=0;
			laststamp=(float)pos/(float)PACKETSPERSECOND;
			mysec_total=(int) laststamp;
			mysec=0;
			InvalidateRect(displayWnd,NULL,TRUE);
	  }

	  void OSCIOBJ::work(void) 
	  {
		float v;
		int t,z;

		timercount++;
		if(timercount>=timer)
		{
			timercount=0;
			z=inports-1; if(z<0) z=0;
			for (t=0;t<z;t++)
			{
				if (input[t]!=INVALID_VALUE)
				{
					if (inputcount[t]>0)
					{
						v=input[t]/inputcount[t];
						inputcount[t]=0;
					} else v=input[t];
					if (within)
					{
						if (v>in_ports[t].in_max) v=in_ports[t].in_max;
						if (v<in_ports[t].in_min) v=in_ports[t].in_min;
					}
					pixelbuffer[t][newpixels]=v;
				}
			    else pixelbuffer[t][newpixels]=INVALID_VALUE;
			}
			if (newpixels<499) newpixels++;
			InvalidateRect(displayWnd,NULL,FALSE);
		}


		//if (newpixels>4/timer) InvalidateRect(displayWnd,NULL,FALSE); 
		
		//if (!TIMING.draw_update) InvalidateRect(displayWnd,NULL,FALSE); 
		
	  }


OSCIOBJ::~OSCIOBJ()
	  {
		DeleteObject(bkbrush);
		DeleteObject(gridpen);
		DeleteObject(captpen);
		for (t=0;t<MAX_EEG_CHANNELS;t++) DeleteObject(drawpen[t]);
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
	  }  
