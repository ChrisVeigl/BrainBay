/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SKINDIALOG.CPP:  functions for the Skinned User Dialog

  based on the SkinStyle- Win32 Skinning Library 
  by John Roark <jroark@cs.usfca.edu>
  http://www.codeproject.com/dialog/skinstyle.asp
 
   This object callows to import a user-draw dialog from a skin.ini-file,
   to define button- and text-fields, and to process the dialog-events.
   the states of the buttons are presented at the object's output ports.

   Due to a Modification of the Skinstyle-Library, two buttons and one text-field
   can be combined to a Slider-Element: the output-value can be increased and
   decreased with the two buttons. 
   
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/

#ifndef MINGW

#include "brainBay.h"
#include "ob_skindialog.h"


class KSkin : public SkinDialog
{
public:
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam)
	{
		if ( wParam == VK_ESCAPE )
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
	}
 
	virtual BOOL OnButtonPressed(char *ButtonName);
	virtual BOOL OnMouseEnter(char *ButtonName);
	virtual BOOL OnMouseExit(char *ButtonName);
	
} win;

HWND hParent=0;
SKINDIALOGOBJ * actskin=0;
char skinbuffer[8192];
HBITMAP g_hbm = NULL;


int GetFile(char * file)
{
	OPENFILENAME ofn;
	
	
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "Skin Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "ini";

	
	return((int)GetOpenFileName(&ofn));
}

int SKINDIALOGOBJ::update_sliders(void)
{
	unsigned int i,actslider=0;
	int x;
	for (i=0;i<win.m_Buttons.GetSize();i++)
	{
		if ((x=win.m_Buttons.operator [](i)->slider)>0) 
		{	sliders[actslider]=x-1; actslider++; }
	}

	num_sliders=actslider;
	return (actslider);
}

int SKINDIALOGOBJ::sliderport(int sliderval)
{
	int i;
	for (i=0;(i<num_sliders)&&(sliders[i]!=sliderval);i++) ;
	
	return (i);
}

int SKINDIALOGOBJ::update_buttons(void)
{
	unsigned int i,x,actbutton=0;
	for (i=0;i<win.m_Buttons.GetSize();i++)
	{
	
		if ((x=win.m_Buttons.operator [](i)->slider)==0) 
		{ buttons[actbutton]=i; actbutton++; }
	}

	num_buttons=actbutton;
	return (actbutton);

}

int SKINDIALOGOBJ::buttonport(int buttonval)
{
	int i;
	for (i=0;(i<num_buttons)&&(buttons[i]!=buttonval);i++) ;
	
	return (i);
}


BOOL KSkin::OnButtonPressed(char *ButtonName)
{
	unsigned int i;
	int actbutton=-1,actslider=0,m;

	for (i=0;(i<m_Buttons.GetSize())&&(actbutton==-1);i++)
			if (!strcmp(m_Buttons.operator [](i)->m_BtnName, ButtonName)) actbutton=i;
	
	if (actbutton>-1) actslider=m_Buttons.operator [](actbutton)->slider;

	
	if (actslider < 0) 
	{
		char tmp[100];
		int d;

		actslider=-1-actslider;
		d=m_Labels.operator [](actslider)->value;
		if (d>m_Labels.operator [](actslider)->min)	d--;
		wsprintf(tmp,"%d",d);
		m_Labels.operator [](actslider)->value=d;
		m_Labels.operator [](actslider)->SetWindowText(tmp);
	
		actskin->setslider[actskin->sliderport(actslider)]=d;
	
	}
	else if (actslider > 0) 
	{
		char tmp[100];
		int d;
		actslider--;

		d=m_Labels.operator [](actslider)->value;
		if (d<m_Labels.operator [](actslider)->max)	d++;
		wsprintf(tmp,"%d",d);

		m_Labels.operator [](actslider)->value=d;
		m_Labels.operator [](actslider)->SetWindowText(tmp);
		
		actskin->setslider[actskin->sliderport(actslider)]=d;

	}
	else if (actbutton>-1)
	{
		m=m_Buttons.operator [](actbutton)->buttonmode;
		if (m==0)
		{
			actskin->setbutton[actskin->buttonport(actbutton)]=1;
			actskin->resetbutton[actskin->buttonport(actbutton)]=100;
		}
		if (m==-1)
		{
			actskin->setbutton[actskin->buttonport(actbutton)]=2;
			//actskin->resetbutton[actskin->buttonport(actbutton)]=500;
		}

	}



	return FALSE;
}

BOOL KSkin::OnMouseEnter(char *ButtonName)
{
	unsigned int i;
	int actbutton=-1,m;

	for (i=0;i<m_Buttons.GetSize();i++)
		if (!strcmp(m_Buttons.operator [](i)->m_BtnName, ButtonName)) actbutton=i;

	if (actbutton>-1)
	{
	   m=m_Buttons.operator [](actbutton)->buttonmode;
	   if (m>0)
	   {
		   if (m_Buttons.operator [](actbutton)->_mouse_over==true)
		   {
			   m_Buttons.operator [](actbutton)->_mouse_over=false;
			   actskin->resetbutton[actskin->buttonport(actbutton)]=1;
		   }
		   else
		   {
				for (i=0;i<m_Buttons.GetSize();i++)
					if ((m_Buttons.operator [](i)->buttonmode==m)&&(m_Buttons.operator [](i)->_mouse_over==true))
					{
						m_Buttons.operator [](i)->_mouse_over=false;
						actskin->resetbutton[actskin->buttonport(i)]=1;
						::InvalidateRect(m_Buttons.operator [](i)->m_hWnd, NULL, false );
					}
				m_Buttons.operator [](actbutton)->_mouse_over=true;
				actskin->setbutton[actskin->buttonport(actbutton)]=1;
		   } 
		   
		   ::InvalidateRect(m_Buttons.operator [](actbutton)->m_hWnd, NULL, false );
		   
	   }
	}

	//	m_Labels.operator [](0)->SetWindowText(tmp);

	return FALSE;
}

BOOL KSkin::OnMouseExit(char *ButtonName)
{
	
//		m_Labels.operator [](0)->SetWindowText(tmp);
//		m_Labels.operator [](0)->SetWindowText(tmp);

	return FALSE;
}

