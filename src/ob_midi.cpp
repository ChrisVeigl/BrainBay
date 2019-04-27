/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_MIDI.CPP:  contains assisting functions for the Midi-Object

  Parameters like Midi-Output Device, Instrument or Controller Number, 
  Tone-Heights, and Tone Mapping can be selected. The Tone Scale can be 
  modified, saved and loaded. 

  update_harmoniclist: fills the tone-values for Scale-Mapping into the listbox
  apply_harmoniclist: get the data out of the listbox into the object's memory
  update_mididialog: display setting in the toolbox-window
  apply_mididialog: stores settings from toolbox-windows into the object's memory
  MidiStreamDlgHandler: processes events for the toolbox-window
  
-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_midi.h"


void update_mididialog(HWND hDlg, MIDIOBJ * st)
{
	static int updating=FALSE;

	if (!updating)
	{ updating=TRUE;
	SendDlgItemMessage( hDlg, IDC_MIDIINSTCOMBO, CB_SETCURSEL, st->instrument, 0L ) ;
    SendDlgItemMessage( hDlg, IDC_MIDIPORTCOMBO, CB_SETCURSEL, get_opened_midiport(st->port), 0L ) ;


	SetDlgItemInt(hDlg, IDC_MIDITONEBUF,st->n_tones ,0);
	SetScrollPos(GetDlgItem(hDlg,IDC_MIDITONEBUFBAR), SB_CTL,st->n_tones,TRUE);
	
	SetDlgItemInt(hDlg, IDC_MIDICHN, st->midichn, 0);
	
	SetDlgItemInt(hDlg, IDC_MIDIVOLUMEFROM, st->from_volume,0);
	SetDlgItemInt(hDlg, IDC_MIDIVOLUMETO, st->to_volume,0);

	SetDlgItemInt(hDlg, IDC_PITCHRANGE, st->pitchrange,0);
	SetDlgItemInt(hDlg, IDC_PITCHINTERVAL, st->pitchinterval,0);
	
	CheckDlgButton(hDlg, IDC_MIDICHANGESCHECK, st->only_changes);
	CheckDlgButton(hDlg, IDC_MIDIMUTEONFALSE, st->mute_on_falsetones);
	CheckDlgButton(hDlg, IDC_MIDIMUTE, st->mute);

	SetDlgItemInt(hDlg, IDC_MIDITIMER,st->timer ,0);
	SetScrollPos(GetDlgItem(hDlg,IDC_MIDITIMERBAR), SB_CTL,st->timer,TRUE);

	updating=FALSE;
	}

}

void apply_midistream(HWND hDlg, MIDIOBJ * st)
{
	static int updating=FALSE;

  if (!updating)
  { 
	updating=TRUE;
	st->n_tones= GetDlgItemInt(hDlg, IDC_MIDITONEBUF,NULL,0);
	st->midichn=GetDlgItemInt(hDlg, IDC_MIDICHN, NULL, 0);
	st->from_volume=GetDlgItemInt(hDlg, IDC_MIDIVOLUMEFROM, NULL,0);
	st->to_volume=GetDlgItemInt(hDlg, IDC_MIDIVOLUMETO,NULL,0);
	st->only_changes= IsDlgButtonChecked(hDlg, IDC_MIDICHANGESCHECK);
    st->timer= GetDlgItemInt(hDlg, IDC_MIDITIMER,NULL,0);
	st->inputtimer=st->timer;
	updating=FALSE;
  }
}




/*-----------------------------------------------------------------------------

FUNCTION: CALLBACK MidiStreamDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )

PURPOSE: Handles Messages for Midi-Stream Dialog

PARAMETERS:
    hDlg - Dialog window handle

-----------------------------------------------------------------------------*/

int x=130;

