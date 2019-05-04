/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_TCP_RECEIVE.CPP:  contains functions for the Neuro Server data reception
  Based on SDL_net-Code by Jeremy Wilkerson

  This object can connect to a running neuroserver and receive a stream of signals
  in the EDF-format. the signals are presented to the object's output ports

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_tcp_receive.h"


 

int get_connections(HWND hDlg,	TCP_RECEIVEOBJ * st)
{
	char writebuf[100],szdata[100];
	char readbuf[readbuflength];
	int result,x;

	st->clear_buffer();
	strcpy(writebuf,"status\n");
	result = SDLNet_TCP_Send(st->sock, writebuf, strlen(writebuf));
	add_to_listbox(hDlg,IDC_LIST, "sending:STATUS");
	st->read_tcp(readbuf, sizeof(readbuf)-1);
	x=get_int(readbuf,0,&result);
	if (result!=200) {   add_to_listbox(hDlg,IDC_LIST,"Could not get Status.");return(FALSE);}
	add_to_listbox(hDlg,IDC_LIST, "OK");
	x=get_int(readbuf,x,&result);

	SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_RESETCONTENT,0,0);
	while (readbuf[x]) 
	{
		x=get_int(readbuf,x,&result);
		if (readbuf[x])
		{
			int pos;
			sprintf(szdata,"%d",result);
			pos=strlen(szdata);
			while ((readbuf[x])&&(readbuf[x]!=10)&&(readbuf[x]!=13)) szdata[pos++]=readbuf[x++];
			szdata[pos]=0;
			if (strstr(szdata,"EEG"))
			{
 				SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) szdata);
				SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_SETITEMDATA, 
					(WPARAM) SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_GETCOUNT,0,0)-1, 
					(LPARAM) result) ;
			}
		}
	}

	if (SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_GETCOUNT,0,0)>0)
	{
		SetDlgItemText(hDlg, IDC_SELECTCOMBO, "select Stream");
		EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), TRUE);
	}
	else 
	{ 
		SetDlgItemText(hDlg, IDC_SELECTCOMBO, "no Stream available");
		EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), FALSE);
	}
	return(TRUE);
}



LRESULT CALLBACK TcpReceiveDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	TCP_RECEIVEOBJ * st;
	char writebuf[100],szdata[100];
	char readbuf[readbuflength];
	int result;
	static int actchn;

	st = (TCP_RECEIVEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_TCP_RECEIVER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_HOST, st->host);
			    EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), FALSE);
				actchn=0;
				update_header(hDlg,&st->header);
				update_channelcombo(hDlg, st->channel, st->header.channels);
				update_channel(hDlg,st->channel,actchn);

				if (st->sock) 
				{	
					char actsession[30];

					EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), FALSE);

					if (st->watching)
					{
						add_to_listbox(hDlg,IDC_LIST, "Watching Values from Server.");
						EnableWindow(GetDlgItem(hDlg, IDC_WATCH), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
					}
					else
					{
						add_to_listbox(hDlg,IDC_LIST, "Socket connected.");
						if (st->streamnum>=0)  EnableWindow(GetDlgItem(hDlg, IDC_WATCH), TRUE);
						else { EnableWindow(GetDlgItem(hDlg, IDC_WATCH), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), TRUE); }
						EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
						get_connections(hDlg,st);
					}
					if (st->streamnum>=0)
					{	sprintf(actsession,"%d:EEG",st->streamnum);
						SetDlgItemText(hDlg, IDC_SELECTCOMBO, actsession);
					}
				}
				else
				{	
					add_to_listbox(hDlg,IDC_LIST, "No Socket connected.");
					EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_WATCH), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				}

				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 
			case IDC_CONNECT:
				GetDlgItemText(hDlg, IDC_HOST, st->host, sizeof(st->host));
				if ((strlen(st->host)<8)||(!st->connect())) 
				{ 
					add_to_listbox(hDlg,IDC_LIST, "Could not connect to Server"); 
					break;
				}

				SendDlgItemMessage(hDlg,IDC_LIST, LB_ADDSTRING, 0, (LPARAM) "Socket connection successful.");
				strcpy(writebuf,"display\n");
				result = SDLNet_TCP_Send(st->sock, writebuf, strlen(writebuf));
				add_to_listbox(hDlg,IDC_LIST, "sending:DISPLAY");
	 			st->read_tcp(readbuf, readbuflength);
				strncpy(szdata,readbuf,6);szdata[6]=0;
				if (strcmp(szdata,"200 OK")) { add_to_listbox(hDlg,IDC_LIST,"Could not select Display-mode"); break;}
				add_to_listbox(hDlg,IDC_LIST, "OK");

				if (!st->sock) { add_to_listbox(hDlg,IDC_LIST,"Socket not connected"); break;}


				if (!get_connections(hDlg,st))
				 { 
					add_to_listbox(hDlg,IDC_LIST, "No Sessions available.");
					st->reset();
					InvalidateRect(ghWndMain,NULL,TRUE);
				    break;
				 }
				EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);

				break; 

			case IDC_SELECTCOMBO:
				 if (HIWORD(wParam)!=CBN_SELCHANGE) break;
  				 if (!st->sock) 
				 { 
					add_to_listbox(hDlg,IDC_LIST, "No Channels available."); 
				    st->streamnum=-1;
				    st->out[0].from_port=-1;
					st->outports=0;
				    break;
				 }
				 
				 st->streamnum=SendDlgItemMessage(hDlg,IDC_SELECTCOMBO,CB_GETITEMDATA, 
								(WPARAM) SendDlgItemMessage(hDlg,IDC_SELECTCOMBO, CB_GETCURSEL , 0, 0), 0);
				 st->clear_buffer();
				 // SELECT SESSION
				 sprintf(writebuf,"getheader %d\n",st->streamnum);
	
				 result = SDLNet_TCP_Send(st->sock, writebuf, strlen(writebuf));
				 add_to_listbox(hDlg,IDC_LIST, "sending:GETHEADER");

				 st->read_tcp(readbuf, sizeof(readbuf)-1);
				 strncpy(szdata,readbuf,6); szdata[6]=0;
				 if (strcmp(szdata,"200 OK")) {   add_to_listbox(hDlg,IDC_LIST,"Could not get EDF-header.");break;}

				 add_to_listbox(hDlg,IDC_LIST, "OK");
				 strcpy(st->edfheader,readbuf+8);
