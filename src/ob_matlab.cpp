/* --------------------------------------------------------------------------------
  
    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  MODULE: OB_MATLAB.CPP:  functions for the Matlab-Object
  Authors: Lester John, Chris Veigl

  this object is currently disabled because the copyright-situation regarding the
  matlab-dlls has to be clarified.
  
  this object provides a basic interface to the matlab-engine.

  the matlab-object collects data on its input-ports, transfers the buffers to matlab,
  and outputs the ANS-variable at the output-port
  2006-07 Revision 1.0: Glen Nilsen & Lester R. John, University of Cape Town
  2006-07 some Modifications to work with Matlab 7.1, Chris Veigl

----------------------------------------------------------------------------------*/

#include "brainBay.h"


#include "ob_matlab.h" 


static double inputA_buffer[MATLABBUFFERLENGTH]={0};
static double inputB_buffer[MATLABBUFFERLENGTH]={0};
static double inputC_buffer[MATLABBUFFERLENGTH]={0};
static double inputD_buffer[MATLABBUFFERLENGTH]={0};
static double inputE_buffer[MATLABBUFFERLENGTH]={0};
static double inputF_buffer[MATLABBUFFERLENGTH]={0};
static double inputG_buffer[MATLABBUFFERLENGTH]={0};
static double inputH_buffer[MATLABBUFFERLENGTH]={0};
static double inputI_buffer[MATLABBUFFERLENGTH]={0};
static double inputJ_buffer[MATLABBUFFERLENGTH]={0};
static double output_buffer[MATLABBUFFERLENGTH]={0};
static double y_VC[MATLABBUFFERLENGTH] = {0};           
static double buffer_interval[1] ={1};
static double buffer_index[1] ={1};
static int num_ports = 0;
static int current_buffer_index = 0;
static int cn=0;

MATLABOBJ::MATLABOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;

 	strcpy(out_ports[0].out_name,"out");
	strcpy(in_ports[0].in_name,"A");
	strcpy(in_ports[1].in_name,"B");
	strcpy(in_ports[2].in_name,"C");
	strcpy(in_ports[3].in_name,"D");
	strcpy(in_ports[4].in_name,"E");
	strcpy(in_ports[5].in_name,"F");
	strcpy(in_ports[6].in_name,"G");
	strcpy(in_ports[7].in_name,"H");
	strcpy(in_ports[8].in_name,"I");
	strcpy(in_ports[9].in_name,"J");

	inputA = 0;
    inputB = 0;
    inputC = 0;
    inputD = 0;
    inputE = 0;
    inputF = 0;
	inputG = 0;
    inputH = 0;
    inputI = 0;
    inputJ = 0;

	periodic=FALSE;

	setExpression(""); // set default matlab expression
	change_interval(1); // set default input buffer length
	num_ports=0;
	
#ifdef MATLAB_RELEASE
	ep = engOpen("\0");
	if (!ep) report_error("engine not open");
#else
	report_error("Matlab-Object not supported in this Version of BrainBay");
#endif
}

void MATLABOBJ::update_inports(void)
{
	int i,z,m;
	m=-1;
	for (i=0;i<GLOBAL.objects;i++)
	  for (z=0;objects[i]->out[z].to_port!=-1;z++)
	    if ((objects[objects[i]->out[z].to_object]==this)&&(objects[i]->out[z].to_port>m)) m=objects[i]->out[z].to_port;
    inports=m+2;
	num_ports=inports-1;	
	if (inports>10) inports=10;
	height=CON_START+inports*CON_HEIGHT+5;
	InvalidateRect(ghWndMain,NULL,TRUE);
}
	
void MATLABOBJ::make_dialog(void) 
{
    display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MATLABBOX, ghWndStatusbox, (DLGPROC)MatlabDlgHandler));
}

void MATLABOBJ::load(HANDLE hFile) 
{
	
	load_object_basics(this);
    load_property("matlab_call",P_STRING,matlab_expression);
	load_property("call_periodic",P_INT,&periodic);
	num_ports=inports-1;
    
}

void MATLABOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"matlab_call",P_STRING,matlab_expression);
	save_property(hFile,"call_periodic",P_INT,&periodic);
	
}
	
void MATLABOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	switch (port)
    {
    	case 0:
        	inputA = value;
			inputA_buffer[current_buffer_index] = value;
            break;
        case 1:
        	inputB = value;
			inputB_buffer[current_buffer_index] = value;
            break;
        case 2:
        	inputC = value;
			inputC_buffer[current_buffer_index] = value;
            break;
        case 3:
        	inputD = value;
			inputD_buffer[current_buffer_index] = value;
            break;
        case 4:
        	inputE = value;
			inputE_buffer[current_buffer_index] = value;
            break;
        case 5:
        	inputF = value;
			inputF_buffer[current_buffer_index] = value;
            break;
        case 6:
        	inputG = value;
			inputG_buffer[current_buffer_index] = value;
            break;
        case 7:
        	inputH = value;
			inputH_buffer[current_buffer_index] = value;
            break;
        case 8:
        	inputI = value;
			inputI_buffer[current_buffer_index] = value;
            break;
        case 9:
        	inputJ = value;
			inputJ_buffer[current_buffer_index] = value;
            break;
    }
	if (port >= num_ports) num_ports = port+1;
	buffer_index[0] = current_buffer_index+1; // for matlab i.e. double and index starting at zero

}
	

void MATLABOBJ::setExpression(char *expr)
{
	int len = strlen(expr);
    strncpy(matlab_expression, expr,len+1);
	setexp=TRUE;
}

void MATLABOBJ::transfer(void)
{
#ifdef MATLAB_RELEASE
	static char *names[] = {"A", "B", "C", "D", "E", "F", "G","H","I","J"};
    static double values[MATLABNUMINPUTS];
	float value;

	

	value = inputA;
	input=value; 
    mxArray *x = NULL, *y = NULL, *x_next = NULL, *y_new = NULL;
	mxArray *A = NULL, *B = NULL, *C = NULL, *D = NULL, *E = NULL, *F = NULL, *G = NULL, *H = NULL, *I = NULL, *J = NULL;
	mxArray *input_buffer_length = NULL, *input_buffer_index = NULL ;
	double *y_st;

    
	if (!ep) { report_error("Matlab engine not opened."); return; }
	
	y = mxCreateDoubleMatrix(interval, 1, mxREAL);  

	switch (num_ports)
	{
		case 10:
			J = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(J), (char *) inputJ_buffer, interval*sizeof(double));   
			engPutVariable(ep, "J", J); //put variable in the MATLAB workspace
		case 9:
			I = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(I), (char *) inputI_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "I", I); 
		case 8:
			H = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(H), (char *) inputH_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "H", H); 
		case 7:
			G = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(G), (char *) inputG_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "G", G); 
		case 6:
			F = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(F), (char *) inputF_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "F", F); 
		case 5:
			E = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(E), (char *) inputE_buffer, MATLABBUFFERLENGTH*sizeof(double)); 
			engPutVariable(ep, "E", E); 
		case 4:
			D = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(D), (char *) inputD_buffer, interval*sizeof(double));  
			engPutVariable(ep, "D", D); 
		case 3:
			C = mxCreateDoubleMatrix(interval, 1, mxREAL);
 			memcpy((char *) mxGetPr(C), (char *) inputC_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "C", C); 
		case 2:
			B = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(B), (char *) inputB_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "B", B); 
		case 1:
			A = mxCreateDoubleMatrix(interval, 1, mxREAL);
 			memcpy((char *) mxGetPr(A), (char *) inputA_buffer, interval*sizeof(double)); 

			if (engPutVariable(ep, "A", A))
			{
				cn++;
				SetDlgItemInt(ghWndStatusbox,IDC_STATUS,cn,0);
			}

		default:
			break;
	}

	// Also place the extra variables in the Matlab workspace
    input_buffer_length = mxCreateDoubleMatrix(1, 1, mxREAL);
	memcpy((char *) mxGetPr(input_buffer_length), buffer_interval, sizeof(double));   
	engPutVariable(ep, "input_buffer_length", input_buffer_length); 

    input_buffer_index = mxCreateDoubleMatrix(1, 1, mxREAL); 
	memcpy((char *) mxGetPr(input_buffer_index), buffer_index, sizeof(double));   
	engPutVariable(ep, "input_buffer_index", input_buffer_index); 


	// Place the string expression into the Matlab workspace 
	engEvalString(ep, matlab_expression);  
	
	// Get the results back from the Matlab workspace and maybe also store them in an output buffer
 	y = engGetVariable(ep, "ans"); //

	if (y)
	{
	    y_st = mxGetPr(y);
		// Insert the y -value in the output array
		y_VC[0] = y_st[interval-1];
		pass_values(0,(float)y_st[interval-1]); 
		update_buffer_index(); // buffer to be updated at the end

	} 
	// else report_error("ans does not exist");
	
