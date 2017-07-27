/* -----------------------------------------------------------------------------

  BrainBay  Version 2.0, GPL 2003-2017, contact: chris@shifz.org
  
  OB_THRESHOLD.H:  contains the THRESHOLD-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define ACCULEN 1000




class THRESHOLDOBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;

  public: 
	float input;

	float gained_value;
	int  last_value;
	float accu[ACCULEN];
	int  accupos;
	int  play_interval;
	int  interval_len;
	int  signal_gain;
	float from_input;
	float to_input;
	float avgsum;
	int  op;
	int  showmeter;
	int  rising,falling;
	int  usemedian;
	int  baseline;
	int  firstadapt;
	int  bigadapt,smalladapt;
	int  adapt_interval;
	int old_y1,old_y2;
	int barsize,fontsize;
	int redraw;
	COLORREF color,bkcolor, fontcolor, fontbkcolor;
	int  top,left,right,bottom;
	int buckets[1025];
    int adapt_num;
	char wndcaption[50];
	HFONT font;


    THRESHOLDOBJ(int num);

	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void update_inports(void);
	void work(void);
    ~THRESHOLDOBJ();
    
  private:
  	void empty_buckets(void);

};
