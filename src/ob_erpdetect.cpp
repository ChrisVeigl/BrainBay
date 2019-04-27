/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_ERPDETECT.CPP:  contains functions for the Detector-Object
  Author: Chris Veigl

  The ERP-Detector - Object can record a signal and average a given number of trials
  (recording phase). Then, the signal stream is compared to this pattern and the 
  similarity (%) to the pattern is presented to the object's output port (detection phase)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_erpdetect.h"



LRESULT CALLBACK ErpdetectboxDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	int t,d;
	char szFileName[256];
	char sztemp[256];
	ERPDETECTOBJ * st;
	
	st = (ERPDETECTOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_ERPDETECT)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			    switch (st->mode) 
				{
					case 0: SetDlgItemText(hDlg,IDC_STATUS, "Detection Mode");
    						EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_LOADERP), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_SAVEERP), TRUE);
							break;
					case 1: SetDlgItemText(hDlg,IDC_STATUS, "recording Epochs");
    						EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_LOADERP), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_SAVEERP), FALSE);
							break;
				}
				SetDlgItemInt(hDlg,IDC_EPOCHS, st->epochs,0);
				SetDlgItemInt(hDlg,IDC_PRESTIM, st->prestim,0);
				SetDlgItemInt(hDlg,IDC_LENGTH, st->length,0);
				SetDlgItemText(hDlg,IDC_ERPFILENAME,st->erpfile);
				
				wsprintf(sztemp,"Signal %d of %d",st->actchn+1,st->channels);
				SetDlgItemText(hDlg,IDC_SIGCAPTION,sztemp);

				SendDlgItemMessage(hDlg, IDC_METHODCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Similarity [0-100%]" ) ;
				SendDlgItemMessage(hDlg, IDC_METHODCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Correlation [-1/1]" ) ;
				SendDlgItemMessage(hDlg, IDC_METHODCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "weighted Correlation [-1/1]" ) ;
				SendDlgItemMessage(hDlg, IDC_METHODCOMBO, CB_SETCURSEL, st->method, 0L ) ;


				return TRUE;
		case WM_CLOSE:		
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
			
					case IDC_STARTCAPTURE:
						st->mode=1;st->trigger=0;
						SetDlgItemText(hDlg,IDC_STATUS, "recording Epochs");
    						EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_LOADERP), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_SAVEERP), FALSE);
						st->trigger=0;
						st->current=0;
						for (t=0;t<ERPBUFLEN;t++) 
						  for (d=0;d<st->channels;d++)
						  {	st->epochbuf[d][t]=0;st->ringbuf[d][t]=0;}
						break;

					case IDC_STOPCAPTURE:
    						EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_LOADERP), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_SAVEERP), TRUE);

						st->mode=0;st->trigger=0;
						sprintf(sztemp,"stopped at trial %d.",st->current);
						SetDlgItemText(hDlg,IDC_STATUS, sztemp);
						break;
					case IDC_PREV:
						if (st->actchn>0) st->actchn--;
						wsprintf(sztemp,"Signal %d of %d",st->actchn+1,st->channels);
						SetDlgItemText(hDlg,IDC_SIGCAPTION,sztemp);
						InvalidateRect(hDlg,NULL,TRUE);
						break;
					case IDC_NEXT:
						if (st->actchn<st->channels-1) st->actchn++;
						wsprintf(sztemp,"Signal %d of %d",st->actchn+1,st->channels);
						SetDlgItemText(hDlg,IDC_SIGCAPTION,sztemp);
						InvalidateRect(hDlg,NULL,TRUE);
						break;

					case IDC_METHODCOMBO:
						st->method=SendDlgItemMessage(hDlg, IDC_METHODCOMBO, CB_GETCURSEL, 0, 0 ) ;
						break;

					case IDC_LOADERP:
						{
						  st->mode=0;st->trigger=0;st->current=0;

						  strcpy(szFileName,GLOBAL.resourcepath);
 					      strcat(szFileName,"PATTERNS\\*.erp");
						  if (open_file_dlg(hDlg, szFileName, FT_ERP, OPEN_LOAD)) 
						  {
							if (!load_from_file(szFileName, st->epochbuf, sizeof(st->epochbuf)))
								report_error("Could not load ERP-signature");
							else 
							{  
								char * fn = szFileName;
								while (strstr(fn,"\\")) fn=strstr(fn,"\\")+1;
								SetDlgItemText(hDlg, IDC_ERPFILENAME, fn);
							    strcpy(st->erpfile,fn);
							}
						  } else report_error("Could not load ERP-signature");
 						  InvalidateRect(hDlg,NULL,TRUE);
						}
						break;
					case IDC_SAVEERP:
						{
						strcpy(szFileName,GLOBAL.resourcepath);
						strcat(szFileName,"PATTERNS\\*.erp");

						if (open_file_dlg(hDlg,szFileName, FT_ERP, OPEN_SAVE)) 
							 if (!save_to_file(szFileName, st->epochbuf, sizeof(st->epochbuf)))
								report_error("Could not save ERP-signature");
							 else 
							 {	char * fn = szFileName;
								while (strstr(fn,"\\")) fn=strstr(fn,"\\")+1;
								strcpy(st->erpfile,fn);
							 }
						}
						break;

					case IDC_LENGTH:
						  d=GetDlgItemInt(hDlg,IDC_LENGTH, NULL, 0);
						  if ((d>0) && (d<ERPBUFLEN))
						  { st->length=d;
 						   InvalidateRect(hDlg,NULL,TRUE);
						  }
						break;
					case IDC_PRESTIM:
						  d=GetDlgItemInt(hDlg,IDC_PRESTIM, NULL, 0);
						  if ((d>=0) && (d<ERPBUFLEN))
						  { st->prestim=d;
 						   InvalidateRect(hDlg,NULL,TRUE);
						  }

						break;
					case IDC_EPOCHS:
						  d=GetDlgItemInt(hDlg,IDC_EPOCHS, NULL, 0);
						  if ((d>0) && (d<1000))
						  { st->epochs=d;
 						   InvalidateRect(hDlg,NULL,TRUE);
						  }

						break;
				}
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;
				RECT rect;
				HPEN	 tpen;
				HBRUSH	 tbrush;
				int height,mid,i;
				float fstep,vstep;
				char sztemp[20];

				hdc = BeginPaint (hDlg, &ps);
				GetClientRect(hDlg, &rect);
				tpen    = CreatePen (PS_SOLID,1,0);
				SelectObject (hdc, tpen);
				tbrush  = CreateSolidBrush(RGB(240,240,240));
				SelectObject(hdc,tbrush);
				rect.top+=65;
				rect.bottom -= 35;
				mid=(rect.bottom-rect.top)/2;
				height= rect.bottom-rect.top;
				Rectangle(hdc,rect.left,rect.top-1,rect.right,rect.bottom);
				Rectangle(hdc,rect.left,rect.bottom-(int)(height/2),rect.right,rect.bottom);
				
				DeleteObject(tbrush);
				DeleteObject(tpen);

				fstep=(float)(st->length)/(rect.right-rect.left);
				vstep=(rect.bottom-rect.top)/(st->in_ports[1].in_max-st->in_ports[1].in_min);

				for (i=0; i<st->channels; i++)
				{
					if (i==st->actchn) tpen = CreatePen (PS_SOLID,2,RGB(240,0,0));
					else tpen = CreatePen (PS_SOLID,1,RGB(240,0,0));
					SelectObject (hdc, tpen);

					
					MoveToEx(hdc,rect.left,rect.bottom-(int)((st->epochbuf[i][0]+st->in_ports[1].in_min)*vstep),NULL);
					for (t=0; t<(rect.right-rect.left); t++)
					{ 
						
						LineTo(hdc,rect.left+t,rect.bottom-mid-(int)((st->epochbuf[i][(int)(fstep*t)])*vstep ));
					}
					DeleteObject(tpen);
				}
				tpen = CreatePen (PS_SOLID,1,RGB(0,0,180));
				SelectObject (hdc, tpen);

				MoveToEx(hdc,rect.left+(int)(st->prestim*(rect.right-rect.left)/st->length),rect.top,NULL);
				LineTo(hdc,rect.left+(int)(st->prestim*(rect.right-rect.left)/st->length),rect.bottom);
				DeleteObject(tpen);

				SelectObject(hdc, DRAW.scaleFont);
				sprintf(sztemp,"Min: %d",(int)st->in_ports[st->actchn+1].in_min); 
				ExtTextOut( hdc, rect.left+2,rect.bottom-15, 0, &rect,sztemp, strlen(sztemp), NULL ) ;
				sprintf(sztemp,"Max: %d",(int)st->in_ports[st->actchn+1].in_max); 
				ExtTextOut( hdc, rect.left+2,rect.top+2, 0, &rect,sztemp, strlen(sztemp), NULL ) ;
				EndPaint(hDlg, &ps );
				}
				break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
    return FALSE;
}



