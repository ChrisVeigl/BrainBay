/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_FFT.CPP:  contains functions for the Fourier-Transform-Object

  The FFT-Object has its own window, using openGl-Drawing to display spectral 
  information of a data-stream. 
  A rectangular or Hanning - windowing function can be used.
  
  To do: 3-d view. selectable bins/sampling - rate, ..

  IsPowerOfTwo, NumberOfBitsNeeded, 
  ReverseBits, Index_to_Frequency: fft-float - assisting functions
  fft_float:  does the fast fourier transform of a given time-buffer
  Create_FFT_GL: initializes an openGl- Drawing context, creates a drawable font
  create_FFT_window: creates the FFT-drawing window for displaying the spectral data
  BK_Color: set the background color
  Draw_FFT_GL: draws the current magnitude/frequency - display 
  FFTDlgHandler: processes the events for the FFT-toolbox window
  FFTWndHandler: processes the events for the FFT-drawing window

     fft-routine
     original source code by Don Cross <dcross@intersrv.com>
     http://www.intersrv.com/~dcross/fft.html
     definitions for doing Fourier transforms and inverse Fourier transforms.
     Windows oriented changes by Murphy McCauley

  The number of samples must be a power of two to do the
  recursive decomposition of the FFT algorithm.
  See Chapter 12 of "Numerical Recipes in FORTRAN" by
  Press, Teukolsky, Vetterling, and Flannery,
  Cambridge University Press.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_fft.h"
//#include <gl\glaux.h>		// Header File For The Glaux Library

extern GLuint	base;				// Base Display List For The Font Set

GLvoid glPrint(const char *, ...);

GLfloat	xrot;				// X Rotation ( NEW )
GLfloat	yrot;				// Y Rotation ( NEW )
GLfloat	zrot;				// Z Rotation ( NEW )


int IsPowerOfTwo ( unsigned x )
{
    if ( x < 2 )    return FALSE;
    if ( x & (x-1) )        // Thanks to 'byang' for this cute trick!
        return FALSE;
    return TRUE;
}


unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo )
{
    unsigned i;
    for ( i=0; ; i++ )
      if ( PowerOfTwo & (1 << i) )   return i;
}


unsigned ReverseBits ( unsigned index, unsigned NumBits )
{
    unsigned i, rev;
    for ( i=rev=0; i < NumBits; i++ )
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }
    return rev;
}


double WINAPI Index_to_frequency ( unsigned NumSamples, unsigned Index )
{
    if ( Index >= NumSamples )
        return 0.0;
    else if ( Index <= NumSamples/2 )
        return (double)Index / (double)NumSamples;

    return -(double)(NumSamples-Index) / (double)NumSamples;
}



void WINAPI fft_float (int chnBufPos, float * buffer, int window, float * fftbands, int bins)
{
    unsigned NumBits;					// Number of bits needed to store indices 
    unsigned i, j, k, n, x;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = 2.0 * DDC_PI;
	double tr, ti;						// temp real, temp imaginary 
	double  RealOut[FFT_BUFFERLEN],
		    ImagOut[FFT_BUFFERLEN];
	unsigned int NUM_FFT_SAMPLES=bins*2;



    NumBits = NumberOfBitsNeeded ( NUM_FFT_SAMPLES );

    //  data copy and bit-reversal ordering 
	x=chnBufPos;
    for ( i=0; i < NUM_FFT_SAMPLES; i++ )
    {		
        j = ReverseBits ( i, NumBits );
        ImagOut[j] = 0.0;
		RealOut[j] = buffer[x];// - 512.0;
		
		if (window==1) RealOut[j] *= (0.5 + 0.5*cos(2*DDC_PI*(i-NUM_FFT_SAMPLES/2)/NUM_FFT_SAMPLES));
		
		if (x==0) x=FFT_BUFFERLEN; 
		x--; 
    }

    
    //   the FFT itself
   
    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NUM_FFT_SAMPLES; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        double ar[3], ai[3];

        for ( i=0; i < NUM_FFT_SAMPLES; i += BlockSize )
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar[0] = w*ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w*ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + BlockEnd;
                tr = ar[0]*RealOut[k] - ai[0]*ImagOut[k];
                ti = ar[0]*ImagOut[k] + ai[0]*RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }

        BlockEnd = BlockSize;
    }

   for (i=0;i<(unsigned int)bins;i++)
   {
      fftbands[i] = (float) sqrt(ImagOut[i]*ImagOut[i]+RealOut[i]*RealOut[i])/bins;
   }

}






