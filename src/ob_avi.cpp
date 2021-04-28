/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_AVI.CPP:  functions for the AVI-Player-Object
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
#include "ob_avi.h"
											


LRESULT CALLBACK AVIDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szFileName[MAX_PATH];
	AVIOBJ * st;
	
	st = (AVIOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_AVI)) return(FALSE);

	switch( message ) 
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_AVIFILE, st->avifile); 
		return TRUE;
        
	case WM_CLOSE:
		 EndDialog(hDlg, LOWORD(wParam));
		break;
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		  case IDC_AVIFILE:
				GetDlgItemText(hDlg, IDC_AVIFILE, st->avifile, 255); 
		   break;
		  case IDC_LOADAVI:
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"MOVIES\\*.avi");
			if (open_file_dlg(hDlg, szFileName, FT_AVI, OPEN_LOAD)) 
			{
			  if (!st->OpenAVI(szFileName))
				report_error("Could not load AVI File");
			  else
			  {
//				reduce_filepath(szFileName,szFileName);
				strcpy(st->avifile,szFileName);
				SetDlgItemText(hDlg, IDC_AVIFILE, st->avifile); 
				if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			  }
			}
			InvalidateRect(hDlg,NULL,FALSE);
			break;

		}
		break;
	
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;

	}
   return FALSE;
}


LRESULT CALLBACK AVIWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int t;
	AVIOBJ * st;
	
	st=NULL;
	
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
	 if (objects[t]!=NULL)
		if (objects[t]->type==OB_AVI)
		{	st=(AVIOBJ *)objects[t];
		    if (st!=NULL) if (st->displayWnd!=hWnd) st=NULL;
		}
		  
	if (st!=NULL)
	switch( message ) 
	{
	
	case WM_DESTROY:
		 Shutdown_GL(st->GLRC);
		 break;
	case WM_KEYDOWN:
			  SendMessage(ghWndMain, message,wParam,lParam);
		break;
	case WM_ACTIVATE:
		if (wParam==WA_CLICKACTIVE)
		{
		  close_toolbox();
		  actobject=st;
//		  actobject->make_dialog();
		  SetActiveWindow(hWnd);
		  SetFocus(hWnd);
		}
		break;

    case WM_COMMAND:
		break;
	case WM_SIZE:
		//	Size_GL(hWnd, st->GLRC,1); 		 
	case WM_MOVE:
			{
  			  WINDOWPLACEMENT  wndpl;
			  GetWindowPlacement(st->displayWnd, &wndpl);

			  if (GLOBAL.locksession) {
				  wndpl.rcNormalPosition.top=st->top;
				  wndpl.rcNormalPosition.left=st->left;
				  wndpl.rcNormalPosition.right=st->right;
				  wndpl.rcNormalPosition.bottom=st->bottom;
				  SetWindowPlacement(st->displayWnd, &wndpl);
 				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE)&~WS_SIZEBOX);
			  }
			  else {
				  st->top=wndpl.rcNormalPosition.top;
				  st->left=wndpl.rcNormalPosition.left;
				  st->right=wndpl.rcNormalPosition.right;
				  st->bottom=wndpl.rcNormalPosition.bottom;
				  SetWindowLong(st->displayWnd, GWL_STYLE, GetWindowLong(st->displayWnd, GWL_STYLE) | WS_SIZEBOX);
			  }
			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
		 
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint(hWnd, &ps);

			if (strcmp(st->avifile,"none"))
			{
				wglMakeCurrent(hDC, st->GLRC);
				//glLoadIdentity ();													// Reset The Modelview Matrix
				//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
				st->GrabAVIFrame(st->frame,hDC);										// Grab A Frame From The AVI
				wglMakeCurrent(0, 0);
			}
			EndPaint(hWnd, &ps);

		}
		break;

	default:
			return DefWindowProc( hWnd, message, wParam, lParam );
	} else 	return DefWindowProc( hWnd, message, wParam, lParam );
   return 0;
}



//
//  Object Implementation
//


AVIOBJ::AVIOBJ(int num) : BASE_CL()	
	  {
	    inports  = 1;
		outports = 0;
		strcpy(in_ports[0].in_name,"in");
		
		left=50;right=300;top=330;bottom=580;
		input=0;data=0;
		frame=0;lastframe=0;
		strcpy(avifile,"none");
		displayWnd=create_AVI_Window(left,right,top,bottom,&GLRC);
 	    
		strcpy(avifile,"none");
	  }


HWND AVIOBJ::create_AVI_Window(int left,int right, int top, int bottom,HGLRC * GLRC)
{
	HWND Wnd;
	char temp[50];
	HDC hDC;
	HGLRC m_hRC;
	PIXELFORMATDESCRIPTOR pfd;
	int nPixelFormat;	

	wsprintf(temp, "Avi - Movie");
	if (!(Wnd= CreateWindow( "AVIClass", temp, WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME | WS_VISIBLE , left, top,right-left,bottom-top, ghWndMain, NULL, hInst, NULL)))
	{	report_error("Can't Create AVI Window");
		return(0);
	}

	memset(&pfd, 0, sizeof(pfd));    
	pfd.nSize      = sizeof(pfd);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	

	// Set pixel format
	
	hDC = GetDC(Wnd);
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, nPixelFormat, &pfd);
	
	m_hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, m_hRC);
	hdc = hDC;
    hdd = DrawDibOpen();										// Grab A Device Context For Our Dib
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						// Black Background
							// Select Smooth Shading
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate


	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity ();													// Reset The Projection Matrix
	gluPerspective (45.0f, (GLfloat)(width)/(GLfloat)(height),			// Calculate The Aspect Ratio Of The Window
					1.0f, 100.0f);		
	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
    *GLRC=m_hRC;

	ReleaseDC(Wnd, hDC);

	ShowWindow(Wnd,1);
	UpdateWindow(Wnd);
	InvalidateRect(Wnd,NULL,TRUE);
	return(Wnd);
}

