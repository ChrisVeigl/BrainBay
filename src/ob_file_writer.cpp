/* -----------------------------------------------------------------------------

  BrainBay  Version 1.9, GPL 2003-2014, contact: chris@shifz.org
    
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




LRESULT CALLBACK FileWriterDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	FILE_WRITEROBJ * st;
	static int actchn;
	
	st = (FILE_WRITEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_FILE_WRITER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_FILENAME, st->filename);
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Integer Values, TAB / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Integer Values, Comma / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Float Values, TAB / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-Float Values, Comma / CR+LF delimited");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "ASCII-BioExplorer with header");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "1-Channel raw integer 16bit signed");
				SendDlgItemMessage(hDlg,IDC_FORMATCOMBO,CB_ADDSTRING,0,(LPARAM) (LPSTR) "1-Channel raw integer 16bit unsigned");
				SendDlgItemMessage(hDlg, IDC_FORMATCOMBO, CB_SETCURSEL, st->format, 0L ) ;
				if (st->state)
				{
					SetDlgItemText(hDlg,IDC_FILESTATUS,"writing File");
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				}
				else
				{
					if (st->file!=INVALID_HANDLE_VALUE) 
					{ 
						SetDlgItemText(hDlg,IDC_FILESTATUS,"File opened");
						EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
					}
					else 
					{
						SetDlgItemText(hDlg,IDC_FILESTATUS,"File not opened.");
						EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
					}
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);

				}
				CheckDlgButton(hDlg,IDC_APPEND,st->append);
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
					  strcat(st->filename,"ARCHIVES\\*.*");
					}
					if (open_file_dlg(ghWndMain,st->filename, FT_TXT, OPEN_SAVE))
					{
						 long filesize=0;

						 if (!st->append) 
						 {
						   st->file=CreateFile(st->filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
						 }
						 else  
						 {
						   st->file=CreateFile(st->filename, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
						   filesize=SetFilePointer(st->file,0,NULL,FILE_END);
						 }

						 if (st->file==INVALID_HANDLE_VALUE) 
						 {
							 SetDlgItemText(hDlg,IDC_FILESTATUS,"Could not create File");
							 strcpy(st->filename,"none");
							 st->state=0;
						 }
						 else 
						 if (st->format==4)
						 { 
							 char tmp[1024],tmp1[256];
							 int i,a;
							 DWORD dwWritten;

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
							 EnableWindow(GetDlgItem(hDlg, IDC_SELECT), FALSE); EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
							 SetDlgItemText(hDlg,IDC_FILESTATUS,"File opened");
						 }
					}
					SetDlgItemText(hDlg,IDC_FILENAME,st->filename);
					InvalidateRect(ghWndDesign,NULL,TRUE);
				 break; 

			case IDC_APPEND:
				st->append=IsDlgButtonChecked(hDlg,IDC_APPEND);
				break;
			case IDC_START:
				if ((st->inports>0) &&(st->file!=INVALID_HANDLE_VALUE))
				{
					SetDlgItemText(hDlg,IDC_FILESTATUS, "writing File");
					// START FILE WRITING
					st->state=1;
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
  					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				} else SetDlgItemText(hDlg,IDC_FILESTATUS, "No Channels available.");
				break; 

			case IDC_STOP:
					SetDlgItemText(hDlg,IDC_FILESTATUS, "stopped");
					st->state=0;
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
					break; 

			case IDC_CLOSE: 
					SetDlgItemText(hDlg,IDC_FILESTATUS, "file closed.");
					st->state=0;
					if (st->file!=INVALID_HANDLE_VALUE) CloseHandle(st->file);
					st->file=INVALID_HANDLE_VALUE;
  				    EnableWindow(GetDlgItem(hDlg, IDC_SELECT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
 				    InvalidateRect(ghWndDesign,NULL,TRUE);
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

		state=0; format=0;append=FALSE;
		file=INVALID_HANDLE_VALUE;
		strcpy(filename,"none");
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
		  height=CON_START+inports*CON_HEIGHT+5;
		  
	  }
		
	  void FILE_WRITEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"filename",P_STRING,&filename);
		  save_property(hFile,"format",P_INT,&format);
		  save_property(hFile,"inports",P_INT,&inports);
		  save_property(hFile,"append",P_INT,&append);
	  }


	  void FILE_WRITEROBJ::incoming_data(int port, float value) 
	  {	
		in_ports[port].value=value;
	  }

	  
	  void FILE_WRITEROBJ::work(void) 
	  {
		int x;
		char sztemp[100];
	
	
		if ((inports==0)||(state==0)||(file==INVALID_HANDLE_VALUE)) return;

	
		for (x=0;x<inports-1;x++)
		{
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
				  else if ((format==1) || (format==3)) strcat(sztemp,", ");
				  else if (format==4) strcat(sztemp,",");
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


FILE_WRITEROBJ::~FILE_WRITEROBJ()
	  {	
		if (file)
		{
			CloseHandle(file);
		}
	  }  