void Draw_FFT_GL(FFTOBJ * st)  // Called each time the FFT-window has to be drawn
{
	double t,step,colstep,ec,every;
	GLfloat x1,y1,t_x,dx;
	GLfloat drawstep,textxpos;
	int NUM_FFT_SAMPLES=fft_bin_values[st->binselect]*2;
    textxpos=-0.14f;
					
	glLoadIdentity(); // Load identity matrix

	t_x=0.0;
  	switch (st->align) 
	{
	  case 0:
			if (st->captions) 
			{ glTranslatef(-0.9f, 0.0f, -4.0f);
			  t_x=0.1f;
			}
			else  glTranslatef(-1.0f, 0.0f, -4.0f);
		break;
	
	  case 1:
			if (st->captions) glTranslated(0, -0.9, -4);	
			else glTranslated(0, -1, -4);	
			glRotatef( 90, 0, 0, -1);
		 	glRotatef(180, 0, 1, 0);
		break;
	  case 2:
		if (st->captions) { glTranslatef(0.8f, 0.0f, -4.0f); textxpos=-0.06f; }
			else glTranslatef(1.0f, 0.0f, -4.0f);
			glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
			glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		break;
	}

	dx=0.016f * (float)st->gain_x/50.0f;
	if (st->captions) dx-=0.002f;
	if (st->kind==1) dx-=0.002f;



	if (st->kind==0) 
	{                  // Bar - Chart

	glClearColor(	
	    (GLclampf)((st->bkcol&0xff))/256.0f,
		(GLclampf)((st->bkcol>>8)&0xff)/256.0f,
		(GLclampf)((st->bkcol>>16)&0xff)/256.0f,
			1.0f); 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
	glDisable (GL_DEPTH_TEST);									// Enable Depth Testing

	step=(double)PACKETSPERSECOND/(double)NUM_FFT_SAMPLES;
	colstep=128.0/PACKETSPERSECOND*2;
	drawstep=((1.0f+(float)st->gain_y/50.0f)/(st->endband-st->startband));

	every=(st->endband-st->startband)/40.0f;
	ec=every;

	for (t=st->startband; t<=st->endband; t+=step)
	{ 
	    y1=(GLfloat)(1.0f-(t-(GLfloat)st->startband)*drawstep);
	    x1=(GLfloat)(st->fftbands[(int)(NUM_FFT_SAMPLES * t/PACKETSPERSECOND)]/20)+t_x;

	    glColor3f(
	      (float)(((st->cols[(int)(colstep*t)])&0xff)/256.0),
		  (float)(((st->cols[(int)(colstep*t)]>>8)&0xff)/256.0),
		  (float)((st-> cols[(int)(colstep*t)]>>16)/256.0));

	    glBegin(GL_QUADS);					
		  glVertex3f(t_x,-y1, 0.0f);		
		  glVertex3f( x1 ,-y1, 0.0f);		
		  glVertex3f( x1 ,(float)(-y1+drawstep*step), 0.0f);
		  glVertex3f(t_x,(float)(-y1+drawstep*step), 0.0f);
	    glEnd();				
		ec+=step;
		if ((ec>=every) && (st->captions)) 
		{
		  ec=0;
		  glRasterPos2f(textxpos, 0.0f-y1);
 		  glPrint("%2.1f", t);	// Print GL Text To The Screen
		}
	 }
	}
	else if (st->kind==1) 
	{          // Spectrogram
	 float actcol;

	glClearColor(	
	    (GLclampf)((st->bkcol&0xff))/256.0f,
		(GLclampf)((st->bkcol>>8)&0xff)/256.0f,
		(GLclampf)((st->bkcol>>16)&0xff)/256.0f,
			1.0f); 
	glDisable (GL_DEPTH_TEST);									// Enable Depth Testing

	step=(double)PACKETSPERSECOND/(double)NUM_FFT_SAMPLES;
	colstep=128.0/(st->endband-st->startband);
	drawstep=((1.0f+(float)st->gain_y/50.0f)/(st->endband-st->startband));

	every=(st->endband-st->startband)/40.0f;
	ec=every;

	// Instead of clearing the window on each pass, more visual continuity would be 
	// served by just rolling back to the start point.

	if (st->h_pos == -1)	// 1st time through h_pos is -1
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
	}

	st->h_pos++; if (st->h_pos>150) 
	{
		st->h_pos=0;
		// now we just clear window on first pass only
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
	}

    x1=(GLfloat) (t_x+st->h_pos*dx);

	for (t=st->startband; t<=st->endband; t+=step)
	{ 
	    y1=(GLfloat)(1.0f-(t-(GLfloat)st->startband)*drawstep);
			
		actcol=(st->fftbands[(int)(NUM_FFT_SAMPLES * t/PACKETSPERSECOND)]);
		if(actcol>127) actcol=127;

		glColor3f(
	      (float)(((st->cols[(int)actcol])&0xff)/256.0),
		  (float)(((st->cols[(int)actcol]>>8)&0xff)/256.0),
		  (float)((st-> cols[(int)actcol]>>16)/256.0));

		 glRectf( x1,-y1, x1+dx ,(float)(-y1+drawstep*step));
		ec+=step;
		if ((ec>=every) && (st->captions)) 
		{ ec=0;
		  glRasterPos2f(textxpos, 0.0f-y1);
 		  glPrint("%2.1f", t);	// Print GL Text To The Screen
		}
	 }
	
	}
	else if (st->kind==2) 
	{          // moving Spectogram


	float actcol;
	COLORREF ac;
	int s,x2,x3;
	 

	glClearColor(	
	    (GLclampf)((st->bkcol&0xff))/256.0f,
		(GLclampf)((st->bkcol>>8)&0xff)/256.0f,
		(GLclampf)((st->bkcol>>16)&0xff)/256.0f,
			1.0f); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glDisable (GL_DEPTH_TEST);									// Enable Depth Testing

	step=(double)PACKETSPERSECOND/(double)NUM_FFT_SAMPLES;
	colstep=128.0/(st->endband-st->startband);
	drawstep=((1.0f+(float)st->gain_y/50.0f)/(st->endband-st->startband));

	every=(st->endband-st->startband)/40.0f;
	ec=every;

  
	for (x2=0,x1=t_x; x2<128; x1+=dx,x2++)
	  for (t=st->startband; t<st->endband; t+=step)
	  { 
	    y1=(GLfloat)(1.0f-(t-(GLfloat)st->startband)*drawstep);
		s=(int)((t-st->startband)/(st->endband-st->startband)*128.0f);

		if (x2==0)
		{

			// memcpy(st->tex_buf[s],st->tex_buf[s]+1,127);
			for (x3=127;x3>0;x3--)
				st->tex_buf[s][x3]=st->tex_buf[s][x3-1];
			
			actcol=(st->fftbands[(int)(NUM_FFT_SAMPLES * t/PACKETSPERSECOND)]);
			st->tex_buf[s][0]=actcol;

		}
		else   actcol= st->tex_buf[s][x2];

		if(actcol>127.0f) actcol=127.0f;
		if(actcol<0.0f) actcol=0.0f;
		
		ac=st->cols[(int)(actcol)];

		
		glColor3f(
			  (float)(ac&0xff)     /256.0f,
			  (float)((ac>>8)&0xff)/256.0f,
			  (float)(ac>>16)      /256.0f );
			
	   
		 glRectf( x1,-y1, x1+dx ,(float)(-y1+drawstep*step));

		 if ((x2==0)&&(st->captions))
		{
			ec+=step;
			if (ec>=every)
			{ 
				ec=0;
				glRasterPos2f(textxpos, 0.0f-y1);
 				glPrint("%2.1f", t);	// Print GL Text To The Screen
			}
		}
	  }

	}
	else if (st->kind>=3) 
	{          // 3d- Spectogram


	float actcol,ocol;
	COLORREF ac;
	int s,x2,x3;
	GLfloat zp,zp2,zp3,zp4,y1,dy;
	

	glClearColor(	
	    (GLclampf)((st->bkcol&0xff))/256.0f,
		(GLclampf)((st->bkcol>>8)&0xff)/256.0f,
		(GLclampf)((st->bkcol>>16)&0xff)/256.0f,
			1.0f); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glEnable (GL_DEPTH_TEST);									// Enable Depth Testing
	
	glRotatef((float)st->xrot, 1.0f, 0.0f, 0.0f);
	glRotatef((float)st->yrot, 0.0f, 0.0f, 1.0f);
		
    glTranslatef(st->ytrans/10.0f, st->xtrans/50.0f, 0.0f);
	

	step=(double)PACKETSPERSECOND/(double)NUM_FFT_SAMPLES;
	colstep=128.0/(st->endband-st->startband);
	drawstep=((1.0f+(float)st->gain_y/50.0f)/(st->endband-st->startband));
	dy=(float)(drawstep*step);

	every=(st->endband-st->startband)/40.0f;
	ec=every;

  
	for (t=st->startband; t<st->endband; t+=step)
	{ 
		s=(int)((t-st->startband)/(st->endband-st->startband)*128.0f);
		// memmove(st->tex_buf[s],st->tex_buf[s]+1,127);
		for (x3=127;x3>0;x3--)
			st->tex_buf[s][x3]=st->tex_buf[s][x3-1];
		st->tex_buf[s][0]=(st->fftbands[(int)(NUM_FFT_SAMPLES * t/PACKETSPERSECOND)]);
	}
	


	for (x2=0,x1=t_x; x2<127; x1+=dx,x2++)
	{ 
	  ocol=-1;
	  for (t=st->startband; t<st->endband; t+=step)
	  { 
		s=(int)((t-st->startband)/(st->endband-st->startband)*128.0f);

		actcol= st->tex_buf[s][x2];
		if (ocol!=-1) actcol=(actcol+ocol)/2;

		if(actcol>127.0f) actcol=127.0f;
		if(actcol<0.0f) actcol=0.0f;
				
		ac=st->cols[(int)(actcol)];
		glColor3f(
			  (float)(ac&0xff)     /256.0f,
			  (float)((ac>>8)&0xff)/256.0f,
			  (float)(ac>>16)      /256.0f );
		
		
		zp=  st->tex_buf[s][x2] / (float)st->gain_z;
		zp2= st->tex_buf[s][x2+1]/ (float)st->gain_z;

		if (s==0) { zp4=zp; zp3=zp2; }

		y1=(float)((t-st->startband)*drawstep-1.0f);

	
		switch (st->kind) 
		{
		case 3:
		  glBegin(GL_QUADS);					
		  glVertex3d( x1   ,y1, zp);		
		  glVertex3d( x1+dx,y1, zp2);		
		  glVertex3d( x1+dx,y1-dy, zp3);
		  glVertex3d( x1   ,y1-dy, zp4);
	      glEnd();				
		  break;
		case 4:	
		  glBegin (GL_TRIANGLES);

		  glVertex3f( x1, y1, zp);
		  glVertex3f( x1+dx ,y1, zp2);		
		  glVertex3f( x1 ,y1-dy, zp4);

  		  glVertex3f( x1+dx ,y1, zp2);		
		  glVertex3f( x1    ,y1-dy, zp4);
		  glVertex3f( x1+dx ,y1-dy, zp3);
	   	  glEnd();		
		  break;
		case 5:
		  glBegin(GL_LINES);					
		  glVertex3d( x1   ,y1, zp);		
		  glVertex3d( x1+dx,y1, zp2);		
		  glVertex3d( x1+dx,y1, zp2);		
		  glVertex3d( x1+dx,y1-dy, zp3);
	      glVertex3d( x1+dx,y1-dy, zp3);
		  glVertex3d( x1   ,y1-dy, zp4);
	      glEnd();				
		break;
	  }
		



		zp3=zp2;
		zp4=zp;
		ocol=actcol;


		if ((x2==0)&&(st->captions))
		{
			ec+=step;
			if (ec>=every)
			{ 
				ec=0;
				glRasterPos2f(textxpos, y1);
 				glPrint("%2.1f", t);	// Print GL Text To The Screen
			}
		}
	  }
	}

	}
	glFlush();
}






