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
#include "ob_evaluator.h"
#include "matheval.h"

#if _MSC_VER < 1900
  #pragma comment(lib, "matheval_v100.lib")
#else
  #pragma comment(lib, "matheval.lib")
#endif


EVALOBJ::EVALOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports=1;
    width = 70;
	strcpy(in_ports[0].in_name,"A");
	strcpy(in_ports[1].in_name,"B");
	strcpy(in_ports[2].in_name,"C");
	strcpy(in_ports[3].in_name,"D");
	strcpy(in_ports[4].in_name,"E");
	strcpy(in_ports[5].in_name,"F");

	strcpy(out_ports[0].out_name,"out");
 
	inputA = 0;
    inputB = 0;
    inputC = 0;
    inputD = 0;
    inputE = 0;
    inputF = 0;
    expression = NULL;    evaluator = NULL;
	setexp=FALSE;

	setExpression("A");
}


void EVALOBJ::update_inports(void)
{
	int i;
	i=count_inports(this);
	
	if (i>6) i=6;
	if (i>inports) inports=i;
		
	height=CON_START+inports*CON_HEIGHT+5;
	InvalidateRect(ghWndMain,NULL,TRUE);
}

	
void EVALOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EVALBOX, ghWndStatusbox, (DLGPROC)EvalDlgHandler));
}

void EVALOBJ::load(HANDLE hFile) 
{
    char expr[MAXEXPRLENGTH];

	load_object_basics(this);
    load_property("expression",P_STRING,expr);
    setExpression(expr);
}

void EVALOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"expression",P_STRING,expression);
}
	
void EVALOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	switch (port)
    {
    	case 0:
        	inputA = value;
            break;
        case 1:
        	inputB = value;
            break;
        case 2:
        	inputC = value;
            break;
        case 3:
        	inputD = value;
            break;
        case 4:
        	inputE = value;
            break;
        case 5:
        	inputF = value;
            break;

    }
}

void EVALOBJ::setExpression(char *expr)
{
	int len = strlen(expr);
    if (expression != NULL)
    	delete expression;
    expression = new char[len+1];
    strcpy(expression, expr);
	setexp=TRUE;
    if (evaluator != NULL)
    	evaluator_destroy(evaluator);
    evaluator = evaluator_create(expr);
	setexp=FALSE;
    if (evaluator == NULL)
    	report_error("Unable to parse expression");
}

void EVALOBJ::work(void)
{
	static char *names[] = {"A", "B", "C", "D", "E", "F"};
    static double values[NUMINPUTS];
    double result=INVALID_VALUE;

    if ((evaluator != NULL)&&(!setexp))
    {
		values[0] = inputA;
		values[1] = inputB;
		values[2] = inputC;
		values[3] = inputD;
		values[4] = inputE;
		values[5] = inputF;
	    result = evaluator_evaluate(evaluator, NUMINPUTS, names, values);
    }
    pass_values(0, (float)result);
}

EVALOBJ::~EVALOBJ()
{
	if (evaluator != NULL) evaluator_destroy(evaluator);
    if (expression != NULL) delete expression;
}

LRESULT CALLBACK EvalDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	EVALOBJ * st;
	
	st = (EVALOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_EVAL)) return(FALSE);
    
	switch( message )
	{
		case WM_INITDIALOG:
	        init = true;
	        if (st->expression != NULL)
		        SetDlgItemText(hDlg, IDC_EVALEXPRESSION, st->expression);
            init = false;
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_EVALAPPLY:
                	char strExpr[MAXEXPRLENGTH+1];
                	GetDlgItemText(hDlg, IDC_EVALEXPRESSION, strExpr, MAXEXPRLENGTH);
                    st->setExpression(strExpr);
					strncpy(st->tag, st->expression,12);
					st->tag[12]='.';st->tag[13]='.';st->tag[14]=0;
					InvalidateRect(ghWndDesign,NULL,TRUE);
                    break;
            }
			return TRUE;
			break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
	}
	return FALSE;
}
