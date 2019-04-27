/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_WAV.CPP
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
#include "OB_WAV.h"
#include <cstring>
#include <iostream>
#include <math.h>

#define defaultInputOn  TRUE_VALUE
//#define defaultInputVolume  1023.0
//#define defaultInputSpeed 512.0
#define defaultInputVolume  512.0
#define defaultInputSpeed 0.0
#define input_reset_delay  1000
#define max_playback_speed 200000   //these values might not be the same for
#define min_playback_speed 100      //all systems
#define LOW 0			   //for range
#define HIGH 1
#define BOTH 2
#define AUDIOPERIOD 20000

using namespace std;

LRESULT CALLBACK WavDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void fill_audio(void *udata, Uint8 *stream, int len);

WAVOBJ::WAVOBJ(int num) : BASE_CL()
{
	outports = 0;
	inports = 3;
	strcpy(in_ports[0].in_name,"on");
	strcpy(in_ports[1].in_name,"vol");
	strcpy(in_ports[2].in_name,"speed");
	inputOn = defaultInputOn;
	inputVolume = defaultInputVolume;
	inputSpeed = 0.0;
	lastInputOn = INVALID_VALUE;
	timeLastPlayed = 0;
	inputOnTime = 0;
	inputVolumeTime = 0;
	inputSpeedTime = 0;
	strcpy(wavfilename, "");
	wav_length_ms = 0;
	repeat_interval = 0;
	on_change_only = false;
	volume_input_from = 0.0f;
	volume_input_to = 1023.0f;
	volume_from = 0;
	volume_to = SDL_MIX_MAXVOLUME;
    volume = SDL_MIX_MAXVOLUME;
//	speed_center = 512.0;
	speed_center = 0.0;
	speed_factor = 0.006f;
	bufsize=DEF_BUFSIZE;
	old_input_speed = 0.0f;
	old_speed_center = 0.0f;
	old_speed_factor = 0.0f;
	speed_adjusting = false;
	range = BOTH;
	reverse = 0;
	mute = true;	
	wav_loaded = false;
	sample=NULL;
	init();
}


void WAVOBJ::init(void)
{/*
	if(SDL_WasInit(SDL_INIT_AUDIO | SDL_INIT_TIMER) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
			report_error("Couldn't init SDL");
	}*/
//	Sound_Init();
	
}

void WAVOBJ::change_bufsize(unsigned int newbuf) 
{

	if (bufsize == newbuf) return;

	bufsize=newbuf;
	
	SDL_PauseAudio(1);
	//if (wav_loaded)	
	SDL_CloseAudio();

	wav_spec.samples = bufsize;
	
	if (SDL_OpenAudio(&wav_spec, NULL) < 0)
	{
		Sound_FreeSample(sample);
		wav_loaded = false;
		//strcpy(wavfilename, "");
        wav_length_ms = 0;
//		SDL_CloseAudio();
		report_error("Could not open SDL_Audio");
	}
	else SDL_PauseAudio(0);

}

void WAVOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_WAVBOX, ghWndStatusbox, (DLGPROC)WavDlgHandler));
}

void WAVOBJ::load(HANDLE hFile) 
{
	int buf;

	load_object_basics(this);

	load_property("bufsize", P_INT, &buf);    bufsize=(unsigned int) buf;
	load_property("filename", P_STRING, wavfilename);
    
	load_property("repeatinterval", P_INT, &repeat_interval);
	load_property("onchangeonly", P_INT, &on_change_only);
	load_property("mute", P_INT, &mute);
	load_property("volumeinputfrom", P_FLOAT, &volume_input_from);
	load_property("volumeinputto", P_FLOAT, &volume_input_to);
	load_property("volumefrom", P_INT, &volume_from);
	load_property("volumeto", P_INT, &volume_to);
	load_property("speedcenter", P_FLOAT, &speed_center);
	load_property("speedfactor", P_FLOAT, &speed_factor);
	load_property("pitchrange", P_INT, &range);
	load_property("reverse", P_INT, &reverse);    
	if (strlen(wavfilename) > 0)
	{
		wav_length_ms = loadWavFile(wavfilename);
        if (wav_length_ms < 0)
        {
        	wav_length_ms = 0;
        	report_error("Unable to load audio file");
            cout << SDL_GetError() << endl;
            cout << Sound_GetError() << endl;
        }
		
    }
}

