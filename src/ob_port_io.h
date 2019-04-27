/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_PORTIO.H
  Authors: Craig Peacock, Chris Veigl


  This Object performs calls to the Beyond Logic Port Talk I/O Port Driver
  by Craig Peacock (http://www.beyondlogic.org) to allow I/O access to
  the Parallel Port. In the context of BrainBay, this object could be used 
  to control external events with Biosignal Parameters. 

  For reuse of the PortTalk driver source pt_ioctl.c refer to the README.txt
  document in ./portalk  

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"

class PORTOBJ : public BASE_CL
{
		
		
	public:
		float trigger,old_trigger;
		unsigned short portaddress;
		unsigned char val0,val1,portval;
		int triggermode;
		
	
	PORTOBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~PORTOBJ();
};
