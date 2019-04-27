/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_CAM.CPP:  contains functions for Webcam functions
  Author: Chris Veigl

  The Camera-Object can connect to an installed webcam an display the live-
  video in a window. A Face-Detection is performed and the Position of the Nose and 
  the Chin are presented at the objects-output ports.
  
  This Module uses the Intel OpenCV Library for Computer Vision, and source
  code from the sample-projects describingf the use of the HaarClassifier-Cascade
  and the Lukas Kanade Optical Flow Algorithm.
  see details about Intels OpenCV: http://www.intel.com/research/mrl/research/opencv
  

  
-----------------------------------------------------------------------------*/

#include "brainBay.h"

class CAMOBJ : public BASE_CL
{
	protected:
		float input;
		
	public:
		int interval;
		int showlive;
		int mode;
		int enable_tracking;
		int trackface;
		char videofilename[256];


	CAMOBJ(int num);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);
	
	~CAMOBJ();
};
