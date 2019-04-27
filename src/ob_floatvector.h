/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_FLOATVECTOR.CPP  declarations for the FloatVector-Object
  Author:  Raymond Khoa

  This Object enables transfer of data from single value ports to array_data_ports
  Data from one inport will be transferred to its corresponding outport

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

class FLOATVECTOROBJ : public BASE_CL
{
	protected:
		float *buffer[MAX_EEG_CHANNELS];
		int bufferSize[MAX_EEG_CHANNELS]; //size of each buffer
		int nchannels;
	public:

	FLOATVECTOROBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value); //for a single value
	
	void save(HANDLE hFile);
	
	void work(void);

	void update_inports(void);

	void UpdateGraphic(void);
	void InitBuffer(int index);

	
	~FLOATVECTOROBJ();

	friend LRESULT CALLBACK FloatVectorDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
