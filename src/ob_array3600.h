/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_ARRAY3600.H:  declarations for the ARRAY Power Supply Controller
  Authors: Chris Veigl, Gerhard Nussbaum

  The ARRAY Power Supply Controller Object can be used to write Bytes to an
  Array 3600 remote controllable Power Supply via a Com-Port.
  Maximum values for Voltage, Current and Power can be specified,
  Voltage can be adjusted periodically according to the element's input values.
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"

class ARRAY3600OBJ : public BASE_CL
{
  private:
	float cnt;

  public:
	float input1;
	HANDLE comdev;
	unsigned int baudrate,comport,connected;

	int periodic;
	int period;
	int maxcurrent,maxvoltage,maxpower,voltage,address;
	DWORD dwWritten;
	
	byte getCheckByte(byte * command);
    byte getHighByte(int val);
    byte getLowByte(int val);
    void setToSelfControl();
    void setToPcControlOn();
    void setToPcControlOff();
    void setParameters(byte address, int maxI, int maxU, int maxP, int u);

	BOOL ARRAY3600OBJ::SetupComPort(int port);
	BOOL ARRAY3600OBJ::WriteComPort(HANDLE device, unsigned char * data, unsigned int len);
	BOOL ARRAY3600OBJ::BreakDownComPort(void);

	ARRAY3600OBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);



	
	
	~ARRAY3600OBJ();
};