LRESULT CALLBACK MidiStreamDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    int t;
	static int init;
	MIDIOBJ * st;
	
	st = (MIDIOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MIDI)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				for (t=0;t<255;t++) 
					SendDlgItemMessage(hDlg, IDC_MIDIINSTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) midi_instnames[t]) ;
				SendDlgItemMessage( hDlg, IDC_MIDIINSTCOMBO, CB_SETCURSEL, 0, 0L ) ;

 			    for (t = 0; t < GLOBAL.midiports; t++) 
				  if (MIDIPORTS[t].midiout) SendDlgItemMessage(hDlg, IDC_MIDIPORTCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) MIDIPORTS[t].portname ) ;


				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				
				lpsi.nMin=1; lpsi.nMax=20;
				SetScrollInfo(GetDlgItem(hDlg,IDC_MIDITONEBUFBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=1; lpsi.nMax=1000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_MIDITIMERBAR),SB_CTL,&lpsi,TRUE);
				lpsi.nMin=1; lpsi.nMax=127;
				SetDlgItemText(hDlg, IDC_HARMONICNAME, st->tonescale.name);

				init=TRUE;
				update_mididialog(hDlg,st);
				apply_midistream(hDlg,st);
				init=FALSE;

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			
			case IDC_MIDIPORTCOMBO:
 				 if (HIWORD(wParam)==CBN_SELCHANGE)
				 	st->port=get_listed_midiport(SendMessage(GetDlgItem(hDlg, IDC_MIDIPORTCOMBO), CB_GETCURSEL , 0, 0));
				
			case IDC_MIDIINSTCOMBO:
 				 if (HIWORD(wParam)==CBN_SELCHANGE)
				 {
						st->instrument=SendMessage(GetDlgItem(hDlg, IDC_MIDIINSTCOMBO), CB_GETCURSEL , 0, 0);
						midi_Instrument(&(MIDIPORTS[st->port].midiout),st->midichn,st->instrument);
					    midi_PitchRange(&(MIDIPORTS[st->port].midiout), st->midichn, st->pitchrange);

				 }
				 break;
			case IDC_STARTSTREAM:
					midi_Message(&(MIDIPORTS[st->port].midiout),264);					
				 break;
			case IDC_STOPSTREAM:
					midi_Message(&(MIDIPORTS[st->port].midiout),270);					
				 break;

			case IDC_MIDIMUTE: 
				st->mute=IsDlgButtonChecked(hDlg, IDC_MIDIMUTE);
				if (st->mute)
				{
					midi_ControlChange(&(MIDIPORTS[st->port].midiout), st->midichn,123, 0);
					for (t=0;t<MAX_MIDITONES;t++) st->tonebuffer[t]=0;
				}	
				break;
			case IDC_MIDIMUTEONFALSE: 
				st->mute_on_falsetones=IsDlgButtonChecked(hDlg, IDC_MIDIMUTEONFALSE);
				break;

			case IDC_PITCHRANGE:
				{  int d= GetDlgItemInt(hDlg, IDC_PITCHRANGE,NULL,1);
				   if ((d>0)&&(d<50)&&(d!=st->pitchrange))
				   {
					  st->pitchrange=d;
					  midi_PitchRange(&(MIDIPORTS[st->port].midiout), st->midichn, st->pitchrange);
				   }
				} break;

			case IDC_PITCHINTERVAL:
				{  int d= GetDlgItemInt(hDlg, IDC_PITCHINTERVAL,NULL,1);
				   if ((d>0)&&(d<300))  st->pitchinterval=d;
				} break;

			case IDC_MIDITONEBUF:
			case IDC_MIDICHN:
			case IDC_MIDIVOLUMEFROM:
			case IDC_MIDIVOLUMETO:
			case IDC_MIDICHANGESCHECK:
			
				 if (!init) apply_midistream(hDlg, st);
					break;

			case IDC_HARMONICNAME:
				 GetDlgItemText(hDlg,IDC_HARMONICNAME,st->tonescale.name,sizeof(st->tonescale.name));
				break;
			case IDC_LOADHARMONIC:
				{
				  char szFileName[MAX_PATH];
				  strcpy(szFileName,GLOBAL.resourcepath);
				  strcat(szFileName,"TONESCALES\\*.sc");
				  if (open_file_dlg(hDlg, szFileName, FT_HARMONIC, OPEN_LOAD)) 
				  {
					if (!load_from_file(szFileName, &(st->tonescale), sizeof(struct SCALEStruct) ))
					{    
						report_error(szFileName);
						report_error("Could not load Harmonic Scale ");
					}
					else
					{
						SetDlgItemText(hDlg, IDC_HARMONICNAME, st->tonescale.name);
						reduce_filepath(szFileName,szFileName);
						strcpy(st->tonefile,szFileName);
					}
				  } // else report_error("Could not load Harmonic Scale");
				}
				break;

			}
			return TRUE;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if ((nNewPos=get_scrollpos(wParam,lParam))>=0)
			{   
			  if (lParam == (long) GetDlgItem(hDlg,IDC_MIDITONEBUFBAR))  
			  {  
				  SetDlgItemInt(hDlg, IDC_MIDITONEBUF,nNewPos,0);
				  midi_ControlChange(&(MIDIPORTS[st->port].midiout), st->midichn,123, 0);
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_MIDITIMERBAR))  SetDlgItemInt(hDlg, IDC_MIDITIMER,nNewPos,0);
			  if (!init) apply_midistream(hDlg, st);
			}
		
		}
		break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}




//
//  Object Implementation
//


