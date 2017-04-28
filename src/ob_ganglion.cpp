/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
  
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
#define GANGLIONHUB_PATH "GanglionHub\\"

#define STATE_IDLE 0
#define STATE_READY 1
#define STATE_WRITING 2
/* Indexes of indicator colors */
enum {
	CI_RED,
	CI_GREEN,
	CI_YELLOW,
	CI_PURPLE,
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
	RGB(150,150, 150)
};

COLORREF ganglion_chncol[MAX_SIGNALS]={CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY};


/* Driver library handle */
GANGLIONOBJ * GANGLION_OBJ=NULL;

float ganglion_chn[5];
char DevTab[2][10]={"test1","test2"};

/*----------------------------------------------------------------------*/


/* Update specified indicator of measurement state at the level of user interface.
	data argument pass additional information specific to a given indicator. */
void NdUserInd()
{
       // chncol[ofs]=  stat<=ND_SIG_LOSS ? SigInd[stat] : CI_GRAY;				
}

/* Process received samples */
void NdProcSamples()
{
	// ganglion_chn[i]=((float)ch->samps[*n] * coeff[i]);
	// write_logfile("chn %d (%.2f * %.12f) -> %.2f",i,(float)ch->samps[*n],coeff[i],ganglion_chn[i]); 
	// process_packets();
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

		      for (int t = 0; t<2; t++) 
				SendDlgItemMessage( hDlg, IDC_GANGLION_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) DevTab[t] ) ;
	  
			  SetDlgItemText(hDlg,IDC_NB_DEVICECOMBO,st->device);
			  SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,st->archivefile);
			  st->update_channelinfo();
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_GANGLION_DEVICECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   int sel;
					    sel=SendDlgItemMessage(hDlg, IDC_NB_DEVICECOMBO, CB_GETCURSEL, 0, 0 ) ;
						strcpy (st->device,DevTab[sel]);
						// if (DevCtx>=0) NdCloseDevContext(DevCtx);
						// DevCtx=NdOpenDevContext(st->device);
				        st->update_channelinfo();
						//InvalidateRect(hDlg,NULL,FALSE);
					}
					break;

			case IDC_SELECT_GANGLION:
				st->sendstring_tcp("c,Ganglion-63ab,;\n");				
				break;

			case IDC_SCAN_GANGLION:
				st->sendstring_tcp("s,start,;\n");				
				break;

			case IDC_START_DATA:
				st->sendstring_tcp("k,b,;\n");				
				break;

			case IDC_STOP_DATA:
				st->sendstring_tcp("k,b,;\n");				
				break;

			case IDC_DISCONNECT_GANGLION:
				st->sendstring_tcp("d,;\n");
				break;
			
			case IDC_OPEN_GANGLION_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.gla");
					
					if (open_file_dlg(ghWndMain,st->archivefile, FT_NB_ARCHIVE, OPEN_LOAD))
					{
						st->filehandle = CreateFile(st->archivefile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
						if (st->filehandle==INVALID_HANDLE_VALUE)
						{
							SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
							report_error("Could not open Archive-File");
							get_session_length();

						}
						else
						{
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
							SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,st->archivefile);
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RESETBUTTON,0);
							st->filemode=FILE_READING;
							get_session_length();

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
						}
					}
				break;
			case IDC_CLOSE_GANGLION_ARCHIVE:
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
					get_session_length();
					GLOBAL.addtime=0;
				}
				break;
			case IDC_REC_GANGLION_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.gla");
					if (open_file_dlg(ghWndMain,st->archivefile, FT_NB_ARCHIVE, OPEN_SAVE))
					{
						st->filehandle = CreateFile(st->archivefile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0,NULL);
						if (st->filehandle==INVALID_HANDLE_VALUE)
						{
							SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");
							report_error("Could not open Archive-File");
						}
						else
						{
							SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,st->archivefile);
							st->filemode=FILE_WRITING;
						}
					}
			break;
			case IDC_END_GANGLION_RECORDING :
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_GANGLION_ARCHIVE_NAME,"none");

				}
				break;
			}
			return TRUE;

		case WM_PAINT:
					color_button(GetDlgItem(hDlg,IDC_QCHN1),GANGLION_COLORS[ganglion_chncol[1]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN2),GANGLION_COLORS[ganglion_chncol[2]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN3),GANGLION_COLORS[ganglion_chncol[3]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN4),GANGLION_COLORS[ganglion_chncol[4]]);
					color_button(GetDlgItem(hDlg,IDC_COMVOLT),GANGLION_COLORS[ganglion_chncol[0]]);
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

		// strcpy(DrvLibName,GLOBAL.resourcepath);
		// strcat(DrvLibName,NEUROBIT_DLL);

		strcpy(archivefile,"none");
	    filehandle=INVALID_HANDLE_VALUE;
	    filemode=0;
		sock=0;

		if (connect()) report_error("could not connect th GanglionHub");
		else printf("\nConnected to GanglionHub\n");

}
	void GANGLIONOBJ::update_channelinfo(void)
	{
		/*
  	  	  NDGETVAL gv;
		  int chans,i;
		  float max_sr;

		  if (DevCtx<0) { 
			  write_logfile ("could not open Neurobit device context");
			  return;
		  }
  		  if (NdGetParam(ND_PAR_CHAN_NUM, 0, &gv) || (gv.type&~ND_T_LIST)!=ND_T_INT) 
			  return;

		  max_sr=0;
		  chans = gv.val.i;
		  outports = chans;

		  strcpy(out_ports[0].out_dim,"uV");
		  chncol[0]=CI_GREEN;
		  for (i=1; i<5; i++) chncol[i]=CI_GRAY;
	  	  for (i=0; i<chans; i++) {

		  if (!NdGetParam(ND_PAR_CH_LABEL, i, &gv)) {
				 strcpy( out_ports[i].out_name,gv.val.t);
			}
			if (!NdGetParam(ND_PAR_CH_RANGE_MAX, i, &gv)) {
				out_ports[i].out_max = gv.val.f;
			}
			if (!NdGetParam(ND_PAR_CH_RANGE_MIN, i, &gv)) {
				out_ports[i].out_min = gv.val.f;
			}

			strcpy(out_ports[i].out_dim,NdParamInfo(ND_PAR_CH_RANGE_MAX, i)->unit);
			if (!NdGetParam(ND_PAR_CH_EN, i, &gv)) {
				if (gv.val.b) {
					chncol[i+1]= CI_GREEN;
					if (!NdGetParam(ND_PAR_CH_SR, i, &gv)) {
						if (gv.val.f>max_sr) max_sr=gv.val.f;
					}
				} else chncol[i+1]=CI_GRAY;
			}
		  }
         if (max_sr>0) update_samplingrate((int)(max_sr+0.5f));

		 dev_chans=0;
		 */
		 if (!GLOBAL.loading) update_dimensions();
 	 	 reset_oscilloscopes();

		 InvalidateRect(ghWndMain,NULL,TRUE);
		 InvalidateRect(ghWndDesign,NULL,TRUE);
		 if (ghWndToolbox == hDlg) InvalidateRect(ghWndToolbox,NULL,FALSE);

	}
	
	  void GANGLIONOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_GANGLIONBOX, ghWndStatusbox, (DLGPROC)GANGLIONDlgHandler));
	  }
	  void GANGLIONOBJ::load(HANDLE hFile) 
	  {
  		  load_object_basics(this);
		  load_property("device",P_STRING,device);

	  //	  if (DevCtx>=0) NdCloseDevContext(DevCtx);
		  update_channelinfo();
 		//  GLOBAL.neurobit_available=1;
	  }
		
	  void GANGLIONOBJ::save(HANDLE hFile) 
	  {	   
		  save_object_basics(hFile, this);
		//  save_property(hFile,"test",P_INT,&test);
		  save_property(hFile,"device",P_STRING,device);
		//  save_devctx();
	  }

	  void GANGLIONOBJ::session_reset(void) 
	  {
	  }

	  void GANGLIONOBJ::session_start(void)
	  {
		  short r;

  		  if ((filehandle==INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
		  {  
				update_channelinfo();
				/*
				GLOBAL.neurobit_available=1;
				r = NdStartMeasurement(DevCtx, ND_MEASURE_NORMAL);
				if(r<0) report_error ("Invalid device, its state or measurement mode");
				else if(r>0) { report_error("Cannot connect with the Device"); 
				      SendMessage(ghWndStatusbox,WM_COMMAND, IDC_STOPSESSION,0);}
					  */
		  } 
		 // else {GLOBAL.neurobit_available=0;  }
	  }
	  void GANGLIONOBJ::session_stop(void)
	  {
		  /*
			if (DevCtx>=0) 
			{
				NdStopMeasurement(DevCtx);
				for (int t =1; t<1000;t++)
				{
					NdProtocolEngine();
					Sleep(1);
				}
			//	NdCloseDevContext(DevCtx);
			}
			//DevCtx=-1;
			*/
	  }

  	  void GANGLIONOBJ::session_pos(long pos) 
	  {	
			if(filehandle==INVALID_HANDLE_VALUE) return;
			if (pos>filelength) pos=filelength;
			SetFilePointer(filehandle,pos*(sizeof(float))*4,NULL,FILE_BEGIN);
	  } 

	  long GANGLIONOBJ::session_length(void) 
	  {
			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode==FILE_READING))
			{
				DWORD sav= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
				filelength= SetFilePointer(filehandle,0,NULL,FILE_END)/(sizeof(float))/4;
				SetFilePointer(filehandle,sav,NULL,FILE_BEGIN);
				return(filelength);
			}
		    return(0);
	  }

	  void GANGLIONOBJ::work(void) 
	  {
		  /*
			DWORD dwWritten,dwRead;

			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
			{
				ReadFile(filehandle,ganglion_chn,sizeof(float)*4, &dwRead, NULL);
				if (dwRead != sizeof(float)*4) SendMessage (ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
				else 
				{
					DWORD x= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
					x=x*1000/filelength/TTY.bytes_per_packet;
					SetScrollPos(GetDlgItem(ghWndStatusbox, IDC_SESSIONPOS), SB_CTL, x, 1);
				}
			}
*/			
			if (read_tcp(readbuf,s_readbuflength) > 0)
			{
				cout<<"received data:";
				printf(readbuf);
			}

			/*
			pass_values(0, ganglion_chn[0]);
			pass_values(1, ganglion_chn[1]);
    		pass_values(2, ganglion_chn[2]); 
    		pass_values(3, ganglion_chn[3]);  

			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_WRITING)) 
					WriteFile(filehandle,ganglion_chn,sizeof(float)*4, &dwWritten, NULL);

			if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
			{
				InvalidateRect(hDlg,NULL,FALSE);
			}
			*/
	  }

	  
	  int GANGLIONOBJ::connect()
	  {
	 	sock=0;
		if(SDLNet_ResolveHost(&ip, "localhost", GANGLIONHUB_PORT) == -1)
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
		return(TRUE);
	  }

	  void GANGLIONOBJ::close_tcp(void)
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

	  int GANGLIONOBJ::read_tcp(char * readbuf, int size)
	  {
		int len=0;
		bool reading=true;	
		char tempbuf[5000];

		if (!sock) return(-1);
		reading=true;
		readbuf[0]=0;
		// while (reading)
		{
			if (SDLNet_CheckSockets(set, sockettimeout) == -1)  return(-1);
				
			if (SDLNet_SocketReady(sock))
			{
				if ((len = SDLNet_TCP_Recv(sock, tempbuf, size)) <= 0) return(-2);
				
				tempbuf[len] = '\0';
				if (strlen(readbuf) + strlen(tempbuf) <= (unsigned int)size) strcat (readbuf, tempbuf);
				if (strlen(readbuf)>=(unsigned int)size) reading=false;
			}
			else reading=false;
		
		}
//		cout << readbuf;
	    return (len);
	  }
	
	  int GANGLIONOBJ::sendstring_tcp(char * buf)
	  {
		  if (sock)
		  {
			SDLNet_TCP_Send(sock, buf, strlen(buf));
		  }
		  return(1);
	  }

GANGLIONOBJ::~GANGLIONOBJ()
	  {
		//	GLOBAL.neurobit_available=0;
		//	if (DevCtx>=0) NdCloseDevContext(DevCtx);

		// free object
		if (sock)
		{
			sendstring_tcp("d,;\n");
			SDLNet_TCP_DelSocket(set, sock);
			SDLNet_TCP_Close(sock);
		}
	  }  
