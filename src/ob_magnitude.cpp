/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_MAGNITUDE.CPP:  contains assisting functions for the Magnitude-Object
  Authors: Jim Peters,Chris Veigl

  The Magnitude is the frequency activity of a signal in a given passband.
  In contrast to the filtered output, the magnitude does not oscillate at 
  that frequency, it describes the power in a frequency-band. This parameter is often
  used in neurofeedback, for example when you want to make an alpha-feedback.
  This module calls into the fid-lib filter library by Jim Peters to provide
  magnitude functionalities. The Filter-Type and order can be selected.
  
  MagnitudeboxDlgHandler: processes events for the magnitude-toolbox window.

  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_magnitude.h"

struct PASSTYPEStruct		PASSTYPE[PASSTYPES]=
{{"Bessel-Bandpass","LpBe"},
{"Butterworth-Bandpass","LpBu"}};
	
//	"Chebyshev-Bandpass","LpCh"

LRESULT CALLBACK MagnitudeDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	int t; // ,z;
	static int tempfilt=0,acttype=0;
	static int dinit=FALSE;
	char sztemp[30];
    
	MAGNITUDEOBJ * st;
	
	st = (MAGNITUDEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MAGNITUDE)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				SendMessage(GetDlgItem(hDlg, IDC_PASSTYPECOMBO), CB_RESETCONTENT,0,0);
			    for (t = 0; t < PASSTYPES; t++) 
					SendMessage( GetDlgItem(hDlg, IDC_PASSTYPECOMBO), CB_ADDSTRING, 0,  (LPARAM) (LPSTR) PASSTYPE[t].tname) ;
				SetDlgItemText(hDlg,IDC_PASSTYPECOMBO, PASSTYPE[st->filtertype].tname);
				acttype=st->filtertype;

				SetDlgItemInt(hDlg,IDC_ORDER, st->order,0);
				SetDlgItemInt(hDlg,IDC_GAIN, st->gain,0);

				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				
				lpsi.nMin=1; lpsi.nMax=1280;
				SetScrollInfo(GetDlgItem(hDlg,IDC_CENTERBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=1; lpsi.nMax=200;
				SetScrollInfo(GetDlgItem(hDlg,IDC_WIDTHBAR),SB_CTL,&lpsi,TRUE);

				SetScrollPos(GetDlgItem(hDlg,IDC_CENTERBAR), SB_CTL,(int)(st->center*10.0f),TRUE);
				sprintf(sztemp,"%.4f",st->center);
				SetDlgItemText(hDlg,IDC_CENTER, sztemp);

				SetScrollPos(GetDlgItem(hDlg,IDC_WIDTHBAR), SB_CTL,(int)(st->wid*10.0f),TRUE);
				sprintf(sztemp,"%.4f",st->wid);
				SetDlgItemText(hDlg,IDC_WIDTH, sztemp);
			}
			return TRUE;
		case WM_CLOSE:		
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			
			case IDC_PASSTYPECOMBO:
				  acttype=SendMessage( GetDlgItem(hDlg, IDC_PASSTYPECOMBO), CB_GETCURSEL, 0, 0 ) ;
				break;
			case IDC_CENTER:
				if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_CENTER, sztemp, 20);
						st->center = (float)atof(sztemp);
						if (st->center<0) { st->center=0; SetDlgItemText(hDlg,IDC_CENTER,"0");}
						if (st->center>PACKETSPERSECOND/2) { st->center=(float)(PACKETSPERSECOND/2); SetDlgItemInt(hDlg,IDC_CENTER,PACKETSPERSECOND/2,0);}
//						SendMessage (hDlg,WM_COMMAND,IDC_STORE,0);
					}
				break;
			case IDC_WIDTH:
				if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_WIDTH, sztemp, 20);
						st->wid = (float)atof(sztemp);
						if (st->wid<0.0001) { st->wid=1; SetDlgItemText(hDlg,IDC_CENTER,"0.0001");}
						if (st->wid>PACKETSPERSECOND/2) { st->wid=(float)(PACKETSPERSECOND/2); SetDlgItemInt(hDlg,IDC_WIDTH,PACKETSPERSECOND/2,0);}
