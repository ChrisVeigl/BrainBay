/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COMPARE.H:  declarations for the Comparator-Object
  Author: Chris Veigl

  This Object compares two inputs (if they are greater, less or equal)
  If the condition is met, the output is copied from input port 1,
  if not, the output is set to INVALID_VALUE


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"  


class COMPAREOBJ : public BASE_CL
{

	public:
	float inputA;
	float inputB;
	int method;
	int out_invalid;
	float out_value ;

	COMPAREOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void incoming_data(int port, float value);
	void save(HANDLE hFile);
	void work(void);
	~COMPAREOBJ();
    
};
