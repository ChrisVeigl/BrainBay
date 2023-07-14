/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, GPL, contact: chris@shifz.org

  OB_FILTER.H:  contains the FILTER-Object
  Authors: Jim Peters, Chris Veigl


  This module calls into the fid-lib filter library by Jim Peters to provide
  filter functionalities. The Filter-Type and order can be selected.
  A brief preview of the filter's frequency-response is shown in the 
  filter-toolbox-window. For a better display check out Jim Peters fiview-tool:
  http://uazu.net/fiview

  do_filt_design: Initialises a new filter using the init-String 
      (for example LpBe for a low-pass - bessel filter, see fiview-documentation)
	  and additional filter-parameters (like order, from-, to-frequency)
  update_filterdialog:  enables/disables init-parameters according to the filter-type
  FilterBoxDlgHandler: processes events for the filter-toolbox window.

  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.


-----------------------------------------------------------------------------*/

#include "brainBay.h"


//  from OB_FILTER.CPP :
LRESULT CALLBACK FilterboxDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class FILTEROBJ : public BASE_CL
{
  protected: 
	DWORD dwRead,dwWritten;
	  
  public:

	float input;
	
	char name[25];
	int filtertype;
	int par0;
	float par1;
	float par2;
	int dispfrom,dispto;
    FidFilter *filt;
    FidFunc *funcp;
	FidRun *run;
	void * fbuf;

	FILTEROBJ(int num);
	void make_dialog(void);
	
	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);

	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
	~FILTEROBJ();

   
};