void WAVOBJ::save(HANDLE hFile) 
{
	int buf= (int) bufsize;

	save_object_basics(hFile, this);
	save_property(hFile, "bufsize", P_INT, &buf);
	save_property(hFile, "filename", P_STRING, wavfilename);

	save_property(hFile, "repeatinterval", P_INT, &repeat_interval);
	save_property(hFile, "onchangeonly", P_INT, &on_change_only);
	save_property(hFile, "mute", P_INT, &mute);
	save_property(hFile, "volumeinputfrom", P_FLOAT, &volume_input_from);
	save_property(hFile, "volumeinputto", P_FLOAT, &volume_input_to);
	save_property(hFile, "volumefrom", P_INT, &volume_from);
	save_property(hFile, "volumeto", P_INT, &volume_to);
	save_property(hFile, "speedcenter", P_FLOAT, &speed_center);
	save_property(hFile, "speedfactor", P_FLOAT, &speed_factor);
	save_property(hFile, "pitchrange", P_INT, &range);
	save_property(hFile, "reverse", P_INT, &reverse);
}
	
void WAVOBJ::update_inports(void)
{
	volume_input_from = in_ports[1].in_min;
	volume_input_to = in_ports[1].in_max;
	speed_center = (in_ports[2].in_min+in_ports[2].in_max)/2;
	speed_factor = 6.144f/(in_ports[2].in_max-in_ports[2].in_min);
}

void WAVOBJ::incoming_data(int port, float value)
{
	if (port == 0)
	{
		inputOn = value;
		inputOnTime = SDL_GetTicks();
	}
	else if (port == 1)
	{
		inputVolume = value;
		inputVolumeTime = SDL_GetTicks();
        volume_change = true;
	}
	else if (port == 2)
	{
		inputSpeed = value;
		inputSpeedTime = SDL_GetTicks();
		speed_adjusting = true;
        speed_change = true;
	}
}
	
void WAVOBJ::work(void)
{
	Uint32 timeNow = SDL_GetTicks();

	checkResetInputs(timeNow);
    
	if (wav_loaded && !mute && !playing && (inputOn != INVALID_VALUE) &&(inputSpeed!=INVALID_VALUE))
	{
		
		if ((timeNow - timeLastPlayed) >= (unsigned int)repeat_interval)
		{
			if (on_change_only)
			{
				if (lastInputOn == INVALID_VALUE)
					play();
			}
			else
				play();
		}
	}
	lastInputOn = inputOn;
}
	
//reset inputs to default values if they haven't been set for time of input_reset_delay
void WAVOBJ::checkResetInputs(Uint32 timeNow)
{
	if ((timeNow - inputOnTime) > input_reset_delay)
		inputOn = defaultInputOn;
	if ((timeNow - inputVolumeTime) > input_reset_delay)
		inputVolume = volume_input_to;
	if ((timeNow - inputSpeedTime) > input_reset_delay)
		speed_adjusting = false;
}

//returns length of wav file (in ms), or -1 if unsuccessful
int WAVOBJ::loadWavFile(char* filename)
{
	int sav;

	sav=mute;
	mute=true;
	strcpy(wavfilename, filename);
	

	//if (playing) { 	
		SDL_PauseAudio(1); SDL_LockAudio(); 
	//}

	//if (wav_loaded)
	//{
		SDL_CloseAudio();
		if (sample)
			Sound_FreeSample(sample);
	//}
	char tempfilename[255];
	HANDLE test=0;
	strcpy(tempfilename,filename);
	test=CreateFile(tempfilename, GENERIC_READ , 0, NULL, OPEN_EXISTING, 0,NULL);
	if (test==INVALID_HANDLE_VALUE)
	{
		strcpy(tempfilename,GLOBAL.resourcepath);
		strcat(tempfilename,filename);
	}
	else CloseHandle(test);
	strcpy (filename, tempfilename);

	sample = Sound_NewSampleFromFile(filename, NULL, 32000);
	if (sample == NULL)
	{
		SDL_CloseAudio();
		wav_loaded = false;
		// strcpy(wavfilename, "");
        wav_length_ms = 0;
		return -1;
	}
	wav_length = Sound_DecodeAll(sample);
	wav_buffer = (Uint8*)sample->buffer;
	wav_spec.format = sample->actual.format;
	wav_spec.channels = sample->actual.channels;
	wav_spec.freq = sample->actual.rate;
	orig_speed = wav_spec.freq;
	wav_spec.samples = bufsize;
	wav_spec.callback = fill_audio;
	wav_spec.userdata = this;
	
	if (SDL_OpenAudio(&wav_spec, NULL) < 0)
	{
		SDL_CloseAudio();
		Sound_FreeSample(sample);
		wav_loaded = false;
		// strcpy(wavfilename, "");
        wav_length_ms = 0;
		return -1;
	}

	wav_loaded = true;
	wav_length_ms = calcWavLength();


    
	if (playing) {  SDL_UnlockAudio(); SDL_PauseAudio(0);}

//	mute=sav;
	
	return wav_length_ms;
}

