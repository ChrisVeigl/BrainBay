/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_MIN.CPP
  Author:  Chris Veigl


  This Object outputs the Minimum of the connected signal values

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_min.h"

MINOBJ::MINOBJ(int num) : BASE_CL()
{
	int t;
	outports = 1;
	inports = 1;
	width=65;
	for (t=0;t<MAX_PORTS;t++) sprintf(in_ports[t].in_name,"in%d",t+1);

	strcpy(in_ports[0].in_name,"in1");
	strcpy(out_ports[0].out_name,"min");
    for (int i = 0; i < 32; i++)
    {
    	signal[i] = 0.0;
    }
}
	

//void MINOBJ::make_dialog(void)
//{
//	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MINBOX, ghWndStatusbox, (DLGPROC)MinDlgHandler));
//}

void MINOBJ::update_inports(void)
{
	int x; 
	inports=count_inports(this);
		
	if (inports>MAX_EEG_CHANNELS) inports=MAX_EEG_CHANNELS;
	for(x=inports;x<MAX_EEG_CHANNELS;x++)
		in_ports[x].get_range=1;

	height=CON_START+inports*CON_HEIGHT+5;
	if (displayWnd) InvalidateRect(displayWnd,NULL,FALSE);
 	InvalidateRect(ghWndDesign,NULL,TRUE);
}

void MINOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
}

void MINOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
}
	
void MINOBJ::incoming_data(int port, float value)
{
	signal[port]=value;
}
	
void MINOBJ::work(void)
{
	int i;
	minimum=INVALID_VALUE;
	for (i=0;i<inports-1;i++)
	{
		if (minimum==INVALID_VALUE) minimum=signal[i];
		if ((signal[i]!=INVALID_VALUE) && (signal[i]<minimum))
			minimum=signal[i];
	}
	pass_values(0, minimum);
}

MINOBJ::~MINOBJ() {}

