/* -----------------------------------------------------------------------------
   BrainBay  -  OpenSource Biofeedback Software
  
  MODULE: Files.cpp: this Module provides global accessible File - Functions.

  create_captfile: creates a new archive file with the specified path/filename
         only P2 and P3 - firmware is supported yet. A BrainBay header with 
		 descriptions and actual devicetype / filetype is written to the beginning 
	     of the archive file.
  open captfile: tries to open the specified file, when a BrainBay header is found, 
         the devicetype / filetype - settings are updated. 
		 the number of Packets in the Archive is determined via the filesize.

  read_captfile: a byte is read from the archive (text or a integer-mode)
			and written to the TTY-read-buffer.
  write_captfile: a byte is written to the archive (text or a integer-mode)
  close_captfile: closes the archive file		

  open_file_dlg: opens a select-file-window, for load or save purposes. 
		 BrainBay File Types are filtered out using directory wildcards
  load_from_file: load a buffer from a file with given name
  save_to_file: saves a buffer to a file with given name (for tonescales and palettes)

  load_configfile: loads a configuration. all existing objects are deleted first, then 
 			 new objects are created and overloaded with data from the configuration-file.
 			 the file contains 1:1 - copies of all objects -> pointers cannot be used
  save_configfile: saves the current settings (all the objects, counters etc.)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

 --------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_midi.h"

HANDLE logfile=INVALID_HANDLE_VALUE;

void append_newline(char * str,int si)
{
	str+=si-2;
	*str++=13;
	*str=10;
}

void create_logfile()
{
	char tmpstr [300];
	strcpy(tmpstr,GLOBAL.resourcepath); 
	strcat(tmpstr,"bbay.log");
    logfile=CreateFile(tmpstr, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0,NULL);
	CloseHandle (logfile);
}

void write_logfile(char * format, ...)
{
	char actdate[10];
	char acttime[10];
	char tmpstr [512];
	SYSTEMTIME st;
	DWORD dwritten;
    va_list argp; 

	strcpy(tmpstr,GLOBAL.resourcepath); 
	strcat(tmpstr,"bbay.log");
	
    logfile=CreateFile(tmpstr, GENERIC_WRITE|GENERIC_READ , 0, NULL, OPEN_EXISTING, 0,NULL);
    SetFilePointer(logfile,0,NULL,FILE_END);

	GetSystemTime(&st);              // gets current time
	GetDateFormat(0, 0, &st, "dd.MM.yy" , actdate, 9);actdate[8]=0;
	GetTimeFormat(0, 0, &st, "HH:mm:ss" , acttime, 9);acttime[8]=0;
	wsprintf(tmpstr,"%s - %s ",actdate,acttime);
	WriteFile(logfile,tmpstr,strlen(tmpstr),&dwritten,NULL);

    va_start(argp, format); 
    vsnprintf(tmpstr,sizeof(tmpstr),format,argp); 
	WriteFile(logfile,tmpstr,strlen(tmpstr),&dwritten,NULL);
    va_end(argp); 

	WriteFile(logfile,"\r\n",2,&dwritten,NULL);
	CloseHandle(logfile);
}

HANDLE create_captfile(LPCTSTR lpFName)
{
    HANDLE hTemp;
	DWORD dwWritten;
	int t;

	CAPTFILEHEADERStruct  header;

	for(t=0;t<sizeof(header);t++)  * (((char *)&header)+t) = ' ';
	append_newline(header.description,sizeof(header.description));

	write_logfile("create capture file: %s",(char *)lpFName);

    hTemp=CreateFile(lpFName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0,NULL);
	if (hTemp==INVALID_HANDLE_VALUE)
          { report_error("CreateFile"); return (INVALID_HANDLE_VALUE); }

	strcpy(header.description, "BrainBay Archive File\r\n");
	strcpy(header.filetype, captfiletypes[CAPTFILE.filetype]);
	append_newline(header.filetype,sizeof(header.filetype));

	strcpy(header.devicetype, devicetypes[TTY.devicetype]);
	append_newline(header.devicetype,sizeof(header.devicetype));

	
    if (!WriteFile(hTemp,&header,sizeof(header), &dwWritten, NULL))  report_error("Could not write to Archive");
	
	CAPTFILE.start=TIMING.packetcounter;
	CAPTFILE.file_action=FILE_WRITING;
	GLOBAL.actcolumn=0;
	return hTemp;
}


void update_devicetype(void)
{
	switch (TTY.devicetype) 
	{
		case DEV_MODEEG_P2:
		case DEV_MODEEG_P3:
		case DEV_MONOLITHEEG_P21:
		case DEV_SBT4:
			if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
				CAPTFILE.length=(SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_END) - sizeof(CAPTFILEHEADERStruct))/BYTES_PER_PACKET[TTY.devicetype];
			else CAPTFILE.length=0;
			CAPTFILE.data_begin=sizeof(CAPTFILEHEADERStruct);
			break;
		case DEV_NIA:
			if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
				CAPTFILE.length=SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_END)/BYTES_PER_PACKET[TTY.devicetype]/2;
												//NIA 2 Channels: 2*24Bit-Sample/packet!!
			else CAPTFILE.length=0;
			CAPTFILE.data_begin=3;
			break;
		case DEV_OPI_EXPLORATION:
			if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
				CAPTFILE.length=SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_END)/BYTES_PER_PACKET[TTY.devicetype];
			else CAPTFILE.length=0;
			CAPTFILE.data_begin=0;

		default:
			if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
				CAPTFILE.length=SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_END)/BYTES_PER_PACKET[TTY.devicetype];
			else CAPTFILE.length=0;
			CAPTFILE.data_begin=0;
			break;
	}
	TTY.bytes_per_packet=BYTES_PER_PACKET[TTY.devicetype];
	TTY.amount_to_read=AMOUNT_TO_READ[TTY.devicetype];
	if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
		SetFilePointer(CAPTFILE.filehandle,CAPTFILE.data_begin,NULL,FILE_BEGIN);
	PACKET.readstate=0;
	get_session_length();
}

int open_captfile(LPCTSTR lpFName)
{
	DWORD dwRead;

	CAPTFILEHEADERStruct  header;
	CAPTFILE.offset=0;
	GLOBAL.addtime =0;

	write_logfile("opening capture file: %s: ", (char *)lpFName);
	CAPTFILE.filehandle = CreateFile(lpFName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
	{
		if(!ReadFile(CAPTFILE.filehandle, &header , sizeof(header), &dwRead, NULL)) 
		{ 
			report_error("Could not read from Archive File");
			CAPTFILE.filehandle=INVALID_HANDLE_VALUE;
			return (0);
		}
		if (strstr(header.description,"BrainBay Archive File")!=NULL)
		{
			if (!strcmp(header.filetype, captfiletypes[FILE_TEXTMODE])) CAPTFILE.filetype=FILE_TEXTMODE; 
			if (!strcmp(header.filetype, captfiletypes[FILE_INTMODE])) CAPTFILE.filetype=FILE_INTMODE;
		}
		else 
		{
			// TTY.devicetype=DEV_RAW;
			CAPTFILE.filetype=FILE_INTMODE;
		}
		strcpy(CAPTFILE.devicetype,header.devicetype);
		update_devicetype();
		CAPTFILE.file_action=FILE_READING;

		FILETIME ftCreate, ftAccess, ftWrite;
		SYSTEMTIME stUTC, stLocal;
		DWORD dwRet;
								
		if (GetFileTime(CAPTFILE.filehandle, &ftCreate, &ftAccess, &ftWrite))
		{ 
			FileTimeToSystemTime(&ftWrite, &stUTC);
			SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
			GLOBAL.addtime=stLocal.wHour*3600 + stLocal.wMinute*60 + stLocal.wSecond+ (float)stLocal.wMilliseconds/1000;
		} 

	}
	else
	{  
		if (GLOBAL.loading==0)
		{
			strcat(CAPTFILE.filename,": Archive not found.");
			report(CAPTFILE.filename);
			//strcpy(CAPTFILE.filename,"none");
		}
		return(0);
	}
	get_session_length();
	return(1);
}


void close_captfile(void)
{
	CAPTFILE.do_read=0;
	CAPTFILE.do_write=0;
	CAPTFILE.file_action=0;

	if (CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)
	{
 	    if (!CloseHandle(CAPTFILE.filehandle)) report_error("could not close Archive file");
		CAPTFILE.filehandle=INVALID_HANDLE_VALUE;
		strcpy(CAPTFILE.filename,"none");
		PACKET.readstate=0;
		PACKET.number=0;
		PACKET.old_number=0;
		//init_system_time();
		reset_oscilloscopes();

	}
	get_session_length();
	GLOBAL.addtime=0;
}


void read_captfile(int amount)
{
	DWORD dwRead;  
	char temp[500];
	char act[500];
	int actpos,actbufpos,tempint=0;
	int res;


	actpos=0;
	for(actbufpos=0;actbufpos<amount;)
	{

	  res=ReadFile(CAPTFILE.filehandle,temp, 1, &dwRead, NULL);
	  if (res &&  (dwRead )) 
	  { 
	  switch (CAPTFILE.filetype)
	  {
	    case FILE_INTMODE:
		     TTY.readBuf[actbufpos++]=(unsigned char)temp[0];
			 break;

		case FILE_TEXTMODE:
			switch (temp[0]) 
			{
				case ' ': case 10: case 13: break;
				case ',':
					    act[actpos]=0; sscanf(act,"%d",&tempint);
						TTY.readBuf[actbufpos++]=(unsigned char)tempint;
						actpos=0;
						break;
				 default: act[actpos++]=temp[0];
				 
			}
			break;
				  
		}
 	  } 
	  else //{ TTY.readBuf[0]=0; return; }
	  { 
	       SetFilePointer(CAPTFILE.filehandle,CAPTFILE.data_begin,NULL,FILE_BEGIN);
		   //res=ReadFile(CAPTFILE.filehandle,temp, 1, &dwRead, NULL);
		   //TTY.readBuf[0]=0;
 	  } 

	  

	}
}


void write_captfile(unsigned char actbyte)
{
	DWORD dwWritten;
	 char buf[10];

	switch (CAPTFILE.filetype)
	{
	  case FILE_TEXTMODE:
	      wsprintf(buf, "%d, ", (int) actbyte);
		  while (strlen(buf)<5) strcat(buf," ");
		  if (++GLOBAL.actcolumn==TTY.bytes_per_packet)
		  { 
			strcat (buf,"\r\n ");
			GLOBAL.actcolumn=0;
		  }
		  if (!WriteFile(CAPTFILE.filehandle,buf,strlen(buf), &dwWritten, NULL))  
		  {    report_error("Could not write to Archive"); 
		       close_captfile();
		  }
		  break;
	  case FILE_INTMODE:
		  buf[0]=actbyte;
		  if (!WriteFile(CAPTFILE.filehandle,buf,1, &dwWritten, NULL))  
		  {   report_error("Could not write to Archive"); 
		      close_captfile(); 
		  }
		  break;
	}
}


int open_file_dlg(HWND hDlg, char * szFileName, int type, int flag_save)
{
    OPENFILENAME ofn;
	int i=0;
    
    ZeroMemory(&ofn, sizeof(ofn));

	while (szFileName[i]!='\0') 
	{	switch (szFileName[i]) {
		   case '/':
		   case '\"':
		   case ',':
		   case '?':
		   case '|':
		   case '>':
		   case '<': szFileName[i]='_'; break;
								}
		i++;
	}
    ofn.lStructSize = sizeof(ofn); 
    ofn.hwndOwner = hDlg;
	switch (type) 
	{
	   case FT_HARMONIC:
			ofn.lpstrFilter = "Scale Files (*.sc)\0*.sc\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "sc";
			break;
	   case FT_ARCHIVE:
			ofn.lpstrFilter = "Archive Files (*.arc)\0*.arc\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "arc";
			break;
	   case FT_CONFIGURATION:
			ofn.lpstrFilter = "Config Files (*.con)\0*.con\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "con";
			break;
	   case FT_PALETTE:
			ofn.lpstrFilter = "Palette Files (*.pal)\0*.pal\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "pal";
			break;
	   case FT_WAV:
			ofn.lpstrFilter = "Sound Files (*.wav,*.mp3,*.voc,*.shn,*.aiff,*.au)\0*.wav;*.mp3;*.voc;*.shn;*.aiff;*.au\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "wav";
			break;
	   case FT_EDF:
			ofn.lpstrFilter = "EDF-Files (*.edf)\0*.edf\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "edf";
			break;
	   case FT_AVI:
			ofn.lpstrFilter = "AVI-Files (*.avi)\0*.avi\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "avi";
			break;
	   case FT_MCI:
			ofn.lpstrFilter = "MCI-Files (*.avi,*.wmv,*.mpg,*.mp3,*.wav)\0*.avi;*.wmv;*.mpg;*.mp3;*.wav\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "avi";
			break;
	   case FT_ERP:
			ofn.lpstrFilter = "ERP-signatures (*.erp)\0*.erp\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "erp";
			break;
	   case FT_TXT:
			ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "txt";
			break;
	   case FT_BMP:
			ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.txt\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "bmp";
			break;
	   case FT_DIC:
			ofn.lpstrFilter = "Dictionary Files (*.dic)\0*.dic\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "dic";
			break;
	   case FT_NB4:
			ofn.lpstrFilter = "Neurobit OPTIMA Configurations (*.nb4)\0*.nb4\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "nb4";
			break;
	   case FT_NB_ARCHIVE:
			ofn.lpstrFilter = "Neurobit Archive File (*.nba)\0*.nba\0All Files (*.*)\0*.*\0";
		    ofn.lpstrDefExt = "nba";
			break;
	}
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;

    if (flag_save==OPEN_SAVE) 
	{  ofn.Flags =OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		if(GetSaveFileName(&ofn)) 
		{
		   strcpy(szFileName,ofn.lpstrFile);
 		   return TRUE; 
		}
		else return FALSE;
	}
	else  
	{
		ofn.Flags= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        if(GetOpenFileName(&ofn))
		{
		   strcpy(szFileName,ofn.lpstrFile);
           return TRUE;
		}
		else return FALSE;
	}

}


BOOL load_from_file(LPCTSTR pszFileName, void * buffer, int size)
{
    
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
         DWORD dwRead;

         if(!ReadFile(hFile, (unsigned char *) buffer , size, &dwRead, NULL)) return FALSE;
		 CloseHandle(hFile);
		 return TRUE;
     }
	 return FALSE;    
}

BOOL save_to_file(LPCTSTR pszFileName, void * buffer, int size)
{
    
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
         DWORD dwWritten;
         if(!WriteFile(hFile, (unsigned char * )buffer, size, &dwWritten, NULL)) return FALSE;
		 CloseHandle(hFile);
		 return TRUE;
    }
	return FALSE;    
}



BOOL load_configfile(LPCTSTR pszFileName)
{

    HANDLE hFile;
	int t, act_samplingrate=DEF_PACKETSPERSECOND; 
	int act_type,num_objects, save_toolbox, save_connected, try_connect;
	const char * d_name;
	char new_name[256],szdata[20];

	SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
	write_logfile("loading design configuration: %s",(char *)pszFileName);

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
		GLOBAL.loading=1;
		d_name=pszFileName;
		while (strstr(d_name,"\\")) d_name=strstr(d_name,"\\")+1;
		strcpy(new_name,"BrainBay - ");strcat(new_name,d_name);
	    SetWindowText(ghWndMain,new_name);
		strcpy(GLOBAL.configfile,pszFileName);
		save_settings();
		save_toolbox=-1;

		save_connected=TTY.CONNECTED;
		SendMessage(ghWndStatusbox,WM_COMMAND, IDC_STOPSESSION,0);
		
		//TTY.read_pause=1;
		close_captfile();

		if (ghWndAnimation!=NULL) SendMessage(ghWndAnimation,WM_CLOSE,0,0);
		ghWndAnimation=NULL;

		close_toolbox();actobject=NULL;
		//InvalidateRect(ghWndMain, NULL, TRUE);

		while (GLOBAL.objects>0)
		    free_object(0);

		deviceobject=NULL;
		GLOBAL.run_exception=0;
		GLOBAL.minimized=FALSE;
		if (GLOBAL.main_maximized)
		{  SendMessage(ghWndMain,WM_SIZE,SIZE_RESTORED,0);		 
		   ShowWindow( ghWndMain, TRUE ); UpdateWindow( ghWndMain );
		}


	     load_next_config_buffer(hFile);
		 load_property("objects",P_INT,&GLOBAL.objects);
		 load_property("main-top",P_INT,&GLOBAL.top);
		 load_property("main-left",P_INT,&GLOBAL.left);
		 load_property("main-right",P_INT,&GLOBAL.right);
		 load_property("main-bottom",P_INT,&GLOBAL.bottom);
		 load_property("anim-top",P_INT,&GLOBAL.anim_top);
		 load_property("anim-left",P_INT,&GLOBAL.anim_left);
		 load_property("anim-right",P_INT,&GLOBAL.anim_right);
		 load_property("anim-bottom",P_INT,&GLOBAL.anim_bottom);
		 load_property("design-top",P_INT,&GLOBAL.design_top);
		 load_property("design-left",P_INT,&GLOBAL.design_left);
		 load_property("design-right",P_INT,&GLOBAL.design_right);
		 load_property("design-bottom",P_INT,&GLOBAL.design_bottom);
		 load_property("tool-top",P_INT,&GLOBAL.tool_top);
		 load_property("tool-left",P_INT,&GLOBAL.tool_left);
		 load_property("tool-right",P_INT,&GLOBAL.tool_right);
		 load_property("tool-bottom",P_INT,&GLOBAL.tool_bottom);
		 load_property("showdesign",P_INT,&GLOBAL.showdesign);
		 load_property("hidestatus",P_INT,&GLOBAL.hidestatus);
		 // load_property("locksession",P_INT,&GLOBAL.locksession);
		 load_property("showtoolbox",P_INT,&GLOBAL.showtoolbox);
		 load_property("autorun",P_INT,&GLOBAL.autorun);
		 load_property("minimized",P_INT,&GLOBAL.minimized);
		 save_toolbox=GLOBAL.showtoolbox;

		 /*
		 if (GLOBAL.left >= GetSystemMetrics(SM_CXVIRTUALSCREEN))
			 GLOBAL.left=10;
		 if (GLOBAL.top >= GetSystemMetrics(SM_CYVIRTUALSCREEN))
			 GLOBAL.top=10;
		 if (GLOBAL.tool_left >= GetSystemMetrics(SM_CXVIRTUALSCREEN))
			 GLOBAL.tool_left=100;
		 if (GLOBAL.tool_top >= GetSystemMetrics(SM_CYVIRTUALSCREEN))
			 GLOBAL.tool_top=100;
			 */

		 TTY.PORT=0;try_connect=0;
		 load_property("comport",P_INT,&TTY.PORT);		 
		 load_property("connected",P_INT,&try_connect);	 
		 load_property("bidirect",P_INT,&TTY.BIDIRECT);
		 load_property("devicetype",P_INT,&TTY.devicetype);
		 load_property("baudtype",P_INT,&TTY.BAUDRATE);
		 load_property("flow_control",P_INT,&TTY.FLOW_CONTROL);

		 if (save_connected) { update_p21state(); save_connected=TTY.CONNECTED; }
		 BreakDownCommPort();  
		 if (try_connect) { TTY.CONNECTED=SetupCommPort(TTY.PORT); update_p21state(); }
		
		 load_property("captfilename",P_STRING,CAPTFILE.filename);
		 load_property("captfiletype",P_INT,&CAPTFILE.filetype);
		 load_property("captfileoffset",P_INT,&CAPTFILE.offset);
	 	 load_property("dialoginterval",P_INT,&GLOBAL.dialog_interval);
		 load_property("drawinterval",P_INT,&GLOBAL.draw_interval);
		 load_property("samplingrate",P_INT,&act_samplingrate);
		 PACKETSPERSECOND=act_samplingrate;

		 MoveWindow(ghWndDesign,GLOBAL.design_left,GLOBAL.design_top,GLOBAL.design_right-GLOBAL.design_left,GLOBAL.design_bottom-GLOBAL.design_top,TRUE);
		 		 InvalidateRect(ghWndMain,NULL,TRUE);

		 if (!GLOBAL.hidestatus)
		 	 ShowWindow(ghWndStatusbox, TRUE); 
		 else	 ShowWindow(ghWndStatusbox,FALSE);
		     
		 num_objects=GLOBAL.objects;
		 GLOBAL.objects=0;
   		 for (t=0;t<num_objects;t++)
		 {
			act_type=load_next_config_buffer(hFile);
			if (act_type>=0)
			{	
				create_object(act_type);
				if (actobject != NULL )
				{
					actobject->load(hFile);
					link_object(actobject);
				}
				else critical_error("Could not load all objects, quitting ...\ndelete brainbay.cfg to prevent this at program startup ... ");

			}
		 }
		 CloseHandle(hFile);
		 for (t=0;t<num_objects;t++) objects[t]->update_inports();
		 update_dimensions();

		 CAPTFILE.filehandle=INVALID_HANDLE_VALUE;
		 CAPTFILE.do_read=0;
		 if (strcmp(CAPTFILE.filename,"none"))
		 {
			 if (open_captfile(CAPTFILE.filename)==0)
			 {
				 char st[150];
				 reduce_filepath(st,CAPTFILE.filename);
				 strcpy(CAPTFILE.filename,GLOBAL.resourcepath);
				 strcat(CAPTFILE.filename,"ARCHIVES\\");
				 strcat(CAPTFILE.filename,st);			 
				 open_captfile(CAPTFILE.filename);
			 }
		 }

		for (int i=0;i<GLOBAL.objects;i++)
			if (objects[i]->displayWnd) {
				SendMessage(objects[i]->displayWnd,WM_MOVE,0,0);
				SendMessage(objects[i]->displayWnd,WM_MOUSEACTIVATE,0,0);
 				InvalidateRect(objects[i]->displayWnd,NULL,TRUE);
			}

		if (GLOBAL.locksession) 
				ShowWindow(GetDlgItem(ghWndStatusbox,IDC_DESIGN), SW_HIDE);
		else 
				ShowWindow(GetDlgItem(ghWndStatusbox,IDC_DESIGN), SW_SHOW);


		 init_system_time();
		 reset_oscilloscopes();
		 PACKET.readstate=0;

 		 update_samplingrate(act_samplingrate);
		 update_devicetype();
		 get_session_length();


		 SetDlgItemInt(ghWndStatusbox,IDC_SAMPLINGRATE,act_samplingrate,0);
		 SetDlgItemText(ghWndStatusbox,IDC_STATUS,"Configuration loaded");
		 SetDlgItemText(ghWndStatusbox,IDC_TIME,"0.0");
		 SetDlgItemText(ghWndStatusbox,IDC_JUMPPOS,"0.0");
		 sprintf(szdata, "%.1f", (float)GLOBAL.session_length/(float)PACKETSPERSECOND);
		 SetDlgItemText(ghWndStatusbox,IDC_SESSLEN,szdata);
 		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,(LONG)0);
		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,(LONG)0);
		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,(LONG)0);


		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,0);
		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,1000);
		 SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,(LONG)(0));

		 if ((!GLOBAL.showdesign) || (GLOBAL.locksession))
		 {  
			 ShowWindow(ghWndDesign, FALSE); 
			 SetDlgItemText(ghWndStatusbox,IDC_DESIGN,"Show Design"); 
		 }
		 else 
		 {
			 ShowWindow(ghWndDesign,TRUE);
		     SetWindowPos(ghWndDesign,0,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
			 SetDlgItemText(ghWndStatusbox,IDC_DESIGN,"Hide Design"); 
		 }


		 MoveWindow(ghWndMain,GLOBAL.left,GLOBAL.top,GLOBAL.right-GLOBAL.left,GLOBAL.bottom-GLOBAL.top,TRUE);
		 ShowWindow( ghWndMain, TRUE ); 
		 UpdateWindow( ghWndMain ); 
		 // SetWindowPos(ghWndMain,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		 InvalidateRect(ghWndMain,NULL,TRUE);
 		 InvalidateRect(ghWndDesign,NULL,TRUE);
		 if (GLOBAL.minimized) ShowWindow(ghWndMain, SW_MINIMIZE);

		 GLOBAL.loading=0;
		 if ((GLOBAL.autorun) && (!GLOBAL.run_exception)) SendMessage(ghWndStatusbox,WM_COMMAND, IDC_RUNSESSION,0);
  	    
		 if (save_toolbox!=-1)
		 {
			GLOBAL.showtoolbox=save_toolbox;
			actobject=objects[GLOBAL.showtoolbox];
			actobject->make_dialog();
		 } 
		 
		write_logfile("load successful");
		return TRUE;
		 
    }
	write_logfile("could not load design configuration file.");
	return FALSE;    	
}

 
BOOL save_configfile(LPCTSTR pszFileName)
{
   
    HANDLE hFile;
	int t;
	int act_type;

	write_logfile("saving design configuration: %s",(char *)pszFileName);
 
    hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {

		if (!IsWindow(ghWndToolbox)) 
			GLOBAL.showtoolbox=-1;

		strcpy(GLOBAL.configfile,pszFileName);
		save_settings();
		save_property(hFile,"objects",P_INT,&GLOBAL.objects);
		save_property(hFile,"main-top",P_INT,&GLOBAL.top);
		save_property(hFile,"main-left",P_INT,&GLOBAL.left);
		save_property(hFile,"main-right",P_INT,&GLOBAL.right);
		save_property(hFile,"main-bottom",P_INT,&GLOBAL.bottom);
		save_property(hFile,"anim-top",P_INT,&GLOBAL.anim_top);
		save_property(hFile,"anim-left",P_INT,&GLOBAL.anim_left);
		save_property(hFile,"anim-right",P_INT,&GLOBAL.anim_right);
		save_property(hFile,"anim-bottom",P_INT,&GLOBAL.anim_bottom);
		save_property(hFile,"design-top",P_INT,&GLOBAL.design_top);
		save_property(hFile,"design-left",P_INT,&GLOBAL.design_left);
		save_property(hFile,"design-right",P_INT,&GLOBAL.design_right);
		save_property(hFile,"design-bottom",P_INT,&GLOBAL.design_bottom);
		save_property(hFile,"tool-top",P_INT,&GLOBAL.tool_top);
		save_property(hFile,"tool-left",P_INT,&GLOBAL.tool_left);
		save_property(hFile,"tool-right",P_INT,&GLOBAL.tool_right);
		save_property(hFile,"tool-bottom",P_INT,&GLOBAL.tool_bottom);
		save_property(hFile,"showdesign",P_INT,&GLOBAL.showdesign);
		save_property(hFile,"hidestatus",P_INT,&GLOBAL.hidestatus);
		// save_property(hFile,"locksession",P_INT,&GLOBAL.locksession);
		save_property(hFile,"showtoolbox",P_INT,&GLOBAL.showtoolbox);
		save_property(hFile,"autorun",P_INT,&GLOBAL.autorun);
		save_property(hFile,"minimized",P_INT,&GLOBAL.minimized);

		save_property(hFile,"comport",P_INT,&TTY.PORT);
		save_property(hFile,"bidirect",P_INT,&TTY.BIDIRECT);
		save_property(hFile,"connected",P_INT,&TTY.CONNECTED);
		save_property(hFile,"devicetype",P_INT,&TTY.devicetype);
		save_property(hFile,"baudtype",P_INT,&TTY.BAUDRATE);
		save_property(hFile,"flow_control",P_INT,&TTY.FLOW_CONTROL);

		save_property(hFile,"captfilename",P_STRING,CAPTFILE.filename);
		save_property(hFile,"captfiletype",P_INT,&CAPTFILE.filetype);
		save_property(hFile,"captfileoffset",P_INT,&CAPTFILE.offset);
		save_property(hFile,"dialoginterval",P_INT,&GLOBAL.dialog_interval);
		save_property(hFile,"drawinterval",P_INT,&GLOBAL.draw_interval);
		save_property(hFile,"samplingrate",P_INT,&PACKETSPERSECOND);
	
		save_property(hFile,"end Object",P_END,NULL);

		for (t=0;t<GLOBAL.objects;t++)
		{  act_type=objects[t]->type;
		   save_property(hFile,"next Object",P_INT,&act_type);
		   objects[t]->save(hFile);
		   store_links(hFile,objects[t]);
		   save_property(hFile,"end Object",P_END,NULL);
		}

		CloseHandle(hFile);
		return TRUE;
    }
	return FALSE;    
	
}

