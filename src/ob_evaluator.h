/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
   Expression-Evaluator - Object
   Provides an interface to the GNU LibMatheval- Library for Expression Evaluation
			   
  Authors: Jeremy Wilkerson, Chris Veigl, Aleksandar B. Samardz (libmatheval)
  Link to GNU-Libmatheval: http://www.gnu.org/software/libmatheval
 

Description:
  String representation of function is allowed to consist of decimal constants, 
  variables, elementary functions, unary and binary operations. 

  Variable name is any combination of alphanumericals and _ characters beginning 
  with a non-digit that is not elementary function name. 

Supported elementary functions are 
(names that should be used are given in parenthesis): 
  exponential (exp), logarithmic (log), square root (sqrt), sine (sin), cosine (cos), 
  tangent (tan), cotangent (cot), secant (sec), cosecant (csc), 
  inverse sine (asin), inverse cosine (acos), inverse tangent (atan), 
  inverse cotangent (acot), inverse secant (asec), inverse cosecant (acsc), 
  hyperbolic sine (sinh), cosine (cosh), hyperbolic tangent (tanh), 
  hyperbolic cotangent (coth), hyperbolic secant (sech), hyperbolic cosecant (csch), 
  hyperbolic inverse sine (asinh), hyperbolic inverse cosine (acosh), 
  hyperbolic inverse tangent (atanh), hyperbolic inverse cotangent (acoth), 
  hyperbolic inverse secant (asech), hyperbolic inverse cosecant (acsch), 
  absolute value (abs), Heaviside step function (step) with value 1 defined 

  for x = 0 and Dirac delta function with infinity (delta) and 
  not-a-number (nandelta) values defined for x = 0. 

Supported unary operation is unary minus ('-'). 

Supported binary operations are:
  addition ('+'), subtraction ('+'), multiplication ('*'), 
  division multiplication ('/') and exponentiation ('^'). 

Usual mathematical rules regarding operation precedence apply. 
Parenthesis ('(' and ')') could be used to change priority order. 

Blanks and tab characters are allowed in string representing function; 
newline characters must not appear in this string. 

  
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; See the
GNU General Public License for more details.


-----------------------------------------------------------------------------  */

#include "brainBay.h"

#define MAXEXPRLENGTH 256
#define NUMINPUTS 6

class EVALOBJ : public BASE_CL
{
	protected:
		float inputA, inputB, inputC, inputD, inputE, inputF;
        
	private:
    	void *evaluator;
		int  setexp;
        
	public:
		
		EVALOBJ(int num);

        char *expression;

		void update_inports(void);
	
		void make_dialog(void);

		void load(HANDLE hFile);

		void save(HANDLE hFile);
	
		void incoming_data(int port, float value);
	
		void work(void);

		~EVALOBJ();
		
friend LRESULT CALLBACK EvalDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	private:
	void setExpression(char *expr);
};
