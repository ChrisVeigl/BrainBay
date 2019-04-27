/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_PEAKDETECT.H    detects tops and/or valleys of a signal
  Author:  Chris Veigl


  This Object outputs the Average of n Samples captured from it's input-port

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/



class PEAKDETECTOBJ : public BASE_CL
{
	
	public:
		int mode;
		int dir,olddir;
		float oldval;
		float input;
		float peak;

	PEAKDETECTOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~PEAKDETECTOBJ();
};