BOOL save_settings(void)
{
   
    HANDLE hFile;
	int t,x;
	char settingsfilename[256],midipname[256];

	strcpy(settingsfilename,GLOBAL.resourcepath);
	strcat(settingsfilename,"brainbay.cfg");

	write_logfile("saving program settings.");

    hFile = CreateFile(settingsfilename, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return FALSE;

	save_property(hFile,"samplingrate",P_INT,&PACKETSPERSECOND);
	save_property(hFile,"dialoginterval",P_INT,&GLOBAL.dialog_interval);
	save_property(hFile,"drawinterval",P_INT,&GLOBAL.draw_interval);
	save_property(hFile,"startup",P_INT,&GLOBAL.startup);
	save_property(hFile,"autorun",P_INT,&GLOBAL.autorun);
	save_property(hFile,"configfile",P_STRING,GLOBAL.configfile);
	save_property(hFile,"use_cvcapture",P_INT,&GLOBAL.use_cv_capture);
	save_property(hFile,"emotivpath",P_STRING,GLOBAL.emotivpath);
	save_property(hFile,"ganglionhubpath",P_STRING,GLOBAL.ganglionhubpath);
	save_property(hFile,"gangliondevicename",P_STRING,GLOBAL.gangliondevicename);
	save_property(hFile,"ganglionbledevice",P_INT,&GLOBAL.ganglion_bledongle);
	save_property(hFile,"neurobit_device",P_STRING,GLOBAL.neurobit_device);
	save_property(hFile,"addtime",P_INT,&GLOBAL.add_archivetime);
	save_property(hFile,"startdesign",P_INT,&GLOBAL.startdesign);
	save_property(hFile,"startdesignpath",P_STRING,GLOBAL.startdesignpath);
	save_property(hFile,"locksession",P_INT,&GLOBAL.locksession);
	save_property(hFile,"statusHeight",P_INT,&GLOBAL.statusWindowHeight);
	save_property(hFile,"statusHeightPlayer",P_INT,&GLOBAL.statusWindowHeightWithPlayer);
	save_property(hFile,"statusMargin",P_INT,&GLOBAL.statusWindowMargin);
	save_property(hFile,"statusMarginPlayer",P_INT,&GLOBAL.statusWindowMarginWithPlayer);

	x=0;
	for (t=0;t<GLOBAL.midiports;t++)
		if (MIDIPORTS[t].midiout)
		{
			x++;
			sprintf(midipname,"midiport%d",x);
			save_property(hFile,midipname,P_STRING,MIDIPORTS[t].portname);
		}
	save_property(hFile,"midiports",P_INT,&x);
	CloseHandle(hFile);	
	return(TRUE);
	
}

BOOL load_settings(void)
{
   
    HANDLE hFile;
	int t,x,c,f;
	char settingsfilename[256],midipname[256],tempname[256];

	strcpy(settingsfilename,GLOBAL.resourcepath);
	strcat(settingsfilename,"brainbay.cfg");

	write_logfile("load setting from: %s",settingsfilename);
    hFile = CreateFile(settingsfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return FALSE;


	load_next_config_buffer(hFile);
	load_property("samplingrate",P_INT,&PACKETSPERSECOND);
	load_property("dialoginterval",P_INT,&GLOBAL.dialog_interval);
	load_property("drawinterval",P_INT,&GLOBAL.draw_interval);
	load_property("startup",P_INT,&GLOBAL.startup);
	load_property("autorun",P_INT,&GLOBAL.autorun);
	load_property("configfile",P_STRING,GLOBAL.configfile);
	load_property("use_cvcapture",P_INT,&GLOBAL.use_cv_capture);
	load_property("addtime",P_INT,&GLOBAL.add_archivetime);
	load_property("emotivpath",P_STRING,GLOBAL.emotivpath);
	load_property("ganglionhubpath",P_STRING,GLOBAL.ganglionhubpath);
	load_property("gangliondevicename",P_STRING,GLOBAL.gangliondevicename);
	load_property("ganglionbledevice",P_INT,&GLOBAL.ganglion_bledongle);
	load_property("neurobit_device",P_STRING,GLOBAL.neurobit_device);
	load_property("startdesign",P_INT,&GLOBAL.startdesign);
	load_property("startdesignpath",P_STRING,GLOBAL.startdesignpath);
	load_property("locksession",P_INT,&GLOBAL.locksession);
	load_property("statusHeight",P_INT,&GLOBAL.statusWindowHeight);
	load_property("statusHeightPlayer",P_INT,&GLOBAL.statusWindowHeightWithPlayer);
	load_property("statusMargin",P_INT,&GLOBAL.statusWindowMargin);
	load_property("statusMarginPlayer",P_INT,&GLOBAL.statusWindowMarginWithPlayer);


	load_property("midiports",P_INT,&x);
	for (c=0;c<GLOBAL.midiports;c++) midiOutClose(MIDIPORTS[c].midiout);
	for (t=1;t<=x;t++)
	{
		sprintf(midipname,"midiport%d",t);
		load_property(midipname,P_STRING,tempname);
		f=0;
		for (c=0;c<GLOBAL.midiports;c++)
		{
			if (!strcmp(tempname,MIDIPORTS[c].portname))
			{
				write_logfile("open midi port: %s", tempname);
				midi_open_port(&(MIDIPORTS[c].midiout),c);
				f=1;
			}
		}
		if (!f) {strcat (tempname," not found"); report_error (tempname); }
	}

	CloseHandle(hFile);	
	return(TRUE);
	
}

int load_property(char * desc,int type, void * ad)
{
	char newdesc[60];
	char param[20000];
	char * propstart;
 	int pos;


	low_chars(newdesc, desc);
  //  report(newdesc);
	strcat(newdesc,"=");

	propstart=strstr(GLOBAL.configbuffer,newdesc);
	if (propstart) 
	{
		while ((*propstart)&&(*propstart!='=')&&(*propstart!=10)&&(*propstart!=13)) propstart++;
		if (*propstart=='=')
		{
		  propstart++;
		  while (*propstart==' ') propstart++;
		  pos=0;
		  while ((*propstart!=10)&&(*propstart!=13)&&(*propstart)&&pos<20000) param[pos++]=*propstart++;
		  param[pos]=0;
		  switch (type)
		  {
			case P_INT: sscanf(param,"%d",(int *)ad); return(1);
			case P_FLOAT: sscanf(param,"%f",(float *)ad); return(1);
		    case P_STRING: strcpy ((char *)ad,param); return(1);
		  }
		}
	}
	return(0);
}



void save_property(HANDLE hFile, char * desc,int type, void * ad)
{
	char str[20000];
    DWORD dwWritten;
	char nl[3]="\r\n";

	low_chars(str,desc);if (strcmp(str,"end object")) strcat(str,"="); else strcat(str,nl);

	WriteFile(hFile, str, strlen(str), &dwWritten, NULL);
	if (type!=P_END)
	{
		switch (type)
		{
		  case P_INT: sprintf(str,"%d",*((int *)ad)); break;
		  case P_FLOAT: sprintf(str,"%.6f",*((float *)ad)); break;
          case P_STRING: strcpy (str,(char *)ad); break;
		}
		strcat(str,nl);
		WriteFile(hFile, str, strlen(str), &dwWritten, NULL);

	}
}


void save_object_basics(HANDLE hFile, BASE_CL * actobj)
{
	int t;
	char temp[100];


	save_property(hFile,"xpos",P_INT,&(actobj->xPos));
	save_property(hFile,"ypos",P_INT,&(actobj->yPos));
	save_property(hFile,"inputports",P_INT,&(actobj->inports));
	save_property(hFile,"outputports",P_INT,&(actobj->outports));
	save_property(hFile,"tag",P_STRING,actobj->tag);

	for (t=1;t<=actobj->inports;t++)
	{
		sprintf(temp,"inport%ddesc",t);
		save_property(hFile,temp,P_STRING,actobj->in_ports[t-1].in_desc);
		sprintf(temp,"inport%ddim",t);
		save_property(hFile,temp,P_STRING,actobj->in_ports[t-1].in_dim);
		sprintf(temp,"inport%dmin",t);
		save_property(hFile,temp,P_FLOAT,&(actobj->in_ports[t-1].in_min));
		sprintf(temp,"inport%dmax",t);
		save_property(hFile,temp,P_FLOAT,&(actobj->in_ports[t-1].in_max));
		sprintf(temp,"inport%drange",t);
		save_property(hFile,temp,P_INT,&(actobj->in_ports[t-1].get_range));

	}
	for (t=1;t<=actobj->outports;t++)
	{
		sprintf(temp,"outport%ddesc",t);
		save_property(hFile,temp,P_STRING,actobj->out_ports[t-1].out_desc);
		sprintf(temp,"outport%ddim",t);
		save_property(hFile,temp,P_STRING,actobj->out_ports[t-1].out_dim);
		sprintf(temp,"outport%dmin",t);
		save_property(hFile,temp,P_FLOAT,&(actobj->out_ports[t-1].out_min));
		sprintf(temp,"outport%dmax",t);
		save_property(hFile,temp,P_FLOAT,&(actobj->out_ports[t-1].out_max));
		sprintf(temp,"outport%drange",t);
		save_property(hFile,temp,P_INT,&(actobj->out_ports[t-1].get_range));

	}


}

void load_object_basics(BASE_CL * actobj)
{
	int t;
	char temp[100];

	load_property("xpos",P_INT,&(actobj->xPos));
	load_property("ypos",P_INT,&(actobj->yPos));
	load_property("inputports",P_INT,&(actobj->inports));
	load_property("outputports",P_INT,&(actobj->outports));
	load_property("tag",P_STRING,actobj->tag);

	for (t=1;t<=actobj->inports;t++)
	{
		sprintf(temp,"inport%ddesc",t);
		load_property(temp,P_STRING,actobj->in_ports[t-1].in_desc);
		sprintf(temp,"inport%ddim",t);
		load_property(temp,P_STRING,actobj->in_ports[t-1].in_dim);
		sprintf(temp,"inport%dmin",t);
		load_property(temp,P_FLOAT,&(actobj->in_ports[t-1].in_min));
		sprintf(temp,"inport%dmax",t);
		load_property(temp,P_FLOAT,&(actobj->in_ports[t-1].in_max));
		sprintf(temp,"inport%drange",t);
		load_property(temp,P_INT,&(actobj->in_ports[t-1].get_range));

	}
	for (t=1;t<=actobj->outports;t++)
	{
		sprintf(temp,"outport%ddesc",t);
		load_property(temp,P_STRING,actobj->out_ports[t-1].out_desc);
		sprintf(temp,"outport%ddim",t);
		load_property(temp,P_STRING,actobj->out_ports[t-1].out_dim);
		sprintf(temp,"outport%dmin",t);
		load_property(temp,P_FLOAT,&(actobj->out_ports[t-1].out_min));
		sprintf(temp,"outport%dmax",t);
		load_property(temp,P_FLOAT,&(actobj->out_ports[t-1].out_max));
		sprintf(temp,"outport%drange",t);
		load_property(temp,P_INT,&(actobj->out_ports[t-1].get_range));

	}

}

int load_next_config_buffer(HANDLE hFile)
{
    DWORD dwRead;
	char buf[2];
	int pos,l;

 	pos=0;
	dwRead=1;l=1;
	while (l&&dwRead)
	{
		ReadFile(hFile, &buf,1,&dwRead,NULL);
		GLOBAL.configbuffer[pos++]=buf[0];	GLOBAL.configbuffer[pos]=0;

		if (strstr(GLOBAL.configbuffer,"end object")!=NULL) l=0;
	}
    //report_error(GLOBAL.configbuffer);
	pos=-1;
	load_property("next object",P_INT,&pos);
	return (pos);
}


void link_object(BASE_CL * act)  
{
	
	char * linkinfo;
 	int from_port,to_obj,to_port;
	int con,getfrom;

	if (!act) return;
	
	linkinfo=strstr(GLOBAL.configbuffer,"linkport");
	con=0;
	while (linkinfo) 
	{
//		if (act->type==OB_EEG) report("link-it");

		act->out[con].from_object=GLOBAL.objects-1;

		getfrom=get_int(linkinfo,0,&from_port);
		getfrom=get_int(linkinfo,getfrom,&to_obj); if (to_obj<0) to_obj=-to_obj;
		getfrom=get_int(linkinfo,getfrom,&to_port);
//		getfrom=get_string(linkinfo+getfrom,":",act->out[con].description);
		
		act->out[con].from_port=from_port;
		act->out[con].to_object=to_obj;
		act->out[con].to_port=to_port;
		con++;
		linkinfo=strstr(linkinfo+1,"linkport");
	}

}

void store_links(HANDLE hFile,BASE_CL * act)
{
	
	char str[100];
	DWORD dwWritten;
 	int con;

	con=0;
	while (act->out[con].to_port!=-1) 
	{
		sprintf(str,"linkport %d-%d,%d\r\n",act->out[con].from_port,act->out[con].to_object,act->out[con].to_port);
		WriteFile(hFile, str, strlen(str), &dwWritten, NULL);
		con++;
	}

}



void reduce_filepath(char * to, char * from)
{
	 char * c2;
	 c2=from;
	 while(*from) if (*from++=='\\') c2=from;
	 strcpy(to,c2);
}


void parse_edf_header(EDFHEADERStruct * to, CHANNELStruct * tochn, char * from)
{
	int start,x,temp;
	char szdata[100];

	temp=0;

	copy_string(from,8,88,to->patient);
	copy_string(from,88,168,to->device);

	copy_string(from,236,244,szdata);
	sscanf(szdata,"%d",&to->segments);

	copy_string(from,244,252,szdata);
	sscanf(szdata,"%d",&to->duration);
	if (to->duration<=0) to->duration=1;

	copy_string(from,252,256,szdata);
	sscanf(szdata,"%d",&to->channels);

	to->samplespersegment=1;
	for (x=0;x<to->channels;x++) 
	{
		copy_string(from,256+x*16,256+(x+1)*16,tochn->label);
		start=256+to->channels*16;
		copy_string(from,start+x*80,start+(x+1)*80,tochn->transducer);
		start+=to->channels*80;
		copy_string(from,start+x*8,start+(x+1)*8,tochn->physdim);
		start+=to->channels*8;
		copy_string(from,start+x*8,start+(x+1)*8,szdata);
		sscanf(szdata,"%d",&tochn->physmin);
		start+=to->channels*8;
		copy_string(from,start+x*8,start+(x+1)*8,szdata);
		sscanf(szdata,"%d",&tochn->physmax);
		start+=to->channels*8;
		copy_string(from,start+x*8,start+(x+1)*8,szdata);
		sscanf(szdata,"%d",&tochn->digmin);
		start+=to->channels*8;
		copy_string(from,start+x*8,start+(x+1)*8,szdata);
		sscanf(szdata,"%d",&tochn->digmax);
		start+=to->channels*8;
		copy_string(from,start+x*80,start+(x+1)*80,tochn->prefiltering);
		start+=to->channels*80;
		copy_string(from,start+x*8,start+(x+1)*8,szdata);
		sscanf(szdata,"%d",&tochn->samples);
		if (tochn->samples>to->samplespersegment) to->samplespersegment=tochn->samples;
		tochn++;
	}
}

void edfheader_to_physical(EDFHEADERStruct * from, EDFHEADER_PHYSICALStruct * to)
{
	int len,t;
	char * ch;
	SYSTEMTIME st;
	char actdate[10];
	char acttime[10];

	len=256+256*from->channels;

	ch=(char *)to;
	for(t=0;t<256;t++,ch++) *ch=' ';

	GetSystemTime(&st);              // gets current time
	GetDateFormat(0, 0, &st, "dd.MM.yy" , actdate, 9);actdate[8]=0;
	GetTimeFormat(0, 0, &st, "HH.mm.ss" , acttime, 9);acttime[8]=0;

	strcpy(to->startdate,actdate); 
	strcpy(to->starttime,acttime); 


	strcpy(to->version,"0"); 
	strcpy(to->patient,from->patient); 
	strcpy(to->recording,from->device);
    sprintf(to->records,"%d",from->segments);
	sprintf(to->duration,"%d",1);
	sprintf(to->channels,"%d",from->channels);
	sprintf(to->headerlength,"%d",len);

	ch=(char *)to;
	for(t=0;t<256;t++,ch++) if (*ch==0) *ch=' ';
	to->end=0;
}


void edfchannels_to_physical(CHANNELStruct * fromchn,char * to,int channels)
{
	char szdata[100];
	int i,t;
	CHANNELStruct * actchn;

	to[0]=0;

	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)   // channel label
	{   strcpy(szdata,actchn->label);  
		for (i=strlen(szdata);i<16;i++) szdata[i]=' '; 
		szdata[16]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)    // transducer
	{   strcpy(szdata,actchn->transducer); 
		for (i=strlen(szdata);i<80;i++) szdata[i]=' ';
		szdata[80]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)    // physical dimension
	{   strcpy(szdata,actchn->physdim); 
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)   // physical minimum
	{   sprintf(szdata,"%d",actchn->physmin); 
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)  // physical maximum
	{   sprintf(szdata,"%d",actchn->physmax); 
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)  // digital minimum
	{   sprintf(szdata,"%d",actchn->digmin); 
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)   // digital maximum
	{   sprintf(szdata,"%d",actchn->digmax); 
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)    // prefiltering
	{   strcpy(szdata,actchn->transducer); 
		for (i=strlen(szdata);i<80;i++) szdata[i]=' ';
		szdata[80]=0; strcat(to,szdata);	}
	actchn=fromchn;	
	for (t=0;t<channels ;t++,actchn++)   //  samples per data record
	{   sprintf(szdata,"%d",PACKETSPERSECOND);
		for (i=strlen(szdata);i<8;i++) szdata[i]=' ';
		szdata[8]=0; strcat(to,szdata);	}
	for (t=0;t<channels;t++)   // reserved
	{   strcpy(szdata," ");
		for (i=strlen(szdata);i<32;i++) szdata[i]=' ';
		szdata[32]=0; strcat(to,szdata);	}

}

void generate_edf_header(char * to, EDFHEADERStruct * header,CHANNELStruct * channels)
{
//	EDFHEADER_PHYSICALStruct file_header;
//	char chnbuf[18000];
    char nl[3];

	edfheader_to_physical(header, (EDFHEADER_PHYSICALStruct *)to);
	edfchannels_to_physical(channels, to+256, header->channels);
	nl[0]=13;nl[1]=10;nl[2]=0;
	strcat(to,nl);

}

HANDLE open_edf_file(EDFHEADERStruct * to, CHANNELStruct * tochn,  char * filename)
{
	char readbuf[8192];
	int t,channels;
	HANDLE temp;
	char fname[256],szdata[300];
	DWORD dwRead;


	if (!(filename[0]))
	{
		strcpy(fname,GLOBAL.resourcepath); 
		strcat(fname,"ARCHIVES\\*.edf");
		if (!open_file_dlg(ghWndMain,fname, FT_EDF, OPEN_LOAD)) return (INVALID_HANDLE_VALUE);
	}
	else strcpy(fname,filename);
	
	write_logfile("open edf file: %s",fname);
	temp= CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (temp==INVALID_HANDLE_VALUE) return (temp);
	ReadFile(temp,readbuf,256, &dwRead,NULL);
	if (dwRead!=256) { CloseHandle(temp); return(INVALID_HANDLE_VALUE); }

	copy_string(readbuf,252,256,szdata);
	sscanf(szdata,"%d",&channels);

	for (t=0;t<channels;t++)
	{

		ReadFile(temp,readbuf+256+256*t,256, &dwRead,NULL);
		if (dwRead!=256) { CloseHandle(temp); return(INVALID_HANDLE_VALUE); }
	}

	parse_edf_header(to, tochn, readbuf);
	//if (filename) 
	strcpy(filename,fname);
	return(temp);
}


HANDLE create_edf_file(EDFHEADERStruct * from, CHANNELStruct * fromchn, char * filename)
{
	HANDLE temp;
	char fname[256];
	DWORD dwWritten;
	EDFHEADER_PHYSICALStruct file_header;
	char chnbuf[18000];
	
	*filename=0;
	strcpy(fname,GLOBAL.resourcepath); 
	strcat(fname,"ARCHIVES\\*.edf");
	if (!open_file_dlg(ghWndMain,fname, FT_EDF, OPEN_SAVE)) return (INVALID_HANDLE_VALUE);

	write_logfile("create edf file: %s",fname);

	temp= CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (temp==INVALID_HANDLE_VALUE) return (temp);

	edfheader_to_physical(from, &file_header);
	edfchannels_to_physical(fromchn, chnbuf, from->channels);

	WriteFile(temp,&file_header,256, &dwWritten,NULL);
	if (dwWritten!=256) { CloseHandle(temp); return(INVALID_HANDLE_VALUE); }

	WriteFile(temp,chnbuf,256*from->channels, &dwWritten,NULL);
	if (dwWritten!=(DWORD)256*from->channels) { CloseHandle(temp); return(INVALID_HANDLE_VALUE); }

	strcpy(filename,fname);
	return(temp);
}

void reset_header(EDFHEADERStruct * header)
{
		header->samplespersegment=PACKETSPERSECOND;
		header->segments=0;
		header->samplingrate=PACKETSPERSECOND;
		header->duration=1;
		header->channels=0;
		strcpy(header->patient,"none");
		strcpy(header->device ,"none");

}
 
void update_header(HWND hDlg, EDFHEADERStruct * header)
{  

	SetDlgItemInt(hDlg,IDC_CHANNELS,header->channels,0);
	SetDlgItemText(hDlg,IDC_PATIENT,header->patient);
	SetDlgItemText(hDlg,IDC_DEVICE,header->device);
	SetDlgItemInt(hDlg,IDC_DURATION,header->duration,0);
	SetDlgItemInt(hDlg,IDC_SEGMENTS,header->segments,1);
	SetDlgItemInt(hDlg,IDC_SAMPLES,header->samplespersegment,0);
	SetDlgItemInt(hDlg,IDC_SAMPLINGRATE,header->samplespersegment/header->duration,0);
}

void get_header(HWND hDlg, EDFHEADERStruct * header)
{  
	GetDlgItemText(hDlg,IDC_PATIENT,header->patient,80);
	GetDlgItemText(hDlg,IDC_DEVICE,header->device,80);
	header->duration=GetDlgItemInt(hDlg,IDC_DURATION,NULL,0);
	header->segments=GetDlgItemInt(hDlg,IDC_SEGMENTS,NULL,0);
	header->samplespersegment=GetDlgItemInt(hDlg,IDC_SAMPLES,NULL,0);
	header->samplingrate=GetDlgItemInt(hDlg,IDC_SAMPLINGRATE,NULL,0);

}

void update_channelcombo(HWND hDlg, CHANNELStruct * channel, int channels)
{	int x;
	char szdata[100];

	SendDlgItemMessage(hDlg,IDC_CHANNELCOMBO,CB_RESETCONTENT,0,0);

	for (x=0;x<channels;x++) 
	{
			sprintf(szdata,"%d : ",x+1);
			strcat (szdata,channel[x].label);
			SendDlgItemMessage(hDlg,IDC_CHANNELCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) szdata);
	}
	SendDlgItemMessage(hDlg, IDC_CHANNELCOMBO, CB_SETCURSEL,0,0);
}


