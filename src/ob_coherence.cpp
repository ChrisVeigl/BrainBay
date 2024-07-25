/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COHERENCE.CPP:  functions for the Coherence-Object
  Author: Chris Veigl

  The Coherence function between two input-streams is calculated and presented 
  at the output port.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_coherence.h"

using namespace std;


// Function to generate a Hanning window
void COHERENCEOBJ::generate_hanning_window() {
    for (int i = 0; i < N; i++) {
        window[i] = 0.5 * (1.0 - cos(2.0 * DDC_PI * (double)i / (double)(N - 1)));
    }
}

// Function to compute the magnitude squared of a complex number
double COHERENCEOBJ::magnitude_squared(fftw_complex z) {
    return z[0] * z[0] + z[1] * z[1];
}

// Function to copy N samples from startPos of a sample buffer to a target buffer, applying the hanning window
void COHERENCEOBJ::apply_hanning_window(double* source, double* target) {
    int sourcePos = segmentStart;
    for (int i = 0; i < N; i++) {
        target[i] = source[sourcePos] * window[i];
        sourcePos = (sourcePos + 1 ) % N;
    }
}

// Function to update signals with new data and compute coherence
double COHERENCEOBJ::compute_coherence() {

    apply_hanning_window(samples1, signal1);
    apply_hanning_window(samples2, signal2);

    fftw_execute(p1); // FFT of windowed signal1
    fftw_execute(p2); // FFT of windowed signal2
    double avg_coherence = 0.0;
    fftw_complex cross_spectrum;

    // Compute cross power spectral density, auto power spectral densities, and average coherence
    for (int i = 0; i < fft_size; i++) {
        cross_spectrum[0] = fft1[i][0] * fft2[i][0] + fft1[i][1] * fft2[i][1];
        cross_spectrum[1] = fft1[i][1] * fft2[i][0] - fft1[i][0] * fft2[i][1];
        double mag_squared_cross_spectrum = magnitude_squared(cross_spectrum);
        double auto_spectrum1 = magnitude_squared(fft1[i]);
        double auto_spectrum2 = magnitude_squared(fft2[i]);

        printf("mag = %f, prod = %f\n", mag_squared_cross_spectrum, auto_spectrum1 * auto_spectrum2);        
        avg_coherence += mag_squared_cross_spectrum / (auto_spectrum1 * auto_spectrum2);
    }
    return avg_coherence / fft_size;
}


COHERENCEOBJ::COHERENCEOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
	width=75;
    strcpy(out_ports[0].out_name, "out");
    out_ports[0].get_range = -1;
    strcpy(out_ports[0].out_dim, "none");
    strcpy(out_ports[0].out_desc, "Coherence");
    out_ports[0].out_max = 1.0f;
    out_ports[0].out_min = -1.0f;

    interval = 10;
    count = 0;
    samplePos = 0;
    segmentStart = 0;
    coherence = 0.0;

    N = DEFAULT_SEGMENT_SIZE; // Length of the window
    fft_size = N / 2 + 1; // FFT size for real input signals

    // Allocate memory for signals, window, and FFTW resources
    signal1 = (double*)calloc(N, sizeof(double));
    signal2 = (double*)calloc(N, sizeof(double));
    samples1 = (double*)calloc(N, sizeof(double));
    samples2 = (double*)calloc(N, sizeof(double));
    window  = (double*)calloc(N, sizeof(double));
    generate_hanning_window();

    fft1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fft_size);
    fft2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fft_size);
    
    p1 = fftw_plan_dft_r2c_1d(N, signal1, fft1, FFTW_ESTIMATE);
    p2 = fftw_plan_dft_r2c_1d(N, signal2, fft2, FFTW_ESTIMATE);


}
	
void COHERENCEOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_COHERENCEBOX, ghWndStatusbox, (DLGPROC)CoherenceDlgHandler));
}

void COHERENCEOBJ::load(HANDLE hFile)
{
	load_object_basics(this);
    load_property("interval",P_INT,&interval);
}

void COHERENCEOBJ::save(HANDLE hFile)
{
	save_object_basics(hFile, this);
    save_property(hFile,"interval",P_INT,&interval);
}
	
  	
void COHERENCEOBJ::incoming_data(int port, float value)
{
	switch (port)
    {
    	case 0:
            samples1[samplePos] = value;
        	break;
        case 1:
            samples2[samplePos] = value;
            break;
        default:
        	return;
     }
}
	
void COHERENCEOBJ::work(void)
{
    samplePos = (samplePos+1) % N;
    count++;
    if (count >= interval) {
        coherence = compute_coherence();
        count = 0;
        segmentStart = (segmentStart + interval) % N;
    }
    pass_values(0, coherence);
}

void COHERENCEOBJ::change_interval(int newinterval)
{
    interval = newinterval;
}

COHERENCEOBJ::~COHERENCEOBJ() {
    // Free allocated memory
    fftw_destroy_plan(p1);
    fftw_destroy_plan(p2);
    fftw_free(fft1);
    fftw_free(fft1);
    free(samples1);
    free(samples2);
    free(signal1);
    free(signal2);
    free(window);
}

LRESULT CALLBACK CoherenceDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
    COHERENCEOBJ * st;
	
	st = (COHERENCEOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_COHERENCE)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:

				SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=1; lpsi.nMax=DEFAULT_SEGMENT_SIZE;
				SetScrollInfo(GetDlgItem(hDlg,IDC_COHERENCEINTERVALBAR),SB_CTL,&lpsi, TRUE);
				SetDlgItemText(hDlg, IDC_TAG, st->tag);
				
				init = true;

				SetScrollPos(GetDlgItem(hDlg,IDC_COHERENCEINTERVALBAR), SB_CTL,st->interval, TRUE);
				SetDlgItemInt(hDlg, IDC_COHERENCEINTERVAL, st->interval, FALSE);
                
				init = false;
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
//			switch (LOWORD(wParam))
//			{
//			}
//			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if (!init && (nNewPos = get_scrollpos(wParam,lParam)) >= 0)
			{   
				if (lParam == (long) GetDlgItem(hDlg,IDC_COHERENCEINTERVALBAR))
				{
					SetDlgItemInt(hDlg, IDC_COHERENCEINTERVAL, nNewPos, TRUE);
                    st->change_interval(nNewPos);
				}
			}
			break;
		}
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}