LRESULT CALLBACK FFTDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szFileName[MAX_PATH],szdata[10];
	int t,i;
	FFTOBJ * st;
	
	st = (FFTOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_FFT)) return(FALSE);

	switch( message ) 
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_STARTBAND, st->startband,0); 
		SetDlgItemInt(hDlg, IDC_ENDBAND, st->endband,0); 
		SetDlgItemInt(hDlg, IDC_FFTDRAWINTERVAL, st->fft_drawinterval,0); 
		SetDlgItemInt(hDlg, IDC_FFTSCALE, st->scale,0); 
		SetDlgItemText(hDlg, IDC_PALETTEFILE, st->palettefile); 
		SetDlgItemText(hDlg, IDC_CAPTION, st->wndcaption); 

		SendDlgItemMessage(hDlg, IDC_FFTALIGNCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Left" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTALIGNCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Bottom" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTALIGNCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Right" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTALIGNCOMBO, CB_SETCURSEL, st->align, 0L ) ;

		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Bar-Graph" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Spectogram" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "moving Spectogram" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "3d-Quads" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "3d-Triangles" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "3d-Line-Grid" ) ;
		SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_SETCURSEL, st->kind, 0L ) ;

		t=0;
		while (fft_bin_values[t])
		{ 
			sprintf(szdata,"%d",fft_bin_values[t++]);
			SendDlgItemMessage(hDlg, IDC_BINCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) szdata ) ;
		}
		SendDlgItemMessage(hDlg, IDC_BINCOMBO, CB_SETCURSEL, st->binselect, 0L ) ;

		SendDlgItemMessage(hDlg, IDC_WINDOWCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Rectangle" ) ;
		SendDlgItemMessage(hDlg, IDC_WINDOWCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "Hanning" ) ;
		SendDlgItemMessage(hDlg, IDC_WINDOWCOMBO, CB_SETCURSEL, st->window, 0L ) ;

		CheckDlgButton(hDlg, IDC_FFTCAPTIONS, st->captions);

		SCROLLINFO lpsi;
		lpsi.cbSize=sizeof(SCROLLINFO);
		lpsi.fMask=SIF_RANGE; // |SIF_POS;
		
		lpsi.nMin=1; lpsi.nMax=500;
		SetScrollInfo(GetDlgItem(hDlg,IDC_FFTDRAWINTERVALBAR),SB_CTL,&lpsi,TRUE);
		SetScrollPos(GetDlgItem(hDlg,IDC_FFTDRAWINTERVALBAR), SB_CTL,st->fft_drawinterval,TRUE);		
		lpsi.nMin=1; lpsi.nMax=15000;
		SetScrollInfo(GetDlgItem(hDlg,IDC_FFTSCALEBAR),SB_CTL,&lpsi,TRUE);
		SetScrollPos(GetDlgItem(hDlg,IDC_FFTSCALEBAR), SB_CTL,st->scale,TRUE);		
		lpsi.nMin=0; lpsi.nMax=1000;
		SetScrollInfo(GetDlgItem(hDlg,IDC_GAINXBAR),SB_CTL,&lpsi,TRUE);
		SetScrollPos(GetDlgItem(hDlg,IDC_GAINXBAR), SB_CTL,st->gain_x,TRUE);		
		SetDlgItemInt(hDlg, IDC_GAINX, st->gain_x,0); 
		SetScrollInfo(GetDlgItem(hDlg,IDC_GAINYBAR),SB_CTL,&lpsi,TRUE);
		SetScrollPos(GetDlgItem(hDlg,IDC_GAINYBAR), SB_CTL,st->gain_y,TRUE);		
		SetDlgItemInt(hDlg, IDC_GAINY, st->gain_y,0); 
		SetScrollInfo(GetDlgItem(hDlg,IDC_GAINZBAR),SB_CTL,&lpsi,TRUE);
		SetScrollPos(GetDlgItem(hDlg,IDC_GAINZBAR), SB_CTL,st->gain_z,TRUE);		
		SetDlgItemInt(hDlg, IDC_GAINZ, st->gain_z,0); 

		return TRUE;
        
	case WM_CLOSE:
		 EndDialog(hDlg, LOWORD(wParam));
		break;
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_STARTBAND:
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
			st->startband=GetDlgItemInt(hDlg, IDC_STARTBAND, 0,0);
			if (st->startband<0) st->endband=0;
			if (st->startband>st->endband) st->startband=st->endband;
	        
			if (st->out_ports[0].get_range==-1)
			{
			  st->out_ports[0].out_min=(float)st->startband;
			  for (i=0;st->out[i].to_port!=-1;i++)
				if (st->out[i].from_port==0)
					st->out[i].min=st->out_ports[0].out_min;
			  update_dimensions();
			}
			//memset(st->tex_buf,0,sizeof(float)*128*128);
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			}
			break;
		case IDC_ENDBAND:
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
			st->endband=GetDlgItemInt(hDlg, IDC_ENDBAND, 0,0);
			if (st->endband<st->startband) st->endband=st->startband;
			if (st->endband>PACKETSPERSECOND/2) st->endband=PACKETSPERSECOND/2;
			if (st->out_ports[0].get_range==-1)
			{
			  
				st->out_ports[0].out_max=(float)st->endband;
				for (i=0;st->out[i].to_port!=-1;i++)
				  if (st->out[i].from_port==0)
					st->out[i].max=(float)st->endband;
				update_dimensions();
			}
			//memset(st->tex_buf,0,sizeof(float)*128*128);
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			}
			break;
		case IDC_FFTALIGNCOMBO:
			st->align=SendDlgItemMessage(hDlg, IDC_FFTALIGNCOMBO, CB_GETCURSEL, 0, 0 ) ;
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_BINCOMBO:
			st->binselect=SendDlgItemMessage(hDlg, IDC_BINCOMBO, CB_GETCURSEL, 0, 0 ) ;
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_WINDOWCOMBO:
			st->window=SendDlgItemMessage(hDlg, IDC_WINDOWCOMBO, CB_GETCURSEL, 0, 0 ) ;
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_FFTKINDCOMBO:
			st->kind=SendDlgItemMessage(hDlg, IDC_FFTKINDCOMBO, CB_GETCURSEL, 0, 0 ) ;
			/*
				if (st->kind==2) {
							HDC	hDC = GetDC(st->displayWnd);
							wglMakeCurrent(hDC, st->GLRC_FFT);
							glGenTextures(1, &textur[0]);				// Create One Texture
							glBindTexture(GL_TEXTURE_2D, textur[0]);
							glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
							glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
							}	
				else if (texture[0])
				{
							HDC	hDC = GetDC(st->displayWnd);
							wglMakeCurrent(hDC, st->GLRC_FFT);
							glDeleteTextures(1,textur);
				}
			*/
			st->h_pos = -1;
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_LOADPAL:
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PALETTES\\*.pal");
			if (open_file_dlg(hDlg, szFileName, FT_PALETTE, OPEN_LOAD)) 
			{
			  if (!load_from_file(szFileName, &(st->cols), sizeof(st->cols)))
				report_error("Could not load Color Palette");
			  else
			  {
				reduce_filepath(szFileName,szFileName);
				strcpy(st->palettefile,szFileName);
				SetDlgItemText(hDlg, IDC_PALETTEFILE, st->palettefile); 
				if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			  }
			}
			InvalidateRect(hDlg,NULL,FALSE);
			break;

		case IDC_FFTBKCOLOR:
			st->bkcol=select_color(hDlg,st->bkcol);
			if (st->displayWnd) InvalidateRect(st->displayWnd,NULL,TRUE);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_FFTCAPTIONS:
			st->captions= IsDlgButtonChecked(hDlg, IDC_FFTCAPTIONS);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_CAPTION:
			GetDlgItemText(hDlg, IDC_CAPTION, st->wndcaption, 50);
			SetWindowText(st->displayWnd,st->wndcaption);
			break;

		}
		break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if ((nNewPos=get_scrollpos(wParam, lParam))>=0)
		    {	  
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FFTDRAWINTERVALBAR))  
			  { SetDlgItemInt(hDlg, IDC_FFTDRAWINTERVAL,nNewPos,0);
			    st->fft_drawinterval=nNewPos; 
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FFTSCALEBAR))  
			  { SetDlgItemInt(hDlg, IDC_FFTSCALE,nNewPos,0);
			    st->scale=nNewPos; 
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_GAINXBAR))  
			  { SetDlgItemInt(hDlg, IDC_GAINX,nNewPos,0);
			    st->gain_x=nNewPos; 
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_GAINYBAR))  
			  { SetDlgItemInt(hDlg, IDC_GAINY,nNewPos,0);
			    st->gain_y=nNewPos; 
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_GAINZBAR))  
			  { SetDlgItemInt(hDlg, IDC_GAINZ,nNewPos,0);
			    st->gain_z=nNewPos; 
			  }

			}
		}
		break;
		
		case WM_ACTIVATE:
		case WM_KILLFOCUS:
	//	case WM_SETFOCUS:
			InvalidateRect(hDlg,NULL,FALSE);
		break;
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_FFTBKCOLOR),st->bkcol);
		break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;

	}
   return FALSE;
}


