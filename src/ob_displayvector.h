/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software,		contact: raymonkhoa@gmail.com

  MODULE:  OB_DISPLAYVECTOR.H  declarations for the DisplayVector-Object
  Author:  Raymond Khoa

  This Object show the value or values (in the case of an array) coming to the
  module's inport (for testing purposes of array_data_ports)


 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

class DISPLAYVECTOROBJ : public BASE_CL
{
	protected:
		char value_str[5000];
	public:

	DISPLAYVECTOROBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);
	void incoming_data(int port, float *value, int count);
	void save(HANDLE hFile);
	
	void work(void);

	
	~DISPLAYVECTOROBJ();

	friend LRESULT CALLBACK DisplayVectorDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
