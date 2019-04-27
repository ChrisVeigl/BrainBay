
 //# define MATLAB_RELEASE    // enables the matlab-code

// if MATLAB_RELEASED is not defined, no matlab-code will be used
// (this will disable the matlab object)


/* --------------------------------------------------------------------------------
  
    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_MATLAB.H:  declarations for the Matlab-Object
  Authors: Lester John, Chris Veigl

  this object is currently disabled because the copyright-situation regarding the
  matlab-dlls has to be clarified.
  
  it provides a basic interface to the matlab-engine.

  the matlab-object collects data on its input-ports, transfers the buffers to matlab,
  and outputs the ANS-variable at the output-port
  2006-07 Revision 1.0: Glen Nilsen & Lester R. John, University of Cape Town
  2006-07 some Modifications to work with Matlab 7.1, Chris Veigl

----------------------------------------------------------------------------------*/


 
#include "brainBay.h"

#ifdef MATLAB_RELEASE                   // include matlab-engine only in special release
  #include "C:\Programme\MATLAB7\extern\include\engine.h"
#endif 

#define NUMSAMPLES 1001
#define MAXEXPRLENGTH 256
#define MATLABBUFFERLENGTH 200
#define MATLABNUMINPUTS 10

class MATLABOBJ : public BASE_CL
{
	//protected:
		float input;
		float inputA,inputB,inputC,inputD,inputE,inputF,inputG,inputH,inputI,inputJ;  
		float accumulator;
		float samples[NUMSAMPLES];
        int interval, writepos, added, periodic;
		char matlab_expression [MAXEXPRLENGTH+1];
#ifdef MATLAB_RELEASE
		Engine *ep;
#endif

	private:
    	void *evaluator;
		int  setexp;
		void shift_buffer_down(void);
		void update_buffer_index(void);

	public:
	
	MATLABOBJ(int num);
	
	char *expression;

	void update_inports(void);
	
	void make_dialog(void);

	void load(HANDLE hFile);

	void save(HANDLE hFile);
	
	void incoming_data(int port, float value);
	
	void work(void);

    void change_interval(int newinterval);
	
	void transfer(void);

	
	~MATLABOBJ();

friend LRESULT CALLBACK MatlabDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	private:
	void setExpression(char *expr);
};