int SKINDIALOGOBJ::parse_skinfile(void)
{
	char tmpfile[250];
	char *c, *ext, *end;
	DWORD dwRead;
	HANDLE h;
	int i,pos;

	buttoninfos=0;
	sliderinfos=0;

	if (!strstr(skinfile,":"))
	{
		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"SKINDIALOGS\\");
		strcat(tmpfile,skinfile);
	} else strcpy(tmpfile,skinfile);
	
	if((strstr(tmpfile,".ini"))&&( (h = CreateFile(tmpfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))!=INVALID_HANDLE_VALUE))
	{

		if (! ReadFile(h,skinbuffer,8192,&dwRead,NULL))  { CloseHandle(h); return(0); }
		CloseHandle(h);
		skinbuffer[dwRead]=0;
		if (!(c=strstr(skinbuffer, "Mask="))) return(0);
		c+=5;i=0;
		while ((*c!=10)&&(*c!=13)&&(*c!='_')&&(*c)) bmpfilename[i++]=*c++;
		bmpfilename[i]=0;

		if ((ext=strstr(skinbuffer,"[BUTTONINFO]"))==NULL) return(0);
		if ((end=strstr(skinbuffer,"[TEXTINFO]")))  *end=0;

		while (ext=strstr(ext,"="))
		{
		  if (ext=strstr(ext,","))
		  {
			pos=get_int(ext,0,&(buttoninfo[buttoninfos].x1));
			pos=get_int(ext,pos,&(buttoninfo[buttoninfos].y1));
			pos=get_int(ext,pos,&(buttoninfo[buttoninfos].x2));
			pos=get_int(ext,pos,&(buttoninfo[buttoninfos].y2));
			ext+=pos+1; i=0;
			while ((*ext)&&(*ext!=',')) buttoninfo[buttoninfos].caption[i++]=*ext++;
			buttoninfo[buttoninfos].caption[i]=0;

			pos=get_int(ext,0,&(buttoninfo[buttoninfos].slidernum));
			pos=get_int(ext,pos,&(buttoninfo[buttoninfos].buttontype));

//			wsprintf(tmpfile,"%d,%d,%d,%d,%d,%s,%d,%d",buttoninfos,buttoninfo[buttoninfos].x1,buttoninfo[buttoninfos].y1,buttoninfo[buttoninfos].x2,buttoninfo[buttoninfos].y2,buttoninfo[buttoninfos].caption,buttoninfo[buttoninfos].slidernum,buttoninfo[buttoninfos].buttontype);
//			report(tmpfile);
			buttoninfo[buttoninfos].num=buttoninfos+1;
			buttoninfos++;
		  }
		}
		if (end)
		{
			*end='['; ext=end;
			while (ext=strstr(ext,"="))
			{
              ext++; i=0;
			  while ((*ext)&&(*ext!=',')) sliderinfo[sliderinfos].caption[i++]=*ext++;
			  sliderinfo[sliderinfos].caption[i]=0;

			  strcpy(sliderinfo[sliderinfos].font,"Arial");

			  pos=get_int(ext,0,&(sliderinfo[sliderinfos].fontsize));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].fontcolor));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].x1));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].y1));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].x2));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].y2));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].min));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].max));
			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].set));
  			  pos=get_int(ext,pos,&(sliderinfo[sliderinfos].type));


//			  wsprintf(tmpfile,"%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",sliderinfo[sliderinfos].caption,sliderinfo[sliderinfos].font,sliderinfo[sliderinfos].fontsize,sliderinfo[sliderinfos].fontcolor,sliderinfo[sliderinfos].x1,sliderinfo[sliderinfos].y1,sliderinfo[sliderinfos].x2,sliderinfo[sliderinfos].y2,sliderinfo[sliderinfos].min,sliderinfo[sliderinfos].max,sliderinfo[sliderinfos].set,sliderinfo[sliderinfos].type);
			// report(tmpfile);
			  sliderinfo[sliderinfos].num=sliderinfos+1;
			  sliderinfos++;
			}
		}

		if (buttoninfos) actbuttoninfo=0;
		if (sliderinfos) actsliderinfo=0;

		return(1);

	}
	return(0);
}

