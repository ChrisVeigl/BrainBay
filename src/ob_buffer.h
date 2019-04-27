/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_BUFFER.CPP  declarations for the Buffer-Object
  Author:  Raymond Khoa

  This Object enables transfer of data from array_data_ports
  to single-value port 
  Data from one inport will be transferred to its corresponding outport

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

class BUFFEROBJ : public BASE_CL
{
	protected:
		float *buffer[MAX_EEG_CHANNELS];
		int bufferSize[MAX_EEG_CHANNELS]; //size of each buffer
		int nchannels;
	public:

	BUFFEROBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value); //for a single value

	void incoming_data(int port, float *value, int count); //for an array
	
	void save(HANDLE hFile);
	
	void work(void);

	void update_inports(void);

	void UpdateGraphic(void);
	void InitBuffer(int index);

	
	~BUFFEROBJ();

	friend LRESULT CALLBACK BufferDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
