/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_TCP_SENDER.CPP:  contains functions for the Neuro Server data transmission
  Based on SDL_net-Code by Jeremy Wilkerson

  This object can connect to a running neuroserver and transmit a stream of signals
  in the EDF-format. the signals are captured from the object's input ports

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_tcp_sender.h"

#define STATE_IDLE 0
#define STATE_READY 1
#define STATE_WRITING 2

void set_gui_ready(HWND hDlg) 
{
	EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);

	EnableWindow(GetDlgItem(hDlg, IDC_PATIENT), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DEVICE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLES), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLINGRATE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_CHANNELCOMBO), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_LABEL), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_ELECTRODE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSDIM), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMIN), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMAX), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMIN), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMAX), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PREFILTERING), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_CHNFROMPORT), FALSE); 
}

void set_gui_idle(HWND hDlg) 
{
	EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);

	EnableWindow(GetDlgItem(hDlg, IDC_PATIENT), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DEVICE), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLES), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLINGRATE), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_CHANNELCOMBO), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_LABEL), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_ELECTRODE), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSDIM), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMIN), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMAX), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMIN), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMAX), TRUE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PREFILTERING), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_CHNFROMPORT), TRUE); 
}

void set_gui_writing(HWND hDlg) 
{
	EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);

	EnableWindow(GetDlgItem(hDlg, IDC_PATIENT), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DEVICE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLES), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLINGRATE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_CHANNELCOMBO), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_LABEL), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_ELECTRODE), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSDIM), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMIN), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PHYSMAX), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMIN), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_DIGMAX), FALSE); 
	EnableWindow(GetDlgItem(hDlg, IDC_PREFILTERING), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_CHNFROMPORT), FALSE); 
}