int SKINDIALOGOBJ::save_skinfile(void)
{
	char tmpfile[250];
	char actline[250];
	DWORD dwWritten;
	HANDLE h;
	int i;
	
	if (!strstr(skinfile,":"))
	{
		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"SKINDIALOGS\\");
//		strcat(tmpfile,"test.ini");
		strcat(tmpfile,skinfile);
	} else strcpy(tmpfile,skinfile);


	
	if((h = CreateFile(tmpfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL))!=INVALID_HANDLE_VALUE)
	{
		strcpy(actline,"[SCREEN]\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
		strcpy(actline,"Mask=");strcat(actline,bmpfilename);strcat(actline,"_Mask.bmp\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
		strcpy(actline,"Main=");strcat(actline,bmpfilename);strcat(actline,"_Active.bmp\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
		strcpy(actline,"Down=");strcat(actline,bmpfilename);strcat(actline,"_Selected.bmp\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
		strcpy(actline,"Over=");strcat(actline,bmpfilename);strcat(actline,"_Selected.bmp\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
		strcpy(actline,"Disabled=");strcat(actline,bmpfilename);strcat(actline,"_Active.bmp\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);
	
		strcpy(actline,"\r\n[BUTTONINFO]\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);

		for (i=0;i<buttoninfos;i++)
		{
			wsprintf(actline,"%d=%s,%d,%d,%d,%d,%s,FALSE,%d,%d\r\n",i+1,buttoninfo[i].caption,buttoninfo[i].x1,buttoninfo[i].y1,buttoninfo[i].x2,buttoninfo[i].y2,buttoninfo[i].caption,buttoninfo[i].slidernum,buttoninfo[i].buttontype);
			WriteFile(h,actline,strlen(actline),&dwWritten,NULL);

		}

		strcpy(actline,"\r\n[TEXTINFO]\r\n");
		WriteFile(h,actline,strlen(actline),&dwWritten,NULL);

		for (i=0;i<sliderinfos;i++)
		{
			wsprintf(actline,"%d=%s,%s,FALSE,TRUE,%d,%d,%d,%d,%d,%d, , ,%d,%d,%d,%d\r\n",i+1,sliderinfo[i].caption,sliderinfo[i].font,sliderinfo[i].fontsize,sliderinfo[i].fontcolor,sliderinfo[i].x1,sliderinfo[i].y1,sliderinfo[i].x2,sliderinfo[i].y2,sliderinfo[i].min,sliderinfo[i].max,sliderinfo[i].set,sliderinfo[i].type);
			WriteFile(h,actline,strlen(actline),&dwWritten,NULL);

		}

		CloseHandle(h);
		return(1);
	}
	return(0);
}

int SKINDIALOGOBJ::apply_skin(void)
{
	int i,s;
	char tmpfile[256];
	HICON hIconBig = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_MYEEG));
	WINDOWPLACEMENT  wndpl;
    HICON hIconSmall = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_MYEEG));

	if (!strstr(skinfile,":"))
	{
		strcpy(tmpfile,GLOBAL.resourcepath);
		strcat(tmpfile,"SKINDIALOGS\\");
		strcat(tmpfile,skinfile);
	} else strcpy(tmpfile,skinfile);
	
	  hParent = win.CreateEx("KSkin", CW_USEDEFAULT, CW_USEDEFAULT, hInst, tmpfile);
	  if (hParent)
	  {
		
		win.SetSticky(true);
	
		SendMessage((HWND) hParent,	WM_SETICON,	(WPARAM) ICON_BIG,(LPARAM) hIconBig);
		SendMessage((HWND) hParent,	WM_SETICON,	(WPARAM) ICON_SMALL,(LPARAM) hIconSmall	);

		win.ShowWindow(TRUE);
		win.UpdateWindow();
		
		actskin=this;

		outports=update_sliders()+update_buttons();
		inports=outports;
		height=CON_START+outports*CON_HEIGHT+5;
		width=110;
	
        for (s=0;s<inports;s++)
			strcpy(in_ports[s].in_name," ");
		
		for (s=0;s<num_sliders;s++)
			strcpy(out_ports[s].out_name,win.m_Labels.operator [](sliders[s])->m_BtnName);

		for (i=0;i<num_buttons;i++)
			strcpy(out_ports[s+i].out_name,win.m_Buttons.operator [](buttons[i])->m_BtnName);
		InvalidateRect(ghWndDesign,NULL,TRUE);
	
		GetWindowPlacement(hParent, &wndpl);
		skin_top=wndpl.rcNormalPosition.top;
		skin_left=wndpl.rcNormalPosition.left;
		skin_right=wndpl.rcNormalPosition.right;
		skin_bottom=wndpl.rcNormalPosition.bottom;
	
		MoveWindow(hParent,skin_left,skin_top,skin_right-skin_left,skin_bottom-skin_top,TRUE);
	
		return(1);
	  }

	report_error("No valid Skin-File");
	return(0);

}

SKINDIALOGOBJ::SKINDIALOGOBJ(int num) : BASE_CL()
{
	int i;
	outports = 0;
	inports = 0;
	width=75;
	height=60;
	num_sliders=0;num_buttons=0;
	buttoninfos=0;sliderinfos=0;
	actsliderinfo=-1;
	actbuttoninfo=-1;
	for (i=0;i<30;i++) { resetbutton[i]=0; setbutton[i]=0; setslider[i]=INVALID_VALUE; }
	strcpy(skinfile,"none");
	strcpy(bmpfilename,"none");

}
	
void SKINDIALOGOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SKINDIALOGBOX, ghWndStatusbox, (DLGPROC)SkinDialogDlgHandler));
}

void SKINDIALOGOBJ::load(HANDLE hFile) 
{
	int i,d;
	char tmp[50];

	for (i=0;i<30;i++) { resetbutton[i]=0; setbutton[i]=0; setslider[i]=INVALID_VALUE; }

	load_object_basics(this);
    load_property("skinfile",P_STRING,skinfile);
	
	if (parse_skinfile())
	if (apply_skin())
	{

		for(i=0;i<num_buttons;i++)
		{
			wsprintf(tmp,"button%d",i+1);
			load_property(tmp,P_INT,&d);
			if (d) win.m_Buttons.operator [](buttons[i])->_mouse_over=true; 
			else win.m_Buttons.operator [](buttons[i])->_mouse_over=false;
			
			if ((win.m_Buttons.operator [](buttons[i])->buttonmode) && (d))
			   setbutton[i]=1;
		}

	    for(i=0;i<num_sliders;i++)
		{
			wsprintf(tmp,"slider%d",i+1);
			load_property(tmp,P_INT,&d);
			win.m_Labels.operator [](sliders[i])->value=d;
			setslider[i]=d;
		}

		load_property("skin_top",P_INT,&skin_top);
		load_property("skin_left",P_INT,&skin_left);
		load_property("skin_right",P_INT,&skin_right);
		load_property("skin_bottom",P_INT,&skin_bottom);
		MoveWindow(hParent,skin_left,skin_top,skin_right-skin_left,skin_bottom-skin_top,TRUE);

		InvalidateRect(hParent,NULL,TRUE);
	}
}

void SKINDIALOGOBJ::save(HANDLE hFile) 
{
	WINDOWPLACEMENT  wndpl;
	int i,d;
	char tmp[50];
	
	GetWindowPlacement(hParent, &wndpl);
    skin_top=wndpl.rcNormalPosition.top;
    skin_left=wndpl.rcNormalPosition.left;
    skin_right=wndpl.rcNormalPosition.right;
    skin_bottom=wndpl.rcNormalPosition.bottom;

	save_object_basics(hFile, this);
    save_property(hFile,"skinfile",P_STRING,skinfile);
	save_property(hFile,"skin_top",P_INT,&skin_top);
	save_property(hFile,"skin_left",P_INT,&skin_left);
	save_property(hFile,"skin_right",P_INT,&skin_right);
	save_property(hFile,"skin_bottom",P_INT,&skin_bottom);

	for(i=0;i<num_buttons;i++)
	{
		wsprintf(tmp,"button%d",i+1);
		if (win.m_Buttons.operator [](buttons[i])->_mouse_over) d=1; else d=0;
		save_property(hFile,tmp,P_INT,&d);
	}

    for(i=0;i<num_sliders;i++)
	{
		wsprintf(tmp,"slider%d",i+1);
		save_property(hFile,tmp,P_INT,&(win.m_Labels.operator [](sliders[i])->value));
	}
		

}
	


