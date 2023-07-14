/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, GPL, contact: chris@shifz.org
  
  MODULE: OB_EDF_READER.CPP:  contains functions for EDF - File import
  Author: Chris Veigl


  Using this Object, an EDF-File can be specified and opened,
  the User Information is displayed in the object's toolbox,
  and the Signals are presented at the output-ports.

  Adjust the sampling rate to correctly display the signals.
  more Info about the EDF-File Format: http://www.edfplus.info/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_edf_reader.h"


void update_state(HWND hDlg, int state)
{
	switch (state)
	{
	case 0:
		add_to_listbox(hDlg,IDC_LIST, "No File opened.");
		EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
		break;

	case 1:
		add_to_listbox(hDlg,IDC_LIST, " File Open.");
		EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
		break;

	case 2:
		add_to_listbox(hDlg,IDC_LIST, "Reading Values from File.");
		EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
		break;
	}
	
}
 

LRESULT CALLBACK EdfReaderDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	EDF_READEROBJ * st;
	int x;
	static int actchn;
	char strfloat[21];

	
	st = (EDF_READEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_EDF_READER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				actchn=0;
				update_header(hDlg,&st->header);
				update_channelcombo(hDlg, st->channel, st->header.channels);
				update_channel(hDlg,st->channel,actchn);
				update_state(hDlg,st->state);

				sprintf(strfloat,"%.2f",(float)st->offset/(float)PACKETSPERSECOND);
				SetDlgItemText(hDlg,IDC_OFFSET,strfloat);

				if (st->edffile!=INVALID_HANDLE_VALUE)
				   SetDlgItemText(hDlg, IDC_EDFFILE, st->filename);
				else
					SetDlgItemText(hDlg, IDC_EDFFILE, "none");


				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 

			case IDC_SELECT:
			 
				 st->filename[0]=0;
				 if ((st->edffile=open_edf_file(&st->header,st->channel,st->filename))==INVALID_HANDLE_VALUE) st->state=0;
				 else if (st->header.channels==0) 
				 {
					SendMessage(hDlg,WM_COMMAND,IDC_CLOSE,0);
					report("EDF-File contained no channel information, file closed.");
					st->state=0;
				 }
				 else st->state=1;

				 update_state(hDlg,st->state);
				 if (!st->state) break;
				 st->calc_session_length();
				 get_session_length();
				 //set_session_pos(0);
				 st->session_pos(0);
				 st->packetcount=0;
				 st->sampos=0;

				 if (st->outports!=st->header.channels)
				 {
				   for (x=0;x<MAX_CONNECTS;x++)
				   {	
					st->out[x].from_port=-1;
					st->out[x].to_port=-1;
					st->out[x].to_object=-1;
				   }
				   for (x=0;x<MAX_PORTS;x++)
					st->out_ports[x].out_name[0]=0;
				 }
				 st->outports=st->header.channels;
				 st->height=CON_START+st->outports*CON_HEIGHT+5;

				 update_header(hDlg,&st->header);
				 update_channelcombo(hDlg, st->channel, st->header.channels);
				 update_channel(hDlg,st->channel,actchn);
				 st->get_captions();
				 update_dimensions();
			     SetDlgItemText(hDlg, IDC_EDFFILE, st->filename);
				 EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
				 EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
				 InvalidateRect(ghWndDesign,NULL,TRUE);
				 
				 break; 

			case IDC_CLOSE: 
					st->state=0;
					update_state(hDlg,st->state);
					SetDlgItemText(hDlg, IDC_EDFFILE, "none");
					add_to_listbox(hDlg,IDC_LIST, "file closed.");
					strcpy(st->filename,"none");
					if (st->edffile==INVALID_HANDLE_VALUE) break;
					CloseHandle(st->edffile);
					st->edffile=INVALID_HANDLE_VALUE;
				    get_session_length();
 				    InvalidateRect(ghWndDesign,NULL,TRUE);

					break;
			case IDC_APPLYOFFSET:
					GetDlgItemText(hDlg, IDC_OFFSET, strfloat, 20);
					st->offset = (int)((float)atof(strfloat) * (float) PACKETSPERSECOND);
					st->calc_session_length();
					get_session_length();
				 break;
			case IDC_CHANNELCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
						get_channel(hDlg, st->channel, actchn);
						actchn=SendMessage(GetDlgItem(hDlg, IDC_CHANNELCOMBO), CB_GETCURSEL , 0, 0);
						update_channel(hDlg, st->channel,actchn);
					}
				 break;

			}
			return TRUE;
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


