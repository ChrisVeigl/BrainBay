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

// SkinDialog.h: interface for the SkinDialog class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _SKINDIALOG_H_
#define _SKINDIALOG_H_

#include "SkinObject.h"
#include "SkinButton.h"
#include "IniFile.h"
#include "TransLabel.h"
#include "KToolTip.h"
#include "DynArray.h"
#include "Bitmap.h"

class SkinDialog : public SkinObject  
{
public:
	CDynArray< SkinButton > m_Buttons;
	CDynArray< TransLabel > m_Labels;
	CDynArray< KToolTip > m_ToolTips;
 
	
	virtual void OnDraw(HDC hDC);
	void Free();
	virtual HWND CreateEx(LPCTSTR lpszName, int x, int y, HINSTANCE hInst, LPCTSTR lpszSkinFile);
	virtual BOOL OnButtonPressed(char *ButtonName) { return TRUE; }
	virtual BOOL OnMouseEnter(char *ButtonName) { return TRUE; }
	virtual BOOL OnMouseExit(char *ButtonName) { return TRUE; }

	SkinDialog();
	virtual ~SkinDialog();

	
	void SetSticky(bool sticky) { _sticky = sticky; }
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	BOOL m_Loading;
	char m_SkinPath[MAX_PATH];
	bool _has_focus;
	bool _sticky;

protected:
	void LoadStatic();
	void LoadButtons();
	void LoadSliders();
	Bitmap m_Disabled;
	Bitmap m_Over;
	Bitmap m_Selected;

	void LoadSkin(char *SkinFile);

	IniFile m_SkinFile;


};

#endif // _SKINDIALOG_H_