void SKINDIALOGOBJ::incoming_data(int port, float value)
{
	int actbutton;
	
	if (port>=num_sliders)
	{
		actbutton=port-num_sliders;

		if ((value!=INVALID_VALUE)&&(win.m_Buttons.operator [](buttons[actbutton])->_mouse_over==false))
		{
			setbutton[actbutton]=1;
		}
		else if ((value==INVALID_VALUE)&&(win.m_Buttons.operator [](buttons[actbutton])->_mouse_over==true))
		{
			resetbutton[actbutton]=1;
		}
		
	}
	else
	{
		int actslider,d;

		actslider=port;
		d=(int)value;
		if ((d>=win.m_Labels.operator [](sliders[actslider])->min) &&
			(d<=win.m_Labels.operator [](sliders[actslider])->max) &&
			(win.m_Labels.operator [](sliders[actslider])->value!=d))  
		{
			setslider[actslider]=d;
			
		}
	}


}

void SKINDIALOGOBJ::work(void)
{
	int i;
	if (GLOBAL.fly) return;

	for(i=0;i<num_buttons;i++)
	{
		if (resetbutton[i])
		{
			resetbutton[i]--;
			if (!(resetbutton[i])) 
			{ 
				win.m_Buttons.operator [](buttons[i])->_mouse_over=false;
			    ::InvalidateRect(win.m_Buttons.operator [](buttons[i])->m_hWnd, NULL, false );
				pass_values(num_sliders+i,INVALID_VALUE);
			}
		}

		if (setbutton[i]==1)
		{
			win.m_Buttons.operator [](buttons[i])->_mouse_over=true;
			::InvalidateRect(win.m_Buttons.operator [](buttons[i])->m_hWnd, NULL, false );
			pass_values(num_sliders+i, 1.0f);
			setbutton[i]=0;
		}	
		if (setbutton[i]==2)
		{
			if (win.m_Buttons.operator [](buttons[i])->_mouse_over==false)
			{
				win.m_Buttons.operator [](buttons[i])->_mouse_over=true;
				::InvalidateRect(win.m_Buttons.operator [](buttons[i])->m_hWnd, NULL, false );
				pass_values(num_sliders+i, 1.0f);
			}
			else
			{
				win.m_Buttons.operator [](buttons[i])->_mouse_over=false;
			    ::InvalidateRect(win.m_Buttons.operator [](buttons[i])->m_hWnd, NULL, false );
				pass_values(num_sliders+i,INVALID_VALUE);
			}
			setbutton[i]=0;
			resetbutton[i]=0;
		}	
		
	}

    for(i=0;i<num_sliders;i++)
	{
		if (setslider[i]!=INVALID_VALUE)
		{

			char tmp[20];
			wsprintf(tmp,"%d",setslider[i]);

			win.m_Labels.operator [](sliders[i])->value=setslider[i];
			win.m_Labels.operator [](sliders[i])->SetWindowText(tmp);

			pass_values(i, (float)setslider[i]);
			setslider[i]=INVALID_VALUE;
		}
	}
	
}


SKINDIALOGOBJ::~SKINDIALOGOBJ() 
{
	if (hParent)  win.OnKeyDown(VK_ESCAPE,0); 
	hParent=0;
	if (g_hbm) { DeleteObject(g_hbm); g_hbm = NULL; }

}



void save_elements(HWND hDlg, SKINDIALOGOBJ * st)
{
	

	if (st->buttoninfos>0)
	{
	  if (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE1)) { st->buttoninfo[st->actbuttoninfo].buttontype=0;st->buttoninfo[st->actbuttoninfo].slidernum=0; }
	  if (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE2)) { st->buttoninfo[st->actbuttoninfo].buttontype=GetDlgItemInt(hDlg,IDC_BUTTONPARAM,NULL,0);st->buttoninfo[st->actbuttoninfo].slidernum=0; }
	  if (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE3)) { st->buttoninfo[st->actbuttoninfo].buttontype=0;st->buttoninfo[st->actbuttoninfo].slidernum=GetDlgItemInt(hDlg,IDC_BUTTONPARAM,NULL,0); }
	  if (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE4)) { st->buttoninfo[st->actbuttoninfo].buttontype=0;st->buttoninfo[st->actbuttoninfo].slidernum=GetDlgItemInt(hDlg,IDC_BUTTONPARAM,NULL,0)*-1; }
	  if (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE5)) { st->buttoninfo[st->actbuttoninfo].buttontype=-1;st->buttoninfo[st->actbuttoninfo].slidernum=0; }

	  GetDlgItemText(hDlg,IDC_BUTTONNAME,st->buttoninfo[st->actbuttoninfo].caption,30);

	}

	if (st->sliderinfos>0)
	{
	  if (IsDlgButtonChecked(hDlg,IDC_SLIDERTYPE1)) st->sliderinfo[st->actsliderinfo].type=0;
	  if (IsDlgButtonChecked(hDlg,IDC_SLIDERTYPE2)) st->sliderinfo[st->actsliderinfo].type=1;
	  if (IsDlgButtonChecked(hDlg,IDC_SLIDERTYPE3)) st->sliderinfo[st->actsliderinfo].type=2;
	  
	  GetDlgItemText(hDlg,IDC_SLIDERNAME,st->sliderinfo[st->actsliderinfo].caption,30);

	  st->sliderinfo[st->actsliderinfo].min=GetDlgItemInt(hDlg,IDC_MIN,NULL,1);
	  st->sliderinfo[st->actsliderinfo].max=GetDlgItemInt(hDlg,IDC_MAX,NULL,1);
	  st->sliderinfo[st->actsliderinfo].set=GetDlgItemInt(hDlg,IDC_SET,NULL,1);
	  st->sliderinfo[st->actsliderinfo].fontsize=GetDlgItemInt(hDlg,IDC_FONTSIZE,NULL,1);
	}

	InvalidateRect(hDlg,NULL,TRUE);

}


