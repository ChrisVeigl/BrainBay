/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_EDF_READER.H:  declarations for EDF - File import
  Author: Chris Veigl


  Using this Object, an EDF-File can be specified and opened,
  the User Information is displayed in the object's toolbox,
  and the Signals are presented at the output-ports.

  Adjust the sampling rate to correctly display the signals.
  more Info about the EDF-File Format: http://www.edfplus.info/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/



#include "brainBay.h"


class EDF_READEROBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];
  
public: 
	struct EDFHEADERStruct header;
	struct CHANNELStruct channel[MAX_EEG_CHANNELS];
	int    packetcount;
	int    sampos;
	HANDLE edffile;
	char   filename[255];
	int	   state,loading;
	long   sessionlength;
	long   offset;



    EDF_READEROBJ(int num);
	void get_captions(void);
	void work(void);
	void session_reset(void);
	void session_start(void);
	void session_stop(void);
	void session_pos(long pos);
	long session_length(void);
    void calc_session_length(void);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~EDF_READEROBJ();

};