EDF_READEROBJ::EDF_READEROBJ(int num) : BASE_CL()	
	  {
  	    int i;
	    outports = 0;
		inports = 0;
		width=80;
		height=50;

		for (i=0;i<MAX_PORTS;i++)
		  out_ports[i].get_range=-1;

		reset_header(&header);
		reset_channel(channel);
		state=0;
		packetcount=0;
		sampos=0;
		sessionlength=0;
		offset=0;
		loading=0;
		edffile=INVALID_HANDLE_VALUE;
		strcpy(filename,"none");
	  }


	  void EDF_READEROBJ::get_captions(void)
	  {
		int x,i;
		char tmp[256];

		for (x=0;x<outports;x++) 
		{
				strcpy(out_ports[x].out_desc,channel[x].label);
				strcpy(tmp,channel[x].label);
				if (strlen(tmp)>8) tmp[8]='\0';
				strcpy(out_ports[x].out_name,tmp);

				if (!loading)
				{
					for (i=0;(i<4)&&(channel[x].physdim[i]);i++) out_ports[x].out_dim[i]=channel[x].physdim[i];
					out_ports[x].out_dim[i]=0;

					out_ports[x].out_min=(float)channel[x].physmin;
					out_ports[x].out_max=(float)channel[x].physmax;
				}
		}
//		update_dimensions();
	  }
	

  	  void EDF_READEROBJ::session_start(void) 
	  {	
		if ((edffile!=INVALID_HANDLE_VALUE)&&(outports>0))
		{
			state=2;
			if (hDlg==ghWndToolbox)
			{
				update_state(hDlg,state);
				add_to_listbox(hDlg,IDC_LIST, "File-Read started.");
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
			}
		}
	  }
  	  void EDF_READEROBJ::session_stop(void) 
	  {	
		if (edffile!=INVALID_HANDLE_VALUE) state=1; else state=0;
		if (hDlg==ghWndToolbox)
		{
				update_state(hDlg,state);
				add_to_listbox(hDlg,IDC_LIST, "File-Read paused");
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
		}
	  }
  	  void EDF_READEROBJ::session_reset(void) 
	  {	
		  if (edffile!=INVALID_HANDLE_VALUE)
		  { 
			  state=1;
			  SetFilePointer(edffile,256+header.channels*256,NULL,FILE_BEGIN);
			  if (hDlg==ghWndToolbox)
			  {  
				update_state(hDlg,state);
				add_to_listbox(hDlg,IDC_LIST, "File-Read stopped");
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
			  } 
		  } else state=0;
			
	  }
  	  void EDF_READEROBJ::session_pos(long pos) 
	  {	
		  int x,i;
		  long p,s;
	  
		  if(edffile!=INVALID_HANDLE_VALUE)
		  {

//			  char sztemp[20];
			  long ui=0;

			  state=1;
			  if (pos>sessionlength) 
				{	SetFilePointer(edffile,0,NULL,FILE_END);
					return;
				}
				pos -= offset; 
				if (pos<0) 
 				{	SetFilePointer(edffile,256+header.channels*256,NULL,FILE_BEGIN);    // set pos to segment start
					return;
				}

 		  	    for (x=0,i=0;x<header.channels;x++) i+=channel[x].samples;

				p=(long)((float)pos/(float)header.samplespersegment);    // which segment ?
				s=p*i;              // segment start

				ui=SetFilePointer(edffile,256+header.channels*256+
						s*2,NULL,FILE_BEGIN);    // set pos to segment start

				for (x=0;x<outports;x++)   // read whole segment
					ReadFile(edffile,channel[x].buffer,2*channel[x].samples,&dwRead,NULL);

				sampos=pos-p*header.samplespersegment;       // pos in segment
//				sprintf(sztemp,"s:%d,ui:%d,sampos:%d",s,ui,sampos);
//			    SetDlgItemText(ghWndStatusbox,IDC_STATUS,sztemp);

		  } else state=0;
		  if (hDlg==ghWndToolbox)	update_state(hDlg,state);
	
	  } 

	  void EDF_READEROBJ::calc_session_length(void)
	  {
		  int x,i;
		  if (edffile!=INVALID_HANDLE_VALUE)
		  {
			for (x=0,i=0;x<header.channels;x++) i+=channel[x].samples;
			sessionlength= SetFilePointer(edffile,0,NULL,FILE_END) - (256+header.channels*256);
			sessionlength=sessionlength/i/2*header.duration*header.samplingrate+offset; 
		  } else sessionlength=0;
	  }

	  long EDF_READEROBJ::session_length(void) 
	  {
		  if (edffile!=INVALID_HANDLE_VALUE)
			return(sessionlength); else return(0);
	  }



	  void EDF_READEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EDF_READERBOX, ghWndStatusbox, (DLGPROC)EdfReaderDlgHandler)); 
	  }

	  void EDF_READEROBJ::load(HANDLE hFile) 
	  {

		  state=0;
		  loading=1;
	  	  load_object_basics(this);
		  load_property("filename",P_STRING,&filename);
		  load_property("offset",P_INT,&offset);
		  if ((edffile=open_edf_file(&header,channel,filename))==INVALID_HANDLE_VALUE)
		  {	 char st[150];
			 reduce_filepath(st,filename);
			 strcpy(filename,GLOBAL.resourcepath);
			 strcat(filename,"ARCHIVES\\");
			 strcat(filename,st);			 
			 edffile=open_edf_file(&header,channel,filename); 
		  }
		  if (edffile!=INVALID_HANDLE_VALUE)
		  {
			  header.samplingrate=PACKETSPERSECOND;
			  calc_session_length();
			  session_pos(0);
			  outports=header.channels;
			  state=1;
		  } else  { report_error("EDF archive file not found, please open file in EDF-Reader"); sessionlength=0; }

		  height=CON_START+outports*CON_HEIGHT+5;
		  sampos=0;
		  get_captions();
		  loading=0;

	  }
		
	  void EDF_READEROBJ::save(HANDLE hFile) 
	  {
	  	  save_object_basics(hFile, this);
		  save_property(hFile,"filename",P_STRING,&filename);
		  save_property(hFile,"offset",P_INT,&offset);
	  }

	  
	  void EDF_READEROBJ::work(void) 
	  {
		int x;
//		short actval;
		double actval;
		char szdata[100];
		double fact;
		DWORD dwRead;
	
	
		if ((outports==0)||(state!=2)||(edffile==INVALID_HANDLE_VALUE)) return;
		if ((TIMING.packetcounter<offset)||(TIMING.packetcounter>sessionlength)) return;
	
		if (sampos<header.samplespersegment-1)
		{
			for (x=0;x<outports;x++)
			{
				actval=channel[x].buffer[sampos];
				if (channel[x].samples==1) actval=channel[x].buffer[0];

				fact=(channel[x].physmax-channel[x].physmin)/(double)(channel[x].digmax-channel[x].digmin);
				actval-=channel[x].digmin;
//				actval=(int)(actval*fact);
				actval=actval*fact;
				pass_values(x,(float)actval+channel[x].physmin);
			}
			sampos++; 
			packetcount++;
			if (!(packetcount%1000))
			{
				sprintf(szdata,"%d Packets read\n",packetcount);
				if (hDlg==ghWndToolbox) 
					add_to_listbox(hDlg,IDC_LIST, szdata); 
			}

		}
		else
		{
		    sampos=0;
		    for (x=0;x<outports;x++)
			{
				ReadFile(edffile,channel[x].buffer,(DWORD)(2*channel[x].samples),&dwRead,NULL);
				if (dwRead<(DWORD)2*channel[x].samples)
				{
//					session_pos(0);
					packetcount=0;sampos=header.samplespersegment;x=outports;
				}
			}
		}

	  }


EDF_READEROBJ::~EDF_READEROBJ()
	  {	
		if (edffile)
		{
			CloseHandle(edffile);
		}
	  }  