//				 report(st->edfheader);

				 EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), FALSE);
				 st->syncloss=0;
				 add_to_listbox(hDlg,IDC_LIST, "Parsing Header");
				 parse_edf_header(&st->header,st->channel, st->edfheader);


 				 add_to_listbox(hDlg,IDC_LIST, "clear OK");
				 st->outports=st->header.channels;
				 st->height=CON_START+st->outports*CON_HEIGHT+5;
				 st->get_captions();
				update_header(hDlg,&st->header);
				update_channelcombo(hDlg, st->channel, st->header.channels);
				update_channel(hDlg,st->channel,actchn);
 				 add_to_listbox(hDlg,IDC_LIST, "OK");
				 EnableWindow(GetDlgItem(hDlg, IDC_WATCH), TRUE);
				 InvalidateRect(ghWndDesign,NULL,TRUE);
				 break; 

			case IDC_WATCH:
				if ((st->outports>0)&&(st->sock))
				{
					add_to_listbox(hDlg,IDC_LIST, "sending:WATCH");
					st->clear_buffer();
					st->syncloss=0;
					result=st->start_watching();
					if (result!=200) { add_to_listbox(hDlg,IDC_LIST,"Could not enter Watch-Mode"); break;}
					add_to_listbox(hDlg,IDC_LIST, "OK");
					EnableWindow(GetDlgItem(hDlg, IDC_WATCH), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
  //				    EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					if (!GLOBAL.running) start_timer();
					 
				} else add_to_listbox(hDlg,IDC_LIST, "No Channels available.");
				break; 
			case IDC_EMPTY:
					st->bufstart=st->bufend;
					add_to_listbox(hDlg,IDC_LIST, "buffer cleared.");
					break; 
			case IDC_STOP:
					add_to_listbox(hDlg,IDC_LIST, "sending:UNWATCH");
					st->unwatch();
					EnableWindow(GetDlgItem(hDlg, IDC_WATCH), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
//  				    EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), TRUE);
					break; 

			case IDC_CLOSE: 
					add_to_listbox(hDlg,IDC_LIST, "sending:CLOSE");
					st->reset();
					EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), TRUE);
//  				    EnableWindow(GetDlgItem(hDlg, IDC_SELECTCOMBO), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_WATCH), FALSE);
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


