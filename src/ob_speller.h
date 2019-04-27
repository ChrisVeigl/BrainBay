

/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_SPELLER.H:  contains the SPELLER-Object

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"


struct dictstruct 
{
	int  count;
	int  type;
	char  tag[30];
	char data[30];
} ;

class SPELLEROBJ : public BASE_CL
{
  
  public: 
	float input,input2,oldinput;
	int paddle,oldpaddle;

	int speed,idle,press,mode,switchtime,presstime,idletime;
	float xpos,ypos;
	
	LONGLONG tstamp,tact;
	int select,selstart,selbegin,selend,selections,os,waitres,enter,wordcount;
	char word[1000],lastword[50];
	char wordfile[256];
	char dictfile[256];

	int sugchars,delchars,entries;
	int  top,left,right,bottom;
	struct dictstruct dict[500];
	int suggest[50],suggestions;
	int upchar;
	int autolearn,directsend;
	HFONT sugfont;

	char fn[256];


	SPELLEROBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void learn_words(void);
	void load_dictionary(char *);
	void save_dictionary(char *);
	void make_suggestions(void);
	void get_suggestions(void);
	void get_dictionary(void);
	void work(void);
	~SPELLEROBJ();


};
