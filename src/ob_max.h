/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_MAX.H  declarations for the Max-Object
  Author:  Chris Veigl


  This Object outputs the maximum value of all connected input 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


class MAXOBJ : public BASE_CL
{
	protected:
		float signal[32];
		float maximum;

	public:

	MAXOBJ(int num);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void update_inports(void);
	void incoming_data(int port, float value);	
	void work(void);
	~MAXOBJ();
};
