/* -----------------------------------------------------------------------------
   BrainBay  -  OpenSource Biofeedback Software

  MODULE: DRAW.CPP: this Module provides global accessible Drawing - Functions.

  init_draw: Creates Pens and Brushes, and a font for GDI-use
  draw_objects: draws the current objects and object-links to the Main-Window
  LoadBMP: load a bitmap from a file to a openGL-surface, using SDL-functions

  LoadGLTextures: Converts a bitmap to a texture for GL-use
  Size_GL:  updates the OGL-viewport when a resize has occurred

  ->OGL-drawing is currently used for the FFT-Specra-Displays and the Animation Window

  Contributors:  many thanks go to Jeff Molofee (NeHe) for his great OGL-tutorial
				 Web Site: nehe.gamedev.net

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.


 --------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_evaluator.h"
#include "ob_compare.h"



#define PARTICLEBITMAP "particle.bmp"


GLuint	base;						// Base Display List For The Font Set
GLuint	texture[1]={0};					// Storage For Our Particle Texture
int SX,SY;


void init_draw(void)
{
    HDC hdc; 
	DRAW.pen_white  = CreatePen (PS_SOLID,1,PALETTERGB(255,255,255));
	DRAW.pen_blue   = CreatePen (PS_SOLID,1,PALETTERGB(0,0,128));
	DRAW.pen_ltblue   = CreatePen (PS_SOLID,1,PALETTERGB(100,100,150));
	DRAW.pen_red    = CreatePen (PS_SOLID,1,PALETTERGB(150,0,0));
	DRAW.brush_blue = CreateSolidBrush(PALETTERGB(0,0,100));
	DRAW.brush_white = CreateSolidBrush(PALETTERGB(255,255,255));
	DRAW.brush_orange = CreateSolidBrush(PALETTERGB(255,165,0));
	DRAW.brush_ltorange = CreateSolidBrush(PALETTERGB(255,220,128));
	DRAW.brush_yellow = CreateSolidBrush(PALETTERGB(250,250,0));
	DRAW.brush_ltgreen = CreateSolidBrush(PALETTERGB(0,128,128));
	DRAW.particles=0;


    hdc = GetDC(NULL);
	if (GLOBAL.os_version==1)  DRAW.scaleFontHeight = -MulDiv(7, GetDeviceCaps(hdc, LOGPIXELSY), 90);
	else DRAW.scaleFontHeight = -MulDiv(7, GetDeviceCaps(hdc, LOGPIXELSY), 75);

    ReleaseDC(NULL, hdc);
    if (!(DRAW.scaleFont = CreateFont(DRAW.scaleFontHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
        report_error("Font creation failed!");
	if (!(DRAW.mediumFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
        report_error("Font creation failed!");
}

void draw_object(HDC hdc, WORD t)
{
		SelectObject (hdc, DRAW.pen_red);
	
		if (objects[t]==actobject) 
		{
			SelectObject (hdc, DRAW.brush_ltorange);
			Rectangle(hdc, SX+objects[t]->xPos-6,SY+objects[t]->yPos-6,
				SX+objects[t]->xPos+6+objects[t]->width,SY+objects[t]->yPos+6+objects[t]->height);
		}
		SelectObject (hdc, DRAW.pen_blue);
		SelectObject (hdc, DRAW.brush_ltgreen);
		RoundRect(hdc, SX+objects[t]->xPos, SY+objects[t]->yPos,
                    SX+objects[t]->xPos+objects[t]->width,SY+objects[t]->yPos+objects[t]->height, 15, 15); 

		SelectObject (hdc, DRAW.brush_blue);
		Rectangle(hdc, SX+objects[t]->xPos,SY+objects[t]->yPos,
			SX+objects[t]->xPos+objects[t]->width,SY+objects[t]->yPos+15);
}


void draw_connections(HDC hdc, WORD t)
{
	int i,k;
	
		SelectObject (hdc, DRAW.pen_red);
		for (i=0;objects[t]->out[i].from_port!=-1;i++)
		{
			if (objects[t]->outports > objects[t]->out[i].from_port) {
				MoveToEx(hdc, SX + objects[t]->xPos + objects[t]->width - 4, SY + objects[t]->yPos + CON_START + objects[t]->out[i].from_port * CON_HEIGHT, NULL);
				k = objects[t]->out[i].to_object;
				if ((GLOBAL.objects > k))
				{
					if (objects[t]->out[i].to_port != -1)
						LineTo(hdc, SX + objects[k]->xPos + 4, SY + objects[k]->yPos + CON_START + objects[t]->out[i].to_port * CON_HEIGHT);
					else LineTo(hdc, SX + GLOBAL.tx, SY + GLOBAL.ty);
				}
				if (&(objects[t]->out[i]) == actconnect)
				{
					MoveToEx(hdc, SX + objects[t]->xPos + objects[t]->width - 4, SY + objects[t]->yPos + CON_START + objects[t]->out[i].from_port * CON_HEIGHT + 1, NULL);
					LineTo(hdc, SX + objects[k]->xPos + 4, SY + objects[k]->yPos + CON_START + objects[t]->out[i].to_port * CON_HEIGHT + 1);

					MoveToEx(hdc, SX + objects[t]->xPos + objects[t]->width - 3, SY + objects[t]->yPos + CON_START + objects[t]->out[i].from_port * CON_HEIGHT, NULL);
					LineTo(hdc, SX + objects[k]->xPos + 5, SY + objects[k]->yPos + CON_START + objects[t]->out[i].to_port * CON_HEIGHT);

					MoveToEx(hdc, SX + objects[t]->xPos + objects[t]->width - 3, SY + objects[t]->yPos + CON_START + objects[t]->out[i].from_port * CON_HEIGHT + 1, NULL);
					LineTo(hdc, SX + objects[k]->xPos + 5, SY + objects[k]->yPos + CON_START + objects[t]->out[i].to_port * CON_HEIGHT + 1);
				}
			}
		}
}

void draw_captions(HDC hdc, WORD t)
{
	int i;
	char szdata[100];

		SetBkColor (hdc, PALETTERGB(0,0,100));
		SetTextColor (hdc, PALETTERGB(255,255,255));
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, DRAW.scaleFont);
		/*
	
        switch (objects[t]->type) 
		{
				case OB_EVAL:
					strncpy(szdata, ((EVALOBJ *) objects[t])->expression,12);
					szdata[12]='.';szdata[13]='.';szdata[14]=0;
					break;
				case OB_COMPARE:
					switch (((COMPAREOBJ *) objects[t])->method)
					{
							case 0: strcpy(szdata, "A>B"); break;
							case 1: strcpy(szdata, "A>=B"); break;
							case 2: strcpy(szdata, "A<B"); break;
							case 3: strcpy(szdata, "A<=B"); break;
							case 4: strcpy(szdata, "A=B"); break;
					}
					break;
				default: strcpy(szdata, objects[t]->tag); 
					break;
		}*/
		strcpy(szdata, objects[t]->tag); 

		ExtTextOut(hdc, SX+objects[t]->xPos+3,SY+objects[t]->yPos+2, 0, NULL,szdata, strlen(szdata), NULL ) ;