void reset_channel(CHANNELStruct * channel)
{
	int x;
	for (x=0; x<MAX_EEG_CHANNELS; x++)
	{
		strcpy(channel[x].label,"none");
		strcpy(channel[x].transducer,"none");
		strcpy(channel[x].prefiltering,"none");
		strcpy(channel[x].physdim," ");
		channel[x].physmin=0;
		channel[x].physmax=1;
		channel[x].digmin=0;
		channel[x].digmax=1024;
	}
}

void update_channel(HWND hDlg, CHANNELStruct * channel, int actchn)
{
	SetDlgItemText(hDlg,IDC_LABEL,channel[actchn].label);
	SetDlgItemText(hDlg,IDC_ELECTRODE,channel[actchn].transducer);
	SetDlgItemText(hDlg,IDC_PREFILTERING,channel[actchn].prefiltering);
	SetDlgItemText(hDlg,IDC_PHYSDIM,channel[actchn].physdim);
	SetDlgItemInt(hDlg,IDC_PHYSMIN,channel[actchn].physmin,1);
	SetDlgItemInt(hDlg,IDC_PHYSMAX,channel[actchn].physmax,1);
	SetDlgItemInt(hDlg,IDC_DIGMIN,channel[actchn].digmin,1);
	SetDlgItemInt(hDlg,IDC_DIGMAX,channel[actchn].digmax,1);
	SetDlgItemInt(hDlg,IDC_SPP,channel[actchn].samples,0);
}

void get_channel(HWND hDlg, CHANNELStruct * channel, int actchn)
{
	GetDlgItemText(hDlg,IDC_LABEL,channel[actchn].label,16);
	GetDlgItemText(hDlg,IDC_PREFILTERING,channel[actchn].prefiltering,80);
	GetDlgItemText(hDlg, IDC_ELECTRODE, channel[actchn].transducer,80); 
	GetDlgItemText(hDlg,IDC_PHYSDIM,channel[actchn].physdim,8);
	channel[actchn].physmin=GetDlgItemInt(hDlg,IDC_PHYSMIN,NULL,1);
	channel[actchn].physmax=GetDlgItemInt(hDlg,IDC_PHYSMAX,NULL,1);
	channel[actchn].digmin=GetDlgItemInt(hDlg,IDC_DIGMIN,NULL,1);
	channel[actchn].digmax=GetDlgItemInt(hDlg,IDC_DIGMAX,NULL,1);
}


