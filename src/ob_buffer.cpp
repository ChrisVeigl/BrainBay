/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_BUFFER.CPP  declarations for the Buffer-Object
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
#include "ob_buffer.h"
#include <iostream>

using namespace std;

void BUFFEROBJ::UpdateGraphic(){
	height=CON_START+inports*CON_HEIGHT+5;
	for (int i=0;i<nchannels;i++){
		sprintf(in_ports[i].in_name,"i%d",i+1);
		in_ports[i].in_type = MFLOAT;  //indicate this port is a square
		sprintf(out_ports[i].out_name,"fd%d",i+1);
		out_ports[i].out_min = 0.000f;
		out_ports[i].out_max = 3.000f;
	}
}

void BUFFEROBJ::InitBuffer(int index){
	if (buffer[index]!= NULL) return;
	buffer[index] = new float[MAX_VECTOR_SIZE];
	for (int i=0;i<MAX_VECTOR_SIZE;i++)
		buffer[index][i]=0.0;
}

BUFFEROBJ::BUFFEROBJ(int num) : BASE_CL()
{
	set_inports(this,1);
	set_outports(this,1);
	width=95;
	nchannels = 1;
	for (int i=0;i<MAX_EEG_CHANNELS;i++){
		buffer[i] = NULL;
	}
    InitBuffer(0);

	UpdateGraphic();
}
	
void BUFFEROBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_VECTORBUFFER, ghWndStatusbox, (DLGPROC)BufferDlgHandler));
}

void BUFFEROBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("nchannels",P_INT,&nchannels);
   for (int i=0;i<nchannels;i++)
	   InitBuffer(i);
   set_inports(this,nchannels);
   set_outports(this,nchannels);
   UpdateGraphic();
}

void BUFFEROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
	save_property(hFile,"nchannels",P_INT,&nchannels);
}
	
void BUFFEROBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	{
		//add new value to the buffer
		buffer[port][0] = value;
		bufferSize[port] = 1;
	}
	else
		bufferSize[port] = 0;
}

void BUFFEROBJ::incoming_data(int port, float *value, int count)
{
	if (count>0)
	{
		//write_logfile("getting values port %d:  %4.2f,%4.2f,%4.2f,%4.2f",port,value[0],value[1],value[2],value[3]);
   		//add new values to the buffer
		for (int i=0;i<count;i++)
			buffer[port][i] = value[i];
	}
	bufferSize[port] = count;
}

void BUFFEROBJ::work(void)
{
	for (int n=0;n<nchannels;n++)
		for (int i=0;i<bufferSize[n];i++)
		{
		    //write_logfile("passing to port %d value: %4.2f",n,buffer[n][i]);
			pass_values(n,buffer[n][i]);
		}
}

void BUFFEROBJ::update_inports(void)
{
	int newinports=count_inports(this);

	if (newinports>MAX_EEG_CHANNELS || newinports==nchannels) return;

	if (newinports>nchannels) InitBuffer(nchannels);
	else{		
		for (int i=newinports;i<nchannels;i++){	
			delete[] buffer[i];
			buffer[i] = NULL;
		}
	}
	nchannels = newinports;
	set_inports(this,nchannels);
	set_outports(this,nchannels);
	
	UpdateGraphic();
	InvalidateRect(ghWndDesign,NULL,TRUE);
}

BUFFEROBJ::~BUFFEROBJ() {
	for (int i=0;i<nchannels;i++){
		delete[] buffer[i];
		buffer[i] = NULL;
	}
}

LRESULT CALLBACK BufferDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	BUFFEROBJ * st;
	
	st = (BUFFEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_BUFFER)) return(FALSE);	

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