//returns length of loaded wav file, in ms
int WAVOBJ::calcWavLength(void)
{
	float samples = (float)wav_length / (float)wav_spec.channels;
	if ((wav_spec.format != AUDIO_U8) && (wav_spec.format != AUDIO_S8))  //then 16 bytes per sample
		samples /= 2.0;
	return (int)((samples / (float)wav_spec.freq) * 1000.0);
}

void WAVOBJ::play(void)
{
	SDL_LockAudio();
	playing = true;
	audio_len = wav_length;
	audio_pos = wav_buffer;
	
	//calcVolume();
	
	SDL_PauseAudio(0);

	timeLastPlayed = SDL_GetTicks();

	//SDL_PauseAudio(1);
	SDL_UnlockAudio();
}

void WAVOBJ::calcVolume(void)
{
	if (inputVolume <= volume_input_from)
		volume = volume_from;
	else if (inputVolume >= volume_input_to)
		volume = volume_to;
	else
	{
		float r = (inputVolume - volume_input_from) / (volume_input_to - volume_input_from);
		volume = (int)((r * (volume_to - volume_from) + volume_from));
	}
}

void WAVOBJ::convertSpeed(float ratio, Uint8 *inputPos, Uint8 *outputPos, Uint32 *inputLength, Uint32 *outputLength)
{
    
   	int numChannels = wav_spec.channels;
    int bytesPerSample;
	if ((!inputPos) || (!outputPos)) return;
    if ((wav_spec.format == AUDIO_U8) || (wav_spec.format == AUDIO_S8))
      	bytesPerSample = 1;
    else bytesPerSample = 2;
    float ceiling = ceil(ratio);

    ratio /= ceiling;
   	int numBytes = bytesPerSample * numChannels;

   	float total = 0.0;
    float oldFloor = 0;
    unsigned int l, i;
    for (i = 0, l = 0; (i < *inputLength) && (l < *outputLength); i += numBytes)
    {
      	for (unsigned int j = 0; j < ceiling && (l < *outputLength); j++)
        {
        	total += ratio;
            float newFloor = floor(total);
            if (newFloor > oldFloor)
            {
               	oldFloor = newFloor;
                for (int k = 0; (k < numBytes) && (l < (int)(*outputLength)); k++)
                {
                    outputPos[l++] = inputPos[i+k];
                }
            }
        }
    }
    *outputLength = l;
    *inputLength = i;
}

float WAVOBJ::calcSpeed(void)
{
 	if ((range == LOW) && (inputSpeed >= speed_center))
		return 1.0;
	else if ((range == HIGH) && (inputSpeed <= speed_center))
		return 1.0;

	float exponent = speed_factor * (inputSpeed - speed_center);
	if (reverse)
	   exponent = -exponent;
	float ratio = pow(2.0f, exponent);
	return ratio;
}

WAVOBJ::~WAVOBJ()
{
	mute=true;
//	SDL_LockAudio();
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	if (wav_loaded)	Sound_FreeSample(sample);
}