LRESULT CALLBACK FFTWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int t;
	FFTOBJ * st;
	static int x,y,b,xs,ys;
	PAINTSTRUCT ps;
	HDC	hDC;

	st=NULL;
	
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
	 if (objects[t]!=NULL)
		if (objects[t]->type==OB_FFT)
		{	st=(FFTOBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}
		  
	if (st!=NULL)
	{
	switch( message ) 
	{
	
	case WM_DESTROY:
		 Shutdown_GL(st->GLRC_FFT);
		 break;
	case WM_KEYDOWN:
			  SendMessage(ghWndMain, message,wParam,lParam);
		break;
	case WM_MOUSEACTIVATE:
		  close_toolbox();
		  actobject=st;
		  SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
		  InvalidateRect(ghWndDesign,NULL,TRUE);
		break;

    case WM_COMMAND:
		break;
		case WM_MOUSEMOVE:
			 x = int((HIWORD(lParam))/5); 
			 y = int((LOWORD(lParam))/5); 
			 if (b==1)
			 {
				st->xrot+=(x-xs);
				st->yrot+=(y-ys);
				
			 }
			 if (b==2)
			 {
				st->xtrans+=(x-xs);
				st->ytrans+=(y-ys);
			 }
			 xs=x;ys=y;
			break;
		case WM_LBUTTONDOWN:
			b=1;xs=x;ys=y; break;
		case WM_LBUTTONUP:
			b=0; break;
		case WM_RBUTTONDOWN:
			b=2;xs=x;ys=y; break;
		case WM_RBUTTONUP:
			b=0; break;

	case WM_SIZE:
			Size_GL(hWnd, st->GLRC_FFT,1); 		 
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
			hDC = BeginPaint(st->displayWnd, &ps);
			if (wglMakeCurrent(hDC, st->GLRC_FFT))
			{
				Draw_FFT_GL(st);
				SwapBuffers(hDC);
				wglMakeCurrent(0, 0);

			}
			EndPaint(st->displayWnd, &ps);
		}
		break;

	default:
			return DefWindowProc( hWnd, message, wParam, lParam );
	} 
	}
	else return DefWindowProc( hWnd, message, wParam, lParam );
   return 0;
}



