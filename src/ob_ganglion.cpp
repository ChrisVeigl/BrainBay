/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_GANGLION.CPP:  contains the interface to the 
          OpenBCI Ganglion devices. 
  Author: Chris Veigl

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_ganglion.h"

#define MAX_SIGNALS 4
#define MAX_DEVICES 25


#define defaulthost "localhost"
#define GANGLIONHUB_PORT 10996
#define sockettimeout 50

#define STATE_IDLE 0
#define STATE_SCANNING 1
#define STATE_CONNECTED 2
#define STATE_READING 3
#define STATE_GETIMPEDANCE 4
#define STATE_INIT_BLE 5
#define STATE_CONNECTING 6

#define MAX_READBUF_LEN 8192
#define MAX_COMMAND_LEN 500

GANGLIONOBJ * GANGLION_OBJ=NULL;

char GanglionNames[MAX_DEVICES][20]={0};
int num_ganglions=0;

float MCP3912_Vref = 1.2f;  // reference voltage for ADC in MCP3912 set in hardware
float MCP3912_gain = 1.0;   // assumed gain setting for MCP3912
float scale_fac_uVolts_per_count = (MCP3912_Vref * 1000000.f) / (8388607.0 * MCP3912_gain * 1.5 * 51.0); //MCP3912 datasheet page 34. Gain of InAmp = 80

IPaddress ip; 
TCPsocket sock;
SDLNet_SocketSet set;

int  state;
int  ganglionImpedance[MAX_SIGNALS+1]={0};
char readbuf[MAX_READBUF_LEN];
char writebuf[1024];
int  intbuffer[10];

int  connect_tcp();
int  read_tcp(char * readbuf, int size);
int  sendstring_tcp(char * buf);
void close_tcp(void);

int tcpReaderThreadDone=0;
DWORD tcpReadStatId=0;
HWND dlgWindow=0;

SHELLEXECUTEINFO shellExInfo;


/* Indexes of indicator colors */
enum {
	CI_RED,
	CI_GREEN,
	CI_YELLOW,
	CI_PURPLE,
	CI_ORANGE,
	CI_GRAY,

	/* Number of color indexes. HAVE TO be at the end. */
	CI_NUM
};

COLORREF GANGLION_COLORS[CI_NUM]=
{
	RGB(255, 0, 0),
	RGB(0,255, 0),
	RGB(255,255, 0),
	RGB(204,0, 204),
	RGB(255,128, 0),
	RGB(150,150, 150)
};

COLORREF ganglion_chncol[MAX_SIGNALS]={CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY};


