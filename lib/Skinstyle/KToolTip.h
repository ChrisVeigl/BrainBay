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

// KToolTip.h: interface for the KToolTip class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _KTOOLTIP_H_
#define _KTOOLTIP_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>

class KToolTip  
{
public:
	HWND CreateEx(HWND hwnd, HINSTANCE hInst, LPCTSTR lpszTip);
	KToolTip() { }
	KToolTip(HWND hwnd, HINSTANCE hInst, LPCTSTR lpszTip);
	virtual ~KToolTip() 
	{
		SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	}

private:
	HWND m_hWnd;
	TOOLINFO ti;
};

#endif // _KTOOLTIP_H_
