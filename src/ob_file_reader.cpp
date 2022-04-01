/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_FILE_READER.CPP:  contains the FILE - Reader-Object
  Author: Chris Veigl


  Using this Object, a File containing raw or ASCII-integer values 
  of signals can be read. Delimiters for the Columns can be selected
  the Signals are fed to the output-ports.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/



#include "brainBay.h"
#include "ob_file_reader.h"


int open_textarchive(FILE_READEROBJ * st)
{
	char tmp1[256],tmp[2];
	int i,op=0;
	unsigned int max=0;
	DWORD dwRead;

	st->file=CreateFile(st->filename, GENERIC_READ,  FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	 if (st->file==INVALID_HANDLE_VALUE) 
	 {
		 strcpy(st->filename,"none");
		 st->state=0;
		 return(0);
	 }

	 i=0;
	 if (st->format<5)
	 {
		 do 
		 {
			 tmp[0]=0;
 			 ReadFile(st->file,tmp,1,&dwRead,NULL);
		 }  while ((dwRead!=0) && (tmp[0]!=10));

		 i=0;tmp[1]=0;tmp1[0]=0;
		 do 
		 {
			 tmp[0]=0;
 			 ReadFile(st->file,tmp,1,&dwRead,NULL);
			 if ((tmp[0] != 13) && (tmp[0] != 10) && (tmp[0] != 9) && (tmp[0] != ','))
			 {
				 if (strlen(tmp1) < 250) strcat(tmp1, tmp);
			 }
			 else if ((tmp[0]==9)||(tmp[0]==',')||(tmp[0]==13))
			 { 
				 if (strlen(tmp1)>19) tmp1[19]=0;
				 if (max<strlen(tmp1)) max=strlen(tmp1);

				 // strcpy(st->out_ports[op].out_name,tmp1);
				 // strcpy(st->out_ports[op++].out_desc,tmp1);

				 op++;
				 sprintf(st->out_ports[op].out_name, "chn%d", op);
				 sprintf(st->out_ports[op].out_desc, "chn%d", op);
				 tmp1[0]=0;
			 }
		 }  while ((dwRead!=0) && (tmp[0]!=10));
		 if (op>0) st->outports=op;
	 }
	 else
	 {
		 st->outports=1;
		 strcpy(st->out_ports[0].out_name,"chn1");
	 }

	 st->height=CON_START+st->outports*CON_HEIGHT+5;
	 if (max<10) max=0; else max-=10;
	 st->width=70+max*5;
	 return(1);
}


LRESULT CALLBACK FileReaderDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	FILE_READEROBJ * st;
	static int actchn;
	
	st = (FILE_READEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_FILE_READER)) return(FALSE);
 

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
					SetDlgItemText(hDlg,IDC_FILESTATUS,"reading File");
					EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), FALSE);
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
					EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);

				}

				return TRUE;
	
		case WM_CLOSE: 
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			
		case WM_COMMAND:
			
			switch (LOWORD(wParam)) 
			{ 
			case IDC_OPENFILE:
					if (!strcmp(st->filename,"none"))
					{
			 		  strcpy(st->filename,GLOBAL.resourcepath);
					  strcat(st->filename,"ARCHIVES\\*.*");
					}
					if (open_file_dlg(ghWndMain,st->filename, FT_TXT, OPEN_LOAD))
					{
						     if (!open_textarchive(st)) 		 
								 SetDlgItemText(hDlg,IDC_FILESTATUS,"Could not open File");
							 else
							 {
								EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), FALSE); EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
								SetDlgItemText(hDlg,IDC_FILESTATUS,"File opened");
							 }
						 
					}
					get_session_length();

					SetDlgItemText(hDlg,IDC_FILENAME,st->filename);
					InvalidateRect(ghWndDesign,NULL,TRUE);
				 break; 

			case IDC_START:
				if ((st->outports>0) &&(st->file!=INVALID_HANDLE_VALUE))
				{
					SetDlgItemText(hDlg,IDC_FILESTATUS, "reading File");
					// START FILE READING
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
					EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
					break; 

			case IDC_CLOSE: 
					SetDlgItemText(hDlg,IDC_FILESTATUS, "file closed.");
					st->state=0;
					if (st->file!=INVALID_HANDLE_VALUE) CloseHandle(st->file);
					st->file=INVALID_HANDLE_VALUE;
  				    EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);

					get_session_length();
					st->samplecount=0;

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