int prepare_fileRead (GANGLIONOBJ * st) {

	st->filehandle = CreateFile(st->archivefile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (st->filehandle==INVALID_HANDLE_VALUE) {
		st->filemode=0;
		return(0);
	}

	get_session_length();
	GLOBAL.ganglion_available=0;
	st->filemode=FILE_READING;

	GLOBAL.addtime=0;
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet;
								
	if (GetFileTime(st->filehandle, &ftCreate, &ftAccess, &ftWrite))
	{ 
		FileTimeToSystemTime(&ftWrite, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		GLOBAL.addtime=stLocal.wHour*3600 + stLocal.wMinute*60 + stLocal.wSecond+ (float)stLocal.wMilliseconds/1000;
	}
	return(1);
}

int prepare_fileWrite(GANGLIONOBJ * st) {
	st->filehandle = CreateFile(st->archivefile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0,NULL);
	if (st->filehandle==INVALID_HANDLE_VALUE)
	{
		st->filemode=0;
		return(0);
	}
	st->filemode=FILE_WRITING;
	return(1);
}


int get_integers(int * buf, char* str, int max)
{
	int i=0,actval=0,sign=1,valid=0;

	while((*str)&&(i<max)) {
	  if ((*str>='0') && (*str<='9'))
	  {
		 actval=actval*10;
		 actval=actval+(*str-'0');
		 valid=1;
	  }
	  else if ((*str=='-') && (*(str+1)>='0') && (*(str+1)<='9')) {
		  sign=-1;
		  valid=1;
	  }
	  else if (valid) {
		  buf[i]=actval*sign;
		  i++; actval=0; sign=1; valid=0;
	      // printf("integer %d received: %d\n",i,buf[i]);
	  }
	  str++;
	}
	if (valid && (i<max)) {
		  buf[i]=actval*sign;
		  i++;
	}
	return(i);
}

	  
int connect_tcp()
{
	char szdata[300];

	if (sock) close_tcp();
	sock=0;
	if(SDLNet_ResolveHost(&ip, "localhost", GANGLIONHUB_PORT) == -1)
	{
		strcpy(szdata, "SDLNet_ResolveHost: "); strcat(szdata, SDLNet_GetError());
		printf(szdata); return(FALSE);
	}
		
	sock = SDLNet_TCP_Open(&ip);
	if(!sock)
	{
		strcpy(szdata, "SDLNet_TCP_Open: "); strcat(szdata, SDLNet_GetError());
		printf(szdata);return(FALSE);
	}
		
	set = SDLNet_AllocSocketSet(1);
	if(!set)
	{
		strcpy(szdata,"SDLNet_AllocSocketSet: "); strcat(szdata, SDLNet_GetError());
		printf(szdata);return(FALSE);
	}
		
	if (SDLNet_TCP_AddSocket(set, sock) == -1)
	{
		strcpy(szdata,"SDLNet_TCP_AddSocket: ");strcat(szdata, SDLNet_GetError());
		printf(szdata);return(FALSE);
	}
	if (!sock) return(FALSE);
	return(TRUE);
}

void close_tcp(void)
{
	state=STATE_IDLE;
	if (sock)
	{
		SDLNet_TCP_DelSocket(set, sock);
		SDLNet_TCP_Close(sock);
		printf( "TCP socket closed.\n");
	} 
}
	
int sendstring_tcp(char * buf)
{
	char tmp[1024];
	strcpy(tmp,buf);
	strcat(tmp,"\r\n");
	if (!sock) return(0);
  	printf("TCP send:%s",tmp);
	SDLNet_TCP_Send(sock, tmp, strlen(tmp));
	return(1);
}

void ganglion_disconnect() {
	//sendstring_tcp("d,;\n");
	sendstring_tcp("{\"type\":\"disconnect\"}");
	Sleep(100);
	state=STATE_IDLE;
}

void ganglion_scan() {
	//sendstring_tcp("s,start,;\n");
    sendstring_tcp("{\"type\":\"scan\", \"action\": \"start\"}");
	Sleep(100);
}

void ganglion_startData() {
	//sendstring_tcp("k,b,;\n");
	sendstring_tcp("{\"type\":\"command\", \"command\": \"b\"}");
}

void ganglion_stopData() {
	//sendstring_tcp("k,s,;\n");				
	sendstring_tcp("{\"type\":\"command\", \"command\": \"s\"}");
}

void ganglion_startImpedance() {
	//sendstring_tcp("i,start,;\n");
	sendstring_tcp("{\"type\":\"impedance\", \"action\": \"start\"}");
}

void ganglion_stopImpedance() {
	//sendstring_tcp("i,stop,;\n");				
	sendstring_tcp("{\"type\":\"impedance\", \"action\": \"stop\"}");
}

void ganglion_startAccel() {
	//sendstring_tcp("a,start,;\n");
	sendstring_tcp("{\"type\":\"accelerometer\", \"action\": \"start\"}");
}

void ganglion_stopAccel() {
	//sendstring_tcp("a,stop,;\n");				
	sendstring_tcp("{\"type\":\"accelerometer\", \"action\": \"stop\"}");
}

void ganglion_initBLE() {
	if (GLOBAL.ganglion_bledongle==1) 
		sendstring_tcp("{\"type\":\"protocol\", \"action\": \"start\", \"protocol\": \"bled112\"}");
	else 
		sendstring_tcp("{\"type\":\"protocol\", \"action\": \"start\", \"protocol\": \"ble\"}");
}


void ganglion_connect() {
	char tmpstr[256];
	static int first=1;

/*
	if (state==STATE_SCANNING)
	{
   	   sendstring_tcp("{\"type\":\"scan\", \"action\": \"stop\"}");	   
	   Sleep(100);
	   state=STATE_IDLE;
	}

	if (state==STATE_READING)
	{
	   ganglion_stopData();
	   Sleep(100);
	   state=STATE_IDLE;
	}
	if (state==STATE_CONNECTED)
	{
	   ganglion_disconnect();
	}

*/
	strcpy(tmpstr,"{\"type\": \"connect\", \"name\": \"");
	strcat(tmpstr,GLOBAL.gangliondevicename);
	strcat(tmpstr,"\"}");

	sendstring_tcp(tmpstr);
//	Sleep(1500);   // dirty! wait a bit until ganglion is connected ... 
}



void update_impedances(int chn, int adjustedImpedance)
{
	 if (chn>3) return;
//     printf("got impedance for channel %d:%d\n",chn,adjustedImpedance);
     ganglionImpedance[chn]=adjustedImpedance;

     if(adjustedImpedance <= 0){ //no data yet...
        ganglion_chncol[chn]= CI_GRAY;
      } else if((adjustedImpedance > 0) && (adjustedImpedance <= 10)){ //very good signal quality
        ganglion_chncol[chn]= CI_GREEN;
      } else if((adjustedImpedance > 10) && (adjustedImpedance <= 50)){ //good signal quality
        ganglion_chncol[chn]= CI_YELLOW;
      } else if((adjustedImpedance > 50) && (adjustedImpedance <= 100)){ //acceptable signal quality
        ganglion_chncol[chn]= CI_ORANGE;
      } else if((adjustedImpedance > 100) && (adjustedImpedance <= 150)){ //questionable signal quality
        ganglion_chncol[chn]= CI_PURPLE;
      } else if(adjustedImpedance > 150){ //bad signal quality
        ganglion_chncol[chn]= CI_RED;
      }
	  InvalidateRect(dlgWindow,NULL,FALSE);
}

char * get_message(char* readbuf, char* actline) {
	char *start, *end;
	start=strstr(readbuf,"{");	
	end=strstr(readbuf,"}");
	if ( start && end && (end>start) && (end-start < MAX_COMMAND_LEN-1)) {
		while (start<=end) *actline++=*start++;
		*actline=0;
		return (start);
	}
	return (NULL);
}


int check_messagebox(char* actline) {
	char msg[MAX_COMMAND_LEN];
	char *start, *end;

	if ((start=strstr(actline,"\"message\"")) && (!strstr(actline,"stop running"))&& (!strstr(actline,"unable to write to BLED112 attribute"))) {
		strcpy (msg,start+11); 
		if ((end=strstr(msg,"\""))) {
			*end=0;
			MessageBox(NULL,msg, "OpenBCI Hub message", MB_OK|MB_TOPMOST);
			return(1);
		}
	}
	return(0);
}

void process_hub_messages(char* readbuf) {
	int cnt=0;
	char *start, *end;
	char actline[MAX_COMMAND_LEN];

	// printf("Received:%s\n",readbuf);

	while (readbuf=get_message(readbuf,actline))
	{
		switch (state) {
			case STATE_INIT_BLE:
				 printf("Hub state:init, message:%s\n",actline);
				 if (strstr(actline,"{\"action\":\"start\",\"protocol\":\"ble\",\"code\":200"))
					state=STATE_IDLE;
				 else check_messagebox(actline);
				 break;
			case STATE_SCANNING:
 			    printf("Hub state:scanning, message:%s\n",actline);
				if (start=strstr(actline,"{\"name\":\"Ganglion-")) {
					char name[MAX_COMMAND_LEN];
					strcpy (name,start+9); 
					if ((end=strstr(name,"\""))) *end=0;
   					state=STATE_SCANNING;
					int found=0;
					for (int t=0;t<num_ganglions;t++)
						if (!strcmp(GanglionNames[t],name)) found=1;

					if (!found) {
  	   					printf("found ganglion device name:%s\n",name); 
						printf(".. adding device to list!\n");
						if (dlgWindow==ghWndToolbox) {
  	   						SendDlgItemMessage(dlgWindow, IDC_GANGLION_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) name ) ;
			  				SendDlgItemMessage(dlgWindow, IDC_GANGLION_DEVICECOMBO, CB_SETCURSEL, 0, 0 ) ;
							InvalidateRect(dlgWindow,NULL,FALSE);
						}
						strcpy(GanglionNames[num_ganglions],name);
						num_ganglions++;
   	   					printf("active devices:%d\n",num_ganglions);

					}
					if (!strcmp(name,GLOBAL.gangliondevicename)) {
	   	   				printf("trying to connect to:%s\n",name);
						ganglion_connect();
						state=STATE_IDLE;
					}
				}
				break;

			default:
				if (start=strstr(actline,"{\"startByte")) {
					// we received a packet with channel values!
					start=strstr(start,"["); // beginning of channel values
					if (start) {
						cnt=get_integers(intbuffer,start,4);
						state=STATE_READING;
						process_packets();  // this triggers all signal processing !
					}
				}
				else if (start=strstr(actline,"{\"channelNumber\"")) {
					state=STATE_GETIMPEDANCE;
					cnt=get_integers(intbuffer,start+17,2);
					if (intbuffer[0]>0)  { 
						printf("Found impedance channel %d: %d\n",intbuffer[0],intbuffer[1]);
	  					update_impedances(intbuffer[0]-1,intbuffer[1]/2);
					}
				} 
				else if (strstr(actline,"{\"code\":200,\"type\":\"connect\"}")) {
					state=STATE_CONNECTED;
   	   				printf("Connected state detected!\n");
				} 
				else if (strstr(actline,"{\"action\":\"start\",\"code\":200,\"type\":\"scan\"}")) {
					state=STATE_SCANNING;
   	   				printf("Scanning state detected!\n");
				}
 			    else if (!check_messagebox(actline)) printf("Unprocessed Hub message:%s\n",actline);
				break;
		}

 	}
}

DWORD WINAPI TcpReaderProc(LPVOID lpv)
{
	int len=0;
    printf("Ganglion Reader Thread running!\n");
	state=STATE_INIT_BLE;
	ganglion_initBLE();
	
	while (!tcpReaderThreadDone) 
	{
		if (sock) {
			if (SDLNet_CheckSockets(set, sockettimeout) == -1)  
			{   printf("socket not active, closing reader thread\n");
			 	return(-1);
			}
				
			if (SDLNet_SocketReady(sock))
			{
				if ((len = SDLNet_TCP_Recv(sock, readbuf, sizeof(readbuf))) > 0)
				{
					if (len > MAX_READBUF_LEN-1) len=MAX_READBUF_LEN-1;
					readbuf[len] = '\0';
					process_hub_messages(readbuf);
				} else  { 
					printf ("read returned zero, closing reader thread\n"); 
					return(-1); 
				}
			} else Sleep(5);
		} else {
			printf ("socket closed\n");
			return (-1);
		}
	}
    printf("Ganglion Reader Thread closed\n");
	write_logfile("Ganglion Reader Thread closed");    
    return 1;
}


void establish_ganglionconnection() {

	shellExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shellExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shellExInfo.hwnd = NULL;
	shellExInfo.lpVerb = "open";
	shellExInfo.lpFile = GLOBAL.ganglionhubpath;
	shellExInfo.lpParameters = NULL;
	shellExInfo.lpDirectory = NULL;
	shellExInfo.nShow = SW_SHOW;
	shellExInfo.hInstApp = NULL;

	GLOBAL.ganglion_available=0;
    tcpReaderThreadDone=1;

	if (connect_tcp()) GLOBAL.ganglion_available=1;
	else { 
		// report_error("could not connect to GanglionHub");
		printf("\nCould not connect to OpenBCIHub ...\n",sock);
		printf("\nTrying to start %s\n",GLOBAL.ganglionhubpath);
		//if ((int)ShellExecute(NULL, "open", GLOBAL.ganglionhubpath, NULL, NULL, SW_SHOWNORMAL) < 32)
		int ret=ShellExecuteEx(&shellExInfo);
		if (!ret) {
			report_error ("Could not start OpenBCIHub.exe - please check path in Application Settings or install OpenBCIHub !");
			shellExInfo.hProcess=0;
		}
		else {
			printf("\nTrying to reconnect ...\n");
			Sleep(2000);
			if (!connect_tcp()) { 
				printf("\nConnection failed!...\n");
				report_error ("Could not connect to OpenBCIHub !");
			}
			else GLOBAL.ganglion_available=1;
		}
	}
	Sleep(100);
    tcpReaderThreadDone=0;


	if (GLOBAL.ganglion_available) {
		printf("\nConnected to OpenBCIHub, socket=%d\n",(int) sock);
		printf("\nStarting Reader Thread!\n");
		CreateThread( NULL, 1000, (LPTHREAD_START_ROUTINE) TcpReaderProc, 0, 0, &tcpReadStatId);
	}
}

void updateDialog(HWND hDlg, GANGLIONOBJ * st)
{
	switch (st->filemode)
	{
		case 0:
			EnableWindow(GetDlgItem(hDlg, IDC_REC_GANGLION_ARCHIVE), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_GANGLION_ARCHIVE), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_GANGLION_RECORDING), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_GANGLION_ARCHIVE), FALSE);			
			SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
			break;
		case FILE_READING:
			EnableWindow(GetDlgItem(hDlg, IDC_REC_GANGLION_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_GANGLION_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_GANGLION_RECORDING), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_GANGLION_ARCHIVE), TRUE);			
			SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,st->archivefile);
		break;
		case FILE_WRITING:
			EnableWindow(GetDlgItem(hDlg, IDC_REC_GANGLION_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_GANGLION_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_GANGLION_RECORDING), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_GANGLION_ARCHIVE), FALSE);			
			SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,st->archivefile);
		break;
	}
	InvalidateRect(ghWndDesign,NULL,TRUE);
}


