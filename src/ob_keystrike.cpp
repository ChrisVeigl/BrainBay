/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_KEYSTRIKE.CPP:  contains the Keystrike-Object
  Author: Chris Veigl

  This Object can generate a selectable Keyboard Sequence 
  (Keyup and Keydown Messages for various Keys)

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_keystrike.h"


char virtkeys[256][30]= {
{"VK_BACKSPACE (08)"},
{"VK_TAB (09)"},
{"VK_CLEAR (0C)"},
{"VK_RETURN (0D)"},
{"VK_SHIFT (10)"},
{"VK_CONTROL (11)"},
{"VK_ALT (12)"},
{"VK_PAUSE (13)"},
{"VK_CAPSLOCK (14)"},
{"VK_ESCAPE (1B)"},
{"VK_SPACE (20)"},
{"VK_PAGEUP (21)"},
{"VK_PAGEDOWN (22)"},
{"VK_END (23)"},
{"VK_HOME (24)"},
{"VK_LEFT (25)"},
{"VK_UP (26)"},
{"VK_RIGHT (27)"},
{"VK_DOWN (28)"},
{"VK_SELECT (29)"},
{"VK_PRINT (2A)"},
{"VK_EXECUTE (2B)"},
{"VK_SNAPSHOT (2C)"},
{"VK_INSERT (2D)"},
{"VK_DELETE (2E)"},
{"VK_HELP (2F)"},
{"0 (30)"},
{"1 (31)"},
{"2 (32)"},
{"3 (33)"},
{"4 (34)"},
{"5 (35)"},
{"6 (36)"},
{"7 (37)"},
{"8 (38)"},
{"9 (39)"},
{"A (41)"},
{"B (42)"},
{"C (43)"},
{"D (44)"},
{"E (45)"},
{"F (46)"},
{"G (47)"},
{"H (48)"},
{"I (49)"},
{"J (4A)"},
{"K (4B)"},
{"L (4C)"},
{"M (4D)"},
{"N (4E)"},
{"O (4F)"},
{"P (50)"},
{"Q (51)"},
{"R (52)"},
{"S (53)"},
{"T (54)"},
{"U (55)"},
{"V (56)"},
{"W (57)"},
{"X (58)"},
{"Y (59)"},
{"Z (5A)"},
{"VK_NUMPAD0 (60)"},
{"VK_NUMPAD1 (61)"},
{"VK_NUMPAD2 (62)"},
{"VK_NUMPAD3 (63)"},
{"VK_NUMPAD4 (64)"},
{"VK_NUMPAD5 (65)"},
{"VK_NUMPAD6 (66)"},
{"VK_NUMPAD7 (67)"},
{"VK_NUMPAD8 (68)"},
{"VK_NUMPAD9 (69)"},
{"VK_MULTIPLY (6A)"},
{"VK_ADD (6B)"},
{"VK_SEPARATOR (6C)"},
{"VK_SUBTRACT (6D)"},
{"VK_DECIMAL (6E)"},
{"VK_DIVIDE (6F)"},
{"VK_F1 (70)"},
{"VK_F2 (71)"},
{"VK_F3 (72)"},
{"VK_F4 (73)"},
{"VK_F5 (74)"},
{"VK_F6 (75)"},
{"VK_F7 (76)"},
{"VK_F8 (77)"},
{"VK_F9 (78)"},
{"VK_F10 (79)"},
{"VK_F11 (7A)"},
{"VK_F12 (7B)"},
{"VK_F13 (7C)"},
{"VK_F14 (7D)"},
{"VK_F15 (7E)"},
{"VK_NUMLOCK (90)"},
{"VK_SCROLL (91)"},
{"VK_LSHIFT (A0)"},
{"VK_RSHIFT (A1)"},
{"VK_LCONTROL (A2)"},
{"VK_RCONTROL (A3)"},
{"VK_BROWSER_BACK (A6)"},
{"VK_BROWSER_FORWARD (A7)"},
{"VK_BROWSER_REFRESH (A8)"},
{"VK_BROWSER_STOP (A9)"},
{"VK_BROWSER_SEARCH (AA)"},
{"VK_BROWSER_FAVORITES (AB)"},
{"VK_BROWSER_HOME (AC)"},
{"VK_VOLUME_MUTE (AD)"},
{"VK_VOLUME_DOWN (AE)"},
{"VK_VOLUME_UP (AF)"},
{"VK_MEDIA_NEXT_TRACK (B0)"},
{"VK_MEDIA_PREV_TRACK (B1)"},
{"VK_MEDIA_STOP (B2)"},
{"VK_MEDIA_PLAY_PAUSE (B3)"},
{"VK_LAUNCH_MAIL (B4)"},
{"VK_LAUNCH_MEDIA_SELECT (B5)"},
{"VK_LAUNCH_APP1 (B6)"},
{"VK_LAUNCH_APP2 (B7)"},
{"\0"}};



unsigned char get_vk(char * vstr)
{
	unsigned char val=0;
	char *pos;
	if (!(pos=strstr(vstr,"("))) return(0);
	pos++;
    if ((*pos>='0')&&(*pos<='9')) val= (*pos-'0') *16; else val= (*pos-'A'+10) *16;
	pos++;
	if ((*pos>='0')&&(*pos<='9')) val+= (*pos-'0'); else val+= (*pos-'A'+10);
	return(val);
}


