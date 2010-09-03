/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
  
  MODULE: OB_MCI.H:  declarations for the Multimedia-Player-Object
  Author: Chris Veigl

  This Object can open standard Multimedia files and play them in an extra window

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include <vfw.h>	




class MCIOBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten,i;
  public: 
  	float    input;
	int      top,left,right,bottom;
	int      speed,updatespeed,actspeed;
	int      volume,updatevolume, actvolume;
	int	 	 step,updatestep;
	int	 	 play,playing;
	int		upd_volume,upd_speed;
	char     mcifile[256];
	HWND m_video;
	
	
	MCIOBJ(int num);
	void session_stop(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    void incoming_data(int port, float value) ;
	void work(void);
 	~MCIOBJ();
   
};

