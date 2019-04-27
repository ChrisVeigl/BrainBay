/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COM_WRITER.H:  declarations for the Com-Port Writer
  Author: Chris Veigl

  The Com-Port Writer Object can be used to write Bytes to an opened Com-Port.
  This will be usually a port where a bidirectional EEG-device like the
  MONOLITH-EEG is connected to. Using this Object, Command- and Data Bytes can be
  sent to the Device, for example to adjust the baud- or sampling rate.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"

class COM_WRITEROBJ : public BASE_CL
{
	public:
	float input,input2;
	unsigned char byte;
	unsigned char command;
	unsigned char data1,data2;
	int en_trigger,cnt;
	float trigger,trigger2;
	
	COM_WRITEROBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~COM_WRITEROBJ();
};
