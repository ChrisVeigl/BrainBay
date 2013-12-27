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

// TransLabel.h: interface for the TransLabel class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _TRANSLABEL_H_
#define _TRANSLABEL_H_

#include "KWindow.h"

class TransLabel : public KWindow  
{
public:
	int min,max,value,type;
	char * m_BtnName;
	HBRUSH m_Brush;
	void SetFont(LPCTSTR lpszFont, int nFontSize, ULONG nColor);
	void SetWindowText(LPCTSTR lpszText);
	virtual HWND CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, 
		LPCTSTR lpszName, DWORD dwStyle,  int x, int y, int nWidth, 
		int nHeight, HWND hParent, HINSTANCE, LPCTSTR lpszFont, int nFontSize, ULONG fColor);
	virtual void OnDraw(HDC hDC);
	TransLabel();
	virtual ~TransLabel();

protected:
	ULONG m_FontColor;
	HFONT m_Font;
};

#endif // _TRANSLABEL_H_
