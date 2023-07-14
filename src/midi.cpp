/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: MIDI.CPP:  contains error-reporting and -handling  functions 

  midi_open_port:  opens a specified Midi Output-Port
  init_midi:  inits Instrument/Controller Names, opens all available midi-out- Ports
  midi_Instrument: Selects the midi-instrument for a given channel
  midi_ControlChange: sends a control-change - message on a given channel
  midi_NoteOn: plays a tone with given height, volume on a given channel
  midi_NoteOff: mutes a tone


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

 --------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_midi.h"


char midi_instnames[256][30]= {
{"Acoustic Grand Piano"},
{"Bright Acoustic Piano"},
{"Electric Grand Piano"},
{"Honky-tonk Piano"},
{"Electric Piano 1"},
{"Electric Piano 2"},
{"Harpsichord"},
{"Clavi"},
{"Celesta"},
{"Glockenspiel"},
{"Music Box"},
{"Vibraphone"},
{"Marimba"},
{"Xylophone"},
{"Tubular Bells"},
{"Dulcimer"},
{"Drawbar Organ"},
{"Percussive Organ"},
{"Rock Organ"},
{"Church Organ"},
{"Reed Organ"},
{"Accordion"},
{"Harmonica"},
{"Tango Accordion"},
{"Acoustic Guitar (Nylon)"},
{"Acoustic Guitar (Steel)"},
{"Electric Guitar (Jazz)"},
{"Electric Guitar (Clean)"},
{"Electric Guitar (Muted)"},
{"Overdriven Guitar"},
{"Distortion Guitar"},
{"Guitar Harmonics"},
{"Acoustic bass"},
{"Electric Bass (Finger)"},
{"Electric Bass (Pick)"},
{"Fretless Bass"},
{"Slap Bass 1"},
{"Slap Bass 2"},
{"Synth Bass 1"},
{"Synth Bass 2"},
{"Violin"},
{"Viola"},
{"Cello"},
{"Contrabass"},
{"Tremolo Strings"},
{"Pizzicato Strings"},
{"Orchestral Harp"},
{"Timpani"},
{"String Ensemble 1"},
{"String Ensemble 2"},
{"Synth Strings 1"},
{"Synth Strings 2"},
{"Choir Aahs"},
{"Voice Oohs"},
{"Synth Voice"},
{"Orchestra Hit"},
{"Trumpet"},
{"Trumbone"},
{"Tuba"},
{"Muted Trumpet"},
{"French Horn"},
{"Brass Section"},
{"Synth Brass 1"},
{"Synth Brass 2"},
{"Soprano Sax"},
{"Alto Sax"},
{"Tenor Sax"},
{"Baritone Sax"},
{"Oboe"},
{"English Horn"},
{"Bassoon"},
{"Clarinet"},
{"Piccolo"},
{"Flute"},
{"Recorder"},
{"Pan Flute"},
{"Blown Bottle"},
{"Shakuhachi"},
{"Whistle"},
{"Ocarina"},
{"Lead 1 (Square)"},
{"Lead 2 (Sawtooth)"},
{"Lead 3 (Calliope)"},
{"Lead 4 (Chiff)"},
{"Lead 5 (Charang)"},
{"Lead 6 (Voice)"},
{"Lead 7 (Fifths)"},
{"Lead 8 (Bass + Lead)"},
{"Pad 1 (New Age)"},
{"Pad 2 (Warm)"},
{"Pad 3 (Polysynth)"},
{"Pad 4 (Choir)"},
{"Pad 5 (Bowed)"},
{"Pad 6 (Metallic)"},
{"Pad 7 (Halo)"},
{"Pad 8 (Sweep)"},
{"FX 1 (Rain)"},
{"FX 2 (Soundtrack)"},
{"FX 3 (Crystal)"},
{"FX 4 (Atmosphere)"},
{"FX 5 (Brightness)"},
{"FX 6 (Goblins)"},
{"FX  7 (Echoes)"},
{"FX 8 (Sci-fi)"},
{"Sitar"},
{"Banjo"},
{"Shamisen"},
{"Koto"},
{"Kalimba"},
{"Bagpipes"},
{"Fiddle"},
{"Shanai"},
{"Tinkle Bell"},
{"Agogo"},
{"Steel Drums"},
{"Wood Block"},
{"Taiko Drum"},
{"Melodic Drum"},
{"Synth Drum"},
{"Reverse Cymbal"},
{"Guitar Fret Noise"},
{"Breath Noise"},
{"Seashore"},
{"Bird Tweet"},
{"Telephone Ring"},
{"Helicopter"},
{"Applause"},
{"Gunshot"}
};