//
//  Object Implementation
//


FFTOBJ::FFTOBJ(int num) : BASE_CL()	
	  {

	    inports  = 1;
		outports = 3;

		strcpy(in_ports[0].in_name,"in");

	    out_ports[0].get_range=-1;
		strcpy(out_ports[0].out_name,"avg");
		strcpy(out_ports[0].out_dim,"Hz");
		strcpy(out_ports[0].out_desc,"Average Frequency");
	    out_ports[0].out_max=(float)DEF_ENDBAND;
        out_ports[0].out_min=(float)DEF_STARTBAND*2-DEF_ENDBAND;

	    out_ports[1].get_range=-1;
		strcpy(out_ports[1].out_name,"pow");
		strcpy(out_ports[1].out_dim,"uV");
		strcpy(out_ports[1].out_desc,"Power in Bands");
		out_ports[1].out_max=50.0f;
        out_ports[1].out_min=0.0f;

	    out_ports[2].get_range=-1;
		strcpy(out_ports[2].out_name,"peak");
		strcpy(out_ports[2].out_dim,"uV");
		strcpy(out_ports[2].out_desc,"Peak Frequency");
		out_ports[2].out_max=50.0f;
        out_ports[2].out_min=0.0f;

		strcpy (wndcaption,"Frequency - Spectrum");
		left=50;right=300;top=330;bottom=580;captions=TRUE;
		scale=100;window=1;align=DEF_ALIGN;input=0;
		startband=DEF_STARTBAND;endband=DEF_ENDBAND;
		for (i=0;i<128;i++)	cols[i]=RGB(0,0,150);
		for (i=0;i<FFT_BUFFERLEN;i++) { buffer[i]=0; fftbands[i>>1]=0; }
		bkcol=RGB(250,250,250);
		strcpy(palettefile,"none");
		chnBufPos=0;fft_interval=0;fft_drawinterval=15;
		binselect=3;
		kind=0;
		h_pos = -1;	// tell spectrogram to clear window and start
		gain_x=50;gain_y=50;gain_z=50;
		xrot=0;yrot=0;xtrans=0.0f,ytrans=0.0f;
		memset(tex_buf,0,sizeof(float)*128*128);
		displayWnd=NULL;
		GLRC_FFT=NULL;

		displayWnd=create_FFT_Window(left,right,top,bottom);
	
/*		{
			char tmp[100];
			wsprintf(tmp,"GLRC:%ld",GLRC_FFT);
			report (tmp);
	    }
*/

	}

	void FFTOBJ::session_start(void)
	{
		int i;
		for (i=0;i<FFT_BUFFERLEN;i++) { buffer[i]=0; fftbands[i>>1]=0; }
		chnBufPos=0;
		h_pos = -1;

	}
	
	void FFTOBJ::session_reset(void)
	{
		int i;
		for (i=0;i<FFT_BUFFERLEN;i++) { buffer[i]=0; fftbands[i>>1]=0; }
		chnBufPos=0;
	}
	void FFTOBJ::session_pos(long pos)
	{
		int i;
		for (i=0;i<FFT_BUFFERLEN;i++) { buffer[i]=0; fftbands[i>>1]=0; }
		chnBufPos=0;
	}
  	  

	  void FFTOBJ::make_dialog(void)
	  {
		 display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_FFTPROPBOX, ghWndStatusbox, (DLGPROC)FFTDlgHandler));
	  }
	  void FFTOBJ::load(HANDLE hFile) 
	  {	
		float temp;

		load_object_basics(this);
		load_property("interval",P_INT,&fft_interval);
		load_property("scale",P_INT,&scale);
		load_property("drawinterval",P_INT,&fft_drawinterval);
		load_property("startband",P_INT,&startband);
		load_property("endband",P_INT,&endband);
		load_property("align",P_INT,&align);
		load_property("top",P_INT,&top);
		load_property("left",P_INT,&left);
		load_property("right",P_INT,&right);
		load_property("bottom",P_INT,&bottom);
		load_property("captions",P_INT,&captions);
		load_property("window",P_INT,&window);
		load_property("kind",P_INT,&kind);
		load_property("xrot",P_INT,&xrot);
		load_property("yrot",P_INT,&yrot);
		load_property("gainx",P_INT,&gain_x);
		load_property("gainy",P_INT,&gain_y);
		load_property("gainz",P_INT,&gain_z);
		load_property("xtrans",P_FLOAT,&xtrans);
		load_property("ytrans",P_FLOAT,&ytrans);
		load_property("binselect",P_INT,&binselect);
		load_property("background",P_FLOAT,&temp);
  	    load_property("wndcaption",P_STRING,wndcaption);

		bkcol=(COLORREF) temp;
		load_property("palette-file",P_STRING,palettefile);
		if (strcmp(palettefile,"none"))
		{
			char szFileName[MAX_PATH] = "";
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PALETTES\\");
			strcat(szFileName,palettefile);
			if (!load_from_file(szFileName, cols, sizeof(cols) ))
			{    
				report_error(szFileName);
				report_error("Could not load Color Palette ");
			}
		}
		MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
		if (GLOBAL.locksession) {
	 		SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
			//SetWindowLong(displayWnd, GWL_STYLE, 0);
		} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
	    SetWindowText(displayWnd,wndcaption);

	  }
	  void FFTOBJ::save(HANDLE hFile) 
	  {	
  	    float temp;
		save_object_basics(hFile, this);
		save_property(hFile,"interval",P_INT,&fft_interval);
		save_property(hFile,"scale",P_INT,&scale);
		save_property(hFile,"drawinterval",P_INT,&fft_drawinterval);
		save_property(hFile,"startband",P_INT,&startband);
		save_property(hFile,"endband",P_INT,&endband);
		save_property(hFile,"align",P_INT,&align);
		save_property(hFile,"top",P_INT,&top);
		save_property(hFile,"left",P_INT,&left);
		save_property(hFile,"right",P_INT,&right);
		save_property(hFile,"bottom",P_INT,&bottom);
		save_property(hFile,"captions",P_INT,&captions);
		save_property(hFile,"window",P_INT,&window);
		save_property(hFile,"kind",P_INT,&kind);
		save_property(hFile,"xrot",P_INT,&xrot);
		save_property(hFile,"yrot",P_INT,&yrot);
		save_property(hFile,"gainx",P_INT,&gain_x);
		save_property(hFile,"gainy",P_INT,&gain_y);
		save_property(hFile,"gainz",P_INT,&gain_z);
		save_property(hFile,"xtrans",P_FLOAT,&xtrans);
		save_property(hFile,"ytrans",P_FLOAT,&ytrans);
		save_property(hFile,"binselect",P_INT,&binselect);
		temp=(float)bkcol;
		save_property(hFile,"background",P_FLOAT,&temp);
		save_property(hFile,"palette-file",P_STRING,palettefile);
		save_property(hFile,"wndcaption",P_STRING,wndcaption);

	  }

	  void FFTOBJ::incoming_data(int port, float value) {	input=value; }

	  void FFTOBJ::work(void) 
	  {
		int i,bins,startbin,endbin,maxindex; 
		float powaccu,avgaccu,avgstep;
		float max;

	    chnBufPos++;  if (chnBufPos>=FFT_BUFFERLEN) chnBufPos=0;
	    buffer[chnBufPos]=input/100.0f*scale;
	    fft_interval++;
	    if (fft_interval>=fft_drawinterval)
		{
  	      fft_float(chnBufPos, (float *) buffer, window, (float *) fftbands, fft_bin_values[binselect]);

		  powaccu=0;
		  avgaccu=0;

		  startbin= (int) (startband * ((float)fft_bin_values[binselect] / PACKETSPERSECOND*2.0f ));
		  endbin=   (int) (  endband * ((float)fft_bin_values[binselect] / PACKETSPERSECOND*2.0f ));

		  bins=endbin-startbin;
		  avgstep=(endband-startband)/(float)bins;
		  max=0;maxindex=0;

  		  for (i=0;i<=bins ;i++)
		  {
			powaccu+=fftbands[startbin+i];
			avgaccu+=(startband+i*avgstep)*fftbands[startbin+i];
			if (max<fftbands[startbin+i]) {
				max=fftbands[startbin+i];
				maxindex=i;
			}
		  }
		  if (powaccu==0) avgaccu=0.0f; else avgaccu/=powaccu;
		  // powaccu=powaccu/i;
          pass_values(0,avgaccu); 
          pass_values(1,powaccu); 
          pass_values(2,startband+(float)maxindex*avgstep); 

		  fft_interval=0;
		}
		if ((!TIMING.draw_update)&&(!GLOBAL.fly)) InvalidateRect(displayWnd,NULL,FALSE);	

	  }