//tried making fill_audio a member function, but wouldn't compile
void fill_audio(void *udata, Uint8 *stream, int len)
{
    static Uint8 converted_data[AUDIOPERIOD];
 
	WAVOBJ * obj = (WAVOBJ *)udata;


	if (obj->mute)
	{
		obj->playing = false;
		SDL_PauseAudio(1);
		return;
	}

	/* Only play if we have data left */
	if (obj->audio_len == 0)
	{
		obj->playing = false;
		SDL_PauseAudio(1);
		return;
	}

    
    if (obj->volume_change)
    {
    	obj->calcVolume();
        obj->volume_change = false;
    }
    if (obj->speed_adjusting)
    {
    	float ratio = obj->calcSpeed();
	    Uint32 input_length = obj->audio_len;
    	Uint32 output_length = (len < AUDIOPERIOD)?len:AUDIOPERIOD;

    	obj->convertSpeed(ratio, obj->audio_pos, converted_data, &input_length, &output_length);
        
        /*if (output_length > len)
        {
        	report_error("Invalid audio buffer length");
            obj->playing = false;
            SDL_PauseAudio(1);
            return;
        }*/
		SDL_MixAudio(stream, converted_data, output_length, obj->volume);
        obj->audio_pos += input_length;
        obj->audio_len -= input_length;
    }
	else
    {
    	len = ( len > (int)(obj->audio_len) ? (int)obj->audio_len : len );
		SDL_MixAudio(stream, obj->audio_pos, len, obj->volume);
		obj->audio_pos += len;
		obj->audio_len -= len;
    }
	
}

