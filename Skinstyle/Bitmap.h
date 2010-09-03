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

// Bitmap.h: interface for the Bitmap class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _BITMAP_H_
#define _BITMAP_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

class Bitmap  
{
public:
	Bitmap () 
		: _hBitmap (0)
	{}
	Bitmap (HBITMAP hBitmap)
		: _hBitmap (hBitmap)
	{}
	Bitmap (Bitmap & bmp)
		: _hBitmap (bmp.Release ())
	{}
	void operator = (Bitmap & bmp)
	{
		if (bmp._hBitmap != _hBitmap)
		{
			Free ();
			_hBitmap = bmp.Release ();
		}
	}
	HBITMAP Release ()
	{
		HBITMAP h = _hBitmap;
		_hBitmap = 0;
		return h;
	}
	~Bitmap () 
	{
		Free ();
	}
	operator HBITMAP () { return _hBitmap; }

	BOOL Load (HINSTANCE hInst, char const * resName);
	BOOL Load (HINSTANCE hInst, int id);
	BOOL Load (char * path);

	void GetSize (int & width, int & height);

	Bitmap (HDC canvas, int dx, int dy)
		: _hBitmap (0)
	{
		CreateCompatible (canvas, dx, dy);
	}

	void CreateCompatible (HDC canvas, int width, int height)
	{
		Free ();
		_hBitmap = ::CreateCompatibleBitmap (canvas, width, height);
	}
	
	void Free ()
	{
		if (_hBitmap)
			::DeleteObject (_hBitmap);
	}
protected:
	HBITMAP _hBitmap;
};

#endif // _BITMAP_H_
