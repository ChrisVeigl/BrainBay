/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_FILE_WRITER.H:  contains the FILE - Writer-Object
  Author: Chris Veigl


  Using this Object, a File containing raw or ASCII-integer values 
  of the connected signale can be written. Delimiters for the Columns can be selected
  the Signals are connected to the input-ports.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

class FILE_WRITEROBJ : public BASE_CL
{
protected:
	DWORD dwWritten;
	
public: 
	HANDLE file;
	char filename[255];
	char headerline[500];
	int  state;
	int  format;
	int  append;
	int  autocreate;
	int  add_date;
	int  semicolon;
	int  averaging;
	float averaging_buffers[MAX_PORTS];
	int  avg_count;

    FILE_WRITEROBJ(int num);
	void update_inports(void);
	void work(void);
	void incoming_data(int port, float value);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void session_reset(void);
	void session_start(void);
	void session_stop (void);
	void session_pos (long pos);
    ~FILE_WRITEROBJ();

};