int AVIOBJ::OpenAVI(LPCSTR szFile)										// Opens An AVI File (szFile)
{
	
	AVIFileInit();												// Opens The AVIFile Library
	pgf=NULL;
	
	if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
		return(0);
	
	AVIStreamInfo(pavi, &psi, sizeof(psi));						// Reads Information About The Stream Into psi
	width=psi.rcFrame.right-psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	height=psi.rcFrame.bottom-psi.rcFrame.top;					// Height Is Bottom Of Frame Minus Top

	lastframe=AVIStreamLength(pavi);							// The Last Frame Of The Stream

	mpf=AVIStreamSampleToTime(pavi,lastframe)/lastframe;		// Calculate Rough Milliseconds Per Frame

	bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	bmih.biPlanes = 1;											// Bitplanes	
	bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
	bmih.biWidth = width;											// Width We Want 
	bmih.biHeight = height;										// Height We Want
	bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	hBitmap = CreateDIBSection (hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, 0);
	SelectObject (hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	pgf=AVIStreamGetFrameOpen(pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode
	if (pgf==NULL) return(0);

    return(1);
}

void AVIOBJ::GrabAVIFrame(int frame,HDC nhdc)									// Grabs A Frame From The Stream
{
	LPBITMAPINFOHEADER lpbi;									// Holds The Bitmap Header Information

	if (pgf)
	{
		lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	// Grab Data From The AVI Stream
		if (lpbi) 
		{
			pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame
			DrawDibDraw (hdd, nhdc, 0, 0, right-left,bottom-top, lpbi, pdata, 0, 0, width, height, 0);
		}
	}
			
}

void AVIOBJ::CloseAVI(void)												// Properly Closes The Avi File
{
	
	DeleteObject(hBitmap);										// Delete The Device Dependent Bitmap Object
	if (hdd) DrawDibClose(hdd);											// Closes The DrawDib Device Context
	if (pgf)
	{
		AVIStreamGetFrameClose(pgf);								// Deallocates The GetFrame Resources
		AVIStreamRelease(pavi);										// Release The Stream
		AVIFileExit();												// Release The File
	}
	
}

										

void AVIOBJ::make_dialog(void)
{
		 display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_AVIPROPBOX, ghWndStatusbox, (DLGPROC)AVIDlgHandler));
}

void AVIOBJ::load(HANDLE hFile) 
{	
		load_object_basics(this);
		load_property("avi-file",P_STRING,avifile);
		load_property("wnd-top",P_INT,&top);
		load_property("wnd-bottom",P_INT,&bottom);
		load_property("wnd-left",P_INT,&left);
		load_property("wnd-right",P_INT,&right);

		if (strcmp(avifile,"none"))
		{
			char szFileName[MAX_PATH] = "";
			strcpy(szFileName,avifile);
			if (!OpenAVI (szFileName)) 
			{ 
				strcpy(szFileName,"could not Open AVI-File: ");
				strcat(szFileName,avifile);
				report_error ( szFileName); 
			}
		}
		MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
		if (GLOBAL.locksession) {
	 		SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
			//SetWindowLong(displayWnd, GWL_STYLE, 0);
		} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
}

void AVIOBJ::save(HANDLE hFile) 
{	
		save_object_basics(hFile,this);
		save_property(hFile,"avi-file",P_STRING,avifile);
		save_property(hFile,"wnd-top",P_INT,&top);
		save_property(hFile,"wnd-bottom",P_INT,&bottom);
		save_property(hFile,"wnd-left",P_INT,&left);
		save_property(hFile,"wnd-right",P_INT,&right);

}

void AVIOBJ::incoming_data(int port, float value) 
{	
		   input=value; 
}

void AVIOBJ::work(void) 
{
	if (GLOBAL.fly) return;
	frame=(long) (lastframe*((input-in_ports[0].in_min)/(in_ports[0].in_max-in_ports[0].in_min)));	// Calculate The Current Frame
	if (frame<0) frame=0;
	if (frame>=lastframe)				// Are We At Or Past The Last Frame?
			frame=lastframe;				// Reset The Frame Back To Zero (Start Of Video)

	if (frame!=old_frame )
	{
		old_frame=frame;
		InvalidateRect(displayWnd,NULL,FALSE);	
	}
	
}

AVIOBJ::~AVIOBJ()
	  {

  	    CloseAVI();													// Close The AVI File
		DestroyWindow(displayWnd); displayWnd=NULL;
	  }  



