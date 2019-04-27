/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_MIDI.H:  contains the MIDI-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"


//  from OB_MIDI.CPP :
LRESULT CALLBACK MidiStreamDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class MIDIOBJ : public BASE_CL
{
  protected:
	  int t,i,newchn,temp,index;
	  DWORD dwRead,dwWritten;
  public: 
	float inputnote;
	float inputvolume;
	int   inputtimer,inputpitch;

	int  timer;
	int  acttime;
	float sum_note;
	float sum_volume;
	int  n_tones;
	int  acttone;
	int  instrument;
	int  port;
	int  tonebuffer[MAX_MIDITONES];
	int  midichn;
	int  only_changes;
	int  from_volume;
	int  to_volume;
	int  mute;
	int  muted;
	int  mute_on_falsetones;
	int  pitch,pitchrange, pitchtime,pitchinterval;
	int volume;

	struct SCALEStruct tonescale;

	char tonefile[100];


	MIDIOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~MIDIOBJ();

    
};