LRESULT CALLBACK GANGLIONDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	GANGLIONOBJ * st;
	
	st = (GANGLIONOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_GANGLION)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
		      for (int t = 0; t<num_ganglions; t++) 
				SendDlgItemMessage( hDlg, IDC_GANGLION_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) GanglionNames[t] ) ;

			   // SetDlgItemText(hDlg,IDC_GANGLION_DEVICECOMBO,st->device);
			   SendDlgItemMessage( hDlg, IDC_GANGLION_BLEDONGLECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "CSR" ) ;
			   SendDlgItemMessage( hDlg, IDC_GANGLION_BLEDONGLECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "BLED112" ) ;
			   SendDlgItemMessage( hDlg, IDC_GANGLION_BLEDONGLECOMBO, CB_SETCURSEL,GLOBAL.ganglion_bledongle ,0 ) ;
			   SetDlgItemText(hDlg,IDC_GANGLION_DEVICECOMBO,GLOBAL.gangliondevicename);
			   SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,st->archivefile);
			   st->update_channelinfo();
			   updateDialog(hDlg, st);
			}
			return TRUE;
	
		case WM_CLOSE:
				dlgWindow=0;
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_GANGLION_DEVICECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   int sel;
					    sel=SendDlgItemMessage(hDlg, IDC_GANGLION_DEVICECOMBO, CB_GETCURSEL, 0, 0 ) ;
						strcpy (st->device,GanglionNames[sel]);
				        st->update_channelinfo();
						//InvalidateRect(hDlg,NULL,FALSE);
					}
					break;

			case IDC_GANGLION_BLEDONGLECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   
					    GLOBAL.ganglion_bledongle=SendDlgItemMessage(hDlg, IDC_GANGLION_BLEDONGLECOMBO, CB_GETCURSEL, 0, 0 ) ;
	 			        if (!save_settings())  report_error("Could not save Settings");
						//ganglion_initBLE();
						establish_ganglionconnection();
					}
					break;

			case IDC_CONNECT_GANGLION:
				GetDlgItemText(hDlg,IDC_GANGLION_DEVICECOMBO,st->device,100);
				strcpy (GLOBAL.gangliondevicename,st->device);
 			    if (!save_settings())  report_error("Could not save Settings");
				ganglion_connect();
				break;

			case IDC_SCAN_GANGLION:
				SendDlgItemMessage( hDlg, IDC_GANGLION_DEVICECOMBO, CB_RESETCONTENT, 0,0) ;
				num_ganglions=0;
				ganglion_disconnect();
				ganglion_scan();
				break;

			case IDC_START_DATA:
				ganglion_startData();
				break;

			case IDC_STOP_DATA:
				ganglion_stopData();
				break;

			case IDC_START_IMPEDANCECHECK:
				ganglion_startImpedance();
				break;

			case IDC_STOP_IMPEDANCECHECK:
				ganglion_stopImpedance();
				break;

			case IDC_DISCONNECT_GANGLION:
				ganglion_disconnect();
				break;
			
			case IDC_OPEN_GANGLION_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.gla");
					
					if (open_file_dlg(ghWndMain,st->archivefile, FT_GANGLION_ARCHIVE, OPEN_LOAD))
					{
						if (prepare_fileRead(st))
						{
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
							SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,st->archivefile);
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RESETBUTTON,0);

						}
						else
						{
							SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
							report_error("Could not open Archive-File");
						}
						get_session_length();
					    updateDialog(hDlg, st);
					}
				break;
			case IDC_CLOSE_GANGLION_ARCHIVE:
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
					GLOBAL.addtime=0;
				}
				st->filemode=0;
				get_session_length();
				updateDialog(hDlg, st);
				break;
			case IDC_REC_GANGLION_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.gla");
					if (open_file_dlg(ghWndMain,st->archivefile, FT_GANGLION_ARCHIVE, OPEN_SAVE))
					{
						if (!prepare_fileWrite(st)) {
							report_error("Could not open Archive-File");
						}
					}
					updateDialog(hDlg, st);
			break;
			case IDC_END_GANGLION_RECORDING :
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");

				}
				st->filemode=0;
				updateDialog(hDlg, st);
				break;
			}
			return TRUE;

		case WM_PAINT:
				color_button(GetDlgItem(dlgWindow,IDC_QCHN1),GANGLION_COLORS[ganglion_chncol[0]]); 
				color_button(GetDlgItem(dlgWindow,IDC_QCHN2),GANGLION_COLORS[ganglion_chncol[1]]);
				color_button(GetDlgItem(dlgWindow,IDC_QCHN3),GANGLION_COLORS[ganglion_chncol[2]]);
				color_button(GetDlgItem(dlgWindow,IDC_QCHN4),GANGLION_COLORS[ganglion_chncol[3]]);
			break;
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