void disp_elements(HWND hDlg, SKINDIALOGOBJ * st)
{

	if (st->buttoninfos>0)
	{
	  CheckDlgButton(hDlg,IDC_BUTTONTYPE1,FALSE);
	  CheckDlgButton(hDlg,IDC_BUTTONTYPE2,FALSE);
	  CheckDlgButton(hDlg,IDC_BUTTONTYPE3,FALSE);
	  CheckDlgButton(hDlg,IDC_BUTTONTYPE4,FALSE);
	  CheckDlgButton(hDlg,IDC_BUTTONTYPE5,FALSE);
	  EnableWindow(GetDlgItem(hDlg,IDC_BUTTONPARAM),TRUE);
	  EnableWindow(GetDlgItem(hDlg,IDC_PARAMNAME),TRUE);

	  SetDlgItemText(hDlg,IDC_BUTTONNAME,st->buttoninfo[st->actbuttoninfo].caption);
	  if (st->buttoninfo[st->actbuttoninfo].slidernum>0)	CheckDlgButton(hDlg,IDC_BUTTONTYPE3,TRUE);
	  else if (st->buttoninfo[st->actbuttoninfo].slidernum<0) CheckDlgButton(hDlg,IDC_BUTTONTYPE4,TRUE);
	  else if (st->buttoninfo[st->actbuttoninfo].buttontype>0) CheckDlgButton(hDlg,IDC_BUTTONTYPE2,TRUE);
	  else if (st->buttoninfo[st->actbuttoninfo].buttontype==0) CheckDlgButton(hDlg,IDC_BUTTONTYPE1,TRUE);
	  else if (st->buttoninfo[st->actbuttoninfo].buttontype==-1) CheckDlgButton(hDlg,IDC_BUTTONTYPE5,TRUE);

	  if (st->buttoninfo[st->actbuttoninfo].slidernum!=0) 
	  {
		SetDlgItemText(hDlg,IDC_PARAMNAME,"Slider Number");
		SetDlgItemInt(hDlg,IDC_BUTTONPARAM,abs(st->buttoninfo[st->actbuttoninfo].slidernum),0);
	  }
	  else if (st->buttoninfo[st->actbuttoninfo].buttontype>0) 
	  {
		SetDlgItemText(hDlg,IDC_PARAMNAME,"Button Group");
		SetDlgItemInt(hDlg,IDC_BUTTONPARAM,st->buttoninfo[st->actbuttoninfo].buttontype,0);
	  }
	  else
	  {
		  EnableWindow(GetDlgItem(hDlg,IDC_BUTTONPARAM),FALSE);
		  EnableWindow(GetDlgItem(hDlg,IDC_PARAMNAME),FALSE);
	  }
	}

	if (st->sliderinfos>0)
	{
	  CheckDlgButton(hDlg,IDC_SLIDERTYPE1,FALSE);CheckDlgButton(hDlg,IDC_SLIDERTYPE2,FALSE);
	  SetDlgItemText(hDlg,IDC_SLIDERNAME,st->sliderinfo[st->actsliderinfo].caption);
	  SetDlgItemInt(hDlg,IDC_MIN,st->sliderinfo[st->actsliderinfo].min,1);
	  SetDlgItemInt(hDlg,IDC_MAX,st->sliderinfo[st->actsliderinfo].max,1);
	  SetDlgItemInt(hDlg,IDC_SET,st->sliderinfo[st->actsliderinfo].set,1);
	  SetDlgItemInt(hDlg,IDC_FONTSIZE,st->sliderinfo[st->actsliderinfo].fontsize,1);
	  if(st->sliderinfo[st->actsliderinfo].type==0) CheckDlgButton(hDlg,IDC_SLIDERTYPE1,TRUE); 
	  if(st->sliderinfo[st->actsliderinfo].type==1) CheckDlgButton(hDlg,IDC_SLIDERTYPE2,TRUE); 
	  if(st->sliderinfo[st->actsliderinfo].type==2) CheckDlgButton(hDlg,IDC_SLIDERTYPE3,TRUE); 
	  else CheckDlgButton(hDlg,IDC_SLIDERTYPE2,TRUE);
	}

	InvalidateRect(hDlg,NULL,TRUE);

}

int test_bitmap(HWND hDlg, SKINDIALOGOBJ * st)
{
	  HANDLE testfile;
	  char path[256],actfn[256],tmp[50];
	  int ok=1;

	  strcpy(path,GLOBAL.resourcepath);
	  strcat(path,"SKINDIALOGS\\");
	  GetDlgItemText(hDlg,IDC_BITMAPFILENAME,tmp,50);

	  strcpy(actfn,path);strcat(actfn,tmp);strcat(actfn,"_Active.bmp");
	  if ((testfile = CreateFile(actfn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))==INVALID_HANDLE_VALUE)
	  ok=0;  else CloseHandle(testfile);
					  
	  strcpy(actfn,path);strcat(actfn,tmp);strcat(actfn,"_Mask.bmp");
	  if ((testfile = CreateFile(actfn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))==INVALID_HANDLE_VALUE)
	  ok=0;  else CloseHandle(testfile);
	  
	  strcpy(actfn,path);strcat(actfn,tmp);strcat(actfn,"_Selected.bmp");
	  if ((testfile = CreateFile(actfn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))==INVALID_HANDLE_VALUE)
	  ok=0;  else CloseHandle(testfile);
					  
	  
	  if (ok) strcpy(st->bmpfilename,tmp); 
	  else 
	  {
		  wsprintf(path,"Bitmaps %s_Active.bmp, %s_Mask.bmp and %s_Selected.bmp not found.",tmp,tmp,tmp);
		  report_error(path); 
	  }
	  return(ok);
			  
}				      
				

int mindist=10000,minelem,mintype=0,actdist,setelem=-1;

