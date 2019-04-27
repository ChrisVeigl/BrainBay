/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_MIXER4.H:  contains the 4 Channel Mixer-Object
  Author: Chris Veigl

  The Mixer-Object can mix 4 input Signals into one output signal.

  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/

#include "brainBay.h"

class MIXER4OBJ : public BASE_CL
{
	protected:
		float input1, input2, input3, input4;
		
	public:
	float chn1vol, chn2vol, chn3vol, chn4vol;
	int invmode;
	
	MIXER4OBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~MIXER4OBJ();
};
