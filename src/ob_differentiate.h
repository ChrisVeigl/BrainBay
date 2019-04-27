/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_DIFFERENTIATE.H  declarations for the Differentiate-Object
  Author:  Chris Veigl


  This Object outputs the arithmetic difference of the current sample value to the previous sample value (current-previous) 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


class DIFFERENTIATEOBJ : public BASE_CL
{
	protected:
		float previous;
		float difference;

	public:

	DIFFERENTIATEOBJ(int num);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);	
	void work(void);
	~DIFFERENTIATEOBJ();
};
