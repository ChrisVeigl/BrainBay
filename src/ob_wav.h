/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_AND.H
  Authors: Jeremy Wilkerson, Chris Veigl


  This Object can open a Wav-Sound File and play it triggered by an input-port-value
  another input-ports allows adjustment of the playing-speed.

  currently, only one wav-player object can be opened at a time.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


#include "brainBay.h"
#include "SDL_sound.h"

#define DEF_BUFSIZE 2048

class WAVOBJ : public BASE_CL
{
	protected:
		float inputOn, inputVolume, inputSpeed;
	private:
		Sound_Sample *sample;
		SDL_AudioSpec wav_spec;
		Uint8 *wav_buffer;
		Uint32 wav_length;
		Uint32 audio_len;
		Uint8 *audio_pos;
		bool wav_loaded;
		bool playing;
		Uint32 timeLastPlayed;
		int volume;
		float lastInputOn;
		int orig_speed;
		float speed_center, speed_factor;
		float old_input_speed, old_speed_center, old_speed_factor;
		bool speed_adjusting;	
		Uint32 inputOnTime, inputVolumeTime, inputSpeedTime;
        bool volume_change, speed_change;

	public:
		char wavfilename [MAX_PATH];
		int wav_length_ms;
		int repeat_interval;
		//int times_repeat;
		//int repeats_left;
		int on_change_only, mute;
		float volume_input_from, volume_input_to;
		int volume_from, volume_to;
		unsigned int bufsize;
		int range;
		int reverse;
		
		WAVOBJ(int num);
	
		void make_dialog(void);

		void load(HANDLE hFile);

		void save(HANDLE hFile);

		void update_inports(void);
	
		void incoming_data(int port, float value);
	
		void work(void);

		void change_bufsize(unsigned int);

		~WAVOBJ();
		
		//tried making fill_audio a member function, but wouldn't compile
		friend void fill_audio(void *udata, Uint8 *stream, int len);
		friend LRESULT CALLBACK WavDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		
	private:
	
		void init(void);
		
		//reset inputs to default values if they haven't been set for time of input_reset_delay
		void checkResetInputs(Uint32 timeNow);
	
		void play(void);

		//returns length of audio buffer, or -1 if unsuccessful
		int loadWavFile(char* filename);

		void calcVolume(void);

		void convertSpeed(float ratio, Uint8 *inputPos, Uint8 *outputPos, Uint32 *inputMaxLength, Uint32 *outputMaxLength);

		//void setSpeed(void);

		float calcSpeed(void);
		
		int calcWavLength(void);
};