LRESULT CALLBACK TcpSenderDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	TCP_SENDEROBJ * st;
	int result;
	static int actchn;

	
	st = (TCP_SENDEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_TCP_SENDER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_HOST, st->host);

				if (st->state==STATE_WRITING)
				{
					add_to_listbox(hDlg,IDC_LIST, "Sending Values to Server.");
					set_gui_writing(hDlg);
				}
				else if (st->state==STATE_READY)
				{
					add_to_listbox(hDlg,IDC_LIST, "Socket connected.");
					set_gui_ready(hDlg);
				}
				else {
					add_to_listbox(hDlg,IDC_LIST, "No Socket connected.");
					set_gui_idle(hDlg);
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
			case IDC_CONNECT:
				GetDlgItemText(hDlg, IDC_HOST, st->host, sizeof(st->host));
					 st->state=STATE_IDLE;
					 set_gui_idle(hDlg);
				if ((strlen(st->host)<8)||(!st->connect())) 
				{ 
					add_to_listbox(hDlg,IDC_LIST, "Could not connect to Server"); 
					break;
				}

				SendDlgItemMessage(hDlg,IDC_LIST, LB_ADDSTRING, 0, (LPARAM) "Socket connection successful.");
				add_to_listbox(hDlg,IDC_LIST, "Entering eeg-mode.");
				strcpy(st->writebuf,"eeg\n");
				result = SDLNet_TCP_Send(st->sock, st->writebuf, strlen(st->writebuf));
				
	 			st->read_tcp(st->readbuf, 10);
				st->readbuf[10]=0;
				if (!strstr(st->readbuf,"200 OK")) { add_to_listbox(hDlg,IDC_LIST,"Could not select eeg-mode"); break;}
				add_to_listbox(hDlg,IDC_LIST, "OK");

				if (!st->sock) { add_to_listbox(hDlg,IDC_LIST,"Socket not connected"); break;}

				st->state=STATE_READY;
				set_gui_ready(hDlg);
				break; 


			case IDC_START:
				if ((st->inports>0)&&(st->sock))
				{
					add_to_listbox(hDlg,IDC_LIST, "Starting sending packages.");
					st->syncloss=0;
					result=st->start_sending();
					if (result!=200) { add_to_listbox(hDlg,IDC_LIST,"Could not enter Send-Mode"); break;}
					st->packetcount=0;
					st->state=STATE_WRITING;
					set_gui_writing(hDlg);
					add_to_listbox(hDlg,IDC_LIST, "OK");					 
				} else add_to_listbox(hDlg,IDC_LIST, "No Channels available.");
				break; 
			case IDC_SENDCMD:
				if (st->sock)
				{ char tmp[1001];
					int i;
					GetDlgItemText(hDlg,IDC_CMD,tmp,50);
					strcat(tmp,"\n");
					result = SDLNet_TCP_Send(st->sock, tmp, strlen(tmp));
 					st->read_tcp(tmp, 1000);
					tmp[1000]=0; for (i=0;tmp[i];i++) if ((tmp[i]==10)||(tmp[i]==13)) tmp[i]='-';
					add_to_listbox(hDlg,IDC_LIST, tmp);
				}
				break;
			case IDC_STOP:
					add_to_listbox(hDlg,IDC_LIST, "Stop sending.");
					st->state=STATE_READY;
					set_gui_ready(hDlg);
					break; 

			case IDC_CLOSE: 
					add_to_listbox(hDlg,IDC_LIST, "Closing connection");
					st->state=STATE_IDLE;
					st->close_tcp();
					set_gui_idle(hDlg);
 				    InvalidateRect(ghWndDesign,NULL,TRUE);
					break;

			case IDC_CHNFROMPORT:
				 if (st->inports>0) {
					 st->get_captions();
					 update_header(hDlg,&st->header);
					 update_channelcombo(hDlg, st->channel, st->header.channels);
					 actchn=0;
					 update_channel(hDlg,st->channel,actchn);
			  		 InvalidateRect(ghWndDesign,NULL,TRUE);
			  		 InvalidateRect(ghWndMain,NULL,TRUE);
				 }


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


TCP_SENDEROBJ::TCP_SENDEROBJ(int num) : BASE_CL()	
	  {
  	    int i,x;
	    outports = 0;
		inports = 1;
		width=80;
		height=50;

		for (i=0;i<MAX_PORTS;i++)
		  out_ports[i].get_range=-1;

		sock=0;
		reset_header(&header);
		reset_channel(channel);
		for (x=0;x<8192;x++) edfinfos[x]=' ';

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


		state=STATE_IDLE;
		packetcount=0;
		packetnum=0;
		streamnum=-1;
		syncloss=0;
		timestamp=0;
		strcpy(host,defaulthost);
	  }

	  void TCP_SENDEROBJ::get_captions(void)
	  {
		int x; //,i;

/*		char tmp[256];
		for (x=0;x<inports;x++) 
		{
				strcpy(in_ports[x].in_desc,channel[x].label);
				strcpy(tmp,channel[x].label);
				if (strlen(tmp)>8) tmp[8]='\0';
				strcpy(in_ports[x].in_name,tmp);
				for (i=0;(i<4)&&(channel[x].physdim[i]);i++) in_ports[x].in_dim[i]=channel[x].physdim[i];
				in_ports[x].in_dim[i]=0;

				in_ports[x].in_min=channel[x].physmin;
				in_ports[x].in_max=channel[x].physmax;
		}*/

		for (x=0;x<inports;x++) 
		{
				strcpy(channel[x].label,in_ports[x].in_desc);
				//for (i=0;(channel[x].label[i])&&i<11;i++) in_ports[x].in_name[i]=channel[x].label[i];
				//in_ports[x].in_name[i]=0;
				strcpy(channel[x].physdim,in_ports[x].in_dim);

				channel[x].physmin=(int)in_ports[x].in_min;
				channel[x].physmax=(int)in_ports[x].in_max;
		}
		header.channels=inports-1;

	  }


  	  void TCP_SENDEROBJ::update_inports(void)
	  {
		if (state!=STATE_WRITING)
		{
			inports=count_inports(this);
			//get_captions();
			header.channels=inports-1;
			height=CON_START+inports*CON_HEIGHT+5;
			InvalidateRect(ghWndDesign,NULL,TRUE);
		}
	  }


	  void TCP_SENDEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
	  }


	  int TCP_SENDEROBJ::connect()
	  {
	 	sock=0;
		if(SDLNet_ResolveHost(&ip, host, PORT) == -1)
		{
			strcpy(szdata, "SDLNet_ResolveHost: "); strcat(szdata, SDLNet_GetError());
			report_error(szdata); return(FALSE);
		}
		
		sock = SDLNet_TCP_Open(&ip);
		if(!sock)
		{
			strcpy(szdata, "SDLNet_TCP_Open: "); strcat(szdata, SDLNet_GetError());
			report_error(szdata);return(FALSE);
		}
		
		set = SDLNet_AllocSocketSet(1);
		if(!set)
		{
			strcpy(szdata,"SDLNet_AllocSocketSet: "); strcat(szdata, SDLNet_GetError());
			report_error(szdata);return(FALSE);
		}
		
		if (SDLNet_TCP_AddSocket(set, sock) == -1)
		{
			strcpy(szdata,"SDLNet_TCP_AddSocket: ");strcat(szdata, SDLNet_GetError());
			report_error(szdata);return(FALSE);
		}
		if (!sock) return(FALSE);
		cout<<"connected.";
		return(TRUE);
	  }

	  void TCP_SENDEROBJ::close_tcp(void)
	  {
      
		  state=STATE_IDLE;
	  	  if (sock)
		  {
			strcpy(writebuf,"close\n");
			SDLNet_TCP_Send(sock, writebuf, strlen(writebuf));
			SDLNet_TCP_Close(sock);
			sock=0;
			cout << "closed.\n";
		  } 
	  }

	  int TCP_SENDEROBJ::read_tcp(char * readbuf, int size)
	  {
		int len;
		bool reading=true;	
		char tempbuf[5000];

		reading=true;
		readbuf[0]=0;
		while (reading)
		{
			if (SDLNet_CheckSockets(set, sockettimeout) == -1)  return(TCP_BAD_REQUEST);
				
			if (SDLNet_SocketReady(sock))
			{
				if ((len = SDLNet_TCP_Recv(sock, tempbuf, size)) <= 0) return(TCP_ERROR);
				
				tempbuf[len] = '\0';
				if (strlen(readbuf) + strlen(tempbuf) <= (unsigned int)size) strcat (readbuf, tempbuf);
				if (strlen(readbuf)>=(unsigned int)size) reading=false;
			}
			else reading=false;
		
		}
//		cout << readbuf;
	    return (TCP_OK);
	  }
	
	  int TCP_SENDEROBJ::start_sending(void)
	  {

	  	if (sock)
		{
  		    strcpy(writebuf,"setheader ");
			
			generate_edf_header(writebuf+10,&header,channel);
/*			
			{HANDLE testfile;
			testfile= CreateFile("c:\\_t.tst", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			WriteFile(testfile,writebuf,strlen(writebuf),&dwWritten,NULL);
			CloseHandle(testfile);
			}
*/
			if (SDLNet_TCP_Send(sock, writebuf, strlen(writebuf))==strlen(writebuf))
			{
				read_tcp(readbuf, 6);
				if (!strcmp(readbuf,"200 OK"))   return(200);
			}
		}
		return(0);
	  }


	  void TCP_SENDEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_TCP_SENDERBOX, ghWndStatusbox, (DLGPROC)TcpSenderDlgHandler)); 
	  }

	  void TCP_SENDEROBJ::load(HANDLE hFile) 
	  {

		  load_object_basics(this);
		  load_property("host",P_STRING,&host);
		  if (load_property("edfinfos",P_STRING,&edfinfos))
			  parse_edf_header(&header, channel, edfinfos);
	//	  else get_captions();
		  height=CON_START+inports*CON_HEIGHT+5;
	  }
		
	  void TCP_SENDEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"host",P_STRING,&host);
