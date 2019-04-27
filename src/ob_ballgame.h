

/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_BALLGAME.H:  contains the BALLGAME-Object

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"



class BALLGAMEOBJ : public BASE_CL
{
  
  public: 
	float input;

	int speed;
	int racket;

	float xpos,ypos,xspeed,yspeed,rpos,adjust_r;
	unsigned int state,points,best;
	int  top,left,right,bottom;
	int reset_middle;


	BALLGAMEOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~BALLGAMEOBJ();


};