DWORD get_fl( char * vstr)
{
	if (*vstr=='R') return(KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY);   //| KEYEVENTF_EXTENDEDKEY | 
	else return(KEYEVENTF_EXTENDEDKEY);
}

LRESULT CALLBACK KeystrikeDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{

	KEYSTRIKEOBJ * st;
	char sztemp[50];
	
	st = (KEYSTRIKEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_KEYSTRIKE)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				int i=0;
				SetDlgItemText(hDlg, IDC_KEY,"test");
				while (virtkeys[i][0]) 
					SendDlgItemMessage(hDlg, IDC_KEYCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) virtkeys[i++]) ;
				
				SendDlgItemMessage( hDlg, IDC_MIDIINSTCOMBO, CB_SETCURSEL, 0, 0L ) ;
				
				for (i=0;i<st->numkeys;i++)
					SendDlgItemMessage(hDlg, IDC_KEYLIST, LB_ADDSTRING, 0,(LPARAM) (LPSTR) st->keylist[i]) ;

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_KEYCOMBO: 
				 if (HIWORD(wParam)==CBN_SELCHANGE)
				 {
				    st->actkey=SendMessage(GetDlgItem(hDlg, IDC_KEYCOMBO), CB_GETCURSEL , 0, 0);
					SetDlgItemText(hDlg, IDC_KEY,virtkeys[st->actkey]);
				 }
				 break;

			case IDC_KEYLIST:
				if (HIWORD(wParam)==LBN_SELCHANGE)
                {
					int i,sel;
					sel=SendDlgItemMessage( hDlg, IDC_KEYLIST, LB_GETCURSEL , 0, 0L ) ;
					SendDlgItemMessage( hDlg, IDC_KEYLIST, LB_DELETESTRING , (WPARAM) sel, 0L ) ;
					 
					 //SendDlgItemMessage(hDlg, IDC_KEYLIST, LB_GETTEXT, st->actkey, (LPARAM) (LPSTR) sztemp) ;	
				    //SetDlgItemText(hDlg, IDC_KEY,sztemp);
					for (i=sel;i<st->numkeys;i++)
						strcpy (st->keylist[i],st->keylist[i+1]);
					st->numkeys--;
				}
				break;
			case IDC_ADDKEYUP: 
				if (st->actkey)
				{
				strcpy (sztemp,"Release ");
				strcat(sztemp,virtkeys[st->actkey]);
				SendDlgItemMessage(hDlg, IDC_KEYLIST, LB_ADDSTRING, 0,(LPARAM) (LPSTR) sztemp) ;
				strcpy(st->keylist[st->numkeys],sztemp);
				st->numkeys++;
				}
				break;
			case IDC_ADDKEYDOWN: 
				if (st->actkey)
				{
				strcpy (sztemp,"Press ");
				strcat(sztemp,virtkeys[st->actkey]);
				SendDlgItemMessage(hDlg, IDC_KEYLIST, LB_ADDSTRING, 0,(LPARAM) (LPSTR) sztemp) ;
				strcpy(st->keylist[st->numkeys],sztemp);
				st->numkeys++;
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



KEYSTRIKEOBJ::KEYSTRIKEOBJ(int num) : BASE_CL()
{
	int i;
	outports = 0;
	inports = 1;
	width=65;
	actkey=0; numkeys=0;
	strcpy(in_ports[0].in_name,"trigger");
	for (i=0;i<50;i++) keylist[i][0]=0;
	
}
	
void KEYSTRIKEOBJ::make_dialog(void) 
{
  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_KEYSTRIKEBOX, ghWndStatusbox, (DLGPROC)KeystrikeDlgHandler));

}

void KEYSTRIKEOBJ::load(HANDLE hFile) 
{
	int i;
	char sztemp[50];

	load_object_basics(this);
    load_property("numkeys",P_INT,&numkeys);
	for (i=0;i<numkeys;i++)
	{
		wsprintf(sztemp,"key%d:",i+1);
		load_property(sztemp,P_STRING,keylist[i]);
	}

}

void KEYSTRIKEOBJ::save(HANDLE hFile) 
{
	int i;
	char sztemp[50];

	save_object_basics(hFile, this);
    save_property(hFile,"numkeys",P_INT,&numkeys);
	for (i=0;i<numkeys;i++)
	{
		wsprintf(sztemp,"key%d:",i+1);
		save_property(hFile,sztemp,P_STRING,keylist[i]);
	}

    
}
	
void KEYSTRIKEOBJ::incoming_data(int port, float value)
{
	input=value;
}
	
void KEYSTRIKEOBJ::work(void)
{
	int i;

	if (GLOBAL.fly) return;

	if ((oldinput==INVALID_VALUE)&&(input!=INVALID_VALUE)) 
	{
		for (i=0;i<numkeys;i++)
		{
			//keybd_event(get_vk(keylist[i]),0x4D, get_fl(keylist[i]),0); 
			keybd_event(get_vk(keylist[i]),MapVirtualKey(get_vk(keylist[i]),0) , get_fl(keylist[i]),0); 
			//keybd_event(get_vk(keylist[i]),0, KEYEVENTF_EXTENDEDKEY,0); 
		}
		
	}

	oldinput=input;

}
	
KEYSTRIKEOBJ::~KEYSTRIKEOBJ() 
{
	
}

