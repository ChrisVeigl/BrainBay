/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_FILE_WRITER.CPP
  Author: Chris Veigl


  Using this Object, a File containing raw or ASCII-integer values 
  of the connected signale can be written. Delimiters for the Columns can be selected
  the Signals are connected to the input-ports.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/



#include "brainBay.h"
#include "ob_file_writer.h"
#include <time.h>

#define STATE_FILE_IDLE 0
#define STATE_FILE_OPEN 1
#define STATE_FILE_WRITING 2


void updateDialog(HWND hDlg, FILE_WRITEROBJ * st)
{
	switch (st->state)
	{
		case STATE_FILE_IDLE:
			SetDlgItemText(hDlg,IDC_FILESTATUS,"no file opened");
			EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);			
			break;
		case STATE_FILE_OPEN:
			SetDlgItemText(hDlg,IDC_FILESTATUS,"file opened, writing stopped");
			EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
		break;
		case STATE_FILE_WRITING:
			SetDlgItemText(hDlg,IDC_FILESTATUS,"writing File");
			EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
		break;
	}
	CheckDlgButton(hDlg,IDC_APPEND,st->append);
	CheckDlgButton(hDlg,IDC_AUTOCREATE,st->autocreate);
	CheckDlgButton(hDlg,IDC_ADD_DATE,st->add_date);
	CheckDlgButton(hDlg,IDC_SEMICOLON,st->semicolon);
	SetDlgItemInt(hDlg,IDC_AVERAGING,st->averaging,0);
	SetDlgItemText(hDlg,IDC_FILENAME,st->filename);
	SetDlgItemText(hDlg,IDC_HEADERLINE,st->headerline);
	InvalidateRect(ghWndDesign,NULL,TRUE);
}

