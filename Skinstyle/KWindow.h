/* SkinStyle - Win32 Skinning Library 
 * Author: John Roark <jroark@cs.usfca.edu>
 */

/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// KWindow.h: interface for the KWindow class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _KWINDOW_H_
#define _KWINDOW_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

class KWindow  
{
public:
	BOOL UpdateWindow(void) const { return ::UpdateWindow(m_hWnd); }
	virtual WPARAM MessageLoop(void);
	BOOL ShowWindow(int nCmdShow) const { return ::ShowWindow(m_hWnd, nCmdShow); }
	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);
	virtual HWND CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParent, HMENU hMenu, HINSTANCE hInst);
	
	virtual void GetWndClassEx(WNDCLASSEX &wc);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) { }
	virtual void OnDraw(HDC hDC) { }
	KWindow() 
	{ 
		m_hWnd = NULL; 
		m_Decon = FALSE; 
		m_ClassName = NULL;
	}
	virtual ~KWindow() 
	{ 
		m_Decon = TRUE;
		::UnregisterClass(m_ClassName, m_hInst);

		if(m_ClassName) 
			::GlobalFree(m_ClassName);
	}

	HWND m_hWnd;
protected:
	HINSTANCE m_hInst;
	char *m_ClassName;
	BOOL m_Decon;		// stupid hack!
						// if this true it means we are 
						// being deconstructed
						// therefore the WndProc is no longer valid
};

#endif // _KWINDOW_H_
