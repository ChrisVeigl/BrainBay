/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
  
  OB_DOKU.H:  contains the DOKU-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
-----------------------------------------------------------------------------*/


#include "brainBay.h"


//  from OB_DOKU.CPP :
LRESULT CALLBACK DokuDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class DOKUOBJ : public BASE_CL
{
  
  public: 
	char text[8192];

	
	DOKUOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	~DOKUOBJ();

};
