/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  TIMER.CPP
  Author:  Chris Veigl


  This Module provides the basic timing-functions for sample-processing
  The TimerProc() - function performs reading from COM-ports or archives and
  triggers the worker-functions of all objects in case of an archive read.
  The Windows PerformanceCounter-functions are used to retrieve excat system time

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
--------------------------------------------------------------------------------*/


#include "brainBay.h"
#include "neurobit_api\\api.h"
#include "ob_emotiv.h"
#include "ob_biosemi.h"
#include "ob_brainflow.h"


extern TProtocolEngine NdProtocolEngine;


void init_system_time(void)
{
	QueryPerformanceFrequency((_LARGE_INTEGER *)&(TIMING.pcfreq));

    TTY.packettime=(LONGLONG)(TIMING.pcfreq/PACKETSPERSECOND); // milliseconds for one Packet-Read

	TIMING.ppscounter=0;
	TIMING.packetcounter=0;
	TIMING.dialog_update=0;
	TIMING.draw_update=0;

}



void process_packets(void)
{
	static double calc;
	int t;

    TIMING.ppscounter++;
    TIMING.packetcounter++;
	

	if (TIMING.dialog_update++ >= GLOBAL.dialog_interval) TIMING.dialog_update=0; 
	if (TIMING.draw_update++ >= GLOBAL.draw_interval) TIMING.draw_update=0; 


	if (GLOBAL.session_length && (TIMING.packetcounter>=GLOBAL.session_end))
	{
		int sav_fly=GLOBAL.fly;
		SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
		
		TIMING.packetcounter= GLOBAL.session_start;
		for (t=0;t<GLOBAL.objects;t++) objects[t]->session_pos(TIMING.packetcounter);
		int pos=get_sliderpos(TIMING.packetcounter);
		SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,pos);

		if ((!sav_fly)&&(GLOBAL.session_loop) )
		{
				reset_oscilloscopes();
				SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RUNSESSION,0);
		}
		 		
	}
	else
	{

		for (t=0;t<GLOBAL.objects;t++)
		 if (objects[t]) objects[t]->work();
		
	}
	if (!TIMING.dialog_update) update_statusinfo();
	
}
	

void CALLBACK TimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	LONGLONG pc;
	MSG msg;

    QueryPerformanceCounter((_LARGE_INTEGER *)&pc);
	//TIMING.acttime=pc;

    check_keys();
    if (GLOBAL.neurobit_available) NdProtocolEngine();
	if (GLOBAL.emotiv_available) process_emotiv();
	if (GLOBAL.biosemi_available) process_biosemi();
#if _MSC_VER >= 1900
	if (GLOBAL.brainflow_available) process_brainflow();
#endif
	//	if (GLOBAL.ganglion_available) process_ganglion();

	if ((!TIMING.pause_timer) && (!GLOBAL.loading))
	{
		// one second passed ? -> update PPS-info
		if (pc-TIMING.timestamp >= TIMING.pcfreq) 
		{	TIMING.timestamp+=TIMING.pcfreq; 
			TIMING.actpps=(int) TIMING.ppscounter;
		    TIMING.ppscounter=0;
		}

		// Reading from Archive & next packet demanded? -> read from File and Process Packets
		while ((pc-TIMING.readtimestamp >= TTY.packettime) || (GLOBAL.fly))
		{
			TIMING.readtimestamp+=TTY.packettime;
			TIMING.acttime=TIMING.readtimestamp;

			if(CAPTFILE.do_read&&(CAPTFILE.offset<=TIMING.packetcounter)&&(CAPTFILE.offset+CAPTFILE.length>TIMING.packetcounter))
			{
				long tmp;
				tmp=TIMING.packetcounter;
				read_captfile(TTY.amount_to_read); 	
				ParseLocalInput(TTY.amount_to_read);
				if ((TIMING.packetcounter-tmp-1)>0)
				 TIMING.readtimestamp+=TTY.packettime*(TIMING.packetcounter-tmp-1);
				//return;
			}

			// process packets in case of no File-Read and no Com-Read
			else if ((TTY.read_pause) && (!GLOBAL.neurobit_available) && (!GLOBAL.emotiv_available) 
				      && (!GLOBAL.biosemi_available) && (!GLOBAL.ganglion_available) && (!GLOBAL.brainflow_available)){
				process_packets();
			}

			if (GLOBAL.fly)
			{
				if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{	
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}	
	}
}


void start_timer(void)
{
	if (TIMING.timerid) stop_timer();

	QueryPerformanceCounter((_LARGE_INTEGER *)&TIMING.timestamp);
	TIMING.readtimestamp=TIMING.timestamp;
	TIMING.ppscounter=0;

	if(!(TIMING.timerid= timeSetEvent(1,0,TimerProc,1,TIME_PERIODIC | TIME_CALLBACK_FUNCTION)))
							report_error("Could not set Timer!");
	else GLOBAL.running=TRUE; 
}


void stop_timer(void)
{
	if(TIMING.timerid)
	{
	  if(timeKillEvent(TIMING.timerid)!=TIMERR_NOERROR)
		report_error("Could not stop Timer!");
	  else TIMING.timerid=0;
	}

	GLOBAL.running=FALSE;
	mute_all_midi();

}


