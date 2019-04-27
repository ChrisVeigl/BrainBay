

/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_MARTINI.H:  contains the MARTINI-Object

  The MARTINI-Object provides a Brainwave controlled Martini mixing game.
  Two Relais control the power supply for fwo liquid-pumps that provide
  Gin and Vermouth. The realais are connected to the Monolith EEG port D
  and can be switched by the Com-Writer element.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"



class MARTINIOBJ : public BASE_CL
{
  
  public: 
	float input1, input2;

	HBITMAP g_hbm1,g_hbm2,g_hbm3;

	float alpha;
	float beta;
	float preset_min,preset_max,ratio,baseline;
	int time,cnt1,pump1,pump2,pump1cnt,pump2cnt;
	int gametime;

	int redraw,redrawcnt;

	int state;
	int top,left,right,bottom;

	long sampletime;

	MARTINIOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~MARTINIOBJ();


};
