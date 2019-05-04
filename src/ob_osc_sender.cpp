/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
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
				SetDlgItemText(hDlg, IDC_ROUTE, st->route);
				SetDlgItemInt(hDlg, IDC_PORT, st->port,0);
				if (st->sending)
					add_to_listbox(hDlg,IDC_LIST, "Sending is activated."); 
				else
					add_to_listbox(hDlg,IDC_LIST, "Sending is deactivated."); 
				SetDlgItemInt(hDlg, IDC_SENDINTERVAL, st->sendInterval,0);
				CheckDlgButton(hDlg, IDC_BLOCK_INVALID_VALUE, st->blockInvalidValue);
				CheckDlgButton(hDlg, IDC_ONLY_CHANGING, st->onlyChanging);
				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 
			case IDC_HOST:
				GetDlgItemText(hDlg, IDC_HOST, st->host, sizeof(st->host));
				break;
			case IDC_ROUTE:
				GetDlgItemText(hDlg, IDC_ROUTE, st->route, sizeof(st->route));
				break;
			case IDC_PORT:
				st->port=GetDlgItemInt(hDlg, IDC_PORT, 0, 0);
				break;
			case IDC_SENDINTERVAL:
				st->sendInterval=GetDlgItemInt(hDlg, IDC_SENDINTERVAL, 0, 0);
				break;
			case IDC_BLOCK_INVALID_VALUE:
					st->blockInvalidValue=IsDlgButtonChecked(hDlg,IDC_BLOCK_INVALID_VALUE);
                break;
			case IDC_ONLY_CHANGING:
					st->onlyChanging=IsDlgButtonChecked(hDlg,IDC_ONLY_CHANGING);
                break;

			case IDC_CONNECT:
				if (!st->udpConnection->Init(st->host,st->port,DEFAULT_LOCALPORT)) 
				{ 
					add_to_listbox(hDlg,IDC_LIST, "Could not connect to Server"); 
					break;
				} else 	add_to_listbox(hDlg,IDC_LIST, "Updated connection"); 
				break; 

			case IDC_STARTSTOP:
				if (st->sending) {
					add_to_listbox(hDlg,IDC_LIST, "Stopped sending packages.");
					st->sending = 0;
				} else {
					add_to_listbox(hDlg,IDC_LIST, "Started sending packages.");
					st->sending = 1;
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


OSC_SENDEROBJ::OSC_SENDEROBJ(int num) : BASE_CL()	
	  {
  	    int i,x;
	    outports = 0;
		inports = 1;
		width=80;
		height=50;
		sending=1;
		sendInterval=1;
		blockInvalidValue=1;
		onlyChanging=0;
	    firstSend=1;
	    intervalCount=0;

		for (i=0;i<MAX_PORTS;i++)
		  out_ports[i].get_range=-1;

		timestamp=0;
		strcpy(host,DEFAULT_HOST);
		strcpy(route,DEFAULT_ROUTE);
		port=DEFAULT_OSCPORT;

		udpConnection = new UDPConnection();
		udpConnection->Init(host, port, DEFAULT_LOCALPORT);
	  }


  	  void OSC_SENDEROBJ::session_start(void)
	  {
		  firstSend=1;
		  intervalCount=0;
	  }

  	  void OSC_SENDEROBJ::update_inports(void)
	  {
			inports=count_inports(this);
			height=CON_START+inports*CON_HEIGHT+5;
			InvalidateRect(ghWndDesign,NULL,TRUE);
	  }


	  void OSC_SENDEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
	  }

	  void OSC_SENDEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_OSC_SENDERBOX, ghWndStatusbox, (DLGPROC)OscSenderDlgHandler)); 
	  }

	  void OSC_SENDEROBJ::load(HANDLE hFile) 
	  {

		  load_object_basics(this);
		  load_property("host",P_STRING,&host);
		  load_property("route",P_STRING,&route);
		  load_property("port",P_INT,&port);
		  load_property("sendInterval",P_INT,&sendInterval);
		  load_property("blockInvalidValue",P_INT,&blockInvalidValue);
		  load_property("onlyChanging",P_INT,&onlyChanging);
		  height=CON_START+inports*CON_HEIGHT+5;

		  udpConnection->Init(host,port,DEFAULT_LOCALPORT);
	  }
		
	  void OSC_SENDEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"host",P_STRING,&host);
		  save_property(hFile,"route",P_STRING,&route);
		  save_property(hFile,"port",P_INT,&port);	  
		  save_property(hFile,"sendInterval",P_INT,&sendInterval);
		  save_property(hFile,"blockInvalidValue",P_INT,&blockInvalidValue);
		  save_property(hFile,"onlyChanging",P_INT,&onlyChanging);
	  }

	  
	  void OSC_SENDEROBJ::work(void) 
	  {
		static int packetcount=0;
		int x;
		float act;
		int doSend;
		
		if (sending) {

			intervalCount++;
			for (x=0;x<inports-1;x++)
			{
				doSend=1;
				if ((blockInvalidValue) && (in_ports[x].value==INVALID_VALUE))
					doSend=0;

				if (intervalCount<sendInterval)
					doSend=0;

				if(doSend) {
					if ((firstSend)||(!onlyChanging)||(in_ports[x].value!=lastValue[x]))  {
						sprintf(szdata,"%s%d",route,x+1);
						udpConnection->OscSendFloat(szdata,in_ports[x].value);
						packetcount++;
						if (!(packetcount%1000))
						{	
							sprintf(szdata,"%d Packets sent",packetcount);
							if (hDlg==ghWndToolbox) 
								add_to_listbox(hDlg,IDC_LIST, szdata); 
						}	
					}
					lastValue[x]=in_ports[x].value;
				}
			}
			if (intervalCount>=sendInterval) intervalCount=0;
			if (firstSend) firstSend=0;
		} 
		else
		{
		  firstSend=1;
		  intervalCount=0;
		}
	  }


OSC_SENDEROBJ::~OSC_SENDEROBJ()
	  {
		  delete (udpConnection);
	  }  
