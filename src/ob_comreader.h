/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COMREADER.H:  declarations for the ComPort Reader Element
  Authors: Chris Veigl

  The ComPort Reader can be used to open a serial port and read data from it
  Thus, external devices or Software like Mitsar Psytask can be connected and
  transfer information to a running BrainBay Session

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#define COMREADERBUFLEN 4096

class COMREADEROBJ : public BASE_CL
{

  public:
	float input1;
	int mintime;
	int cnt;
	unsigned char act_value;
	HANDLE comdev;
	unsigned int baudrate,comport,connected;
	unsigned int inpos,outpos, received,processed, sent;
	unsigned char buffer[COMREADERBUFLEN];

	DWORD dwRead;
	
	BOOL COMREADEROBJ::SetupComPort(int port);
	BOOL COMREADEROBJ::ReadComPort(HANDLE device, unsigned char * buffer, unsigned int pos, unsigned int maxlen);
	BOOL COMREADEROBJ::WriteComPort(HANDLE device, unsigned char * data, unsigned int len);
	BOOL COMREADEROBJ::BreakDownComPort(void);

	COMREADEROBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);

	
	~COMREADEROBJ();
};