GANGLIONOBJ::GANGLIONOBJ(int num) : BASE_CL()	
	  {
		outports = 4;
		inports = 0;
		width=135;

		device[0]=0;
		state=STATE_IDLE;
		shellExInfo.hProcess=0;

		strcpy(out_ports[0].out_name,"chn1");
	    strcpy(out_ports[0].out_dim,"uV");
	    out_ports[0].get_range=-1;
	    strcpy(out_ports[0].out_desc,"channel1");
	    out_ports[0].out_min=-500.0f;
	    out_ports[0].out_max=500.0f;

		strcpy(out_ports[1].out_name,"chn2");
	    strcpy(out_ports[1].out_dim,"uV");
	    out_ports[1].get_range=-1;
	    strcpy(out_ports[1].out_desc,"channel2");
	    out_ports[1].out_min=-500.0f;
	    out_ports[1].out_max=500.0f;

		strcpy(out_ports[2].out_name,"chn3");
	    strcpy(out_ports[2].out_dim,"uV");
	    out_ports[2].get_range=-1;
	    strcpy(out_ports[2].out_desc,"channel3");
	    out_ports[2].out_min=-500.0f;
	    out_ports[2].out_max=500.0f;

		strcpy(out_ports[3].out_name,"chn4");
	    strcpy(out_ports[3].out_dim,"uV");
	    out_ports[3].get_range=-1;
	    strcpy(out_ports[3].out_desc,"channel4");
	    out_ports[3].out_min=-500.0f;
	    out_ports[3].out_max=500.0f;

		strcpy(archivefile,"none");
	    filehandle=INVALID_HANDLE_VALUE;
	    filemode=0;
		sock=0;
        tcpReaderThreadDone=0;

		establish_ganglionconnection();
}


