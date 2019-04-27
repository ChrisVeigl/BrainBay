/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_EDF_WRITER.H:  contains the EDF-File - Writer-Object
  Author: Chris Veigl


  Using this Object, an EDF-File can be written,
  the Signals are connected to the input-ports.

  more Info about the EDF-File Format: http://www.edfplus.info/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

class EDF_WRITEROBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];
	char packet[500];
  
public: 
	struct EDFHEADERStruct header;
	struct CHANNELStruct channel[MAX_EEG_CHANNELS];
	int    samplecount,recordcount;
	HANDLE edffile;
	char   filename[255];
	char   edfinfos[8192];
	int  state;


    EDF_WRITEROBJ(int num);
	void get_captions(void);
	void update_inports(void);
	void work(void);
	void incoming_data(int port, float value);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~EDF_WRITEROBJ();

};