FILE_READEROBJ::FILE_READEROBJ(int num) : BASE_CL()	
	  {
	    outports = 1;
		inports = 0;
		width=70;
		height=50;
		samplecount=0;

		state=0; format=0;
		file=INVALID_HANDLE_VALUE;
		strcpy(filename,"none");
	  }


  	  void FILE_READEROBJ::session_start(void) 
	  {	
		if ((file!=INVALID_HANDLE_VALUE)&&(outports>0))
		{
				state=1;
				if (hDlg==ghWndToolbox)
				{
					SetDlgItemText(hDlg,IDC_FILESTATUS, "reading File");
					EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
  					EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
				}
		}
	  }

  	  void FILE_READEROBJ::session_stop(void) 
	  {	
		if (file!=INVALID_HANDLE_VALUE) state=0;
		if (hDlg==ghWndToolbox)
		{
					SetDlgItemText(hDlg,IDC_FILESTATUS, "stopped");
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
		}
	  }
  	  void FILE_READEROBJ::session_reset(void) 
	  {	
		  state=0;
		  if (file!=INVALID_HANDLE_VALUE)
		  { 
			  if (format<5)
			  SetFilePointer(file,0,NULL,FILE_BEGIN);
			  else SetFilePointer(file,0,NULL,FILE_BEGIN+1);
			  if (hDlg==ghWndToolbox)
			  {  
					SetDlgItemText(hDlg,IDC_FILESTATUS, "stopped");
					EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
			  } 
		  }	
	  }
  	  void FILE_READEROBJ::session_pos(long pos) 
	  {	
		  char tmp[2];
		  DWORD dwRead=1;
		  long p=0,i=0;

		  if((pos<0) || (file==INVALID_HANDLE_VALUE)) return;

		  if (pos>samplecount) 
		  {	
			  SetFilePointer(file,0,NULL,FILE_END);
			  return;
		  }
	  
		  SetFilePointer(file,0,NULL,FILE_BEGIN);

  		  p=0;tmp[0]=0;
		  while ((p<pos) && (dwRead))
		  {
 			     ReadFile(file,tmp,1,&dwRead,NULL);
				 if (format<5) { if (tmp[0]==13) p++; }
				 else {i++; if (i%2) p++; }
		  }
	  } 

	  long FILE_READEROBJ::session_length(void) 
	  {
		  char tmp[2];
		  DWORD dwRead=1;
		  long p=0;
		  int i=0;

		  if (file==INVALID_HANDLE_VALUE) return(0);

		  SetFilePointer(file,0,NULL,FILE_BEGIN);

  		  p=0;tmp[0]=0;
		  while (dwRead)
		  {
 			     ReadFile(file,tmp,1,&dwRead,NULL);
				 if (format<5) { if (tmp[0]==13) p++; }
				 else p++;
		  }

		  SetFilePointer(file,0,NULL,FILE_BEGIN);

		  if (format==4)
			while ((dwRead)&&(i<2));
			{
 			     ReadFile(file,tmp,1,&dwRead,NULL);
				 if (tmp[0]==13) i++; 
			}

		  if (format<5) samplecount=p; else samplecount=p/2;
		  return(samplecount);
	  }


	  void FILE_READEROBJ::make_dialog(void) 
	  {  
		display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_FILE_READERBOX, ghWndStatusbox, (DLGPROC)FileReaderDlgHandler)); 
	  }

	  void FILE_READEROBJ::load(HANDLE hFile) 
	  {

	 	  load_object_basics(this);
		  load_property("filename",P_STRING,&filename);
		  load_property("format",P_INT,&format);
		  load_property("outports",P_INT,&outports);
		  height=CON_START+outports*CON_HEIGHT+5;

		  if (strcmp(filename,"none")) open_textarchive(this);
		  
	  }
		
	  void FILE_READEROBJ::save(HANDLE hFile) 
	  {
		  save_object_basics(hFile, this);
		  save_property(hFile,"filename",P_STRING,&filename);
		  save_property(hFile,"format",P_INT,&format);
		  save_property(hFile,"outports",P_INT,&outports);
	  }

	  
	  void FILE_READEROBJ::work(void) 
	  {
		int x,d;
		float f;
		char tmp[2],tmp1[256];
		DWORD dwRead;
		int s_int;
		unsigned int u_int;
	
	
		if ((outports==0)||(state==0)||(file==INVALID_HANDLE_VALUE)) return;

	
		if (format<5)
		{
 		  for (x=0;x<outports;x++)
		  {
			tmp1[0]=0;tmp[1]=0;
			do 
			{
				 tmp[0]=0;
 			     ReadFile(file,tmp,1,&dwRead,NULL); 
				 if (tmp[0] == 13) ReadFile(file, tmp, 1, &dwRead, NULL);

				 if (((tmp[0]>='0')&&(tmp[0]<='9'))||(tmp[0]=='.')||(tmp[0]=='-') || (tmp[0] == 'E') || (tmp[0] == 'e'))
				 {
					 if (strlen(tmp1) < 250) strcat(tmp1, tmp);
				 }					 
			}  while ((tmp[0]!=0) && (tmp[0]!=13) && (tmp[0] != 10) &&(tmp[0]!=9)&&(tmp[0]!=','));

			tmp1[strlen(tmp1)]=0;

            if ((format==2)||(format==3)||(format==4))
			{
//				SetDlgItemText(ghWndStatusbox,IDC_STATUS,tmp1);
				sscanf(tmp1,"%f",&f);
				if (format==4) pass_values(x,f*1000000); else pass_values(x,f);
			}
			else 
			{
				  sscanf(tmp1,"%d",&d);
				  pass_values(x,(float)d);
			}
		  }
		}
		else
		{
             ReadFile(file,tmp1,2,&dwRead,NULL);
			 if (format==5)
			 {
				   s_int= (tmp1[1]<<8) + tmp1[0];
				   f=(float)(s_int);
			 }
			 else
			 {
				  u_int= (tmp1[1]<<8) + tmp1[0];
		 	      f=(float)(u_int);
			 }
		     pass_values(0,f);
		}
			
	  }


FILE_READEROBJ::~FILE_READEROBJ()
	  {	
		if (file)
		{
			CloseHandle(file);
		}
	  }  