MIDIOBJ::MIDIOBJ(int num) : BASE_CL()	
	  {
	    outports = 0;
		inports = 4;
		width=65;
		strcpy(in_ports[0].in_name,"note");
		strcpy(in_ports[1].in_name,"vol");
		strcpy(in_ports[2].in_name,"interval");
		strcpy(in_ports[3].in_name,"pitch");

		n_tones=5; acttone=0; instrument=10;
		from_volume=120; to_volume=120;
		newchn=1;
		for (t=0;t<GLOBAL.objects;t++) 
		  if (objects[t]->type==OB_MIDI)
			  {  MIDIOBJ * st2= (MIDIOBJ *)objects[t];
			     if (st2->midichn>=newchn) newchn=st2->midichn+1;
			  }

		midichn=newchn;
		only_changes=0; mute=TRUE; muted=TRUE; mute_on_falsetones=FALSE; port=DEF_MIDIPORT;
//		inputnote=512.0f; inputvolume=1024.0f;
		inputnote=0.0f; inputvolume=512.0f;
		sum_note=0.0f; sum_volume=0.0f;
		pitch=64; inputpitch=64;
		timer=20; inputtimer=20;
		pitchrange=4; pitchinterval=15; pitchtime=0;
		acttime=0;
  		for (t=1;t<128;t++)  tonescale.tones[t-1]=t; tonescale.len=127;
		strcpy(tonefile,"none");
  		strcpy(tonescale.name,"All Tones");
	  }
	  void MIDIOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MIDISTREAMBOX, ghWndStatusbox, (DLGPROC)MidiStreamDlgHandler));
	  }
	  void MIDIOBJ::load(HANDLE hFile) 
	  {	
		  char tempport[400];
		  int t;

	  	  load_object_basics(this);
		  load_property("interval",P_INT,&timer);
		  inputtimer=timer;
		  load_property("hold-tones",P_INT,&n_tones);
		  load_property("instrument",P_INT,&instrument);
		  load_property("midichn",P_INT,&midichn);
		  load_property("only_changes",P_INT,&only_changes);
		  load_property("from_volume",P_INT,&from_volume);
		  load_property("to_volume",P_INT,&to_volume);
		  load_property("mute",P_INT,&mute);
		  load_property("mute_on_falsetones",P_INT,&mute_on_falsetones);
		  load_property("scalename",P_STRING,tonescale.name);
		  load_property("length",P_INT,&(tonescale.len));

		  load_property("tonefile",P_STRING,tonefile);
		  if (strcmp(tonefile,"none"))
		  {
				char szFileName[MAX_PATH] = "";
				strcpy(szFileName,GLOBAL.resourcepath);
				strcat(szFileName,"TONESCALES\\");
				strcat(szFileName,tonefile);
				if (!load_from_file(szFileName, &tonescale, sizeof(struct SCALEStruct) ))
				{    
					report_error(szFileName);
					report_error("Could not load Harmonic Scale for Midi Object");
				}
		  }
		  load_property("pitchrange",P_INT,&pitchrange);
		  load_property("pitchinterval",P_INT,&pitchinterval);
		  load_property("portname",P_STRING,tempport);

		  port=-1;
		  for (t=0;(t<GLOBAL.midiports)&&(port==-1);t++) 
		  {
			  if (!strcmp(MIDIPORTS[t].portname,tempport))
			  {
				  if (MIDIPORTS[t].midiout) port=t;
				  else if (midi_open_port(&(MIDIPORTS[t].midiout),t))  port=t;
			  }
		  }
		  if (port==-1) 
		  {
			  strcat(tempport,": Midi Device not found.");
		      write_logfile(tempport);

			  if (GLOBAL.midiports) 
				  write_logfile("-> Using default midi port.");

			  if (GLOBAL.midiports) 
			  {
				  port=0;
				  if (!MIDIPORTS[0].midiout) midi_open_port(&(MIDIPORTS[0].midiout),0);
			  }
		  }
		  if (port!=-1)  
		  { 
			  midi_Instrument(&(MIDIPORTS[port].midiout), midichn, instrument);
			  midi_PitchRange(&(MIDIPORTS[port].midiout), midichn, pitchrange);
		  }

	      
	  }
	  void MIDIOBJ::save(HANDLE hFile) 
	  {	
	 	  save_object_basics(hFile, this);
		  save_property(hFile,"interval",P_INT,&timer);
		  save_property(hFile,"hold-tones",P_INT,&n_tones);
		  save_property(hFile,"instrument",P_INT,&instrument);
		  save_property(hFile,"portname",P_STRING,MIDIPORTS[port].portname);
		  save_property(hFile,"midichn",P_INT,&midichn);
		  save_property(hFile,"only_changes",P_INT,&only_changes);
		  save_property(hFile,"from_volume",P_INT,&from_volume);
		  save_property(hFile,"to_volume",P_INT,&to_volume);
		  save_property(hFile,"mute",P_INT,&mute);
		  save_property(hFile,"mute_on_falsetones",P_INT,&mute_on_falsetones);
		  save_property(hFile,"scalename",P_STRING,tonescale.name);
		  save_property(hFile,"length",P_INT,&(tonescale.len));
		  save_property(hFile,"tonefile",P_STRING,tonefile);
  		  save_property(hFile,"pitchrange",P_INT,&pitchrange);
		  save_property(hFile,"pitchinterval",P_INT,&pitchinterval);

	  }

	  void MIDIOBJ::incoming_data(int port, float value) 
	  {	
		if (port==0) inputnote=value; 
	    if (port==1) inputvolume=value; 
		if (port==2) inputtimer=(int)((value-in_ports[2].in_min)/(in_ports[2].in_max-in_ports[2].in_min) * 100.0f);
		if (port==3) inputpitch=(int)((value-in_ports[3].in_min)/(in_ports[3].in_max-in_ports[3].in_min) * 65000.0f) ;
		
	  }

	  void MIDIOBJ::work(void) 
	  {
	    if (GLOBAL.fly) return;
		if (mute==FALSE)
		{
			if (inputvolume==INVALID_VALUE) inputvolume=0;
			if(inputnote==INVALID_VALUE)
			{
				if ((!muted)&&(mute_on_falsetones))
				{
				  midi_ControlChange(&(MIDIPORTS[port].midiout), midichn,123, 0);
				  muted=true;
				  sum_note=0;
				  sum_volume=0;
				}
				tonebuffer[acttone]=0;
			}
			else 
			{ 
				if (inputtimer!=timer) timer=inputtimer;

				pitchtime++;
				if (pitchtime>=pitchinterval)
				{  pitchtime=0;
				   if (inputpitch!=pitch) 
				   {
					 pitch=inputpitch;
					 midi_Pitch(&(MIDIPORTS[port].midiout),midichn,pitch); 
				     if (inputvolume!=volume) 
				     {
					   volume=(int)inputvolume;
					   midi_Vol(&(MIDIPORTS[port].midiout),midichn,(int)((inputvolume-in_ports[1].in_min)/(in_ports[1].in_max-in_ports[1].in_min) * 127.0f));
				     } 
				   } 
				}

				muted=false;
				sum_note+=inputnote;
				sum_volume+=inputvolume;
			
				acttime++;
				if (acttime>=timer)
				{
					int note,vol;
	   
					sum_note/=timer;
					sum_note=(sum_note-in_ports[0].in_min)/(in_ports[0].in_max-in_ports[0].in_min);

					sum_volume/=timer;
					sum_volume=(sum_volume-in_ports[1].in_min)/(in_ports[1].in_max-in_ports[1].in_min);

					index=(int)(sum_note*tonescale.len);
					if (index>tonescale.len-1) index=tonescale.len-1;
					if (index<0) index=0;
					note=tonescale.tones[index];

					vol=(int)(sum_volume*(to_volume-from_volume)) + from_volume;
					if (vol>to_volume) vol=to_volume; if (vol<from_volume) vol=from_volume;
	
					if ((MIDIPORTS[port].midiout) && ((!only_changes)||(note!=tonebuffer[acttone])))
					{
						if (instrument<128) midi_NoteOn(&(MIDIPORTS[port].midiout), midichn, note, vol);
						else midi_ControlChange(&(MIDIPORTS[port].midiout), midichn, instrument-127, note);
						acttone++;	if (acttone==MAX_MIDITONES) acttone=0;
						tonebuffer[acttone]=note;
				  		temp=acttone-n_tones;
						if (temp<0) temp=temp+MAX_MIDITONES;
						if (instrument<128) midi_NoteOff(&(MIDIPORTS[port].midiout),midichn,tonebuffer[temp]);
					}
					sum_note=0.0f;
					sum_volume=0.0f;
					acttime=0;
				}
			}

			
		}

		if ((hDlg==ghWndToolbox)&&(!TIMING.dialog_update))
		{
  			SetScrollPos(GetDlgItem(hDlg,IDC_MIDITIMERBAR), SB_CTL,timer,TRUE);
			SetDlgItemInt(hDlg,IDC_MIDITIMER,timer,0);
		}

	  }


MIDIOBJ::~MIDIOBJ()
	  {
		 for (t=0;t<=n_tones;t++)
		 {
			temp=acttone-t;
			if (temp<0) temp=temp+MAX_MIDITONES;
			if (instrument<128) midi_NoteOff(&(MIDIPORTS[port].midiout),midichn,tonebuffer[temp]);
		 } 
	  }  


