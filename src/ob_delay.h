/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_DELAY.H  declarations for the Delay-Object
  Author:  Chris Veigl


  This Object outputs the input signal, delayed by n samples

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#define DELAYSAMPLES 20000

class DELAYOBJ : public BASE_CL
{
	protected:
		float samples[DELAYSAMPLES],act_out;
        int interval, writepos, added;

	public:

	DELAYOBJ(int num);

	void session_start(void);
	void session_reset(void);
	void session_pos(long pos);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~DELAYOBJ();

	friend LRESULT CALLBACK DelayDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    private:
    
    void change_interval(int newinterval);
};
