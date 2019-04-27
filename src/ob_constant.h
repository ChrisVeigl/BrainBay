/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_CONSTANT.H:  declaration for the Constant-Source Object
  Author: Chris Veigl

  This Object outputs a constant value on it's port.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"


class CONSTANTOBJ : public BASE_CL
{
	protected:
        float value;

	public:

	CONSTANTOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);

	void incoming_data(int port, float value);
		
	void work(void);

	~CONSTANTOBJ();

	friend LRESULT CALLBACK ConstantDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
