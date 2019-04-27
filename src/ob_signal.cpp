/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SIGNAL.CPP:  contains functions for the Signal-Generator-Object
  Author: Chris Veigl

  Frequency, Gain and Phase of a sinus, sawtooth or rectangle signal 
  can be selected.

  SignalDlgHandler: processes events of the toolbox window

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_signal.h"


LRESULT CALLBACK SignalDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	char sztemp[20];
	SIGNALOBJ * st;
	
	st = (SIGNALOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_SIGNAL)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				
				lpsi.nMin=0; lpsi.nMax=6000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_FREQUENCYBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=0; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_GAINBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=0; lpsi.nMax=360;
				SetScrollInfo(GetDlgItem(hDlg,IDC_PHASEBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=-5000; lpsi.nMax=5000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_CENTERBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=0; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_NOISEBAR),SB_CTL,&lpsi,TRUE);

				SetScrollPos(GetDlgItem(hDlg,IDC_FREQUENCYBAR), SB_CTL,(int)(st->frequency*100.0f),TRUE);
				sprintf(sztemp,"%.2f",st->frequency);
				SetDlgItemText(hDlg, IDC_FREQUENCY,sztemp);
			    
				SetScrollPos(GetDlgItem(hDlg,IDC_CENTERBAR), SB_CTL,(int)st->center,TRUE);
				SetDlgItemInt(hDlg, IDC_CENTER, (int)st->center,1);

				SetScrollPos(GetDlgItem(hDlg,IDC_GAINBAR), SB_CTL,(int)st->gain,TRUE);
				sprintf(sztemp,"%.2f",(st->gain/1000.0f*st->out_ports[0].out_max));
				SetDlgItemText(hDlg, IDC_GAIN,sztemp);
				//SetDlgItemInt(hDlg, IDC_GAIN, (int)(st->gain/1000.0f*st->out_ports[0].out_max),0);

				SetScrollPos(GetDlgItem(hDlg,IDC_PHASEBAR), SB_CTL,(int)st->phase,TRUE);
				SetDlgItemInt(hDlg, IDC_PHASE, (int)st->phase,0);
				
				SetScrollPos(GetDlgItem(hDlg,IDC_NOISEBAR), SB_CTL,(int)st->noise,TRUE);
				SetDlgItemInt(hDlg, IDC_NOISE, (int)st->noise,0);

				SendDlgItemMessage( hDlg, IDC_SIGNALCOMBO, CB_ADDSTRING, 0,(LPARAM) "Sine") ;
				SendDlgItemMessage( hDlg, IDC_SIGNALCOMBO, CB_ADDSTRING, 0,(LPARAM) "Sawtooth") ;
				SendDlgItemMessage( hDlg, IDC_SIGNALCOMBO, CB_ADDSTRING, 0,(LPARAM) "Rectangle") ;
				SendDlgItemMessage( hDlg, IDC_SIGNALCOMBO, CB_ADDSTRING, 0,(LPARAM) "Ramp") ;
				SendDlgItemMessage( hDlg, IDC_SIGNALCOMBO, CB_SETCURSEL, st->sigtype, 0L ) ;

				CheckDlgButton(hDlg, IDC_ENABLE_IN, st->enable_in);
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_SIGNALCOMBO:
				if (HIWORD(wParam)==CBN_SELCHANGE)
				    st->sigtype=SendMessage(GetDlgItem(hDlg, IDC_SIGNALCOMBO), CB_GETCURSEL , 0, 0);
				break;
			case IDC_ENABLE_IN:
				st->enable_in= IsDlgButtonChecked(hDlg, IDC_ENABLE_IN);
				if (st->enable_in) { st->width=80; st->inports=2; st->height=CON_START+2*CON_HEIGHT+5;}
				else {st->width=50; st->inports=0; st->height=CON_START+CON_HEIGHT+5; }
				InvalidateRect(ghWndDesign,NULL,TRUE);
				break;
			}
			return TRUE;
		case WM_HSCROLL:
		{
			int nNewPos; 
			//if ((
			nNewPos=get_scrollpos(wParam, lParam);
			//)>=0)
		    {
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FREQUENCYBAR))  
			  {   
				  sprintf(sztemp,"%.2f",nNewPos/100.0f);
				  SetDlgItemText(hDlg, IDC_FREQUENCY,sztemp);
			      st->frequency=(float)nNewPos/100.0f;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CENTERBAR)) 
			  {
				   st->center=(float)nNewPos;
				   SetDlgItemInt(hDlg, IDC_CENTER,(int)(st->center),1);
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_GAINBAR)) 
			  {
				   st->gain=(float)nNewPos;
				   //SetDlgItemInt(hDlg, IDC_GAIN,(int)(st->gain/1000.0f*st->out_ports[0].out_max),0);
				   sprintf(sztemp,"%.2f",(st->gain/1000.0f*st->out_ports[0].out_max));
				   SetDlgItemText(hDlg, IDC_GAIN,sztemp);

			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_PHASEBAR)) 
			  {
				   SetDlgItemInt(hDlg, IDC_PHASE,nNewPos,0);
				   st->phase=(float)nNewPos;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_NOISEBAR)) 
			  {
				   SetDlgItemInt(hDlg, IDC_NOISE,nNewPos,0);
				   st->noise=nNewPos;
			  }
		  
			}
		
		} break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}





