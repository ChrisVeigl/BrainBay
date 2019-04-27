/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_SIGNAL.H:  contains the SIGNAL-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

#define SIG_SINUS 0
#define SIG_SAWTOOTH 1
#define SIG_RECTANGLE 2
#define SIG_RAMP 3



class SIGNALOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;

  public: 
	float input;
	float frequency;
	float gain;
	float center;
	float phase;
	float angle;
	int   noise;
	int   sigtype;
	int   enable_in;

    SIGNALOBJ(int num);
	void session_reset(void);
	void incoming_data(int port, float value);
	void work(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~SIGNALOBJ();
  
};
