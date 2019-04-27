/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_VOLUME.H  declarations for the Round-Object
  Author:  Chris Veigl


  This object sets the master volume to 0-100 according to the connected signal 

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


class VOLUMEOBJ : public BASE_CL
{
	protected:
		float actvolume;

	public:

	VOLUMEOBJ(int num);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);	
	void work(void);
	~VOLUMEOBJ();
};
