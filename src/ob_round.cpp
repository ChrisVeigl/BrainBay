/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_ROUND.CPP
  Author:  Chris Veigl


  This Object outputs the integer (rounded) value of a connected signal 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_round.h"

ROUNDOBJ::ROUNDOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
	round=INVALID_VALUE;
}
	

//void ROUNDOBJ::make_dialog(void)
//{
//	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ROUNDBOX, ghWndStatusbox, (DLGPROC)RoundDlgHandler));
//}

void ROUNDOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
}

void ROUNDOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
}

	
void ROUNDOBJ::incoming_data(int port, float value)
{
	if (value==INVALID_VALUE) round=value;
	else
	{
		if (value>0) round=(float)((int)(value+0.5f));
		else if (value<0) round=(float)((int)(value-0.5f));
	}
}
	
void ROUNDOBJ::work(void)
{
	pass_values(0, round);
}

ROUNDOBJ::~ROUNDOBJ() {}

