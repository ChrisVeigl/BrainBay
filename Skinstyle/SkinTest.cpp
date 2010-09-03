#include <windows.h>
#include <winuser.h>

#include "..\SkinObject.h"
#include "..\SkinDialog.h"
#include "..\SkinButton.h"
#include "..\TransLabel.h"
#include "..\DynArray.h"
#include "..\KToolTip.h"

#include "resource.h" 


int c=10;

char *GetFile(HWND hwnd)
{
	OPENFILENAME ofn;
	char *file = NULL;
		
	file = (char *)malloc(MAX_PATH); 
	memset(file, 0, MAX_PATH);
	
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Skin Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "ini";

	GetOpenFileName(&ofn);

	return file;
}

class KSkinTest : public SkinDialog
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

};



BOOL KSkinTest::OnButtonPressed(char *ButtonName)
{
	unsigned int i;
	int actbutton=-1,actslider=0;

	for (i=0;i<m_Buttons.GetSize();i++)
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

	}



	if(strcmp(ButtonName, "BUTTON_EXIT") == 0)
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
	else if(strcmp(ButtonName, "BUTTON_MINIMIZE") == 0)
		::ShowWindow(m_hWnd, SW_MINIMIZE);
	else if(strcmp(ButtonName, "BUTTON_BABEL") == 0)
	{
	//	LoadSkin("c:\\works\\c\\skin\\skintest\\skin2.ini");
/*		char *tmp = GetFile(m_hWnd);
		if(tmp[0] != 0)
		{
			LoadSkin(tmp);
			::GlobalFree(tmp);
		}
		*/

		char tmp[100];
//		 ::MessageBox(m_hWnd, "Status", "Status", MB_OK);
		c--;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);


	}
	else if(strcmp(ButtonName, "BUTTON_STATUS") == 0)
	{
		char tmp[100];
//		 ::MessageBox(m_hWnd, "Status", "Status", MB_OK);
		c++;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);

		
	}
	return FALSE;
}

BOOL KSkinTest::OnMouseEnter(char *ButtonName)
{
	if(strcmp(ButtonName, "BUTTON_BABEL") == 0)
	{

		char tmp[100];

		c-=10;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);


	}
	else if(strcmp(ButtonName, "BUTTON_STATUS") == 0)
	{
		char tmp[100];
		c+=10;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);

		
	}
	return FALSE;
}

BOOL KSkinTest::OnMouseExit(char *ButtonName)
{
	if(strcmp(ButtonName, "BUTTON_BABEL") == 0)
	{

		char tmp[100];

		c-=10;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);


	}
	else if(strcmp(ButtonName, "BUTTON_STATUS") == 0)
	{
		char tmp[100];
		c+=10;
		wsprintf(tmp,"%d",c);
		m_Labels.operator [](0)->SetWindowText(tmp);

		
	}
	return FALSE;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmd, int nShow)
{

KSkinTest win;

	HWND hParent;
	HICON hIconSmall = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN));
	HICON hIconBig = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN));

	//char *SkinFile = GetFile(NULL);
	char *SkinFile = "c:\\works\\c\\skin\\skintest\\skin.ini";

	if(SkinFile[0] == 0) 
	{
		GlobalFree(SkinFile);
		return 0;
	}

	hParent = win.CreateEx("SkinTest", CW_USEDEFAULT, CW_USEDEFAULT, 
		hInst, SkinFile);

	GlobalFree(SkinFile);
	
	win.SetSticky(true);


	SendMessage( 
		(HWND) hParent,			// handle to destination window 
		WM_SETICON,				// message to send
		(WPARAM) ICON_BIG,		// icon type
		(LPARAM) hIconBig		// handle to icon (HICON)
	);

	SendMessage( 
		(HWND) hParent,			// handle to destination window 
		WM_SETICON,				// message to send
		(WPARAM) ICON_SMALL,	// icon type
		(LPARAM) hIconSmall		// handle to icon (HICON)
	);

	win.ShowWindow(nShow);
	win.UpdateWindow();

	return win.MessageLoop();
}