#endif
	
}
	
void MATLABOBJ::work(void)
{

#ifdef MATLAB_RELEASE

if (periodic) 
{
	static char *names[] = {"A", "B", "C", "D", "E", "F", "G","H","I","J"};
    static double values[MATLABNUMINPUTS];
	float value;


	value = inputA;
	input=value; 
    mxArray *x = NULL, *y = NULL, *x_next = NULL, *y_new = NULL;
	mxArray *A = NULL, *B = NULL, *C = NULL, *D = NULL, *E = NULL, *F = NULL, *G = NULL, *H = NULL, *I = NULL, *J = NULL;
	mxArray *input_buffer_length = NULL, *input_buffer_index = NULL ;
	double *y_st;

    
	if (!ep) { report_error("Matlab engine not opened."); return; }
	
	y = mxCreateDoubleMatrix(interval, 1, mxREAL);  

	switch (num_ports)
	{
		case 10:
			J = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(J), (char *) inputJ_buffer, interval*sizeof(double));   
			engPutVariable(ep, "J", J); //put variable in the MATLAB workspace
		case 9:
			I = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(I), (char *) inputI_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "I", I); 
		case 8:
			H = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(H), (char *) inputH_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "H", H); 
		case 7:
			G = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(G), (char *) inputG_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "G", G); 
		case 6:
			F = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(F), (char *) inputF_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "F", F); 
		case 5:
			E = mxCreateDoubleMatrix(interval, 1, mxREAL);
		 	memcpy((char *) mxGetPr(E), (char *) inputE_buffer, MATLABBUFFERLENGTH*sizeof(double)); 
			engPutVariable(ep, "E", E); 
		case 4:
			D = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(D), (char *) inputD_buffer, interval*sizeof(double));  
			engPutVariable(ep, "D", D); 
		case 3:
			C = mxCreateDoubleMatrix(interval, 1, mxREAL);
 			memcpy((char *) mxGetPr(C), (char *) inputC_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "C", C); 
		case 2:
			B = mxCreateDoubleMatrix(interval, 1, mxREAL);
			memcpy((char *) mxGetPr(B), (char *) inputB_buffer, interval*sizeof(double)); 
			engPutVariable(ep, "B", B); 
		case 1:
			A = mxCreateDoubleMatrix(interval, 1, mxREAL);
 			memcpy((char *) mxGetPr(A), (char *) inputA_buffer, interval*sizeof(double)); 
			if (engPutVariable(ep, "A", A))
			{
				cn++;
				SetDlgItemInt(ghWndStatusbox,IDC_STATUS,cn,0);
			}

		default:
			break;
	}

	// Also place the extra variables in the Matlab workspace
    input_buffer_length = mxCreateDoubleMatrix(1, 1, mxREAL);
	memcpy((char *) mxGetPr(input_buffer_length), buffer_interval, sizeof(double));   
	engPutVariable(ep, "input_buffer_length", input_buffer_length); 

    input_buffer_index = mxCreateDoubleMatrix(1, 1, mxREAL); 
	memcpy((char *) mxGetPr(input_buffer_index), buffer_index, sizeof(double));   
	engPutVariable(ep, "input_buffer_index", input_buffer_index); 


	// Place the string expression into the Matlab workspace 
	engEvalString(ep, matlab_expression);  
	
	// Get the results back from the Matlab workspace and maybe also store them in an output buffer
 	y = engGetVariable(ep, "ans"); //

	if (y)
	{
	    y_st = mxGetPr(y);
		// Insert the y -value in the output array
		y_VC[0] = y_st[interval-1];
		pass_values(0,(float)y_st[interval-1]); 
		update_buffer_index(); // buffer to be updated at the end

	} 
	// else report_error("ans does not exist");
	
	}
