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

// SkinObject.h: interface for the SkinObject class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _SKINOBJECT_H_
#define _SKINOBJECT_H_

#include "KWindow.h"
#include "Bitmap.h"

class SkinObject : public KWindow  
{
public:
	SkinObject() { }
	virtual ~SkinObject() 
	{
		m_Main.Free();
		m_Mask.Free();
	}

	virtual HWND CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, 
		int x, int y, HWND hParent, HINSTANCE hInst, LPCTSTR lpszMask, LPCTSTR lpszMain);

	HWND CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, 
		int x, int y, HWND hParent, HINSTANCE hInst, Bitmap Mask, Bitmap Main);

	virtual void OnDraw(HDC hDC);

protected:
	Bitmap m_Main;
	Bitmap m_Mask;

	int m_Height;
	int m_Width;
};

#endif // _SKINOBJECT_H_