//						SendMessage (hDlg,WM_COMMAND,IDC_STORE,0);
					}
				break;
			case IDC_STORE:
				{
					char sztemp[30];
					char szorder[5];
					int test;
					
					test=GetDlgItemInt(hDlg,IDC_ORDER, NULL, 0);
					if ((test<1)||((test>10)&&(acttype==0))||((test>60)&&(acttype==1))) 
					{ SetDlgItemInt(hDlg,IDC_ORDER,st->order,0); return(TRUE); }
					st->filtertype=acttype;
					st->order=test; 
					st->gain=GetDlgItemInt(hDlg,IDC_GAIN, NULL, 0);

					strcpy(sztemp,PASSTYPE[st->filtertype].init);
					GetDlgItemText(hDlg, IDC_ORDER,  szorder, sizeof(szorder));
					strcat(sztemp,szorder);
	
					st->lp1filt= fid_design(sztemp, PACKETSPERSECOND, (double)st->wid, 0, 0, 0);
					st->lp1run= fid_run_new(st->lp1filt, &(st->lp1funcp));
					if (st->lp1fbuf!=NULL)
					{
						fid_run_freebuf(st->lp1fbuf);
	   				    st->lp1fbuf=fid_run_newbuf(st->lp1run);
					}

					st->lp2filt= fid_design(sztemp, PACKETSPERSECOND,(double)st->wid, 0, 0, 0);
					st->lp2run= fid_run_new(st->lp2filt, &(st->lp2funcp));
					if (st->lp2fbuf!=NULL)
					{
						fid_run_freebuf(st->lp2fbuf);
   					    st->lp2fbuf=fid_run_newbuf(st->lp2run);
					}
					return TRUE;
				}
				break;
			}
			return (TRUE);
		case WM_HSCROLL:
		{
			int nNewPos;
  		    if((nNewPos=get_scrollpos(wParam,lParam))>=0)
			{
			  
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CENTERBAR))  
			  {   
				  st->center=(float)nNewPos/10.0f;
				  sprintf(sztemp,"%.2f",st->center);
				  SetDlgItemText(hDlg, IDC_CENTER,sztemp);
			      
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_WIDTHBAR)) 
			  {
				  st->wid=(float)nNewPos/10.0f;
				  sprintf(sztemp,"%.2f",st->wid);
				  SetDlgItemText(hDlg, IDC_WIDTH,sztemp);
			  }

			} 
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