//		SetBkColor (hdc, PALETTERGB(0,128,128));
//		SetTextColor (hdc, PALETTERGB(100,255,200));
		SetBkColor(hdc, PALETTERGB(0, 80, 80));
		SetTextColor(hdc, PALETTERGB(150, 255, 220));
		SelectObject (hdc, DRAW.brush_yellow);
		for (i=0;i<objects[t]->inports;i++)
		{
			switch(objects[t]->in_ports[i].in_type){
				case SFLOAT:
					RoundRect(hdc, SX+objects[t]->xPos, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT,
					SX+objects[t]->xPos+CON_MAGNETIC, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT+CON_MAGNETIC,
					10, 10);
					break;
				case MFLOAT:
					Rectangle(hdc, SX+objects[t]->xPos, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT,
					SX+objects[t]->xPos+CON_MAGNETIC, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT+CON_MAGNETIC);
					break;
			}
		   if (!objects[t]->in_ports[i].in_name[0]) wsprintf(szdata,"%d",i+1); else strcpy(szdata,objects[t]->in_ports[i].in_name);
		   ExtTextOut(hdc, SX+objects[t]->xPos+12,SY+objects[t]->yPos-4+CON_START+i*CON_HEIGHT, 0, NULL,szdata, strlen(szdata), NULL ) ;
		  
		}
		SelectObject (hdc, DRAW.brush_orange);
		for (i=0;i<objects[t]->outports;i++)
		{
			switch(objects[t]->out_ports[i].out_type){
				case SFLOAT:
					RoundRect(hdc, SX+objects[t]->xPos+objects[t]->width-CON_MAGNETIC, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT,
					SX+objects[t]->xPos+objects[t]->width, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT+CON_MAGNETIC,
					10, 10); 
					break;
				case MFLOAT:
					Rectangle(hdc, SX+objects[t]->xPos+objects[t]->width-CON_MAGNETIC, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT,
					SX+objects[t]->xPos+objects[t]->width, SY+objects[t]->yPos+CON_START-4+i*CON_HEIGHT+CON_MAGNETIC);					
					break;
			}
		    if (!objects[t]->out_ports[i].out_name[0]) wsprintf(szdata,"%d",i+1); else strcpy(szdata,objects[t]->out_ports[i].out_name);
			SetTextAlign(hdc,TA_RIGHT);
		    ExtTextOut(hdc, SX+objects[t]->xPos+objects[t]->width-CON_MAGNETIC-4, SY+objects[t]->yPos-4+CON_START+i*CON_HEIGHT, 0, NULL,szdata, strlen(szdata), NULL ) ;
			SetTextAlign(hdc,TA_LEFT); 
		}
}

