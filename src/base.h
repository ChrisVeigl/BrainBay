/* -----------------------------------------------------------------------------


   BrainBay  -  OpenSource Biofeedback Software
  OpenSource Application for realtime BioSignalProcessing & HCI with OpenEEG hardware. 
			   
  Author: Chris Veigl, contact: chris@shifz.org
  
  Co-Authors:
		 Jeremy Wilkerson (Modules: AND, OR, NOT, WAV, CORRELATION, EVALUATOR)
		 Lester John (Module MATLAB-transfer)

  Credits: Jim Peters (digital filter works), Jeff Molofee (OpenGL-tutorial), John Roark (SkinDialog)
  		   AllenD (COM-Port control), Aleksandar B. Samardz (Expression Evaluator Library)

  the used non-standard Libraries are:

  Multimedia and OpenGL: winmm.lib opengl32.lib glu32.lib vfw32.lib glaux.lib
  SDL (Simple Direct Media Layer): SDL.lib SDL_net.lib SDL_sound.lib modplug.lib 
  OpenCV - Intels's Computer Vision Library: cv.lib cvcam.lib cxcore.lib highgui.lib
  Matlab Engine (only in special Matlab Release): libeng.lib libmx.lib 
  Jim Peters's Filter Library: fidlib.lib (http://uazu.net)
  Skinned Dialog by John Roark: skinstyle.lib (http://www.codeproject.com/dialog/skinstyle.asp)
  GNU LibMatheval by Aleksandar B. Samardz: matheval.lib (http://www.gnu.org/software/libmatheval)
  

  Project-Site: http://brainbay.lo-res.org
  Link to the OpenEEG-Project: http://openeeg.sf.net
  
 
  
 -------------------------------------------------------------------------------------


  base.h:  contains the declaration of the base - class

  the base class provides properties like the position in the GUI, 
  the number of in- and outports and the pass_value() - function, which copies 
  a specified value to the in-ports of the linked objects


  
-----------------------------------------------------------------------------*/

// array bounds for objects and connections
#define MAX_CONNECTS 150
#define MAX_PORTS 100
#define MAX_EEG_CHANNELS  100
#define MAX_OBJECTS 150
#define MAX_VECTOR_SIZE 2000

// Signal value definitions
#define TRUE_VALUE         512 
#define INVALID_VALUE      -32767

//  default sampling rate
#define DEF_PACKETSPERSECOND  256

//  default refresh rates
#define DIALOG_UPDATETIME  50
#define DRAW_UPDATETIME    20


void report_error( char * Message );
extern class BASE_CL * objects[MAX_OBJECTS];
extern class BASE_CL * actobject;
extern class BASE_CL * deviceobject;
extern class BASE_CL * copy_object;
extern int actport;
extern struct LINKStruct * actconnect;

typedef enum PORTTYPE{
	SFLOAT = 0, MFLOAT = 1
} PORTTYPE;

typedef struct OUTPORTStruct
{
	char   out_name[20];
	char   out_desc[50];
	float  out_min;
	float  out_max;
	char   out_dim[10];
	int    get_range;
		// get_range values can be:
		//  (-1) = output port is defining it's own range as out_min and out_max
		//  (0 - N) = get the output range from origin of our input port# (0 = 1st port)
	PORTTYPE out_type;
} OUTPORTStruct ;

typedef struct INPORTStruct
{
	char  in_name[20];
	char  in_desc[50];
	float in_min;
	float in_max;
	char  in_dim[10];
	int   get_range;
		// get_range values can be:
		//  (TRUE, nonzero) = get the output range from source of this input port
		//  (FALSE, 0) = port is defining it's own range as in_min and in_max
	float value;
	PORTTYPE in_type;
} INPORTStruct ;

typedef struct LINKStruct
{
	int from_object;
    int from_port;
	int to_object;
	int to_port;
	float  min;
	float  max;
	char   dimension[10];
	char   description[50];
	int visited;

} LINKStruct ;

class BASE_CL
{
  public:
	int type;
	size_t object_size;
	int inports;
	int outports;
	int xPos; 
	int yPos;
	int width;
	int height;
	char tag[30];
	HWND displayWnd;

	OUTPORTStruct out_ports[MAX_PORTS];
	INPORTStruct  in_ports[MAX_PORTS];

	LINKStruct  out[MAX_CONNECTS];
    HWND hDlg;

	BASE_CL (void)
	{
		int i;
		width=0; height=0; displayWnd=NULL;
		tag[0]=0;
		for (i=0;i<MAX_PORTS;i++) 
		{  
		   in_ports[i].in_name[0]=0; 
		   strcpy(in_ports[i].in_desc,"none");
		   in_ports[i].in_min=-1.0f; 
		   in_ports[i].in_max=1.0f; 
		   strcpy(in_ports[i].in_dim,"none");
		   in_ports[i].get_range=1;
		   in_ports[i].in_type = SFLOAT;

		   out_ports[i].out_name[0]=0;
		   strcpy(out_ports[i].out_desc,"none");
		   out_ports[i].out_min=-1.0f;
		   out_ports[i].out_max=1.0f;
		   strcpy(out_ports[i].out_dim,"none");
		   out_ports[i].get_range=0;
		   out_ports[i].out_type = SFLOAT;
		}

		for (i=0;i<MAX_CONNECTS;i++) 
		{  out[i].min=-1.0f;
		   out[i].max=1.0f;
		   strcpy(out[i].dimension,"none");
		   strcpy(out[i].description,"none");
		}
	}
	virtual ~BASE_CL (void) {}
	virtual void work (void) {}
	virtual void update_inports (void) {}
	virtual void session_start (void) {}
	virtual void session_stop (void) {}
	virtual void session_end (void) {}
	virtual void session_reset (void) {}
	virtual void session_pos (long pos) {}
	virtual long session_length (void) { return 0; }
	virtual void make_dialog (void) {}
	virtual void load (HANDLE hFile) {}
	virtual void save (HANDLE hFile) {}
	virtual void incoming_data(int port, float value) {}
	virtual void incoming_data(int port, float *value, int count) {}
	
	void pass_values (int port, float value)
	{
		LINKStruct * act_link;
		for (act_link=&(out[0]);act_link->to_port!=-1;act_link++)
			if (act_link->from_port==port)
			   objects[act_link->to_object]->incoming_data(act_link->to_port, value );
	}
	void pass_values (int port, float *value, int count)
	{
		LINKStruct * act_link;
		for (act_link=&(out[0]);act_link->to_port!=-1;act_link++)
			if (act_link->from_port==port)
				if (objects[act_link->to_object])
					objects[act_link->to_object]->incoming_data(act_link->to_port, value, count );
	}
};