#endif
}

void MATLABOBJ::change_interval(int newinterval)
{
	interval = newinterval;
	buffer_interval[0] = newinterval; 
	added = 0;
	accumulator = 0;
}

// Internal data buffer - shift buffer down by one element
void MATLABOBJ::shift_buffer_down(void) 
{
	int loop;
	for (loop=0;loop<interval-1;loop++)
	{
		switch (num_ports)
		{
			case 10:
				inputJ_buffer[loop] = inputJ_buffer[loop+1];
			case 9:
				inputI_buffer[loop] = inputI_buffer[loop+1];
			case 8:
				inputH_buffer[loop] = inputH_buffer[loop+1];
			case 7:
				inputG_buffer[loop] = inputG_buffer[loop+1];
			case 6:
				inputF_buffer[loop] = inputF_buffer[loop+1];
			case 5:
				inputE_buffer[loop] = inputE_buffer[loop+1];
			case 4:
				inputD_buffer[loop] = inputD_buffer[loop+1];
			case 3:
				inputC_buffer[loop] = inputC_buffer[loop+1];
			case 2:
				inputB_buffer[loop] = inputB_buffer[loop+1];
			case 1:
				inputA_buffer[loop] = inputA_buffer[loop+1];
			default:
				break;
		}
	}

}


// Internal data buffer 
void MATLABOBJ::update_buffer_index(void) 
{
	current_buffer_index++;					// increment buffer index
	if (current_buffer_index >= interval)
	{	
		shift_buffer_down();
		current_buffer_index = interval-1; // clamp the current index at the top of buffer
	}
}


MATLABOBJ::~MATLABOBJ() {}

LRESULT CALLBACK MatlabDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	MATLABOBJ * st;
	st = (MATLABOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_MATLAB)) return(FALSE);
	    
	switch( message )
	{
		case WM_INITDIALOG:
			SCROLLINFO lpsi;
			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin = 1; lpsi.nMax=MATLABBUFFERLENGTH - 1;
			SetScrollInfo(GetDlgItem(hDlg,IDC_MATLABINTERVALBAR),SB_CTL,&lpsi, TRUE);
			init = true;
			SetScrollPos(GetDlgItem(hDlg,IDC_MATLABINTERVALBAR), SB_CTL,st->interval, TRUE);
			SetDlgItemInt(hDlg, IDC_MATLABINTERVAL, st->interval, FALSE);
			init = false;

			init = true;
	        if (st->matlab_expression != NULL)
		        SetDlgItemText(hDlg, IDC_MATLABEXPRESSION, st->matlab_expression);
			else
				SetDlgItemText(hDlg, IDC_MATLABEXPRESSION, "");
            init = false;
			CheckDlgButton(hDlg,IDC_CALLPERIODIC,st->periodic);
			break;
			
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_MATLABAPPLY:
					GetDlgItemText(hDlg, IDC_MATLABEXPRESSION, st->matlab_expression, MAXEXPRLENGTH);
                    break;
				case IDC_MATLABTRANSFER:
					st->transfer();
                    break;
				case IDC_CALLPERIODIC:
					st->periodic=IsDlgButtonChecked(hDlg,IDC_CALLPERIODIC);
					break;

            }
			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if (!init && (nNewPos = get_scrollpos(wParam,lParam)) >= 0)
			{   
				if (lParam == (long) GetDlgItem(hDlg,IDC_MATLABINTERVALBAR))  
				{
					SetDlgItemInt(hDlg, IDC_MATLABINTERVAL, nNewPos, TRUE);
                    st->change_interval(nNewPos);
				}
			}
			break;
		} 
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
	}
	return FALSE;
}