MAGNITUDEOBJ::MAGNITUDEOBJ(int num) : BASE_CL()	
	  {
	    outports = 1;
		inports = 1;
		width=65;
		strcpy(in_ports[0].in_name,"in");
		strcpy(out_ports[0].out_name,"out");
		out_ports[0].get_range=-1;
		out_ports[0].out_min=0;
		out_ports[0].out_max=100;
		strcpy(out_ports[0].out_dim,"uV");
		
				
		lp1filt= fid_design("LpBu4", PACKETSPERSECOND, 2, 0, 0, 0);
		lp1run= fid_run_new(lp1filt, &(lp1funcp));
		lp1fbuf=fid_run_newbuf(lp1run);

		lp2filt= fid_design("LpBu4", PACKETSPERSECOND, 2, 0, 0, 0);
		lp2run= fid_run_new(lp2filt, &(lp2funcp));
		lp2fbuf=fid_run_newbuf(lp2run);

		filtertype=1; order=4; gain=100; center=10.0f; wid=2.0f;
	  }
	  void MAGNITUDEOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MAGNITUDEBOX, ghWndStatusbox, (DLGPROC)MagnitudeDlgHandler));
	  }

	void MAGNITUDEOBJ::session_start(void)
	{
		if (lp1fbuf==NULL)	// fid_run_freebuf(lp1fbuf);
		  lp1fbuf=fid_run_newbuf(lp1run);
		if (lp2fbuf==NULL)	//  fid_run_freebuf(lp2fbuf);
	  	  lp2fbuf=fid_run_newbuf(lp2run);

	}
	void MAGNITUDEOBJ::session_reset(void)
	{
		if (lp1fbuf!=NULL)	 fid_run_freebuf(lp1fbuf);
		lp1fbuf=fid_run_newbuf(lp1run);
		if (lp2fbuf!=NULL)	 fid_run_freebuf(lp2fbuf);
		lp2fbuf=fid_run_newbuf(lp2run);
	}
	void MAGNITUDEOBJ::session_pos(long pos)
	{
		if (lp1fbuf!=NULL)	 fid_run_freebuf(lp1fbuf);
		lp1fbuf=fid_run_newbuf(lp1run);
		if (lp2fbuf!=NULL)	 fid_run_freebuf(lp2fbuf);
		lp2fbuf=fid_run_newbuf(lp2run);
	}

	  void MAGNITUDEOBJ::load(HANDLE hFile) 
	  {
		char sztemp[30];
		char szorder[5];

		load_object_basics(this);
		load_property("type",P_INT,&filtertype);
		load_property("order",P_INT,&order);
		load_property("center",P_FLOAT,&center);
		load_property("width",P_FLOAT,&wid);
		load_property("gain",P_INT,&gain);
		
		strcpy(sztemp,PASSTYPE[filtertype].init);
		sprintf(szorder,"%d",order);
		strcat(sztemp,szorder);

		if (lp1fbuf!=NULL)	 fid_run_freebuf(lp1fbuf);
		if (lp1filt!=NULL)   fid_run_free(lp1filt);
		if (lp2fbuf!=NULL)	 fid_run_freebuf(lp2fbuf);
		if (lp2filt!=NULL)   fid_run_free(lp2filt);
		lp1filt= fid_design(sztemp, PACKETSPERSECOND, (double)wid, 0, 0, 0);
		lp1run= fid_run_new(lp1filt, &(lp1funcp));
		lp1fbuf=fid_run_newbuf(lp1run);
		lp2filt= fid_design(sztemp, PACKETSPERSECOND,(double)wid, 0, 0, 0);
		lp2run= fid_run_new(lp2filt, &(lp2funcp));
		lp2fbuf=fid_run_newbuf(lp2run);
	  }
 		
	  void MAGNITUDEOBJ::save(HANDLE hFile) 
	  {	  
		  save_object_basics(hFile, this);
		  save_property(hFile,"type",P_INT,&filtertype);
		  save_property(hFile,"order",P_INT,&order);
		  save_property(hFile,"center",P_FLOAT,&center);
		  save_property(hFile,"width",P_FLOAT,&wid);
		  save_property(hFile,"gain",P_INT,&gain);

	  }


	  void MAGNITUDEOBJ::incoming_data(int port, float value) {	input=value;  }


      void MAGNITUDEOBJ::work(void)
      {
        float sig1,sig2;
       
        
		sig1=(float)sin(TIMING.packetcounter*2*DDC_PI/PACKETSPERSECOND*center)*(input);
        
		sig2=(float)cos(TIMING.packetcounter*2*DDC_PI/PACKETSPERSECOND*center)*(input);

        sig1= (float)(lp1funcp(lp1fbuf,sig1));
        sig2= (float)(lp2funcp(lp2fbuf,sig2));

		pass_values(0,(float)(2*sqrt(sig1*sig1+sig2*sig2)*gain/100.0f));
      }




MAGNITUDEOBJ::~MAGNITUDEOBJ()
	  {
			if (lp1fbuf!=NULL)	 fid_run_freebuf(lp1fbuf);
			if (lp1filt!=NULL)   fid_run_free(lp1filt);

			if (lp2fbuf!=NULL)	 fid_run_freebuf(lp2fbuf);
			if (lp2filt!=NULL)   fid_run_free(lp2filt);
	  }  


