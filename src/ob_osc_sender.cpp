/* -----------------------------------------------------------------------------

  BrainBay  Version 2.3 (04/2019), contact: chris@shifz.org
  
  MODULE: OB_OSC_SENDER.CPP:  contains functions for sending OSC messages via UDP

  special thanks to 
  http://headerphile.com/sdl2/sdl2-part-12-multiplayer/
  for the SDL-net UDP example

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_osc_sender.h"


LRESULT CALLBACK OscSenderDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	OSC_SENDEROBJ * st;
	int result;
	static int actchn;

	
	st = (OSC_SENDEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_OSC_SENDER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_HOST, st->host);


				actchn=0;
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
				break; 

			case IDC_START:
				if ((st->inports>0))
				{
					st->udpConnection->Send("hallo");
					add_to_listbox(hDlg,IDC_LIST, "Starting sending packages.");
				} else add_to_listbox(hDlg,IDC_LIST, "No Channels available.");
				break; 
			case IDC_STOP:
					add_to_listbox(hDlg,IDC_LIST, "Stop sending.");
					break; 

			case IDC_CLOSE: 
					add_to_listbox(hDlg,IDC_LIST, "Closing connection");
					st->close_tcp();
 				    InvalidateRect(ghWndDesign,NULL,TRUE);
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


OSC_SENDEROBJ::OSC_SENDEROBJ(int num) : BASE_CL()	
	  {
  	    int i,x;
	    outports = 0;
		inports = 1;
		width=80;
		height=50;

		for (i=0;i<MAX_PORTS;i++)
		  out_ports[i].get_range=-1;

		timestamp=0;
		strcpy(host,defaulthost);

		std::string IP = "127.0.0.1";
		udpConnection = new UDPConnection();
		udpConnection->Init(IP, OSCPORT, LOCALPORT);

	  }

	  void OSC_SENDEROBJ::get_captions(void)
	  {
		int x; 
		for (x=0;x<inports;x++) 
		{
			/*
				strcpy(channel[x].label,in_ports[x].in_desc);
				strcpy(channel[x].physdim,in_ports[x].in_dim);
				channel[x].physmin=(int)in_ports[x].in_min;
				channel[x].physmax=(int)in_ports[x].in_max;
				*/
		}
		//header.channels=inports-1;

	  }


  	  void OSC_SENDEROBJ::update_inports(void)
	  {
		//if (state!=STATE_WRITING)
		{
			inports=count_inports(this);
			//header.channels=inports-1;
			height=CON_START+inports*CON_HEIGHT+5;
			InvalidateRect(ghWndDesign,NULL,TRUE);
		}
	  }


	  void OSC_SENDEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
	  }


	  int OSC_SENDEROBJ::connect()
	  {
		cout<<"connected.";
		return(TRUE);
	  }


	  void OSC_SENDEROBJ::close_tcp(void)
	  {
	  }


	  void OSC_SENDEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_OSC_SENDERBOX, ghWndStatusbox, (DLGPROC)OscSenderDlgHandler)); 
	  }

	  void OSC_SENDEROBJ::load(HANDLE hFile) 
	  {

		  load_object_basics(this);
		  load_property("host",P_STRING,&host);
		  height=CON_START+inports*CON_HEIGHT+5;
	  }
		
	  void OSC_SENDEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"host",P_STRING,&host);
//		  save_property(hFile,"header",P_STRING,&edfheader);
//		  save_property(hFile,"edfinfos",P_STRING,&edfinfos);	  
	  }

	  
	  void OSC_SENDEROBJ::work(void) 
	  {
		int x;
		float act,fact;
		char tmp[20];
		
	/*
		packetcount++;
		sprintf(writebuf,"! %d %d",packetcount,header.channels);
		for (x=0;x<header.channels;x++)
		{
			fact=((float)(channel[x].digmax-channel[x].digmin))/((float)(channel[x].physmax-channel[x].physmin));
			act=in_ports[x].value-(float)channel[x].physmin;
			sprintf(tmp," %d",(int)(act+(float)channel[x].digmin));
			strcat(writebuf,tmp);
		}
		*/
		/*
		if (((int)(packetcount/1000))*1000==packetcount)
		{		
			sprintf(szdata,"%d Packets sent",packetcount);
			if (hDlg==ghWndToolbox) 
				add_to_listbox(hDlg,IDC_LIST, szdata); 
		}
		*/
		
	  }


OSC_SENDEROBJ::~OSC_SENDEROBJ()
	  {
		// free object
	  }  