HWND FFTOBJ::create_FFT_Window(int left,int right, int top, int bottom)
{
	HWND hWnd;
	HDC hDC;
	HGLRC m_hRC;
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping
	PIXELFORMATDESCRIPTOR pfd;
	int nPixelFormat;	

	if (hWnd= CreateWindow( "FFTClass", "Frequency - Spectrum",WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME | WS_VISIBLE , left, top,right-left,bottom-top, ghWndMain, NULL, hInst, NULL)) 
	{
		ShowWindow(hWnd,1);
		UpdateWindow(hWnd);
				
		memset(&pfd, 0, sizeof(pfd));    
		pfd.nSize      = sizeof(pfd);
		pfd.nVersion   = 1;
		pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;
	
		// Set pixel format
	
		hDC = GetDC(hWnd);
		if (hDC==NULL) return(0);
		nPixelFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, nPixelFormat, &pfd);
	
		m_hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, m_hRC);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Background color
		//glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
		glShadeModel(GL_SMOOTH);	
		glClearDepth(1.0f);							    // Depth Buffer Setup
		glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
		glDepthFunc(GL_LESS);					// The Type Of Depth Test To Do
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Really Nice Perspective Calculations



		base = glGenLists(96);								// Storage For 96 Characters

		font = CreateFont(	-10,							// Height Of Font
							0,								// Width Of Font
							0,								// Angle Of Escapement
							0,								// Orientation Angle
							FW_BOLD,						// Font Weight
							FALSE,							// Italic
							FALSE,							// Underline
							FALSE,							// Strikeout
							ANSI_CHARSET,					// Character Set Identifier
							OUT_TT_PRECIS,					// Output Precision
							CLIP_DEFAULT_PRECIS,			// Clipping Precision
							ANTIALIASED_QUALITY,			// Output Quality
							FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
							"Courier New");					// Font Name

		oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
		wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
		SelectObject(hDC, oldfont);							// Selects The Font We Want
		DeleteObject(font);									// Delete The Font
		ReleaseDC(hWnd, hDC);

		Size_GL(hWnd, m_hRC, 1);

		GLRC_FFT= m_hRC;
		
		//////////////

		InvalidateRect(hWnd,NULL,TRUE);
		return(hWnd);
	}
	report_error("Can't Create FFT Window");
	return(0);
}




FFTOBJ::~FFTOBJ()
	  {
		DestroyWindow(displayWnd); displayWnd=NULL;
	  }  