int openFile(FILE_WRITEROBJ * st)
{						 
	time_t rawtime;
	struct tm * timeinfo;
    long filesize=0;
	char * findext;
	char filename[255];
	char tmp[1024],tmp1[256];
	DWORD dwWritten;

	strcpy(filename,st->filename);
	if (st->add_date)
	{
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		//The years are since 1900 according the documentation, so add 1900 to the actual year result.
		char timestr[100];
		wsprintf(timestr, "_%d-%02d-%02d_%02d-%02d-%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec  );
		//printf("time is: %s\n",timestr);

		findext=strstr(filename,".");
		if (findext) {
			strcat (timestr,findext);
		    strcpy (findext,timestr);
		}
		else strcat (filename,timestr);
		//printf("new filename is: %s\n",filename);
	}

	if (!st->append) 
	{
		st->file=CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	}
	else  
	{
		st->file=CreateFile(filename, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		filesize=SetFilePointer(st->file,0,NULL,FILE_END);
	}

	if (st->file==INVALID_HANDLE_VALUE) {
		st->state=STATE_FILE_IDLE;
		return(0);
	}

	if ((strlen(st->headerline)) && (st->format != 4) && (!st->append))  // write headerline
	{
		WriteFile(st->file,st->headerline,strlen(st->headerline),&dwWritten,NULL);
	    wsprintf(tmp,"\r\n");
		WriteFile(st->file,tmp,strlen(tmp),&dwWritten,NULL);
	}

	if (st->format==4) // bioexplorer format: write bioexplorer header!
	{ 
		int i,a;

		tmp[0]=0;
		for(i=0;i<st->inports-1;i++)
		{
			sprintf(tmp1,"Sample Rate: %.4f",(float)PACKETSPERSECOND);
			strcat(tmp,tmp1);
			if (i!=st->inports-2) strcat (tmp,","); 
			else { a=strlen(tmp); tmp[a++]=13;tmp[a++]=10;tmp[a]=0;}
		}
							 
		for(i=0;i<st->inports-1;i++)
		{
			sprintf(tmp1,"Source %d:",i+1);
			strcat(tmp,tmp1);
			strcat(tmp,st->in_ports[i].in_desc);
			if (i!=st->inports-2) strcat (tmp,","); 
			else { a=strlen(tmp); tmp[a++]=13;tmp[a++]=10;tmp[a]=0;}
		}
		if ((!st->append)||(filesize<10))
		WriteFile(st->file,tmp,strlen(tmp),&dwWritten,NULL);
	}
	st->state=STATE_FILE_OPEN;	
	return(1);
}


LRESULT CALLBACK FileWriterDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	FILE_WRITEROBJ * st;
	static int actchn;
	
	st = (FILE_WRITEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_FILE_WRITER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Integer Values, TAB / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Integer Values, Comma / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Float Values, TAB / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Float Values, Comma / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-BioExplorer with header");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "1-Channel raw integer 16bit signed");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "1-Channel raw integer 16bit unsigned");
				SendDlgItemMessage(hDlg, IDC_FORMATCOMBO, CB_SETCURSEL, st->format, 0L ) ;
				updateDialog(hDlg, st);
				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			
			switch (LOWORD(wParam)) 
			{ 
				/*
			case IDC_GENTESTFILE:
				{
					int i;
					unsigned short chn1,chn2;
					DWORD dwWritten;
					// char tmp[1024];
					struct { 
						      unsigned char stream;
							  unsigned char nu1;
							  unsigned char chn1hi;
							  unsigned char chn1lo;
							  unsigned char chn2hi;
							  unsigned char chn2lo;
							  unsigned short nu2,nu3;
							  unsigned char data1;
							  unsigned char data2;
							  unsigned char data3;
  							  unsigned char nu4;
					} TX;

                    st->file=CreateFile(".\\archives\\sbg12bit.arc", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
					for (i=0;i<2048;i++)
					{

						TX.nu1=0;
						TX.nu2=0;
						TX.nu3=0;
						TX.nu4=0;

						TX.stream= (unsigned char)  ((i&3) << 6);
						chn1 =  (unsigned short)  (2048.0+2000.0* sin ( ((float)i/256.0) * 3.1415 ));
						chn2 =  (unsigned short)  (2048.0+1000.0* sin ( ((float)i/64.0) * 3.1415 ));;
						TX.chn1hi= (unsigned char) ((chn1 & 0x0f00) >> 8);
						TX.chn1lo= (unsigned char) (chn1 & 0xff);
						TX.chn2hi= (unsigned char) ((chn2 & 0x0f00) >> 8);
						TX.chn2lo= (unsigned char) (chn2 & 0xff);
						TX.data1=  (unsigned char)  (i & 0x3f);
						TX.data2=  (unsigned char)  (i & 0x80);
						TX.data3=  (unsigned char)  ((i & 0x200)>>2);

			            WriteFile(st->file,& TX,sizeof(TX),&dwWritten,NULL);

					}
					CloseHandle(st->file);
				}
            break;

			*/
			case IDC_SELECT:
					if (!strcmp(st->filename,"none"))
					{
			 		  strcpy(st->filename,GLOBAL.resourcepath);
					  strcat(st->filename,"REPORTS\\*.*");
					}
					if (open_file_dlg(ghWndMain,st->filename, FT_TXT, OPEN_SAVE))
					{
						 if (!openFile(st)) 
						 {
							 SetDlgItemText(hDlg,IDC_FILESTATUS,"Could not create File");
							 strcpy(st->filename,"none");
						 }
					}
					updateDialog(hDlg, st);

				 break; 

			case IDC_HEADERLINE:
				GetDlgItemText(hDlg, IDC_HEADERLINE, st->headerline, 500);
				break;
			case IDC_APPEND:
				st->append=IsDlgButtonChecked(hDlg,IDC_APPEND);
				break;
			case IDC_AUTOCREATE:
				st->autocreate=IsDlgButtonChecked(hDlg,IDC_AUTOCREATE);
				break;
			case IDC_ADD_DATE:
				st->add_date=IsDlgButtonChecked(hDlg,IDC_ADD_DATE);
				break;
			case IDC_SEMICOLON:
				st->semicolon=IsDlgButtonChecked(hDlg,IDC_SEMICOLON);
				break;
			case IDC_AVERAGING:
				st->averaging=GetDlgItemInt(hDlg,IDC_AVERAGING,NULL,0);
				break;

			case IDC_START:
				if ((st->inports>0) && (st->file!=INVALID_HANDLE_VALUE))
				{
					// START FILE WRITING
					st->state=STATE_FILE_WRITING;
					st->avg_count=0;
					updateDialog(hDlg, st);
				} 
				break; 

			case IDC_STOP:
				if (st->file!=INVALID_HANDLE_VALUE)
					st->state=STATE_FILE_OPEN;
				else
					st->state=STATE_FILE_IDLE;
				updateDialog(hDlg, st);
				break; 

			case IDC_CLOSE: 
					if (st->file!=INVALID_HANDLE_VALUE) CloseHandle(st->file);
					st->file=INVALID_HANDLE_VALUE;
					st->state=STATE_FILE_IDLE;
					updateDialog(hDlg, st);
					break;

			case IDC_FORMATCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
					    st->format=SendMessage(GetDlgItem(hDlg, IDC_FORMATCOMBO), CB_GETCURSEL , 0, 0);		
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


FILE_WRITEROBJ::FILE_WRITEROBJ(int num) : BASE_CL()	
	  {
	    outports = 0;
		inports = 1;
		width=70;
		height=50;


		state=STATE_FILE_IDLE; 
		format=0;
		append=FALSE;
		autocreate=FALSE;
		add_date=FALSE;
		semicolon=FALSE;
		averaging=1;
		avg_count=0;
		for (int i=0;i<MAX_PORTS;i++)
			averaging_buffers[i]=0;

		file=INVALID_HANDLE_VALUE;
		strcpy(filename,"none");
		strcpy(headerline,"");
	  }


  	  void FILE_WRITEROBJ::update_inports(void)
	  {
		if (state!=1)
		{
			inports=count_inports(this);
			height=CON_START+inports*CON_HEIGHT+5;
			InvalidateRect(ghWndMain,NULL,TRUE);
		}
	  }


	  void FILE_WRITEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_FILE_WRITERBOX, ghWndStatusbox, (DLGPROC)FileWriterDlgHandler)); 
	  }

	  void FILE_WRITEROBJ::load(HANDLE hFile) 
	  {

	 	  load_object_basics(this);
		  load_property("filename",P_STRING,&filename);
		  load_property("format",P_INT,&format);
		  load_property("inports",P_INT,&inports);
		  load_property("append",P_INT,&append);
		  load_property("averaging",P_INT,&averaging);
		  load_property("autocreate",P_INT,&autocreate);
		  load_property("add_date",P_INT,&add_date);
		  load_property("semicolon",P_INT,&semicolon);
		  load_property("headerline",P_STRING,&headerline);

		  height=CON_START+inports*CON_HEIGHT+5;
		  
	  }
		
	  void FILE_WRITEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"filename",P_STRING,&filename);
		  save_property(hFile,"format",P_INT,&format);
		  save_property(hFile,"inports",P_INT,&inports);
		  save_property(hFile,"append",P_INT,&append);
		  save_property(hFile,"averaging",P_INT,&averaging);
		  save_property(hFile,"autocreate",P_INT,&autocreate);
		  save_property(hFile,"add_date",P_INT,&add_date);
		  save_property(hFile,"semicolon",P_INT,&semicolon);
		  save_property(hFile,"headerline",P_STRING,&headerline);
	  }

	  void FILE_WRITEROBJ::session_reset(void) 
	  {
		  avg_count=0;
	  }

	  void FILE_WRITEROBJ::session_start(void)
	  {
  			if ((file==INVALID_HANDLE_VALUE) || (state != STATE_FILE_WRITING))
			{  
				if( openFile(this)) {
					state=STATE_FILE_WRITING;
					avg_count=0;
				}
			} 
	  }

	  void FILE_WRITEROBJ::session_stop(void)
	  {
			if (file!=INVALID_HANDLE_VALUE) CloseHandle(file);
			file=INVALID_HANDLE_VALUE;
			state=STATE_FILE_IDLE;
	  }

  	  void FILE_WRITEROBJ::session_pos(long pos) 
	  {	
			avg_count=0;
	  } 

	  void FILE_WRITEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
		averaging_buffers[port]+=value;
	  }
	  
	  void FILE_WRITEROBJ::work(void) 
	  {
		int x;
		char sztemp[100];
		
		if ((inports==0)||(state!=STATE_FILE_WRITING)||(file==INVALID_HANDLE_VALUE)) return;

		if (++avg_count>=averaging) 
		{
			avg_count=0;

			for (x=0;x<inports-1;x++)
			{
				in_ports[x].value=averaging_buffers[x]/averaging;
				averaging_buffers[x]=0;

				if ((format==0) || (format==1))
  				   wsprintf(sztemp,"%d",(int)in_ports[x].value);
				else if ((format==2) || (format==3))
  				   sprintf(sztemp,"%.2f",in_ports[x].value);
				else if (format==4)
				{
					sprintf(sztemp,"%.11f",in_ports[x].value/1000000);
				}
			
				if (format<5)
				{
					if (x<inports-2)
					{
					  if ((format==0) || (format==2)) strcat(sztemp,"\t");
					  else if ((format==1) || (format==3)) {
						  if (semicolon) strcat(sztemp,"; ");
						  else strcat(sztemp,", ");
					  }
					  else if (format==4) {
						  if (semicolon) strcat(sztemp,";");
						  else strcat(sztemp,",");
					  }
					}
					WriteFile(file,sztemp,strlen(sztemp),&dwWritten,NULL);
				}

				if ((format==5)&&(x==0))
				{
					sztemp[0]= (((int)in_ports[x].value) >> 8);
					sztemp[1]= (((int)in_ports[x].value) & 0xff);
					WriteFile(file,sztemp,2,&dwWritten,NULL);
				}
				if ((format==6)&&(x==0))
				{
					sztemp[0]= (((unsigned int)in_ports[x].value) >> 8);
					sztemp[1]= (((unsigned int)in_ports[x].value) & 0xff);
					WriteFile(file,sztemp,2,&dwWritten,NULL);
				}
			
			}
			if (format<5)
			{
			  wsprintf(sztemp,"\r\n");
			  WriteFile(file,sztemp,strlen(sztemp),&dwWritten,NULL);
			}
		  }
	  }


FILE_WRITEROBJ::~FILE_WRITEROBJ()
	  {	
		if (file)
		{
			CloseHandle(file);
		}
	  }  