int midi_open_port(HMIDIOUT * midiout, int portnum)
{
	if (midiout) midiOutClose(* midiout);
	if (midiOutOpen(midiout,portnum-1,0,0,CALLBACK_NULL)==MMSYSERR_NOERROR) 
	   return TRUE;

	write_logfile("could not open midi port"); 
	return FALSE;
}

void mute_all_midi(void)
{
	int i,t;

	for(i=0;i<GLOBAL.objects;i++)
	{
		if (objects[i]->type==OB_MIDI)
		{ 
			MIDIOBJ * st= (MIDIOBJ *) objects[i];

			if (MIDIPORTS[st->port].midiout) 
			{
			  midi_ControlChange(&(MIDIPORTS[st->port].midiout), st->midichn,123, 0);
			  midi_ControlChange(&(MIDIPORTS[st->port].midiout), st->midichn,120, 0);
			  for (t=0;t<MAX_MIDITONES;t++) st->tonebuffer[t]=0;
			}
	
		}
	}
}

void init_midi (void)
{
	char sztemp[50];
    MIDIOUTCAPS ocaps;

	for (int t=128; t<255; t++)
	{
 		wsprintf(sztemp,"Controller Nr.%d",t-127);
  		strcpy(midi_instnames[t],sztemp);
	}

	GLOBAL.midiports=0;

	for (int t = -1;  t < (int) midiOutGetNumDevs(); t++) 
//	for (unsigned int t = 0;  t < midiOutGetNumDevs(); t++) 
	{
	  midiOutGetDevCaps(t, &(ocaps), sizeof (MIDIOUTCAPS));
	  strcpy(MIDIPORTS[GLOBAL.midiports].portname, ocaps.szPname ) ;
	  MIDIPORTS[GLOBAL.midiports].midiout=0;
	  GLOBAL.midiports++;
	} 
}

int get_listed_midiport(int num)
{
	int i,c;
	c=-1;
	for (i=0;i<GLOBAL.midiports;i++)
	{
		if (MIDIPORTS[i].midiout) c++;
		if (c==num)	return(i);
	}
	return(GLOBAL.midiports);
}

int get_opened_midiport(int num)
{
	int i,c;
	c=-1;
	for (i=0;i<=num;i++)
		if (MIDIPORTS[i].midiout) c++;
	
	return(c);
}


void midi_Instrument(HMIDIOUT * midiout, int chn, int inst)
{
		if (inst<128) midiOutShortMsg(*midiout, 256*inst+191+chn);
		// 1-127: Instruments for noteOn,  128 - 255: Controller numbers
}

void midi_ControlChange(HMIDIOUT * midiout, int chn, int cont, int val)
{
		midiOutShortMsg(*midiout,65536*val+256*cont+175+chn);
}

void midi_NoteOn(HMIDIOUT * midiout, int chn, int note, int vol)
{
		midiOutShortMsg(*midiout,65536*vol+256*note+143+chn);
}

void midi_NoteOff(HMIDIOUT * midiout, int chn, int note)
{
		//midiOutShortMsg(*midiout,256*note+143+chn);
			midiOutShortMsg(*midiout,256*note+127+chn);
}

void midi_PitchRange(HMIDIOUT * midiout, int chn, int pitchrange)
{
/*	 'B0 65 00  Controller/chan 0, RPN coarse (101), Pitch Bend Range
	 'B0 64 00  Controller/chan 0, RPN fine (100), Pitch Bend Range
	 'B0 06 02  Controller/chan 0, Data Entry coarse, +/- 2 semitones
	 'B0 26 04  Controller/chan 0, Data Entry fine, +/- 4 cents
			  */
	 midi_ControlChange(midiout, chn,101, 0);
	 midi_ControlChange(midiout, chn,100, 0);
     
	 midi_ControlChange(midiout, chn,6, pitchrange);
     midi_ControlChange(midiout, chn,38, 0);

}

void midi_Vol(HMIDIOUT * midiout, int chn, int vol)
{
	 midi_ControlChange(midiout, chn,7, vol);

}

void midi_Pitch(HMIDIOUT * midiout, int chn, int wheel)
{
	int wl,wh;

	wh=(wheel>>8)&255;
	wl=wheel&255;
	//SetDlgItemInt(ghWndStatusbox,IDC_STATUS,wl,0);
	midiOutShortMsg(*midiout,65536*wh+256*wl+223+chn); //
	
}

void midi_Message(HMIDIOUT * midiout, int status)
{
	midiOutShortMsg(*midiout, status*255);
}
