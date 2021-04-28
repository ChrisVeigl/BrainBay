/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_AVI.H:  declarations for the AVI-Player-Object
  Author: Chris Veigl

  This Object can open a standard avi-file, grab a frame 
  and display it in a sizeable window. the frame number is given to the object's 
  input port. Sound of the AVI-File is not processed.

  Inspired by   Jeff Molofee's Lesson 35: Playing AVI-Files with OpenGl
  http://nehe.gamedev.net

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include <vfw.h>	


HWND			create_AVI_Window(int left,int right, int top, int bottom, HGLRC * GLRC_FFT);
LRESULT CALLBACK AVIDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class AVIOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten,i;
  public: 
  	float    input;
	int      top,left,right,bottom;
	long	 frame,old_frame,lastframe;				// Frame Counter
	char     avifile[256];

	int					width;										// Video Width
	int					height;										// Video Height
	char				*pdata;										// Pointer To Texture Data
	int					mpf;										// Will Hold Rough Milliseconds Per Frame
	HGLRC    GLRC;													// Handle to OPENGL-RC
	AVISTREAMINFO		psi;										// Pointer To A Structure Containing Stream Info
	PAVISTREAM			pavi;										// Handle To An Open Stream
	PGETFRAME			pgf;										// Pointer To A GetFrame Object	
	BITMAPINFOHEADER	bmih;										// Header Information For DrawDibDraw Decoding

	GLUquadricObj *quadratic;										// Storage For Our Quadratic Objects

	HDRAWDIB hdd;													// Handle For Our Dib
	HBITMAP hBitmap;												// Handle To A Device Dependent Bitmap
	HDC hdc;								// Creates A Compatible Device Context
	unsigned char* data;										// Pointer To Our Resized Image


	AVIOBJ(int num);
	HWND create_AVI_Window(int left,int right, int top, int bottom,HGLRC * GLRC);
	int  OpenAVI(LPCSTR szFile);
	void GrabAVIFrame(int,HDC);									// Grabs A Frame From The Stream
	void CloseAVI(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    void incoming_data(int port, float value) ;
	void work(void);
 	~AVIOBJ();
   
};

