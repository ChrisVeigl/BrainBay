/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_ROUND.H  declarations for the Round-Object
  Author:  Chris Veigl


  This Object outputs the integer (rounded) value of a connected input 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


class ROUNDOBJ : public BASE_CL
{
	protected:
		float round;

	public:

	ROUNDOBJ(int num);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);	
	void work(void);
	~ROUNDOBJ();
};
