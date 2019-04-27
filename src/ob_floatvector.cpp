/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_FLOATVECTOR.CPP  declarations for the Floatvector-Object
  Author:  Raymond Khoa

  This Object enables transfer of data from single float values 
  into array_data_ports.
  Data from one inport will be transferred to its corresponding outport

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_floatvector.h"
#include <iostream>

using namespace std;

void FLOATVECTOROBJ::UpdateGraphic(){
	height=CON_START+inports*CON_HEIGHT+5;
	out_ports[0].out_type = MFLOAT;
	out_ports[0].out_min = -1.000f;
	out_ports[0].out_max = 1.000f;
	sprintf(out_ports[0].out_name,"out");
	for (int i=0;i<nchannels;i++){
		sprintf(in_ports[i].in_name,"in%d",i+1);
		in_ports[i].in_type = SFLOAT;  //indicate this port is a square
	}
}

void FLOATVECTOROBJ::InitBuffer(int index){
	if (buffer[index]!= NULL) return;
	buffer[index] = new float[MAX_VECTOR_SIZE];
	for (int i=0;i<MAX_VECTOR_SIZE;i++)
		buffer[index][i]=0.0;
}

FLOATVECTOROBJ::FLOATVECTOROBJ(int num) : BASE_CL()
{
	set_inports(this,1);
	set_outports(this,1);
	width=95;
	nchannels = 1;
	buffer[0] = NULL;
    InitBuffer(0);
	UpdateGraphic();
}
	
void FLOATVECTOROBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_FLOATVECTOR, ghWndMain, (DLGPROC)FloatVectorDlgHandler));
}

void FLOATVECTOROBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("nchannels",P_INT,&nchannels);
   InitBuffer(0);
   set_inports(this,nchannels);
   set_outports(this,1);
   UpdateGraphic();
}

void FLOATVECTOROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
	save_property(hFile,"nchannels",P_INT,&nchannels);
}
	
void FLOATVECTOROBJ::incoming_data(int port, float value)
{
		//add new value to the buffer
		buffer[0][port] = value;
		bufferSize[0] = nchannels;
}

void FLOATVECTOROBJ::work(void)
{
		pass_values(0,buffer[0],bufferSize[0]);
}

void FLOATVECTOROBJ::update_inports(void)
{
	int newinports=count_inports(this);
	if (newinports>MAX_EEG_CHANNELS || newinports==nchannels) return;
	nchannels = newinports;
	set_inports(this,nchannels);
	UpdateGraphic();
	InvalidateRect(ghWndDesign,NULL,TRUE);
}

FLOATVECTOROBJ::~FLOATVECTOROBJ() {
		delete[] buffer[0];
		buffer[0] = NULL;
}

LRESULT CALLBACK FloatVectorDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	FLOATVECTOROBJ * st;
	
	st = (FLOATVECTOROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_FLOATVECTOR)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
			break;		
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_STATIC:
					break;
			}
			return TRUE;
			break;
		case WM_HSCROLL:			
		
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}
