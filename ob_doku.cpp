/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
  
  MODULE: OB_DOKU.CPP:  contains functions for the Documentation-Object
  Author: Chris Veigl

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_doku.h"



LRESULT CALLBACK DokuDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	
	DOKUOBJ * st;
	
	st = (DOKUOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_DOKU)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_DOKU, st->text);
			return TRUE;
		case WM_CLOSE:		
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_DOKU:
				GetDlgItemText(hDlg,IDC_DOKU,st->text,sizeof(st->text));
				break;
			}
			return TRUE;
	}
    return FALSE;
}



DOKUOBJ::DOKUOBJ(int num) : BASE_CL()	
	  {
	    outports = 0;
		inports = 0;
		width=60;
		height=40;
		strcpy(text,"    ---------   Design Documentation   --------");
	  }
	  
	  void DOKUOBJ::make_dialog(void) 
	  {  
		  display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_DOKUBOX, ghWndStatusbox, (DLGPROC)DokuDlgHandler)); 
	  }

	  void DOKUOBJ::load(HANDLE hFile) 
	  {
  		  unsigned int x;
	  	  load_object_basics(this);
		  load_property("text",P_STRING,text);
		  for (x=0;x<strlen(text);x++) 
		  {
			  if (text[x]=='#') text[x]=10;
			  if (text[x]=='/') text[x]=13;
		  }

	  }
		
	  void DOKUOBJ::save(HANDLE hFile) 
	  {	  
		  unsigned int x;
		  for (x=0;x<strlen(text);x++) 
		  {
			  if (text[x]==10) text[x]='#';
			  if (text[x]==13) text[x]='/';
		  }

		  save_object_basics(hFile, this);
		  save_property(hFile,"text",P_STRING,text);
	  }
	  

DOKUOBJ::~DOKUOBJ()
	  {
		// free object
	  }  