TCP_RECEIVEOBJ::TCP_RECEIVEOBJ(int num) : BASE_CL()	
	  {
  	    int i;
	    outports = 0;
		inports = 0;
		width=80;
		height=50;

		for (i=0;i<MAX_PORTS;i++)
		  out_ports[i].get_range=-1;

		sock=0;
		reset_header(&header);
		reset_channel(channel);
		state=0;
		watching=FALSE;
		packetcount=0;
		packetnum=0;
		streamnum=-1;
		syncloss=0;
		timestamp=0;
		bufend=0;
		bufstart=0;
		strcpy(edfheader,"none");
		strcpy(host,defaulthost);
	  }


	  int TCP_RECEIVEOBJ::connect()
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

	  void TCP_RECEIVEOBJ::get_captions(void)
	  {
		int x,i;
		char tmp[256];


		for (x=0;x<outports;x++) 
		{
				strcpy(out_ports[x].out_desc,channel[x].label);
				strcpy(tmp,channel[x].label);
				if (strlen(tmp)>8) tmp[8]='\0';
				strcpy(out_ports[x].out_name,tmp);
				for (i=0;(i<4)&&(channel[x].physdim[i]);i++) out_ports[x].out_dim[i]=channel[x].physdim[i];
				out_ports[x].out_dim[i]=0;

				out_ports[x].out_min=(float)channel[x].physmin;
				out_ports[x].out_max=(float)channel[x].physmax;
		}
//		update_dimensions();
	  }
	
	  void TCP_RECEIVEOBJ::clear_buffer(void)
	  {
		  int x;
		  for (x=0;x<1000;x++) watch_tcp(watchbuf, watchbuflength);
	  }

	  void TCP_RECEIVEOBJ::reset(void)
	  {
      /*
		int x;
	  		  for (x=0;x<MAX_CONNECTS;x++)
		  {
			  out[x].from_port=-1;
			  out[x].to_port=-1;
			  out[x].to_object=-1;
		  }
		  for (x=0;x<MAX_PORTS;x++)
		  {
			  out_ports[x].out_name[0]=0;
			  strcpy(out_ports[x].out_dim,"none");
			  strcpy(out_ports[x].out_desc,"none");
		      out_ports[x].out_min=-1.0f;
		      out_ports[x].out_max=1.0f;
		  }
		  outports=0;
		  inports=0;
		  height=50;
*/
		  if (sock)
		  {
			clear_buffer();
			strcpy(writebuf,"close\n");
			SDLNet_TCP_Send(sock, writebuf, strlen(writebuf));
			SDLNet_TCP_Close(sock);
			sock=0;
			cout << "closed.\n";
		  } 
		  streamnum=-1;
		  watching=false;

	  }

	  int TCP_RECEIVEOBJ::read_tcp(char * readbuf, int size)
	  {
		int len;
		bool reading=true;	
		char tempbuf[readbuflength];

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
		cout << readbuf;
	    return (TCP_OK);
	  }
	
	  int TCP_RECEIVEOBJ::watch_tcp(char * readbuf, int size)
	  {
		int len,x,socks_ready,b,temp;
		int stream,channels;
		char szdata[200];
		float fact;
	
	
		readbuf[0]=0;
	
		if ((socks_ready=SDLNet_CheckSockets(set, 0)) == -1)
		{ cout << "TCP Check Socket error"; return(TCP_ERROR);	}
		
		if (SDLNet_SocketReady(sock))
		{
			if ((len = SDLNet_TCP_Recv(sock, readbuf, size)) <= 0) return(TCP_ERROR);
			readbuf[len] = '\0';
			len=0;
			while (readbuf[len])
			{
				switch (state)
				{
					case 0: if (readbuf[len++]=='!') {state=1; ppos=0;} 
							break;
					case 1: if ((readbuf[len]==10)||(readbuf[len]==13)) state=2;
							else packet[ppos++]=readbuf[len];
							len++;
							break;
					case 2: packetcount++;
						if (++bufend>=samplebuflen) bufend=0; 
						if (bufend==bufstart) 
						{  
							if (hDlg==ghWndToolbox) 
								add_to_listbox(hDlg,IDC_LIST, "TCP-buffer overrun");
							if (++bufstart>=samplebuflen) bufstart=0; 
						}
						packet[ppos]=0;
						ppos=get_int(packet,0,&stream);
						ppos=get_int(packet,ppos,&temp);
						packetnum++; if (packetnum==64) packetnum=0;
						if (packetnum !=temp) {syncloss++; GLOBAL.syncloss++; packetnum=temp;}
						ppos=get_int(packet,ppos,&channels);
						if (channels==outports)
						{
							for (x=0;x<channels;x++)
							{
								ppos=get_int(packet,ppos,&b);
								fact=(channel[x].physmax-channel[x].physmin)/(float)(channel[x].digmax-channel[x].digmin);
								b-=channel[x].digmin;
								
								channel[x].buffer[bufend]=(short)((float)b*fact+(float)channel[x].physmin);
							}
						}
						else if (hDlg==ghWndToolbox) 
								add_to_listbox(hDlg,IDC_LIST, "channel count error");
						
						
	
						if (!(packetcount%1000))
						{
							int buflen;
							if (bufstart<=bufend) buflen=bufend-bufstart; else buflen=bufend-bufstart+watchbuflength;
							sprintf(szdata,"%d Packets read, the buffer holds %d Packets\n",packetcount, buflen);
							//cout << szdata;
							if (hDlg==ghWndToolbox) 
								add_to_listbox(hDlg,IDC_LIST, szdata); 
						}
						state=0;
						break;
					default: state=0;
						break;
				}
			
			}
		}
		return (TCP_OK);
	  }

	  int TCP_RECEIVEOBJ::start_watching(void)
	  {
	  	if (sock)
		{
			clear_buffer();
  		    sprintf(writebuf,"watch %d\n",streamnum);
			if (SDLNet_TCP_Send(sock, writebuf, strlen(writebuf))==strlen(writebuf))
			{
				read_tcp(watchbuf, 6);
				if (!strcmp(watchbuf,"200 OK"))
				{
				   watching=TRUE;
				   cout << "start watching.\n";
				   packetcount=0;bufstart=0; bufend=0;
				   QueryPerformanceCounter((_LARGE_INTEGER *)&timestamp);
				   return(200);
				}
			}
			SDLNet_TCP_Close(sock);
		    sock=0;
		}
		watching=FALSE;
/*		if (hDlg==ghWndToolbox)
		{
			close_toolbox();
			actobject=this;
			make_dialog();
		}*/
		return(0);
	  }


	  void TCP_RECEIVEOBJ::unwatch(void)
	  {
	  	if (sock)
		{
			clear_buffer();
			strcpy(writebuf,"unwatch\n");
			SDLNet_TCP_Send(sock, writebuf, strlen(writebuf));
	 		cout << "stopped watching.\n";		
		}
		watching=FALSE;
	  }

	  void TCP_RECEIVEOBJ::session_stop(void) 
	  {  
			if (watching) unwatch();
	  }

	  void TCP_RECEIVEOBJ::session_reset(void) 
	  {  
			if (watching) unwatch();
	  }

	  void TCP_RECEIVEOBJ::session_start(void) 
	  {  
			if (watching) start_watching();
	  }

	  void TCP_RECEIVEOBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_TCP_RECEIVEBOX, ghWndStatusbox, (DLGPROC)TcpReceiveDlgHandler)); 
	  }

	  void TCP_RECEIVEOBJ::load(HANDLE hFile) 
	  {

		  load_object_basics(this);
		  load_property("host",P_STRING,&host);
		  load_property("header",P_STRING,&edfheader);
		  parse_edf_header(&header, channel, edfheader);
		  outports=header.channels;
		  height=CON_START+outports*CON_HEIGHT+5;
		  get_captions();
	  }
		
	  void TCP_RECEIVEOBJ::save(HANDLE hFile) 
	  {
	  	  save_object_basics(hFile, this);
		  save_property(hFile,"host",P_STRING,&host);
		  save_property(hFile,"header",P_STRING,&edfheader);	  
	  }

	  
	  void TCP_RECEIVEOBJ::work(void) 
	  {
		int x;
		if ((outports==0)||(watching==FALSE)) return;
	    if (watch_tcp(watchbuf,sizeof(watchbuf))==TCP_ERROR)  
		{	if (hDlg==ghWndToolbox) add_to_listbox(hDlg,IDC_LIST, "--WATCH-ERROR---");
		}
		
		if (bufend!=bufstart)
		{
			for (x=0;x<outports;x++)
				pass_values(x,(float)channel[x].buffer[bufstart]);

			if (++bufstart>=samplebuflen) bufstart=0;
			//bufstart=bufend;
		}

		if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
		{
			SetDlgItemInt(hDlg, IDC_STATUS, syncloss, 0); 
		}
	  }


TCP_RECEIVEOBJ::~TCP_RECEIVEOBJ()
	  {
		// free object
		if (sock)
		{
			SDLNet_TCP_DelSocket(set, sock);
			SDLNet_TCP_Close(sock);
		}
	  }  
