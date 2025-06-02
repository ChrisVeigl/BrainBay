/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_LSL_READER.H:  contains the LSL Stream Reader-Object
  Author: Keum, D.S.


  This Object reads data from an LSL stream and feeds the signals to 
  the output ports.

  lib from : https://github.com/sccn/liblsl
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "lsl_cpp.h"

class LSL_RECEIVEOBJ : public BASE_CL
{
protected:
	DWORD dwWritten;   // declare protected variables here
	
public:                // public variables that can be accessed from Dialog Handler
	lsl_inlet inlet;
	char stream_name[255];
	int  state;
	long samplecount;

	// STANDARD METHODS (nearly every element has these) :
    LSL_RECEIVEOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void work(void);
	~LSL_RECEIVEOBJ();

	// NON_STABDARD METHODS (some elements need these) :
	void session_reset(void);
	void session_start(void);
	void session_stop(void);


	//Dialog handler to process user interaction with IDD_LSLBOX
	friend LRESULT CALLBACK LSLReceiveDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	std::vector<lsl::stream_info> m_streams;
	lsl::stream_inlet* m_inlet;
	int select_stream;
	lsl::stream_info m_current_stream;
	int m_numChannels;
	std::vector<double> m_xData;
	std::vector<std::vector<float>> m_yData;

	

private: // declare private variables here
};