LRESULT CALLBACK SkinDialogDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SKINDIALOGOBJ * st;
	RECT rect;
	char tmpfile[256];
	int i;
	
	st = (SKINDIALOGOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_SKINDIALOG)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemText(hDlg, IDC_SKINFILE, st->skinfile);
				SetDlgItemText(hDlg, IDC_BITMAPFILENAME, st->bmpfilename);

				if ((!g_hbm)&&(strcmp(st->skinfile,"none")))
				{
					strcpy(tmpfile,GLOBAL.resourcepath);
					strcat(tmpfile,"SKINDIALOGS\\");
					strcat(tmpfile,st->bmpfilename);
					strcat(tmpfile,"_Active.bmp");
 				    g_hbm = (HBITMAP) LoadImage(NULL, tmpfile,IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
					//if (!g_hbm) report_error("could not open Bitmap File");

				}
				GetClientRect(hDlg, &rect);
				st->dlgheight=rect.bottom-rect.top;
				st->dlgwidth=rect.right-rect.left;
				disp_elements(hDlg,st);
	
				break;		
		case WM_CLOSE:
			if (g_hbm)  { DeleteObject(g_hbm); g_hbm=NULL; }
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_OPENSKIN:
					if (!strcmp(st->skinfile,"none"))
					{
						strcpy(st->skinfile,GLOBAL.resourcepath);
						strcat(st->skinfile,"SKINDIALOGS\\*.ini");
					}
					GetFile(st->skinfile);
					reduce_filepath(st->skinfile,st->skinfile);
					SetDlgItemText(hDlg, IDC_SKINFILE, st->skinfile);
					break;
				case IDC_APPLYSKIN:
					{
						if (hParent)
						{
							SendMessage( (HWND) hParent,WM_CLOSE,(WPARAM) 0,(LPARAM) 0);
							hParent=0; actskin=0; 
						}

						save_elements(hDlg,st);
						GetDlgItemText(hDlg,IDC_SKINFILE,st->skinfile,30);
						
						if (test_bitmap(hDlg,st))
						{
						  st->save_skinfile();
						  if (st->parse_skinfile())  st->apply_skin();
						}
						disp_elements(hDlg,st);
					}
					break;
				case IDC_OPENBITMAP:
				  	  
					  if (test_bitmap(hDlg,st))
					  {
							if (g_hbm)  DeleteObject(g_hbm);
				 			
							strcpy(tmpfile,GLOBAL.resourcepath);
							strcat(tmpfile,"SKINDIALOGS\\");
							strcat(tmpfile,st->bmpfilename);
							strcat(tmpfile,"_Active.bmp");
 							g_hbm = (HBITMAP) LoadImage(NULL, tmpfile,IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
							if (!g_hbm) report_error("could not open Bitmap File");
			
					  }
					  disp_elements(hDlg,st);

					break;
				case IDC_BUTTONTYPE1:
				case IDC_BUTTONTYPE2:
				case IDC_BUTTONTYPE3:
				case IDC_BUTTONTYPE4:
				case IDC_BUTTONTYPE5:
				case IDC_SLIDERTYPE1:
				case IDC_SLIDERTYPE2:
				case IDC_SLIDERTYPE3:   
					save_elements(hDlg,st);
					if ((IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE1)) || (IsDlgButtonChecked(hDlg,IDC_BUTTONTYPE5)))
					{
					    EnableWindow(GetDlgItem(hDlg,IDC_BUTTONPARAM),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_PARAMNAME),FALSE);
					}
					else
					{
					    EnableWindow(GetDlgItem(hDlg,IDC_BUTTONPARAM),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_PARAMNAME),TRUE);
						if (st->buttoninfo[st->actbuttoninfo].slidernum!=0) 
						{
							SetDlgItemText(hDlg,IDC_PARAMNAME,"Slider Number");
							SetDlgItemInt(hDlg,IDC_BUTTONPARAM,abs(st->buttoninfo[st->actbuttoninfo].slidernum),0);
						}
						else if (st->buttoninfo[st->actbuttoninfo].buttontype>0) 
						{
							SetDlgItemText(hDlg,IDC_PARAMNAME,"Button Group");
							SetDlgItemInt(hDlg,IDC_BUTTONPARAM,st->buttoninfo[st->actbuttoninfo].buttontype,0);
						}
					}
					break;
				

				case IDC_NEXTBUTTON:
					save_elements(hDlg,st);
					st->actbuttoninfo++;
					if (st->actbuttoninfo>=st->buttoninfos) st->actbuttoninfo=0;
					disp_elements(hDlg,st);
					break;
				case IDC_NEXTSLIDER:
					
					save_elements(hDlg,st);
					st->actsliderinfo++;
					if (st->actsliderinfo>=st->sliderinfos) st->actsliderinfo=0;
					disp_elements(hDlg, st);
					break;
				case IDC_CREATEBUTTON:
						st->actbuttoninfo=st->buttoninfos;
						strcpy(st->buttoninfo[st->actbuttoninfo].caption,"new Button");
						st->buttoninfo[st->actbuttoninfo].num=st->actbuttoninfo;
						wsprintf(st->buttoninfo[st->actbuttoninfo].name,"%d",st->actbuttoninfo);
						st->buttoninfo[st->actbuttoninfo].buttontype=0;
						st->buttoninfo[st->actbuttoninfo].slidernum=0;
						st->buttoninfo[st->actbuttoninfo].x1=10;
						st->buttoninfo[st->actbuttoninfo].x2=10;
						st->buttoninfo[st->actbuttoninfo].y1=10;
						st->buttoninfo[st->actbuttoninfo].y2=10;
						st->buttoninfos++;
						disp_elements(hDlg,st);
					break;


				case IDC_CREATESLIDER:
						st->actsliderinfo=st->sliderinfos;
						strcpy(st->sliderinfo[st->actsliderinfo].caption,"new slider");
						st->sliderinfo[st->actsliderinfo].num=st->actsliderinfo;
						st->sliderinfo[st->actsliderinfo].type=0;
						st->sliderinfo[st->actsliderinfo].min=0;
						st->sliderinfo[st->actsliderinfo].max=10;
						st->sliderinfo[st->actsliderinfo].set=5;
						st->sliderinfo[st->actsliderinfo].x1=20;
						st->sliderinfo[st->actsliderinfo].x2=10;
						st->sliderinfo[st->actsliderinfo].y1=20;
						st->sliderinfo[st->actsliderinfo].y2=10;
						strcpy(st->sliderinfo[st->actsliderinfo].font,"Arial");
						st->sliderinfo[st->actsliderinfo].fontsize=-30;
						st->sliderinfo[st->actsliderinfo].fontcolor=255;
						st->sliderinfos++;
						disp_elements(hDlg,st);
					break;
				case IDC_DELETEBUTTON:
					if (st->buttoninfos>0)
					{
						memcpy(&(st->buttoninfo[st->actbuttoninfo]),&(st->buttoninfo[st->actbuttoninfo+1]), sizeof(struct buttoninfoStruct) * (st->buttoninfos-st->actbuttoninfo));
						st->buttoninfos--; if (st->actbuttoninfo>=st->buttoninfos) st->actbuttoninfo=st->buttoninfos-1;

						disp_elements(hDlg,st);
					}
					break;
				case IDC_DELETESLIDER:
					if (st->sliderinfos>0)
					{
						memcpy(&(st->sliderinfo[st->actsliderinfo]),&(st->sliderinfo[st->actsliderinfo+1]), sizeof(struct sliderinfoStruct) * (st->sliderinfos-st->actsliderinfo));
						st->sliderinfos--; if (st->actsliderinfo>=st->sliderinfos) st->actsliderinfo=st->sliderinfos-1;

						disp_elements(hDlg,st);
					}
					break;
			}
			return TRUE;
			break;
	 	case WM_LBUTTONUP: setelem=-1;
				break;
 	 	case WM_LBUTTONDOWN:

				  st->actmousex=(int)LOWORD(lParam);
				  st->actmousey=(int)HIWORD(lParam)-st->dlgheight;
				  SetDlgItemInt(hDlg,IDC_XPOS, st->actmousex,1); 
				  SetDlgItemInt(hDlg,IDC_YPOS, st->actmousey,1);

				  mindist=10000;minelem=-1;mintype=-1;

				  for (i=0;i<st->buttoninfos;i++)
				  {
				       actdist = (st->buttoninfo[i].x1-st->actmousex)*(st->buttoninfo[i].x1-st->actmousex)
						   + (st->buttoninfo[i].y1-st->actmousey)*(st->buttoninfo[i].y1-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=1; }

   				       actdist = (st->buttoninfo[i].x1+st->buttoninfo[i].x2-st->actmousex)*(st->buttoninfo[i].x1+st->buttoninfo[i].x2-st->actmousex)
						   + (st->buttoninfo[i].y1-st->actmousey)*(st->buttoninfo[i].y1-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=2; }

					   actdist = (st->buttoninfo[i].x1+st->buttoninfo[i].x2-st->actmousex)*(st->buttoninfo[i].x1+st->buttoninfo[i].x2-st->actmousex)
						   + (st->buttoninfo[i].y1+st->buttoninfo[i].y2-st->actmousey)*(st->buttoninfo[i].y1+st->buttoninfo[i].y2-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=3; }

					   actdist = (st->buttoninfo[i].x1-st->actmousex)*(st->buttoninfo[i].x1-st->actmousex)
						   + (st->buttoninfo[i].y1+st->buttoninfo[i].y2-st->actmousey)*(st->buttoninfo[i].y1+st->buttoninfo[i].y2-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=4; }
				  }

				  for (i=0;i<st->sliderinfos;i++)
				  {	   
				
					   actdist = (st->sliderinfo[i].x1-st->actmousex)*(st->sliderinfo[i].x1-st->actmousex)
						   + (st->sliderinfo[i].y1-st->actmousey)*(st->sliderinfo[i].y1-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=5; }

   				       actdist = (st->sliderinfo[i].x1+st->sliderinfo[i].x2-st->actmousex)*(st->sliderinfo[i].x1+st->sliderinfo[i].x2-st->actmousex)
						   + (st->sliderinfo[i].y1-st->actmousey)*(st->sliderinfo[i].y1-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=6; }

					   actdist = (st->sliderinfo[i].x1+st->sliderinfo[i].x2-st->actmousex)*(st->sliderinfo[i].x1+st->sliderinfo[i].x2-st->actmousex)
						   + (st->sliderinfo[i].y1+st->sliderinfo[i].y2-st->actmousey)*(st->sliderinfo[i].y1+st->sliderinfo[i].y2-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=7; }

					   actdist = (st->sliderinfo[i].x1-st->actmousex)*(st->sliderinfo[i].x1-st->actmousex)
						   + (st->sliderinfo[i].y1+st->sliderinfo[i].y2-st->actmousey)*(st->sliderinfo[i].y1+st->sliderinfo[i].y2-st->actmousey) ;
					   if (actdist<mindist) { mindist=actdist; minelem=i; mintype=8; }
				  }

				  if (mindist<100) setelem=minelem; else { setelem=-1; break; }
				  

		case WM_MOUSEMOVE:
				  st->actmousex=(int)LOWORD(lParam);
				  st->actmousey=(int)HIWORD(lParam)-st->dlgheight;
				  SetDlgItemInt(hDlg,IDC_XPOS, st->actmousex,1); 
				  SetDlgItemInt(hDlg,IDC_YPOS, st->actmousey,1); 
				  
				  if (setelem!=-1)
				  {
					  switch (mintype)
					  {

						 case 1: if (st->buttoninfo[setelem].x1+st->buttoninfo[setelem].x2-st->actmousex>0) {st->buttoninfo[setelem].x2-=st->actmousex-st->buttoninfo[setelem].x1; st->buttoninfo[setelem].x1=st->actmousex;}
						  if ((st->actmousey>0) && (st->actmousey<st->buttoninfo[setelem].y1+st->buttoninfo[setelem].y2)) {st->buttoninfo[setelem].y2-=st->actmousey-st->buttoninfo[setelem].y1; st->buttoninfo[setelem].y1=st->actmousey;}
							 break;
						 case 2: if (st->buttoninfo[setelem].x1<st->actmousex) st->buttoninfo[setelem].x2=st->actmousex-st->buttoninfo[setelem].x1;
							 if ((st->actmousey>0) && (st->actmousey<st->buttoninfo[setelem].y1+st->buttoninfo[setelem].y2)) {st->buttoninfo[setelem].y2-=st->actmousey-st->buttoninfo[setelem].y1; st->buttoninfo[setelem].y1=st->actmousey;}
							 break;
						 case 3: if (st->buttoninfo[setelem].x1<st->actmousex) st->buttoninfo[setelem].x2=st->actmousex-st->buttoninfo[setelem].x1;
							     if ((st->actmousey>0) && (st->actmousey>st->buttoninfo[setelem].y1)) st->buttoninfo[setelem].y2=st->actmousey-st->buttoninfo[setelem].y1;
							 break;
						 case 4: if (st->buttoninfo[setelem].x1+st->buttoninfo[setelem].x2>st->actmousex) { st->buttoninfo[setelem].x2-=st->actmousex-st->buttoninfo[setelem].x1; st->buttoninfo[setelem].x1=st->actmousex;}
							     if ((st->actmousey>0) && (st->actmousey>st->buttoninfo[setelem].y1)) st->buttoninfo[setelem].y2=st->actmousey-st->buttoninfo[setelem].y1;
							 break;

						 case 5: if (st->sliderinfo[setelem].x1+st->sliderinfo[setelem].x2-st->actmousex>0) {st->sliderinfo[setelem].x2-=st->actmousex-st->sliderinfo[setelem].x1; st->sliderinfo[setelem].x1=st->actmousex;}
						  if ((st->actmousey>0) && (st->actmousey<st->sliderinfo[setelem].y1+st->sliderinfo[setelem].y2)) {st->sliderinfo[setelem].y2-=st->actmousey-st->sliderinfo[setelem].y1; st->sliderinfo[setelem].y1=st->actmousey;}
							 break;
						 case 6: if (st->sliderinfo[setelem].x1<st->actmousex) st->sliderinfo[setelem].x2=st->actmousex-st->sliderinfo[setelem].x1;
							 if ((st->actmousey>0) && (st->actmousey<st->sliderinfo[setelem].y1+st->sliderinfo[setelem].y2)) {st->sliderinfo[setelem].y2-=st->actmousey-st->sliderinfo[setelem].y1; st->sliderinfo[setelem].y1=st->actmousey;}
							 break;
						 case 7: if (st->sliderinfo[setelem].x1<st->actmousex) st->sliderinfo[setelem].x2=st->actmousex-st->sliderinfo[setelem].x1;
							     if ((st->actmousey>0) && (st->actmousey>st->sliderinfo[setelem].y1)) st->sliderinfo[setelem].y2=st->actmousey-st->sliderinfo[setelem].y1;
							 break;
						 case 8: if (st->sliderinfo[setelem].x1+st->sliderinfo[setelem].x2>st->actmousex) { st->sliderinfo[setelem].x2-=st->actmousex-st->sliderinfo[setelem].x1; st->sliderinfo[setelem].x1=st->actmousex;}
							     if ((st->actmousey>0) && (st->actmousey>st->sliderinfo[setelem].y1)) st->sliderinfo[setelem].y2=st->actmousey-st->sliderinfo[setelem].y1;
							 break;
					  }
					  InvalidateRect(hDlg,NULL,TRUE);
				  }
				 break;


		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				BITMAP bm;
				HDC hdc;
				int i;
				HPEN	 tpen;
