/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_FILE_READER.H:  contains the FILE - Reader-Object
  Author: Chris Veigl


  Using this Object, a File containing raw or ASCII-integer values 
  of signals can be read. Delimiters for the Columns can be selected
  the Signals are fed to the output-ports.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"

class FILE_READEROBJ : public BASE_CL
{
protected:
	DWORD dwWritten;
	
public: 
	HANDLE file;
	char filename[255];
	int  state;
	int  format;
	long samplecount;


    FILE_READEROBJ(int num);
	void work(void);

	void session_reset(void);
	void session_start(void);
	void session_stop(void);
	void session_pos(long pos);
	long session_length(void);

	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~FILE_READEROBJ();

};