void draw_objects(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc;

	WORD t;
	int a=-1;

	SCROLLINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
    GetScrollInfo(ghWndDesign, SB_HORZ, &si);
    SX=-si.nPos;
    GetScrollInfo(ghWndDesign, SB_VERT, &si);
    SY=-si.nPos;

	hdc = BeginPaint (hWnd, &ps);

	for (t=0;t<GLOBAL.objects;t++)
	{
		draw_object(hdc,t);
//		draw_connections(hdc,t);
		draw_captions(hdc,t);
		if (objects[t]==actobject) a=t;
	}
//	for (t=0;t<GLOBAL.objects;t++)	draw_captions(hdc,t);
	if (a!=-1) { draw_object(hdc,a); draw_connections(hdc,a); draw_captions(hdc,a); }
	for (t=0;t<GLOBAL.objects;t++)	draw_connections(hdc,t);

	EndPaint( hWnd, &ps );

}



SDL_Surface *LoadBMP(char *filename)
{
	Uint8 *rowhi, *rowlo;
	Uint8 *tmpbuf, tmpch;
	SDL_Surface *image;
	int i, j;

	image = SDL_LoadBMP(filename);
	if ( image == NULL ) {
		fprintf(stderr, "Unable to load %s: %s\n", filename, SDL_GetError());
		return(NULL);
	}

	/* GL surfaces are upsidedown and RGB, not BGR :-) */
	tmpbuf = (Uint8 *)malloc(image->pitch);
	if ( tmpbuf == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(NULL);
	}
	rowhi = (Uint8 *)image->pixels;
	rowlo = rowhi + (image->h * image->pitch) - image->pitch;
	for ( i=0; i<image->h/2; ++i ) {
		for ( j=0; j<image->w; ++j ) {
			tmpch = rowhi[j*3];
			rowhi[j*3] = rowhi[j*3+2];
			rowhi[j*3+2] = tmpch;
			tmpch = rowlo[j*3];
			rowlo[j*3] = rowlo[j*3+2];
			rowlo[j*3+2] = tmpch;
		}
		memcpy(tmpbuf, rowhi, image->pitch);
		memcpy(rowhi, rowlo, image->pitch);
		memcpy(rowlo, tmpbuf, image->pitch);
		rowhi += image->pitch;
		rowlo -= image->pitch;
	}
	free(tmpbuf);
	return(image);
}

int LoadGLTextures()									// Load Bitmap And Convert To A Texture
{
	char particlefilename[200];
    int Status=FALSE;								// Status Indicator
    SDL_Surface *TextureImage[1];				// Create Storage Space For The Textures
  

	memset(TextureImage,0,sizeof(void *)*1);		// Set The Pointer To NULL
	strcpy(particlefilename,GLOBAL.resourcepath);
	strcat(particlefilename,"GRAPHICS\\");
	strcat(particlefilename,PARTICLEBITMAP);
    if (TextureImage[0]=LoadBMP(particlefilename))	// Load Particle Texture
    {
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[0]);				// Create One Texture
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->w, TextureImage[0]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->pixels);
    }

    if (TextureImage[0])							// If Texture Exists
	{
		SDL_FreeSurface(TextureImage[0]);
	}
    return Status;		
}


//
// Called when a GL window is resized. Resizes the OpenGL
// viewport.
//
void Size_GL(HWND hWnd, HGLRC m_hRC, int asp)
{
			
GLfloat fFovy  = 30.0f; // Field-of-view
GLfloat fZNear = 0.1f;  // Near clipping plane
GLfloat fZFar  = 10000.0f;  // Far clipping plane
RECT rv;
GLfloat fAspect;
HDC hDC;

	if(hWnd==NULL) return;

	hDC = GetDC(hWnd);
	wglMakeCurrent(hDC, m_hRC);
	
	// Calculate viewport aspect
	GetClientRect(hWnd, &rv);
	fAspect = (GLfloat)(rv.right-rv.left) / (GLfloat)(rv.bottom-rv.top);
	if (asp==1) fAspect = 1.0;

	// Define viewport
	glViewport(rv.left, rv.top, rv.right-rv.left, rv.bottom-rv.top);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fFovy, fAspect, fZNear, fZFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();									// Reset The Modelview Matrix

	wglMakeCurrent(0, 0);
	ReleaseDC(hWnd, hDC);
}


//
// Shutdown_GL()
// Called when a GL window is destroyed. Shuts down OpenGL
//
void Shutdown_GL(HGLRC m_hRC)
{
	wglDeleteContext(m_hRC);
}



GLvoid KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(base, 96);							// Delete All 96 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}


 