//
//  Object Implementation
//


SIGNALOBJ::SIGNALOBJ(int num) : BASE_CL()	
	  {
		outports = 1;
		inports = 0;
		width=50;
		frequency=2.0f;
		gain=200.0f;
		center=0.0f;
		noise=0;
		angle=0;
		enable_in=0;


		strcpy(out_ports[0].out_name,"out");
	    strcpy(out_ports[0].out_dim,"uV");
	    out_ports[0].get_range=-1;
	    strcpy(out_ports[0].out_desc,"Signal Generator");
	    out_ports[0].out_min=-500.0f;
	    out_ports[0].out_max=500.0f;

		strcpy(in_ports[0].in_name,"freq");
	    strcpy(in_ports[0].in_dim,"Hz");
	    in_ports[0].get_range=-1;
	    strcpy(in_ports[0].in_desc,"Frequency");
	    in_ports[0].in_min=0.0f;
	    in_ports[0].in_max=60.0f;

		strcpy(in_ports[1].in_name,"phase");
	    strcpy(in_ports[1].in_dim,"none");
	    in_ports[1].get_range=-1;
	    strcpy(in_ports[1].in_desc,"Phase");
	    in_ports[1].in_min=0.0f;
	    in_ports[1].in_max=360.0f;


		phase=0.0;
		sigtype=SIG_SINUS;
	  }
	  void SIGNALOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SIGNALBOX, ghWndStatusbox, (DLGPROC)SignalDlgHandler));
	  }
	  void SIGNALOBJ::load(HANDLE hFile) 
	  {
		  load_object_basics(this);
		  load_property("frequency",P_FLOAT,&frequency);
		  load_property("center",P_FLOAT,&center);
		  load_property("gain",P_FLOAT,&gain);
		  load_property("phase",P_FLOAT,&phase);
		  load_property("noise",P_INT,&noise);
		  load_property("type",P_INT,&sigtype);
		  load_property("enable_in",P_INT,&enable_in);
  		  if (enable_in) { width=80; height=CON_START+2*CON_HEIGHT+5;}
		  else {width=50; height=CON_START+CON_HEIGHT+5; }
	  }
		
	  void SIGNALOBJ::save(HANDLE hFile) 
	  {	   
		  save_object_basics(hFile, this);
		  save_property(hFile,"frequency",P_FLOAT,&frequency);
		  save_property(hFile,"center",P_FLOAT,&center);
		  save_property(hFile,"gain",P_FLOAT,&gain);
		  save_property(hFile,"phase",P_FLOAT,&phase);
		  save_property(hFile,"noise",P_INT,&noise);
		  save_property(hFile,"type",P_INT,&sigtype);
		  save_property(hFile,"enable_in",P_INT,&enable_in);
	  }

	  void SIGNALOBJ::incoming_data(int port, float value) 
	  {	
		  if (value!=INVALID_VALUE)
		  {
		    if (port==0)  frequency=value; 
		    if (port==1)  phase=value;
		  }
	  }

	  void SIGNALOBJ::session_reset(void) 
	  {
		  angle=0;
	  }

	  void SIGNALOBJ::work(void) 
	  {
	    float x,s,g;

		angle+=frequency/PACKETSPERSECOND*2*(float)DDC_PI; if (angle>(float)(DDC_PI*2.0f)) angle-=(float)(DDC_PI*2.0f);
//		s=(float)sin((float)(TIMING.packetcounter+phase*PACKETSPERSECOND/720)/PACKETSPERSECOND*2*DDC_PI*frequency);

		//s=(float)sin((angle+phase/360)*2*DDC_PI);
		s=(float)sin(angle+(phase/360)*2*DDC_PI);
        g=gain/1000.0f*out_ports[0].out_max;
		switch (sigtype)
		{
			case SIG_SINUS:
				x=s*g;
				break;
			case SIG_SAWTOOTH:
				x=(float)asin(s)*g*0.625f;
				break;
			case SIG_RECTANGLE:
				if (s>0) x=g; else x=-g;
				break;
			case SIG_RAMP:
				x=(float)((angle-DDC_PI)/DDC_PI*g);
				{
					//char tmp[20];
					//sprintf(tmp,"%.2f",angle);
					//SetDlgItemText(ghWndStatusbox,IDC_STATUS,tmp);
				}
				
				break;
		}

		if (noise) x+=(float)(rand()%noise-noise/2);

		if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox) && (enable_in)) 
		{
			char sztemp[25];
			sprintf(sztemp,"%.2f",frequency);
			SetDlgItemText(hDlg, IDC_FREQUENCY,sztemp);
			SetScrollPos(GetDlgItem(hDlg, IDC_FREQUENCYBAR), SB_CTL, (int)(frequency*100.0f), 1); 
			sprintf(sztemp,"%.2f",phase);
			SetDlgItemText(hDlg, IDC_PHASE,sztemp);
			SetScrollPos(GetDlgItem(hDlg, IDC_PHASEBAR), SB_CTL, (int)(phase), 1); 
		}

	    pass_values(0,x+center);
	  }

SIGNALOBJ::~SIGNALOBJ()
	  {
		// free object
	  }  
