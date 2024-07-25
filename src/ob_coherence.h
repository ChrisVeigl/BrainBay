/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COHERENCE.H:  declarations for the Coherence-Object
  Author: Chris Veigl

  The Coherence function between two input-streams is calculated and presented 
  at the output port.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include <math.h>
#include <fftw3.h>

#define DEFAULT_SEGMENT_SIZE 512

class COHERENCEOBJ : public BASE_CL
{
	protected:
        double new_sample1, new_sample2, coherence;
		int interval, count, samplePos, segmentStart;

		int N; // Length of the window
		int fft_size; // FFT size for real input signals

		double* signal1 = NULL;
		double* signal2 = NULL;
		double* samples1 = NULL;
		double* samples2 = NULL;
		double* window = NULL;

		fftw_complex* fft1;
		fftw_complex* fft2;

		fftw_plan p1;
		fftw_plan p2;

	public:

	COHERENCEOBJ(int num);


	double magnitude_squared(fftw_complex z);
	void apply_hanning_window(double* source, double* target);
	void generate_hanning_window();
	void update_samples(double new_sample1, double new_sample2);
	double compute_coherence();

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~COHERENCEOBJ();

	friend LRESULT CALLBACK CoherenceDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    private:
    
    void change_interval(int newinterval);
};
