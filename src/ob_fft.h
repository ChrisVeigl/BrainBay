/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_FFT.H:  contains the FFT-Object
  Author: Chris Veigl

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


#define DEF_ZOOM 100
#define DEF_STARTBAND 1
#define DEF_ENDBAND 40
#define DEF_ALIGN 0
#define FT_PALETTE 3
#define OPEN_SAVE 1
#define OPEN_LOAD 2
#define BITS_PER_WORD   (sizeof(unsigned) * 8)



//  from OB_FFT.CPP :
  HWND			create_FFT_Window(int left,int right, int top, int bottom, HGLRC * GLRC_FFT);
  void WINAPI	fft_float (int chnBufPos, float * buffer, int window, float * fftbands, int bins);  
  LRESULT CALLBACK FFTDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#define FFT_BUFFERLEN    8192

class FFTOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten,i;
  public: 
  	float    input;
	float	 buffer[FFT_BUFFERLEN];
	unsigned int chnBufPos;
	int		 fft_interval;
	int      scale;
	int		 fft_drawinterval;
	float	 fftbands[FFT_BUFFERLEN>>1];
	float    tex_buf[128][128];
	
	int		 xrot,yrot;
	float    xtrans,ytrans;
	int      gain_x,gain_y,gain_z;
	int      h_pos;
	int      startband;
	int      endband;
	int		 align;
	int		 binselect;
	int      top,left,right,bottom;
	int      captions;
	int		 window;
	int      kind;
	char     palettefile[100];
	char     wndcaption[50];
	COLORREF cols[128];
	COLORREF bkcol;
	HGLRC    GLRC_FFT;		  // Handle to OPENGL-RC


	FFTOBJ(int num);
	
	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	HWND create_FFT_Window(int left,int right, int top, int bottom);
	~FFTOBJ();
   
};