void GANGLIONOBJ::update_channelinfo(void)
{
		if (!GLOBAL.loading) update_dimensions();
 	 	reset_oscilloscopes();

		InvalidateRect(ghWndMain,NULL,TRUE);
		InvalidateRect(ghWndDesign,NULL,TRUE);
		if (ghWndToolbox == hDlg) InvalidateRect(ghWndToolbox,NULL,FALSE);

}
	
	void GANGLIONOBJ::make_dialog(void)
	{
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_GANGLIONBOX, ghWndStatusbox, (DLGPROC)GANGLIONDlgHandler));
		dlgWindow=hDlg;
	}
	void GANGLIONOBJ::load(HANDLE hFile) 
	{
  		load_object_basics(this);
		load_property("archivefile",P_STRING,archivefile);
		load_property("filemode",P_INT,&filemode);
		if (filemode == FILE_READING) {
			prepare_fileRead (this);
		} else if (filemode == FILE_WRITING) {
			prepare_fileWrite (this);
		}
		if (hDlg==ghWndToolbox) updateDialog(hDlg, this);
		//load_property("device",P_STRING,device);
		//if ((GLOBAL.ganglion_available) && (strlen(device)>2)) {
		//	printf("\nTrying to connect to Device: %s\n",device);
		//	ganglion_connect(device);
		//}
		update_channelinfo();
	}
		
	void GANGLIONOBJ::save(HANDLE hFile) 
	{	   
		save_object_basics(hFile, this);
		save_property(hFile,"archivefile",P_STRING,archivefile);
		save_property(hFile,"filemode",P_INT,&filemode);

		// save_property(hFile,"test",P_INT,&test);
		// save_property(hFile,"device",P_STRING,device);
	}

	void GANGLIONOBJ::session_reset(void) 
	{
	}

	void GANGLIONOBJ::session_start(void)
	{
  		if ((filehandle==INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
		{  
			update_channelinfo();
			ganglion_startData();
			//else { report_error("Cannot connect with the Device"); 
			//SendMessage(ghWndStatusbox,WM_COMMAND, IDC_STOPSESSION,0);}
		} 
	}
	void GANGLIONOBJ::session_stop(void)
	{
		ganglion_stopData();
	}

  	void GANGLIONOBJ::session_pos(long pos) 
	{	
		if(filehandle==INVALID_HANDLE_VALUE) return;
		if (pos>filelength) pos=filelength;
		SetFilePointer(filehandle,pos*(sizeof(float))*5,NULL,FILE_BEGIN);
	} 

	long GANGLIONOBJ::session_length(void) 
	{
		if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode==FILE_READING))
		{
			DWORD sav= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
			filelength= SetFilePointer(filehandle,0,NULL,FILE_END)/(sizeof(int))/5;
			SetFilePointer(filehandle,sav,NULL,FILE_BEGIN);
			return(filelength);
		}
		return(0);
	}

	void GANGLIONOBJ::work(void) 
	{
		DWORD dwWritten,dwRead;

		if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
		{
			ReadFile(filehandle,intbuffer,sizeof(int)*5, &dwRead, NULL);
			if (dwRead != sizeof(int)*5) SendMessage (ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
			else 
			{
				DWORD x= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
				x=x*1000/filelength/sizeof(int)*5  ; //TTY.bytes_per_packet;
				SetScrollPos(GetDlgItem(ghWndStatusbox, IDC_SESSIONPOS), SB_CTL, x, 1);
			}
		}

		pass_values(0, (float)intbuffer[0] * scale_fac_uVolts_per_count);
		pass_values(1, (float)intbuffer[1] * scale_fac_uVolts_per_count);
    	pass_values(2, (float)intbuffer[2] * scale_fac_uVolts_per_count); 
    	pass_values(3, (float)intbuffer[3] * scale_fac_uVolts_per_count);  

		if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_WRITING)) 
				WriteFile(filehandle,intbuffer,sizeof(int)*5, &dwWritten, NULL);

		/*
		if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
		{
			InvalidateRect(hDlg,NULL,FALSE);
		}
		*/
	}


GANGLIONOBJ::~GANGLIONOBJ()
{
	// free object
	ganglion_disconnect();
	Sleep(800);         // omg!  should be improved with synchronisation ...
	GLOBAL.ganglion_available=0;
	tcpReaderThreadDone=1;
	close_tcp();
	dlgWindow=0;
	if (shellExInfo.hProcess)
       TerminateProcess(shellExInfo.hProcess, 1);
//	killProcess("OpenBCIHub.exe");
//	Sleep(200);
}  