//				HBRUSH	 tbrush;
				WINDOWPLACEMENT  wndpl;


				if (g_hbm)
				{
					hdc = BeginPaint (hDlg, &ps);
					HDC hdcMem = CreateCompatibleDC(hdc);
					HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, g_hbm);
					GetObject(g_hbm, sizeof(bm), &bm);

					if (st->dlgwidth<bm.bmWidth+30) st->dlgwidth=bm.bmWidth+30;
					GetWindowPlacement(hDlg, &wndpl);
  					MoveWindow(hDlg,wndpl.rcNormalPosition.left,wndpl.rcNormalPosition.top,
						st->dlgwidth,
						st->dlgheight+bm.bmHeight+80,TRUE); 

					BitBlt(hdc, 0,st->dlgheight, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

					tpen    = CreatePen (PS_SOLID,3,50);
					SelectObject (hdc, tpen);
					SelectObject (hdc, DRAW.brush_ltorange);

					for (i=0;i<st->buttoninfos;i++)
					{
						MoveToEx(hdc,st->buttoninfo[i].x1,st->buttoninfo[i].y1+st->dlgheight,NULL);
						LineTo(hdc,st->buttoninfo[i].x1+st->buttoninfo[i].x2,st->buttoninfo[i].y1+st->dlgheight);
						LineTo(hdc,st->buttoninfo[i].x1+st->buttoninfo[i].x2,st->buttoninfo[i].y1+st->buttoninfo[i].y2+st->dlgheight);
						LineTo(hdc,st->buttoninfo[i].x1,st->buttoninfo[i].y1+st->buttoninfo[i].y2+st->dlgheight);
						LineTo(hdc,st->buttoninfo[i].x1,st->buttoninfo[i].y1+st->dlgheight);
						if (i==st->actbuttoninfo)
							Rectangle(hdc,st->buttoninfo[i].x1+st->buttoninfo[i].x2/2-5,st->buttoninfo[i].y1+st->buttoninfo[i].y2/2-5+st->dlgheight,st->buttoninfo[i].x1+st->buttoninfo[i].x2/2+5,st->buttoninfo[i].y1+st->buttoninfo[i].y2/2+5+st->dlgheight);
					}
					DeleteObject(tpen);
					tpen    = CreatePen (PS_SOLID,3,240);
					SelectObject (hdc, tpen);
					SelectObject (hdc, DRAW.brush_blue);
					for (i=0;i<st->sliderinfos;i++)
					{
						MoveToEx(hdc,st->sliderinfo[i].x1,st->sliderinfo[i].y1+st->dlgheight,NULL);
						LineTo(hdc,st->sliderinfo[i].x1+st->sliderinfo[i].x2,st->sliderinfo[i].y1+st->dlgheight);
						LineTo(hdc,st->sliderinfo[i].x1+st->sliderinfo[i].x2,st->sliderinfo[i].y1+st->sliderinfo[i].y2+st->dlgheight);
						LineTo(hdc,st->sliderinfo[i].x1,st->sliderinfo[i].y1+st->sliderinfo[i].y2+st->dlgheight);
						LineTo(hdc,st->sliderinfo[i].x1,st->sliderinfo[i].y1+st->dlgheight);
						if (i==st->actsliderinfo)
							Rectangle(hdc,st->sliderinfo[i].x1+st->sliderinfo[i].x2/2-5,st->sliderinfo[i].y1+st->sliderinfo[i].y2/2-5+st->dlgheight,st->sliderinfo[i].x1+st->sliderinfo[i].x2/2+5,st->sliderinfo[i].y1+st->sliderinfo[i].y2/2+5+st->dlgheight);
					}
					DeleteObject(tpen);
					SelectObject(hdcMem, hbmOld);
					DeleteDC(hdcMem);

				}

				EndPaint(hDlg, &ps );
			}
			break;

				/*
				
				DeleteObject(tpen);

				tpen = CreatePen (PS_SOLID,1,RGB(100,0,0));
				SelectObject (hdc, tpen);

				DeleteObject(tpen);

				SelectObject(hdc, DRAW.scaleFont);
				strcpy(sztemp,st->bmpfilename); 
				ExtTextOut( hdc, rect.left+2,rect.bottom-15, 0, &rect,sztemp, strlen(sztemp), NULL ) ;
*/
		
		case WM_SIZE:
		case WM_MOVE:  InvalidateRect(hDlg,NULL,TRUE);update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}

#endif


