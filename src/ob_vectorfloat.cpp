/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_VECTORFLOAT.CPP  declarations for the VectorFloat-Object
  Author:  Raymond Khoa

  This Object enables transfer of data from array_data_ports
  to single-value ports. 
  Data from one inport will be transferred to its corresponding outport

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_vectorfloat.h"
#include <iostream>

using namespace std;

void VECTORFLOATOBJ::UpdateGraphic(){
	height=CON_START+outports*CON_HEIGHT+5;
	sprintf(in_ports[0].in_name,"in");
	in_ports[0].in_type = MFLOAT;  //indicate this port is a square
	for (int i=0;i<nchannels;i++){
		sprintf(out_ports[i].out_name,"out%d",i+1);
		out_ports[i].out_min = -1.000f;
		out_ports[i].out_max = 1.000f;
	}
}

void VECTORFLOATOBJ::InitBuffer(int index){
	if (buffer[index]!= NULL) return;
	buffer[index] = new float[MAX_VECTOR_SIZE];
	for (int i=0;i<MAX_VECTOR_SIZE;i++)
		buffer[index][i]=0.0;
}

VECTORFLOATOBJ::VECTORFLOATOBJ(int num) : BASE_CL()
{
	set_inports(this,1);
	set_outports(this,1);
	width=95;
	nchannels = 1;
	buffer[0] = NULL;
    InitBuffer(0);
	UpdateGraphic();
}
	
void VECTORFLOATOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_VECTORFLOAT, ghWndStatusbox, (DLGPROC)VectorFloatDlgHandler));
}

void VECTORFLOATOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("nchannels",P_INT,&nchannels);
   set_outports(this,nchannels);
   UpdateGraphic();
}

void VECTORFLOATOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
	save_property(hFile,"nchannels",P_INT,&nchannels);
}
	
void VECTORFLOATOBJ::incoming_data(int port, float *value, int count)
{
	if (count>0)
	{
   		//add new values to the buffer
		for (int i=0;i<count;i++)
			buffer[0][i] = value[i];
	}
	if ((count != bufferSize[0]) && (count<MAX_EEG_CHANNELS))
	{
		bufferSize[0]=count;
		nchannels=count-1;
		set_outports(this,nchannels);
		UpdateGraphic();
		InvalidateRect(ghWndDesign,NULL,TRUE);
	}
}

void VECTORFLOATOBJ::work(void)
{
		for (int i=0;i<bufferSize[0];i++)
			pass_values(i,buffer[0][i]);
}


VECTORFLOATOBJ::~VECTORFLOATOBJ() {
		delete[] buffer[0];
		buffer[0] = NULL;
}

LRESULT CALLBACK VectorFloatDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	VECTORFLOATOBJ * st;
	
	st = (VECTORFLOATOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_VECTORFLOAT)) return(FALSE);	

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
