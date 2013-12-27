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

// SkinButton.h: interface for the SkinButton class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _SKINBUTTON_H_
#define _SKINBUTTON_H_

#include "SkinObject.h"
#include "Bitmap.h"

#define SB_LBUTTONDOWN	(WM_USER + 1)
#define SB_RBUTTONDOWN	(WM_USER + 2)

#define SB_LBUTTONUP	(WM_USER + 3)
#define SB_RBUTTONUP	(WM_USER + 4)

#define SB_MOUSEENTER	(WM_USER + 5)
#define SB_MOUSEEXIT	(WM_USER + 6)

#define SBS_MOUSEOVER	0x00000800

class SkinButton : public SkinObject  
{
public:
	int slider;
	int buttonmode;
	char * m_BtnName;

	void Enable(BOOL set);
	virtual BOOL OnMouseMove(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnRButtonUp(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnLButtonUp(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnRButtonDown(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnLButtonDown(WPARAM wParam, LPARAM lParam);

	SkinButton()
	{
		m_BtnName = NULL;
		_left_button_down = false;
		_mouse_over = false;
		_has_focus = TRUE;
		m_BeingTracked = FALSE;
		slider=0;
		buttonmode=0;
	}
	virtual ~SkinButton() 
	{
		Free();
	}

	void Free()
	{
		m_Clicked.Free();
		m_Over.Free();
		m_Disabled.Free();

		if(m_BtnName != NULL)
			::GlobalFree(m_BtnName);
	}

	virtual HWND CreateEx(LPCTSTR lpszName, DWORD dwStyle, int x, int y, HWND hParent, 
		HINSTANCE hInst, LPCTSTR lpszMask, LPCTSTR lpszMain, LPCTSTR lpszClick, LPCTSTR lpszOver, LPCTSTR lpszDisabled);

	HWND CreateEx(LPCTSTR lpszName, DWORD dwStyle, int x, int y, HWND hParent, 
		HINSTANCE hInst, Bitmap Mask, Bitmap Main, Bitmap Click, Bitmap Over, Bitmap Disabled);

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnDraw(HDC hDC);

//protected:
	Bitmap m_Over;
	Bitmap m_Clicked;
	Bitmap m_Disabled;

	HWND m_hWndParent;

	bool _left_button_down;
	bool _mouse_over;
	BOOL _has_focus;


private:
	BOOL m_BeingTracked;
};

#endif // _SKINBUTTON_H_