//		  save_property(hFile,"header",P_STRING,&edfheader);
		  edfheader_to_physical(&header, (EDFHEADER_PHYSICALStruct *) edfinfos);
		  edfchannels_to_physical(channel,edfinfos+256,header.channels);
		  save_property(hFile,"edfinfos",P_STRING,&edfinfos);	  
	  }

	  
	  void TCP_SENDEROBJ::work(void) 
	  {
		int x;
		float act,fact;
		char tmp[20];
		
		if ((inports==0)||(state!=STATE_WRITING)||(!sock)) return;
	
		packetcount++;
		sprintf(writebuf,"! %d %d",packetcount,header.channels);
		for (x=0;x<header.channels;x++)
		{
			fact=((float)(channel[x].digmax-channel[x].digmin))/((float)(channel[x].physmax-channel[x].physmin));
			act=in_ports[x].value-(float)channel[x].physmin;
			sprintf(tmp," %d",(int)(act+(float)channel[x].digmin));
			strcat(writebuf,tmp);
		}
		strcat(writebuf,"\n");
		SDLNet_TCP_Send(sock, writebuf, strlen(writebuf));
	
		SDLNet_CheckSockets(set, 0);
				
		if (SDLNet_SocketReady(sock))
			SDLNet_TCP_Recv(sock, readbuf, 100);
		
		if (!(packetcount%1000))
		{		
			sprintf(szdata,"%d Packets sent",packetcount);
			if (hDlg==ghWndToolbox) 
				add_to_listbox(hDlg,IDC_LIST, szdata); 
		}
		
	  }


TCP_SENDEROBJ::~TCP_SENDEROBJ()
	  {
		// free object
		if (sock)
		{
			SDLNet_TCP_DelSocket(set, sock);
			SDLNet_TCP_Close(sock);
		}
	  }  
