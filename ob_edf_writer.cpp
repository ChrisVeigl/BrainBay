/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
    
  MODULE: OB_EDF_WRITER.CPP:  contains the EDF-File - Writer-Object
  Author: Chris Veigl


  Using this Object, an EDF-File can be written,
  the Signals are connected to the input-ports.

  more Info about the EDF-File Format: http://www.edfplus.info/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_edf_writer.h"




LRESULT CALLBACK EdfWriterDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	EDF_WRITEROBJ * st;
	static int actchn;
	
	st = (EDF_WRITEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_EDF_WRITER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_EDFFILE, st->filename);
				if (st->state)
				{
					add_to_listbox(hDlg,IDC_LIST, "Writing Values to File.");
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
				}
				else
				{
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					if (st->edffile==INVALID_HANDLE_VALUE)
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
					else EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				}

				actchn=0;
				update_header(hDlg,&st->header);
				update_channelcombo(hDlg, st->channel, st->header.channels);
				update_channel(hDlg,st->channel,actchn);

				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			if (HIWORD(wParam)==EN_KILLFOCUS) {get_header(hDlg,&st->header); get_channel(hDlg, st->channel, actchn);}
			switch (LOWORD(wParam)) 
			{ 
			case IDC_SELECT:
			 
				 st->edffile=create_edf_file(&st->header, st->channel, st->filename);
				 if (st->edffile==INVALID_HANDLE_VALUE) report("Could not create EDF-File");
				 else { EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE); EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);}

				 SetDlgItemText(hDlg,IDC_EDFFILE,st->filename);
			     InvalidateRect(ghWndDesign,NULL,TRUE);
				 break; 

			case IDC_START:
				if ((st->inports>0) &&(st->edffile!=INVALID_HANDLE_VALUE))
				{
					add_to_listbox(hDlg,IDC_LIST, "starting File-Write");
					// START FILE WRITING
					st->state=1;st->samplecount=0;st->recordcount=0;
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
  					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				} else add_to_listbox(hDlg,IDC_LIST, "No Channels available.");
				break; 

			case IDC_STOP:
					add_to_listbox(hDlg,IDC_LIST, "stopped");
					st->state=0;
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
					break; 

			case IDC_CLOSE: 
					add_to_listbox(hDlg,IDC_LIST, "file closed.");
					st->state=0;
					if (st->edffile!=INVALID_HANDLE_VALUE)
					{
						char str[8];
						DWORD dwWritten;

						sprintf(str,"%d",st->recordcount);
						SetFilePointer(st->edffile,236,NULL,FILE_BEGIN);
						WriteFile(st->edffile,str,strlen(str),&dwWritten, NULL);
						SetFilePointer(st->edffile,0,NULL,FILE_END);
						

						CloseHandle(st->edffile);
						st->edffile=INVALID_HANDLE_VALUE;
					}
  				    EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
 				    InvalidateRect(ghWndDesign,NULL,TRUE);
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


EDF_WRITEROBJ::EDF_WRITEROBJ(int num) : BASE_CL()	
	  {
  	    int x;
	    outports = 0;
		inports = 1;
		width=80;
		height=50;

		reset_header(&header);
		reset_channel(channel);

		header.samplespersegment=PACKETSPERSECOND;
		header.segments=-1;
		header.samplingrate=PACKETSPERSECOND;
		header.duration=1;
		strcpy (header.patient,"standard EEG");
		strcpy (header.device,"Modular EEG  Unit");
		header.channels=0;

		for (x=0;x<MAX_EEG_CHANNELS;x++) 
		{
			sprintf(szdata,"%d : ",x+1);
			strcpy (channel[x].transducer, "Ag/AgCl Electrode");
			strcpy (channel[x].physdim, "uV");
			strcpy (channel[x].label, "none");
			strcpy (channel[x].prefiltering, "HP:0.16Hz, LP:59Hz");
			channel[x].physmin=500;
			channel[x].physmax=500;
			channel[x].digmin=0;
			channel[x].digmax=1024;
		}

		state=0;
		samplecount=0;recordcount=0;
		edffile=INVALID_HANDLE_VALUE;
		strcpy(filename,"none");
	  }


	  void EDF_WRITEROBJ::get_captions(void)
	  {
		int x,i;

		for (x=0;x<inports;x++) 
		{
				strcpy(channel[x].label,in_ports[x].in_desc);
				for (i=0;(channel[x].label[i])&&i<11;i++) in_ports[x].in_name[i]=channel[x].label[i];
				in_ports[x].in_name[i]=0;
				strcpy(channel[x].physdim,in_ports[x].in_dim);

				channel[x].physmin=(int)in_ports[x].in_min;
				channel[x].physmax=(int)in_ports[x].in_max;
		}
		header.channels=inports-1;
	  }
	
  	  void EDF_WRITEROBJ::update_inports(void)
	  {
		if ((edffile!=INVALID_HANDLE_VALUE)&& (inports!=count_inports(this)))
		{
			close_toolbox();
			CloseHandle(edffile);
			edffile=INVALID_HANDLE_VALUE;
			report("EDF-Writer Input Ports changed, File has been closed");
			state=0;
		}
		//if (state!=1)
		{
			inports=count_inports(this);
			get_captions();
			height=CON_START+inports*CON_HEIGHT+5;
			InvalidateRect(ghWndMain,NULL,TRUE);
		}
	  }


	  void EDF_WRITEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EDF_WRITERBOX, ghWndStatusbox, (DLGPROC)EdfWriterDlgHandler)); 
	  }

	  void EDF_WRITEROBJ::load(HANDLE hFile) 
	  {

	 	  load_object_basics(this);
		  load_property("filename",P_STRING,&filename);
//		  load_property("header",P_STRING,&edfheader);
//		  parse_edf_header(&header, channel, edfheader);
		  inports=header.channels;
		  height=CON_START+inports*CON_HEIGHT+5;
		  get_captions();
	  }
		
	  void EDF_WRITEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"filename",P_STRING,&filename);
//		  save_property(hFile,"header",P_STRING,&edfheader);	  
	  }


	  void EDF_WRITEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
	  }

	  
	  void EDF_WRITEROBJ::work(void) 
	  {
		int x;
		float fact;
	
	
		if ((inports==0)||(state==0)||(edffile==INVALID_HANDLE_VALUE)) return;

	
		for (x=0;x<inports-1;x++)
		{
			// WRITE FILE
			if (channel[x].physmax-channel[x].physmin==0) fact=0.0f;
			else fact=(float)(channel[x].digmax-channel[x].digmin)/(float)(channel[x].physmax-channel[x].physmin);
			channel[x].buffer[samplecount]=(short)((in_ports[x].value-channel[x].physmin)*fact+channel[x].digmin);
		}		
		samplecount++;

		if (samplecount==PACKETSPERSECOND)
		{
			samplecount=0; recordcount++;
			for (x=0;x<inports-1;x++)
				WriteFile(edffile,channel[x].buffer,PACKETSPERSECOND*2,&dwWritten,NULL);

			sprintf(szdata,"%d Seconds written",recordcount);
			if (hDlg==ghWndToolbox) 
				add_to_listbox(hDlg,IDC_LIST, szdata);
		}

	  }


EDF_WRITEROBJ::~EDF_WRITEROBJ()
	  {	
		if (edffile)
		{
			char str[8];
			sprintf(str,"%d",recordcount);
			SetFilePointer(edffile,236,NULL,FILE_BEGIN);
			WriteFile(edffile,str,strlen(str),&dwWritten, NULL);
			SetFilePointer(edffile,0,NULL,FILE_END);
			CloseHandle(edffile);
		}
	  }  
