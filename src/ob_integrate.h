/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_INTEGRATE.H:  declarationss for the Intergrator-Object
  Author: Chris Veigl

  The Integrator - Object sums up the stream of input-values and outputs the result.
  It can be reset to a desired value

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"

class INTEGRATEOBJ : public BASE_CL
{
	public:
    float i_value,min,max;

	INTEGRATEOBJ(int num);

	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);
	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~INTEGRATEOBJ();

	friend LRESULT CALLBACK IntegrateDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