LRESULT CALLBACK WavDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	WAVOBJ * st;
    char strFloat[21];
	
	st = (WAVOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_WAV)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:

				SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=0; lpsi.nMax=SDL_MIX_MAXVOLUME;
				SetScrollInfo(GetDlgItem(hDlg,IDC_WAVVOLUMEFROMBAR),SB_CTL,&lpsi, TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_WAVVOLUMETOBAR),SB_CTL,&lpsi, TRUE);				
				
				init = true;
				SetDlgItemText(hDlg, IDC_WAVFILENAME, st->wavfilename);
				SetDlgItemInt(hDlg, IDC_WAVLENGTH, st->wav_length_ms, FALSE);

				SetDlgItemInt(hDlg, IDC_WAVREPINTERVAL, st->repeat_interval, FALSE);
				CheckDlgButton(hDlg, IDC_WAVCHANGESONLY, st->on_change_only);

				SetDlgItemInt(hDlg, IDC_WAVBUFSIZE, st->bufsize, FALSE);


				sprintf(strFloat, "%.1f", st->volume_input_from);
				SetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTFROM, strFloat);
				sprintf(strFloat, "%.1f", st->volume_input_to);
				SetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTTO, strFloat);

				SetScrollPos(GetDlgItem(hDlg,IDC_WAVVOLUMEFROMBAR), SB_CTL,st->volume_from, TRUE);
				SetDlgItemInt(hDlg, IDC_WAVVOLUMEFROM, st->volume_from, FALSE);
				SetScrollPos(GetDlgItem(hDlg,IDC_WAVVOLUMETOBAR), SB_CTL,st->volume_to, TRUE);
				SetDlgItemInt(hDlg, IDC_WAVVOLUMETO, st->volume_to, FALSE);
				
				sprintf(strFloat, "%.1f", st->speed_center);
				SetDlgItemText(hDlg, IDC_WAVSPEEDCENTER, strFloat);

				sprintf(strFloat, "%.4f", st->speed_factor*10000.0f);
				SetDlgItemText(hDlg, IDC_WAVSPEEDFACTOR, strFloat);
				
				CheckDlgButton(hDlg, IDC_WAVMUTE, st->mute);
				
				CheckDlgButton(hDlg, IDC_WAVLOW, st->range == LOW);
				CheckDlgButton(hDlg, IDC_WAVHIGH, st->range == HIGH);
				CheckDlgButton(hDlg, IDC_WAVLOWHIGH, st->range == BOTH);
				
				CheckDlgButton(hDlg, IDC_WAVREVERSE, st->reverse);
				init = false;
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_WAVFILENAME:
					GetDlgItemText(hDlg,IDC_WAVFILENAME,st->wavfilename,255);
					break;
				case IDC_OPENWAVFILE:
					char filename [MAX_PATH];
					int saved;
					saved=st->mute;
					ZeroMemory(filename, MAX_PATH);
					if (open_file_dlg(hDlg, filename, FT_WAV, OPEN_LOAD))
					{
						int length;
						if ((length = st->loadWavFile(filename)) < 0)
						{
							SetDlgItemInt(hDlg, IDC_WAVLENGTH, 0, FALSE);
							report_error("Unable to load audio file");
							report_error(SDL_GetError());
						   // cout << SDL_GetError() << endl;
						   // cout << Sound_GetError() << endl;
						}
						else
						{
							SetDlgItemText(hDlg, IDC_WAVFILENAME, filename);
							SetDlgItemInt(hDlg, IDC_WAVLENGTH, length, FALSE);
						}
					}
					st->mute = saved;
					break;
				case IDC_WAVREPINTERVAL:
					if (HIWORD(wParam) == EN_KILLFOCUS)
                    	st->repeat_interval = GetDlgItemInt(hDlg, IDC_WAVREPINTERVAL, NULL, FALSE);
					break;
				case IDC_WAVCHANGESONLY:
					st->on_change_only = IsDlgButtonChecked(hDlg, IDC_WAVCHANGESONLY);
					break;
				case IDC_WAVVOLUMEINPUTFROM:
                	if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTFROM, strFloat, 20);
						st->volume_input_from = (float)atof(strFloat);
	                    if (!init)
	                    {
							sprintf(strFloat, "%.1f", st->volume_input_from);
							SetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTFROM, strFloat);
	                    }
                        st->volume_change = true;
                    }
					break;
				case IDC_WAVVOLUMEINPUTTO:
                	if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTTO, strFloat, 20);
						st->volume_input_to = (float)atof(strFloat);
	                    if (!init)
	                    {
							sprintf(strFloat, "%.1f", st->volume_input_to);
							SetDlgItemText(hDlg, IDC_WAVVOLUMEINPUTTO, strFloat);
	                    }
                        st->volume_change = true;
                    }
					break;
				case IDC_WAVSPEEDCENTER:
                	if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_WAVSPEEDCENTER, strFloat, 20);
						st->speed_center = (float)atof(strFloat);
	                    if (!init)
	                    {
							sprintf(strFloat, "%.1f", st->speed_center);
							SetDlgItemText(hDlg, IDC_WAVSPEEDCENTER, strFloat);
	                    }
                        st->speed_change = true;
                    }
					break;
				case IDC_WAVSPEEDFACTOR:
                	if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						GetDlgItemText(hDlg, IDC_WAVSPEEDFACTOR, strFloat, 20);
						st->speed_factor = (float)atof(strFloat) / 10000.0f;
	                    if (!init)
	                    {
							sprintf(strFloat, "%.4f", st->speed_factor*10000.0f);
							SetDlgItemText(hDlg, IDC_WAVSPEEDFACTOR, strFloat);
	                    }
                        st->speed_change = true;
                    }
					break;
				case IDC_WAVBUFSIZE:
                	if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
						unsigned int temp;
						temp = GetDlgItemInt(hDlg, IDC_WAVBUFSIZE, NULL, FALSE);
						if ((temp>128)&&(temp<65536)) st->change_bufsize(temp);
						SetDlgItemInt(hDlg,IDC_WAVBUFSIZE,st->bufsize,0);
                    }
					break;
				case IDC_WAVMUTE:
					st->mute = IsDlgButtonChecked(hDlg, IDC_WAVMUTE);
					break;
				case IDC_WAVLOW:
					 st->range = LOW;
					 break;
				case IDC_WAVHIGH:
					 st->range = HIGH;
					 break;
				case IDC_WAVLOWHIGH:
					 st->range = BOTH;
					 break;
				case IDC_WAVREVERSE:
					st->reverse = IsDlgButtonChecked(hDlg, IDC_WAVREVERSE);
                    if (st->reverse)
					{
                    	SetDlgItemText(hDlg, IDC_WAVLOW, "high");
                    	SetDlgItemText(hDlg, IDC_WAVHIGH, "low");
                    }
                    else
					{
                    	SetDlgItemText(hDlg, IDC_WAVLOW, "low");
                    	SetDlgItemText(hDlg, IDC_WAVHIGH, "high");
                    }
					break; 
			}
			
			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if (!init && (nNewPos = get_scrollpos(wParam,lParam)) >= 0)
			{   
				if (lParam == (long) GetDlgItem(hDlg,IDC_WAVVOLUMEFROMBAR))  
				{
					SetDlgItemInt(hDlg, IDC_WAVVOLUMEFROM, nNewPos, TRUE);
					st->volume_from = nNewPos;
					st->volume_change = true;
				}
				if (lParam == (long) GetDlgItem(hDlg,IDC_WAVVOLUMETOBAR))  
				{  
					SetDlgItemInt(hDlg, IDC_WAVVOLUMETO, nNewPos, TRUE);
					st->volume_to = nNewPos;
					st->volume_change = true;
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