//
//  Object Implementation
//


ERPDETECTOBJ::ERPDETECTOBJ(int num) : BASE_CL()	
	  {
		int t,i;
	    outports = 1;
		inports = 3;
		width=65;
		strcpy(in_ports[0].in_name,"mode");
		strcpy(in_ports[1].in_name,"trigger");
		strcpy(in_ports[2].in_name,"signal1");
		strcpy(in_ports[3].in_name,"signal2");
		strcpy(in_ports[4].in_name,"signal3");
		strcpy(in_ports[5].in_name,"signal4");
		strcpy(in_ports[6].in_name,"signal5");
		strcpy(in_ports[7].in_name,"signal6");

	    out_ports[0].get_range=-1;
		strcpy(out_ports[0].out_name,"out");
		strcpy(out_ports[0].out_dim,"%");
		strcpy(out_ports[0].out_desc,"ERP-recognition");
	    out_ports[0].out_max=100;
        out_ports[0].out_min=0;

		width=70;
		method=0;channels=0;actchn=0;

		epochs=10;current=epochs;length=512;mode=0;
		prestim=0;trigger=0;bufpos=0;recpos=0;
		strcpy(erpfile,"none");
		for (t=0;t<ERPBUFLEN;t++)
		  for (i=0;i<MAXCHN;i++) 
			{epochbuf[i][t]=0;ringbuf[i][t]=0;}
	  }


	  void ERPDETECTOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ERPDETECTBOX, ghWndStatusbox, (DLGPROC)ErpdetectboxDlgHandler));
	  }
	  
  	  void ERPDETECTOBJ::update_inports(void)
	  {
		  int x; //,i;
		  inports=count_inports(this);
		  if (inports<3) inports=3;
		
		  if (inports>MAXCHN+2) inports=MAXCHN+2;
		  for(x=inports;x<MAXCHN;x++)
			  in_ports[x].get_range=1;

		  height=CON_START+inports*CON_HEIGHT+5;
	      if (hDlg==ghWndToolbox) InvalidateRect(hDlg,NULL,TRUE);

		  channels=inports-3;
		  
		  
 	      InvalidateRect(ghWndDesign,NULL,TRUE);
	  }

	  void ERPDETECTOBJ::load(HANDLE hFile) 
	  {	
		  load_object_basics(this);

		  load_property("epochs",P_INT,&epochs);
		  load_property("length",P_INT,&length);
		  load_property("prestim",P_INT,&prestim);
		  load_property("method",P_INT,&method);
		  load_property("erpfile",P_STRING,erpfile);
		  current=epochs;
		if (strcmp(erpfile,"none"))
		{
			char szFileName[MAX_PATH] = "";
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PATTERNS\\");
			strcat(szFileName,erpfile);
			if (!load_from_file(szFileName, epochbuf, sizeof(epochbuf) ))
			{    
				report_error(szFileName);
				report_error("Could not load ERP-signature");
			}
		}

			actchn=0;
	//	update_inports();

	  }


	  void ERPDETECTOBJ::save(HANDLE hFile) 
	  {	  
		  save_object_basics(hFile, this);
	  	  save_property(hFile,"epochs",P_INT,&epochs);
		  save_property(hFile,"length",P_INT,&length);
		  save_property(hFile,"prestim",P_INT,&prestim);
  		  save_property(hFile,"method",P_INT,&method);
		  save_property(hFile,"erpfile",P_STRING,erpfile);
	  }

	  void ERPDETECTOBJ::incoming_data(int port, float value)  
	  {	
		if (port==0)
		{
			if((value!=INVALID_VALUE)&&(mode!=1))
			{
				int t,d;
				mode=1;	trigger=0;	current=0;
				for (t=0;t<ERPBUFLEN;t++) 
				  for (d=0;d<channels;d++)
				  {	epochbuf[d][t]=0;ringbuf[d][t]=0;}

				if (hDlg==ghWndToolbox) 
				{ 
    				EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), FALSE);
    				EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), TRUE);
    				SetDlgItemText(hDlg,IDC_STATUS,"Recording Mode.");
				}
			}
		}
		if (port==1)
		{
			if ((trigger==INVALID_VALUE) && (value==INVALID_VALUE)) trigger=0;
			if ((trigger==0)&&(value!=INVALID_VALUE))
			{
				trigger=1; recpos=0; 
				bufstart=bufpos-prestim; if (bufstart<0) bufstart+=ERPBUFLEN;
			}
		}
		if ((port>1)&&(port<MAXCHN+2))
		{
		  input[port-2]=value; 
		}
	  }

	  void ERPDETECTOBJ::work(void) 
	  {  	
		  int c;
			
		  if (!channels) {pass_values(0,INVALID_VALUE); return;}

		  for (c=0;c<channels;c++)  ringbuf[c][bufpos]=input[c];
		  bufpos++; if (bufpos>=ERPBUFLEN) bufpos=0;

		  if (mode==1)   // RECORDING MODE
		  {
			  if ((trigger==1)&&(current<epochs))
			  {
	   		     for (c=0;c<channels;c++) epochbuf[c][recpos]+=ringbuf[c][bufstart]/epochs;
				 bufstart++; if(bufstart>=ERPBUFLEN) bufstart=0;
				 recpos++; 

			     if (recpos==length)
				 {
					 trigger=INVALID_VALUE; 
					 current++; if (current==epochs) mode=0;  // all epochs summed
					 if (hDlg==ghWndToolbox) 
					 {
						if (current==epochs)
						{ 
    						EnableWindow(GetDlgItem(hDlg, IDC_STARTCAPTURE), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_STOPCAPTURE), FALSE);
    						EnableWindow(GetDlgItem(hDlg, IDC_LOADERP), TRUE);
    						EnableWindow(GetDlgItem(hDlg, IDC_SAVEERP), TRUE);
						 	SetDlgItemText(hDlg,IDC_STATUS,"Detection Mode.");
							UpdateWindow(hDlg);
						}
						
					 }
	
					 InvalidateRect(hDlg,NULL,TRUE);
				 }
			  }
			  if ((!TIMING.dialog_update)&&(hDlg==ghWndToolbox))
			  {
				  char sztemp[50];
				  if (trigger!=1)
				  {   sprintf(sztemp,   "waiting for trail %d",current+1);
				      SetDlgItemText(hDlg,IDC_STATUS,sztemp);
				  }
				  else 
				  {     sprintf(sztemp, "  recording trial %d",current+1); 
				        SetDlgItemText(hDlg,IDC_STATUS,sztemp);
				  }
			  }
  			  pass_values(0,INVALID_VALUE);

		  }
		  if (mode==0)   // DETECTION MODE
		  {
			  if (method==0)     // difference
			  {
				int t,i; double e=0.0f;
				i=bufpos;
				for (t=length-1;t>=0;t--)
				{
 				  if (i>0) i--; else i=ERPBUFLEN-1;
				  for (c=0;c<channels;c++)
				     e+=fabs(ringbuf[c][i]-epochbuf[c][t]);
				}
				e/=length;
				e/=(in_ports[1].in_max-in_ports[1].in_min);
				e/=channels;
				e*=100.0;  

				pass_values(0,100.0f-(float)e);
			  }
			  else if ((method==1)||(method==2))    // correlation
			  {
				
				float diff1,diff2,accum1,accum2,mean1,mean2;
				float C12, V1, V2;
				float correlation;
				int i,t;

				correlation=0;

				for (c=0;c<channels;c++)
				{

				  accum1=0;accum2=0;
				  for (i=0;i<length;i++)
				  {
					t=bufpos-length+i;if (t<0) t+=ERPBUFLEN;
					accum1+=ringbuf[c][t];
					accum2+=epochbuf[c][i];
				  }

				  mean1 = accum1 / length;
				  mean2 = accum2 / length;
				  C12 = 0; V1 = 0; V2 = 0;
	
				  for (i = length-1; i >= 0; i--)  // rev
				  {
					t=bufpos-length+i;if (t<0) t+=ERPBUFLEN;
    				diff1 = ringbuf[c][t] - mean1;
					diff2 = epochbuf[c][i] - mean2;
					C12 += diff1 * diff2;
					V1 += diff1 * diff1;
					V2 += diff2 * diff2;
				  }
    
				  C12 /= length;
				  V1 /= length;
				  V2 /= length;

				  //dev1=sqrt(V1);
				 // dev2=sqrt(V2);
				
				  
				  if (method==2)
				  {
				    if (V1>V2) 
				      correlation+= (float) (C12 / sqrt(V1 * V2) / V1 * V2);
				    else
					  correlation+= (float) (C12 / sqrt(V1 * V2) / V2 * V1);
				  }
				  else correlation+= (float) (C12 / sqrt(V1 * V2));
				}
				correlation/=channels;
				pass_values(0,correlation);
			  }

		  } 

	  }


ERPDETECTOBJ::~ERPDETECTOBJ()
	  {
	  }  

