/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_LIMITER.H  declarations for the Limiter-Object
  Author:  Chris Veigl


  This Object outputs a min/max-limited input signal

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


class LIMITEROBJ : public BASE_CL
{
	protected:
		float upper, lower,actval;
	public:

	LIMITEROBJ(int num);

	void make_dialog(void);
	void load(HANDLE hFile);
	void incoming_data(int port, float value);
	void save(HANDLE hFile);
	void work(void);
	~LIMITEROBJ();

	friend LRESULT CALLBACK LimiterDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
};
