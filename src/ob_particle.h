/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  OB_PARTICLE.H:  contains the PARTICLE-Object
  the object's propertries are declared and the
  constructor-, make_dialog-, load-, save-, incoming_data-, work-, and destructor-
  methods are implemented here
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"


#define	MAX_PARTICLES	1200		// Number Of Particles To Create
#define SLOWDOWN 300.0f
#define FADING   0.01f
#define PARTICLE_PARAMS 15


typedef struct PARTICLEPARAMETERStruct
{
	char  paramname[25];
	float min;
	float max;
} PARTICLEPARAMETERStruct;

typedef struct						// Create A Structure For Particle
{
	BOOL	active;					// Active (Yes/No)
	float	life;					// Particle Life
	float	fade;					// Fade Speed
	float	r;						// Red Value
	float	g;						// Green Value
	float	b;						// Blue Value
	float	x;						// X Position
	float	y;						// Y Position
	float	z;						// Z Position
	float	xi;						// X Direction
	float	yi;						// Y Direction
	float	zi;						// Z Direction
	float	xg;						// X Gravity
	float	yg;						// Y Gravity
	float	zg;						// Z Gravity
}
particles;							// Particles Structure

static GLfloat defcolors[12][3]=		// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};


class PARTICLEOBJ : public BASE_CL
{
protected:
	int i,t;
	DWORD dwRead,dwWritten;
	char actpropname[100];

  public: 
	float input1;
	float input2;
	float input3;
	float input4;
	float input5;
	float input6;

    float value[PARTICLE_PARAMS];
	float min[PARTICLE_PARAMS];
	float max[PARTICLE_PARAMS];
	int   remote[PARTICLE_PARAMS];
	int   act_parameter;
	int	  counter;
	int	  mute;
	int   oldest_particle;
	char  palettefile[100];
	COLORREF cols[128];
	GLfloat colors[128][3];

	particles particle[MAX_PARTICLES];	// Particle Array (Room For Particle Info)
	
    PARTICLEOBJ(int num);
    void init_particlestream(void);
	float get_paramvalue(int param);
	void update_inports(void);
    void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work(void);
    ~PARTICLEOBJ();

   
};



/*

  Particle-Parameters:
	1:		how many particles
	2:		generatinon frequency
	3:		slowdown
	4:		color
	5:		xpos
	6:		ypos
	7:		zpos
	8:		xspeed
	9:		yspeed
	10:		zspeed
    11:		xgrav
	12:		ygrav
	13:		zgrav
	14:		live
	15: 	random	
*/

