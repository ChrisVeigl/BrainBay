/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
               OpenSource Application for realtime BodySignalProcessing & HCI
               with the OpenEEG hardware
			   
  Author: Chris Veigl, contact: chris@shifz.org
  
  Co-Authors:
		 Jeremy Wilkerson (Modules: AND, OR, NOT, WAV, CORELLATION, EVALUATOR)
		 Lester John (Module MATLAB-transfer)
		 Stephan Gerhard (QDS parser)

  Credits: Jim Peters (digital filter works), Jeff Molofee (OpenGL-tutorial), John Roark (SkinDialog)
  		   AllenD (COM-Port control), Aleksandar B. Samardz (Expression Evaluator Library)

  the used non-standard Libraries are:

  Multimedia and OpenGL: winmm.lib opengl32.lib glu32.lib vfw32.lib glaux.lib
  SDL (Simple Direct Media Layer): SDL.lib SDL_net.lib SDL_sound.lib modplug.lib 
  OpenCV - Intels's Computer Vision Library: cv.lib cvcam.lib cxcore.lib highgui.lib
  Matlab Engine (only in special Matlab Release): libeng.lib libmx.lib 
  Jim Peters's Filter Library: fidlib.lib (http://uazu.net)
  Skinned Dialog by John Roark: skinstyle.lib (http://www.codeproject.com/dialog/skinstyle.asp)
  GNU LibMatheval by Aleksandar B. Samardz: matheval.lib (http://www.gnu.org/software/libmatheval)
  

  Project-Site: http://brainbay.lo-res.org
  Link to the OpenEEG-Project: http://openeeg.sf.net
  

  MODULE: OB_EEG.CPP:  contains functions for the EEG-Object

  The EEG Objects provides access to a biosignal-data-stream.
  This can be live data (currently only Modular EEG is supported) or
  data from an archive file. When opening an archive that contains a Brain-Bay-header,
  the devicetype and fileformat is detected automatically. 
  6 output-Ports are provided, to meet the needs of the ModualEEG-Hardware.

  The EEG-Object has a special status compared to all other objects, due to
  the interaction with com-port, files and system-timer. 
  The EEG-Object appears on screen
  when the application is started, and it cannot be deleted by hand.

  EEGDlgHandler: processes Dialog events for the Toolbox-Window
  init_sytem_time: Uses QueryPerformanceFrequency() to determine the most accurate
			time-varibale of the system, calculates the corresponding Packettime
			and the counter-value for one millisecond.
  TimerProc: the timer-handler, currently used for reading archive-files and determining
            the packets-per second - value

  process_packets:  calls the worker-functions of all existing objects.
  parse_byte_P2: parses a ModEEG-P2 datastream and 
			stores channel values into the packetstructure
  parse_byte_P21: parses a P21 datastream (by Jarek Foltynski, modified by Reiner Münch)
			     P21 is a bidirectional protocol
  parse_byte_P3: parses a ModEEG-P3 datastream and stores channel values 
  parse_byte_raw: parses a 1 channel-raw datastream and stores channel values
  parse_byte_QDS: parses OpenEXG datastream with QDS NFB 256 protocol and
			stores channel values into the packetstructure
 
  ParseLocalInput: Chooses parsing-algorithm, according to TTY.devicetype


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

 
#include "windows.h"
#include "brainBay.h"
#include "ob_eeg.h"


#define MAXLEN_TEMPSTR  20

// Definitions for the bidirectional Protocol P21 developed by Jarek and MooseC
// command ids (set in 7 lower bits of the command byte)
#define CMD_PORT_SET 1+128
#define CMD_PORTD_BIT_SET 2+128
#define CMD_PORTD_BIT_BRIGHT_ONTIME 3+128
#define CMD_PORTD_BIT_BRIGHT_PERIOD 4+128
#define CMD_CHANNEL_SET 5+128
#define CMD_GET_VINFO 6+128
#define CMD_SET_VINFO 7+128

// values that can be requested or set with RECV_GET_VINFO and RECV_SET_VINFO
#define VINFO_PROTOCOL_NUMBER 1		// Protocol: P2 or P21
#define VINFO_CHANNELS_MATRIX 2		// active channels (only for P21)
#define VINFO_BAUD_RATE 3           // set baud rate (set only)
#define VINFO_PORTD 4				// value of Port D
#define VINFO_PORTB 5				// value of Port B
#define VINFO_SAMPLE_RATE 6     	// sample rate
#define VINFO_RUNEEG 7          	// start/stop EEG function                  0:stop, 1:run
#define VINFO_DEVICE 8          	// what kind of EEG is online (read only)   answer 0:modEEG, 1:MonolithEEG
#define VINFO_PROTOCOL_SUBNUMBER 9 	// view sub version of protocoll 21 (read only)


  char devicetypes[20][40]   = {"ModularEEG P2","ModularEEG P3","1 Channel Raw Data","MonolithEEG P21","SmartBrainGames 4Chn","1Chn of 8bit values", "Pendant EEG v3", "QDS NFB 256", "NIA USB HDI Ver 1.1","\0"};
  int  AMOUNT_TO_READ   [20]  = {     68,               66,               8           ,  21 ,            32   ,                  4   ,                  32,              80         ,    3 };
  int  BYTES_PER_PACKET [20]  = {     17,               11,               2           ,  7  ,             5   ,                  1   ,                  5,               20         ,    3 };



char captfiletypes[10][40] = {"Text (Debug Mode)","Integer Values","\0"};

char * szBaud[] = {"9600", "19200", "38400", "57600", "115200", "230400", "460800", "\0"};  // Combobox - Items for Baudrate
DWORD   BaudTable[] =  {9600, 19200, 38400, 57600, 115200, 230400, 460800,0 } ;  // Constants for Baudrate Setting

char * samplingrates[] = {"256","300","512","600","1024","1200", "\0"};  // Combobox - Items for Samplingrate
int    SamplingRateTable[] =  { 256,300,512,600,1024,1200 } ;  // Constants for Samplingrate Setting

#define NIASAMPLINGRATE 1000			// NIA Sampling Rate = 1000/sec.
int cc=0;
int once =0;


void check_sync(unsigned int num)
{
	if ((TTY.devicetype=DEV_MODEEG_P3)&&(PACKET.old_number==63))
		PACKET.old_number=0;
	else PACKET.old_number++;

	if (PACKET.old_number != num) GLOBAL.syncloss++;
	PACKET.old_number=num;
}


/********************************************************************

  ModularEEG Packet Format NIA
 

  One packet has a 55 byte 
     	16 Samples (three bytes each: 48 Bytes total)
	2 Byte Sync sequence (56, 189)
	2 Byte Latency Timing information
	2 Byte Total number of samples read
	1 Byte number of valid samples within this packet
 giving a total of 55 bytes.

 **********************************************************************/



void parse_byte_NIA(unsigned char actbyte)
{
	switch (PACKET.readstate) {
		case 0:
			PACKET.buffer[PACKET.extract_pos]=actbyte ;
			PACKET.extract_pos=0;
			PACKET.readstate++; 
			break;
		case 1:
			PACKET.buffer[PACKET.extract_pos]+=actbyte*0x100 ;
			PACKET.readstate++; 
			break;
		case 2:
			PACKET.buffer[PACKET.extract_pos]+=actbyte*0x10000;	// 24-Bit Werte
			PACKET.buffer[PACKET.extract_pos]/=0x100;			// auf 16 Bit beschränken!!
			PACKET.extract_pos++;
			PACKET.readstate=0; 
			process_packets();
			break;
		default: PACKET.readstate=0;


	}
}

/********************************************************************

  ModularEEG Packet Format Version 2
 

  One packet has a 2 byte sync sequence (165,90),
     the version number, (1 byte)
     the packet number (1 byte)
     six channels (two bytes each)
     and the button state (1 bate)
  giving a total of 17 bytes.

 **********************************************************************/



void parse_byte_P2(unsigned char actbyte)
{
    switch (PACKET.readstate) 
	{
		  case 0: if (actbyte==165) PACKET.readstate++; 
				  break;
		  case 1: if (actbyte==90)  PACKET.readstate++; 
				  else PACKET.readstate=0;
				  break;
		  case 2: PACKET.readstate++; 
				  break;
	      case 3: PACKET.number = actbyte;
			      // if (++PACKET.old_number != actbyte) GLOBAL.temp++;
				  // PACKET.old_number=actbyte;
			      PACKET.extract_pos=0;PACKET.readstate++;
				  break;
		  case 4: if (PACKET.extract_pos < 12)
				  {   if ((PACKET.extract_pos & 1) == 0)
					     PACKET.buffer[PACKET.extract_pos>>1]=actbyte*256;
			          else PACKET.buffer[PACKET.extract_pos>>1]+=actbyte;
					  PACKET.extract_pos++;
				  }
				  else
				  {  PACKET.switches= actbyte;
					 PACKET.readstate=0;
		  	  	     process_packets();
				  }
		  		  break;
		  default: PACKET.readstate=0;
		}		
}





/********************************************************************

  ModularEEG Packet Format Version 3
 
  One packet can have zero, two, four or six channels (or more).
  The default is a 6-channel packet, shown below.
 
  0ppppppx     packet header
  0xxxxxxx
  
  0aaaaaaa     channel 0 LSB
  0bbbbbbb     channel 1 LSB
  0aaa-bbb     channel 0 and 1 MSB
  
  0ccccccc     channel 2 LSB
  0ddddddd     channel 3 LSB
  0ccc-ddd     channel 2 and 3 MSB
  
  0eeeeeee     channel 4 LSB
  0fffffff     channel 5 LSB
  1eee-fff     channel 4 and 5 MSB
  
  Key:
  
  1 and 0 = sync bits.
    Note that the '1' sync bit is in the last byte of the packet,
    regardless of how many channels are in the packet.
  p = 6-bit packet counter
  x = auxillary channel byte
  a - f = 10-bit samples from ADC channels 0 - 5
  - = unused, must be zero
 
  There are 8 auxillary channels that are transmitted in sequence.
  The 3 least significant bits of the packet counter determine what
  channel is transmitted in the current packet.
 
  Aux Channel Allocations:
 
  0: Zero-terminated ID-string (ASCII encoded).
  1: 
  2:
  3:
  4: Port D status bits
  5:
  6:
  7:
 
  The ID-string is currently "mEEGv1.0".

 **********************************************************************/


void parse_byte_P3(unsigned char actbyte)
{
   int i, j, n;
//   int sync;


    switch (PACKET.readstate) 
	{
		  case 0: if (actbyte & 0x80) {PACKET.extract_pos=0;PACKET.readstate++;}
				  break;

		  case 1: PACKET.buffer[PACKET.extract_pos++]=actbyte;
			      
				  if (actbyte & 0x80)  
				  {
					n=PACKET.extract_pos-2;
					if (n%3) PACKET.readstate=0;
					else
					{
 						PACKET.number= (PACKET.buffer[0] >> 1) & 0x3f;
 						PACKET.aux = (PACKET.buffer[0] << 7) & 0x80 | PACKET.buffer[1] & 0x7f;
						if ((PACKET.number & 7)==4) PACKET.switches = PACKET.aux;

						for (i = 0, j = 2; i < n/3 ; i++, j += 3)
						{
						  // Decode and store even-channel sample
						  PACKET.buffer[i << 1] = ((unsigned short) PACKET.buffer[j]) & 0x7f | (((unsigned short) PACKET.buffer[j+2]) << 3) & 0x380;
						  // Decode and store odd-channel sample
						  PACKET.buffer[(i << 1) + 1] = ((unsigned short) PACKET.buffer[j+1]) & 0x7f | (((unsigned short) PACKET.buffer[j+2]) << 7) & 0x380;
						}

						PACKET.extract_pos=0;
						process_packets();
					} 
  				    
				     
				  }
				  break;
		  default: PACKET.readstate=0;
		}
	
/*
switch (PACKET.readstate) 
	{
		  case 0: if (actbyte & 0x80) {PACKET.extract_pos=0;PACKET.readstate++;}
				  break;
		  case 1: PACKET.buffer[PACKET.extract_pos]=actbyte;
			      PACKET.extract_pos++;
				  if (PACKET.extract_pos==11) 
				  {
				    PACKET.number= (PACKET.buffer[0] >> 1) & 0x3f;
				    PACKET.aux = (PACKET.buffer[0] << 7) & 0x80 | PACKET.buffer[1] & 0x7f;
				    if ((PACKET.number & 7)==4) PACKET.switches = PACKET.aux;
				    sync = (PACKET.buffer[0] >> 6) & 2 | (PACKET.buffer[1] >> 7) & 1;

				    for (i = 0, j = 2; i < (6 >> 1); i++, j += 3)
					{
					// Decode and store even-channel sample
			        PACKET.buffer[i << 1] = ((unsigned short) PACKET.buffer[j]) & 0x7f | (((unsigned short) PACKET.buffer[j+2]) << 3) & 0x380;
			        // Decode and store odd-channel sample
			        PACKET.buffer[(i << 1) + 1] = ((unsigned short) PACKET.buffer[j+1]) & 0x7f | (((unsigned short) PACKET.buffer[j+2]) << 7) & 0x380;
					// Decode and store the sync bits
					sync = (sync << 3) | (int) ((PACKET.buffer[j] >> 5) & 4 | (PACKET.buffer[j+1] >> 6) & 2 | (PACKET.buffer[j+2] >> 7) & 1);
					}	
  				    
				    if (sync==1) // The sync marker is last, so sync should be == 1, or there was an error.
					{   
						PACKET.extract_pos=0;
						process_packets();
					} else PACKET.readstate=0;
				  }
				  break;
		  default: PACKET.readstate=0;
		}
*/
}

/********************************************************************

  Raw binary Data Format:
   
	 1 channel, ´given in two byte (low byte first)

 **********************************************************************/

void parse_byte_raw(unsigned char actbyte)
{
    switch (PACKET.readstate) 
	{
		  case 0: PACKET.buffer[0]=actbyte;
                  PACKET.readstate++; 
				  break;

		  case 1: actbyte++;
				  actbyte++;
			      
				  PACKET.buffer[0]+=actbyte*256; 
				  PACKET.readstate=0;
		  	  	  process_packets();
				  break;
		  default: PACKET.readstate=0;
	}		
}



/********************************************************************

  Raw binary Data Format:
   
	 1 channel, stream of 8 bit values 

 **********************************************************************/

void parse_byte_raw_8bit(unsigned char actbyte)
{
		  PACKET.buffer[0]=actbyte;
		  process_packets();	
}



/*********************************************************************************

  Modular EEG Protocol P21 with add-on ( by Jarek Foltynski and Reiner Münch)

  Packet has variable length, it depends on how many channels we want to receive
  Each channel can be set via backward communication
  
  Packet format
  1cccnnnn
  0pppaaaa
  0bbbbbbb
  ....
  0pppaaaa
  0bbbbbbb
  
  
  ccc  - number of channels in packet, total packet length = 1 + (2 * ccc);
  nnnn - 4 bit control sequence number
  ppp  - channel id, 0-5 are A/D channels, 
  6 is for backward information (requested or error code)
  7 - not used now
  aaaa - if the ppp is 0-5, then the lower three bits of aaaa contains 3 highest 
  bits of 10bit channel value, the highest bit of aaaa is set to 0
         if the ppp is 6, then it contains id/selector of backward information       
  bbbbbbb - if the ppp is 0-5, then it contains 7 lower bits of 10bit value
            if the ppp is 6, then it contains backward information value
			
********************************************************************************/


void parse_byte_P21(unsigned char actbyte)
{

	if (actbyte&128) PACKET.readstate=0;
    
	switch (PACKET.readstate) 
	{
		  case 0: if (actbyte & 128)
				  {
					 PACKET.extract_pos = ((actbyte>>4) & 7);
					 SetDlgItemInt(ghWndStatusbox,IDC_DBG,(unsigned int)((actbyte>>4) & 7),0);
				     PACKET.number = actbyte & 15;
				     PACKET.readstate = 1;
				  }
				  break;

		  case 1: PACKET.chn= (actbyte>>4) & 7;
				  if (PACKET.chn<6) 
				  {
					  PACKET.buffer[PACKET.chn]=(actbyte&7) << 7;
					  PACKET.readstate = 2;
				  } 
				  else 
				  {
					  PACKET.requestedinfo=(actbyte&15);
					  PACKET.readstate=3;
				  }
				  break;

		  case 2: PACKET.buffer[PACKET.chn]+=actbyte; 
				  if (--PACKET.extract_pos) PACKET.readstate=1; 
				  else { PACKET.readstate=0; process_packets(); }
				  break;

		  case 3: PACKET.info=actbyte;
				  if (--PACKET.extract_pos) PACKET.readstate=1;
   				  else { PACKET.readstate=0; process_packets(); }
				  break;

		  default: PACKET.readstate=0;
			      break;
	}		
}

/********************************************************************

  SBG - Smart BrainGames / Pocket Neurobics A3 Protocol

  The two most significant bits of the framing byte are used for 
  synchronisation. They sequence, frame by frame, 00..11. 
  The framing byte is followed by 4 bytes of (signed) channel data (4 chn mode).
  
  Following information is multiplexed to bits 5-0 of the framing bytes
  00:  signal type info: low bat (5) resolution (4,3) sampling rate (2,1,0)
  01:  EEG/EMG: ALC value for ch1 & ch2 (2/4ch op) or ch2 (2ch bipolar op)          
       or HEG: 3 least significant bits for each of ch1 & ch2
  10:  status info: button1+2 state (5,4) chn4-1 state (3,2,1,0)
  11:  EEG/EMG: ALC value for ch3 & ch4 (2/4ch op*) or ch2 (2ch bipolar op)          
       or HEG: 3 least significant bits for each of ch3 & ch4
  

 **********************************************************************/

void parse_byte_SBG(unsigned char actbyte)
{
	static char synctry=0;
	static int syncval=3;
	static int syncpos=0;
    static int success=0;
	char tmpstr[20];

	if (!syncpos)
	{
	    if (((int)actbyte & 0xc0) == (syncval<<6))
		{    if (success<10) success++; 
		     syncval = (syncval+1) % 4; 
	         synctry=0; 
		}
	    else 
		{
		  success=0;
		  synctry++;
		  if (synctry>5) {syncpos=1; synctry=0; }
		}
	}

    switch (syncpos) 
	{
		  case 0:  if (success==10) PACKET.buffer[4]=100; 
  			       else PACKET.buffer[4]=0; 
			       process_packets();
				  break;

		  case 1: PACKET.buffer[0]=(int)((char)actbyte+128)*4; 
				  break;

		  case 2: PACKET.buffer[1]=(int)((char)actbyte+128)*4; 
				  break;

		  case 3: PACKET.buffer[2]=(int)((char)actbyte+128)*4; 
				  break;

		  case 4: PACKET.buffer[3]=(int)((char)actbyte+128)*4; 
				  break;
	}
	if (!syncpos) wsprintf(tmpstr,"snc=%d (%d)",actbyte,success);
	else wsprintf(tmpstr,"%d",syncpos);
	//SendDlgItemMessage(ghWndStatusbox,IDC_LIST2, LB_ADDSTRING, 0, (LPARAM) tmpstr);
	if (++syncpos>4) syncpos=0;
		  
}



/********************************************************************

  PendantV3 - Pendant EEG Protocol

  The two most significant bits of the framing byte are used for 
  synchronisation. They sequence, frame by frame, 00..11. 
  The framing byte is followed by 4 bytes of (signed) channel data:
     chn1 MSB (8bit)
	 chn2 MSB (8bit)
	 chn1 LSB (4bit, left justified)
	 chn2 LSB (4bit, left justified)
  
  Following information is multiplexed to bits 5-0 of the framing bytes
  00:  signal type info: low bat (5) resolution (4,3) sampling rate (2,1,0)
  01:  EEG/EMG: ALC value for chn1           
  10:  status info: button1+2 state (5,4) chn2-1 state (1,0)
  11:  EEG/EMG: ALC value for chn2          
  

 **********************************************************************/

void parse_byte_PendantV3(unsigned char actbyte)
{
	static char synctry=0;
	static int syncval=3;
	static int syncpos=0;
    static int success=0;
	static int SR=256;
	static int O_SR=256;
//	char tmpstr[20];

	if (!syncpos)
	{
	    if (((int)actbyte & 0xc0) == (syncval<<6))
		{    if (success<10) success++; 
		     syncval = (syncval+1) % 4; 
	         synctry=0; 
		}
	    else 
		{
		  success=0;
		  synctry++;
		  if (synctry>5) {syncpos=1; synctry=0; }
		}
	}

	if (success<10) PACKET.buffer[2]=0;
	else
	{ 
		PACKET.buffer[2]=100; 
		switch (syncpos) 
		{
		  case 0:  
				   switch (actbyte >> 6)
				   { 
				     case 0: if ((actbyte & 7)==0) SR=122;
							 if ((actbyte & 7)==1) SR=128;
							 if ((actbyte & 7)==2) SR=256;
							 if ((actbyte & 7)==3) SR=512;
							  if (O_SR!=SR) { O_SR=SR; update_samplingrate (SR);}
							 break;
				     case 1: GLOBAL.P3ALC1= actbyte & 0x3f; break;
					 case 2: PACKET.switches = (actbyte >> 4) & 3; break;
				     case 3: GLOBAL.P3ALC2= actbyte & 0x3f; break;
				   }
			       process_packets();
				  break;

		  case 1: if (actbyte>128) PACKET.buffer[0]=actbyte;
				  else PACKET.buffer[0]=actbyte+256;
				  PACKET.buffer[0]<<=4;
				  break;

		  case 2: if (actbyte<128) PACKET.buffer[1]=256+actbyte;
				  else   PACKET.buffer[1]=actbyte;
				  PACKET.buffer[1]<<=4; 
				  break;

		  case 3: PACKET.buffer[0]+=(actbyte>>4);
				  break;

		  case 4: PACKET.buffer[1]+=(actbyte>>4); 
				  break;
		}
	}
	if (++syncpos>4) syncpos=0;
		  
}


/********************************************************************

    Package the data according to the "QDS NFB 256" protocol

    Byte 0      1st Sync byte    = 0xcc
    Byte 1      2nd Sync byte    = 0x33
    Byte 2      3nd Sync byte    = 0xcc
    Byte 3      Unknown meaning  = 0x00
    Byte 4/5    Channel 1 data
    Byte 6/7    Channel 2 data
    Byte 8/9    Channel 3 data, not used
    Byte 10/11  Channel 4 data, not used
    Byte 12/13  Channel 5 data, not used
    Byte 14/15  Channel 6 data, not used
    Byte 16/17  Channel 6 data, not used
    Byte 18/19  Channel 6 data, not used

    LSB is sent first, then MSB
    Data is 16 bits in 2's complement (15-bits + sign)

 **********************************************************************/


void parse_byte_QDS(unsigned char actbyte)
{
    switch (PACKET.readstate) 
	{
		  case 0: if (actbyte==204) PACKET.readstate++;  break;
		  case 1: if (actbyte==51)  PACKET.readstate++;  break;
		  case 2: if (actbyte==204) PACKET.readstate++;  break;
		  case 3: PACKET.readstate++;  break;
		  case 4: if (PACKET.extract_pos < 16)
				  {   if ((PACKET.extract_pos & 1) == 0)
					     PACKET.buffer[PACKET.extract_pos>>1]=actbyte*256;
			          else PACKET.buffer[PACKET.extract_pos>>1]+=actbyte;
					  PACKET.extract_pos++;
				  }
				  else
				  {  PACKET.switches= actbyte;
					 PACKET.readstate=0;
		  	  	     process_packets();
				  }
		  		  break;

		  default: PACKET.readstate=0;
		}		
}

void ParseLocalInput(int BufLen)
{
	unsigned char actbyte;

	unsigned int pbufcnt;
	
	for (pbufcnt=0;pbufcnt<(unsigned int)BufLen;pbufcnt++)
	{
		actbyte=TTY.readBuf[pbufcnt];
		if (CAPTFILE.do_write) write_captfile(actbyte);
        switch (TTY.devicetype)
		{
			case DEV_MODEEG_P2:	parse_byte_P2 (actbyte); break;
			case DEV_MODEEG_P3:	parse_byte_P3 (actbyte); break;
			case DEV_RAW:       parse_byte_raw(actbyte); break;
			case DEV_P21:       parse_byte_P21(actbyte); break;
			case DEV_SBG:       parse_byte_SBG(actbyte); break;
			case DEV_RAW8BIT:   parse_byte_raw_8bit(actbyte); break;
			case DEV_PENDANT3:  parse_byte_PendantV3(actbyte); break;
			case DEV_QDS:       parse_byte_QDS(actbyte); break;
			case DEV_NIA:		parse_byte_NIA(actbyte); break;

		}
	}
	
	return;
}



void enable_buttons(HWND hDlg)
{
	EnableWindow(GetDlgItem(hDlg, IDC_REC_ARCHIVE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_REC), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_OPEN_ARCHIVE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_ARCHIVE), FALSE);

	if ((TTY.CONNECTED)) //&&(!TTY.read_pause))
	{
	  if (CAPTFILE.do_write)
		  EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_REC), TRUE);
	  else if (CAPTFILE.file_action!=FILE_WRITING)
	  {
		  EnableWindow(GetDlgItem(hDlg, IDC_REC_ARCHIVE), TRUE);
		  if (CAPTFILE.file_action==FILE_READING)
   		      EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_ARCHIVE), TRUE);
   		  else  EnableWindow(GetDlgItem(hDlg, IDC_OPEN_ARCHIVE), TRUE);
	  }
	}
	else
	{
		if (strcmp(CAPTFILE.filename,"none"))
		{
			if (CAPTFILE.do_read)
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_ARCHIVE), TRUE);
			else
				EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_ARCHIVE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_PLAY_ARCHIVE), TRUE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_ARCHIVE), TRUE);
			if (TTY.CONNECTED)
				EnableWindow(GetDlgItem(hDlg, IDC_PLAY_COM), TRUE);
		}
				
	}
}

int sendcommand(unsigned char cmd, unsigned char data1, unsigned char data2)
{


/*	if (!TTY.BIDIRECT)  report("not bidirect");
	if (TTY.COMDEV==INVALID_HANDLE_VALUE)  report("inv. comdev");
	if (!TTY.CONNECTED) report("not connected");

  	if (data1==VINFO_CHANNELS_MATRIX)
	{
		char tmp[200];

		wsprintf(tmp,"chnmatix: %04X",data2);
		report(tmp);
	}

*/

	if ((!TTY.BIDIRECT) || (TTY.COMDEV==INVALID_HANDLE_VALUE) || (!TTY.CONNECTED)) return(0);

	//if (data1!=VINFO_CHANNELS_MATRIX)
	{
		write_to_comport(cmd);
		write_to_comport(data1);
		write_to_comport(data2);
   	    PACKET.readstate=0;
	}

	return(1); //WriterProc(NULL));
}

void update_ports(HWND hDlg, EEGOBJ * st)
{
	int i,x;

	switch (TTY.devicetype)
	{	

	case DEV_PENDANT3:
		st->chnmatrix=0x0f;
		x=0;
  	    sprintf(st->out_ports[x].out_name,"Chn1");
	    sprintf(st->out_ports[x].out_desc,"EEG Channel1");
	    strcpy(st->out_ports[x].out_dim,"none");
	    st->out_ports[x].get_range=-1;
     	st->out_ports[x].out_min=-1000.0f;
	    st->out_ports[x].out_max=1000.0f;
        x++;
 	    sprintf(st->out_ports[x].out_name,"Chn2");
	    sprintf(st->out_ports[x].out_desc,"EEG Channel2");
	    strcpy(st->out_ports[x].out_dim,"none");
	    st->out_ports[x].get_range=-1;
     	st->out_ports[x].out_min=-1000.0f;
	    st->out_ports[x].out_max=1000.0f;
        x++;
 	    sprintf(st->out_ports[x].out_name,"valid");
	    sprintf(st->out_ports[x].out_desc,"Signal valid indication");
	    strcpy(st->out_ports[x].out_dim,"none");
	    st->out_ports[x].get_range=-1;
     	st->out_ports[x].out_min=0.0f;
	    st->out_ports[x].out_max=100.0f;
        x++;
 	    sprintf(st->out_ports[x].out_name,"bt1+2");
	    sprintf(st->out_ports[x].out_desc,"Button 1+2 state");
	    strcpy(st->out_ports[x].out_dim,"none");
	    st->out_ports[x].get_range=-1;
     	st->out_ports[x].out_min=0.0f;
	    st->out_ports[x].out_max=3.0f;
		break;

	default:
		if (hDlg)
		{
		  st->chnmatrix=0;
		  if (IsDlgButtonChecked(hDlg,IDC_CH1)) st->chnmatrix|=1; 
		  if (IsDlgButtonChecked(hDlg,IDC_CH2)) st->chnmatrix|=2; 
		  if (IsDlgButtonChecked(hDlg,IDC_CH3)) st->chnmatrix|=4; 
		  if (IsDlgButtonChecked(hDlg,IDC_CH4)) st->chnmatrix|=8; 
		  if (IsDlgButtonChecked(hDlg,IDC_CH5)) st->chnmatrix|=16;
		  if (IsDlgButtonChecked(hDlg,IDC_CH6)) st->chnmatrix|=32; 
		  if (IsDlgButtonChecked(hDlg,IDC_EXTEND)) st->chnmatrix|=128;
		}

		//st->chnmatrix=127;
		for (i=0,x=0;i<6;i++)
		{
		  if (st->chnmatrix&(1<<i))
		  {
			  sprintf(st->out_ports[x].out_name,"chn %d",i+1);
			  sprintf(st->out_ports[x].out_desc,"EEG Channel %d",i+1);
			  strcpy(st->out_ports[x].out_dim,"uV");
			  //st->out_ports[x].get_range=-1;
			  //st->out_ports[x].out_min=-256.0f;
			  //st->out_ports[x].out_max=256.0f;
			  x++;
		  }
		}
		sprintf(st->out_ports[x].out_name,"b1-b4");
		sprintf(st->out_ports[x].out_desc,"EEG Buttons");
		strcpy(st->out_ports[x].out_dim,"none");
		st->out_ports[x].get_range=-1;
		st->out_ports[x].out_min=0.0f;
		st->out_ports[x].out_max=15.0f;
		break;
	}

	st->outports=x+1;
	st->height=CON_START+st->outports*CON_HEIGHT+5;

	if ((st->chnmatrix)&128)
	{
		/* extend buttons */
		x++;
		sprintf(st->out_ports[x].out_name,"b1");
		sprintf(st->out_ports[x].out_desc,"EEG Button 1");
		strcpy(st->out_ports[x].out_dim,"none");
		st->out_ports[x].get_range=-1;
		st->out_ports[x].out_min=0.0f;
		st->out_ports[x].out_max=1.0f;

		x++;
		sprintf(st->out_ports[x].out_name,"b2");
		sprintf(st->out_ports[x].out_desc,"EEG Button 2");
		strcpy(st->out_ports[x].out_dim,"none");
		st->out_ports[x].get_range=-1;
		st->out_ports[x].out_min=0.0f;
		st->out_ports[x].out_max=1.0f;

		x++;
		sprintf(st->out_ports[x].out_name,"b3");
		sprintf(st->out_ports[x].out_desc,"EEG Button 3");
		strcpy(st->out_ports[x].out_dim,"none");
		st->out_ports[x].get_range=-1;
		st->out_ports[x].out_min=0.0f;
		st->out_ports[x].out_max=1.0f;

		x++;
		sprintf(st->out_ports[x].out_name,"b4");
		sprintf(st->out_ports[x].out_desc,"EEG Button 4");
		strcpy(st->out_ports[x].out_dim,"none");
		st->out_ports[x].get_range=-1;
		st->out_ports[x].out_min=0.0f;
		st->out_ports[x].out_max=1.0f;
  
		st->outports=x+1;
		st->height=CON_START+st->outports*CON_HEIGHT+5; 
	}

}


void update_p21state(void)
{
	if ((TTY.devicetype!=DEV_P21) && (TTY.devicetype!=DEV_MODEEG_P2)) return;

	if (TTY.devicetype==DEV_P21) sendcommand(CMD_SET_VINFO, VINFO_PROTOCOL_NUMBER, 21);	
	if (TTY.devicetype==DEV_MODEEG_P2) sendcommand(CMD_SET_VINFO, VINFO_PROTOCOL_NUMBER, 2);

	switch(SamplingRateTable[TTY.samplingrate])
	{
		case 256:  sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 0);break;
		case 300:  sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 1);break;
		case 512:  sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 2);break;
		case 600:  sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 3);break;
		case 1024: sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 4);break;
		case 1200: sendcommand(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 5);break;
	}
	switch(TTY.BAUDRATE)
	{
		case 57600:   sendcommand(CMD_SET_VINFO, VINFO_BAUD_RATE, 0);break;
		case 115200:  sendcommand(CMD_SET_VINFO, VINFO_BAUD_RATE, 1);break;
		case 230400:  sendcommand(CMD_SET_VINFO, VINFO_BAUD_RATE, 2);break;
	}


}

/*--------------------------------------------------------------------------------

  FUNCTION: CALLBACK EEGDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
  PURPOSE : Handles Messages for the EEGOBJ-Settings Dialog

-----------------------------------------------------------------------------*/

LRESULT CALLBACK EEGDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	char szBuffer[ MAXLEN_TEMPSTR ], strfloat[21];
    WORD wPosition;
	int t;
	EEGOBJ * st =(EEGOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_EEG)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
		{
			SCROLLINFO lpsi;
				
		    for (t = 0; t < MAX_COMPORT; t++) 
			{
				wsprintf( szBuffer, "COM%d", t + 1 ) ;
				SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) szBuffer ) ;
			}
			if (TTY.PORT) SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_SETCURSEL, (WPARAM) (TTY.PORT - 1), 0L ) ;
			else SetDlgItemText( hDlg, IDC_PORTCOMBO, "none") ;
			for (t = 0; BaudTable[t]!=0 ; t++) 
			{
				wPosition = LOWORD( SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) szBaud[t] ) ) ;
				SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_SETITEMDATA, (WPARAM) wPosition, (LPARAM) BaudTable[t]) ;
				if (BaudTable[t] == TTY.BAUDRATE) SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_SETCURSEL, (WPARAM) wPosition, 0L ) ;
			}
			for (t=0; devicetypes[t][0]!=0;t++)
				SendDlgItemMessage( hDlg, IDC_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) devicetypes[t]) ;
			SendDlgItemMessage( hDlg, IDC_DEVICECOMBO, CB_SETCURSEL, TTY.devicetype, 0L ) ;

			for (t=0; samplingrates[t][0]!=0;t++)
				SendDlgItemMessage( hDlg, IDC_SAMPLINGCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) samplingrates[t]) ;
			SendDlgItemMessage( hDlg, IDC_SAMPLINGCOMBO, CB_SETCURSEL, TTY.samplingrate, 0L ) ;

			for (t=0; captfiletypes[t][0]!=0;t++)
				SendDlgItemMessage( hDlg, IDC_FILEMODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) captfiletypes[t]) ;
			SendDlgItemMessage( hDlg, IDC_FILEMODECOMBO, CB_SETCURSEL, CAPTFILE.filetype, 0L ) ;
	
			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=0; lpsi.nMax=1000;
			SetScrollInfo(GetDlgItem(hDlg,IDC_ARCHIVE_POSBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_ARCHIVE_POSBAR), SB_CTL, 0, 1);

			if(st->chnmatrix&1) CheckDlgButton(hDlg,IDC_CH1, TRUE); 
			if(st->chnmatrix&2) CheckDlgButton(hDlg,IDC_CH2, TRUE); 
			if(st->chnmatrix&4) CheckDlgButton(hDlg,IDC_CH3, TRUE); 
			if(st->chnmatrix&8) CheckDlgButton(hDlg,IDC_CH4, TRUE); 
			if(st->chnmatrix&16) CheckDlgButton(hDlg,IDC_CH5, TRUE); 
			if(st->chnmatrix&32) CheckDlgButton(hDlg,IDC_CH6, TRUE); 
			if(st->chnmatrix&128) CheckDlgButton(hDlg,IDC_EXTEND, TRUE); 

			CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
			CheckDlgButton(hDlg, IDC_BIDIRECT, TTY.BIDIRECT);
			CheckDlgButton(hDlg, IDC_FLOW_CONTROL, TTY.FLOW_CONTROL);
			SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,CAPTFILE.filename);
			SetDlgItemInt(hDlg,IDC_SAMPLINGRATE,PACKETSPERSECOND,0);
			sprintf(strfloat,"%.2f",(float)CAPTFILE.offset/(float)PACKETSPERSECOND);
			SetDlgItemText(hDlg,IDC_OFFSET,strfloat);
			SetDlgItemInt(hDlg,IDC_RESOLUTION,(long)st->resolution,0);
			enable_buttons(hDlg);

			return TRUE;
		}
		case WM_COMMAND:
            switch (LOWORD(wParam)) 
			{
				case IDC_PORTCOMBO:
					if ((HIWORD(wParam)==CBN_SELCHANGE) && (TTY.devicetype != DEV_NIA))
					{
						if (TTY.COMDEV!=INVALID_HANDLE_VALUE) 
							BreakDownCommPort(); 
						TTY.PORT=SendDlgItemMessage(hDlg, IDC_PORTCOMBO, CB_GETCURSEL, 0, 0 )+1 ;

						CheckDlgButton(hDlg,IDC_CONNECTED,FALSE);
						enable_buttons(hDlg);
					}
					break;
				case IDC_BAUDCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   int sel;
					    
						sel=SendDlgItemMessage(hDlg, IDC_BAUDCOMBO, CB_GETCURSEL, 0, 0 ) ;
						TTY.BAUDRATE=BaudTable[sel];
						update_p21state();

						if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (TTY.devicetype != DEV_NIA))
						{
						    BreakDownCommPort(); 
							TTY.CONNECTED=SetupCommPort(TTY.PORT);
							CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
						}
						
						enable_buttons(hDlg);
					}
					break;
				case IDC_SAMPLINGCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{						
						TTY.samplingrate=SendMessage(GetDlgItem(hDlg, IDC_SAMPLINGCOMBO), CB_GETCURSEL , 0, 0);
						update_samplingrate(SamplingRateTable[TTY.samplingrate]);
						update_p21state();						
					}
					break;

				case IDC_CONNECT:
						if (TTY.CONNECTED) {
							if (TTY.devicetype == DEV_NIA){		
								DisconnectNIA(); 
								TTY.CONNECTED=FALSE;			
							} else {
								BreakDownCommPort();
							}
							CheckDlgButton(hDlg, IDC_CONNECTED, FALSE);
						} else {
							if (TTY.devicetype == DEV_NIA){		
								if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
									DisconnectNIA();								
								if ((TTY.CONNECTED = ConnectNIA(hDlg)) == FALSE) {
									report_error("No NIA found!");
									break;
								}
								update_samplingrate(NIASAMPLINGRATE);
							} else {
								if (TTY.COMDEV!=INVALID_HANDLE_VALUE) 
									BreakDownCommPort();								
								TTY.CONNECTED=SetupCommPort(TTY.PORT);
							}
							CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
						}
						enable_buttons(hDlg);
					break;

				case IDC_CONNECTED:
						CheckDlgButton(hDlg,IDC_CONNECTED, TTY.CONNECTED);
					break;

				case IDC_BIDIRECT:
						TTY.BIDIRECT=IsDlgButtonChecked(hDlg,IDC_BIDIRECT);
					break;
				case IDC_FLOW_CONTROL:
					TTY.FLOW_CONTROL=IsDlgButtonChecked(hDlg,IDC_FLOW_CONTROL);
  				    if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
					{
					    BreakDownCommPort(); 
						TTY.CONNECTED=SetupCommPort(TTY.PORT);
						CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
					}
					break;

				case IDC_DEVICECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
						TTY.devicetype=SendMessage(GetDlgItem(hDlg, IDC_DEVICECOMBO), CB_GETCURSEL , 0, 0);
					
						update_devicetype ();
						update_ports(hDlg, st);
						update_p21state();
							
					}
					break;
				case IDC_FILEMODECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE) CAPTFILE.filetype=SendMessage(GetDlgItem(hDlg, IDC_FILEMODECOMBO), CB_GETCURSEL , 0, 0);
					break;

				case IDC_OPEN_ARCHIVE:
					close_captfile();strcpy(CAPTFILE.filename,GLOBAL.resourcepath);
					strcat(CAPTFILE.filename,"ARCHIVES\\*.arc");
					
					if (open_file_dlg(ghWndMain,CAPTFILE.filename, FT_ARCHIVE, OPEN_LOAD))
					{
						open_captfile(CAPTFILE.filename);
						if (CAPTFILE.filehandle==INVALID_HANDLE_VALUE)
						{
							SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,"none");
							report_error("Could not open Archive-File");
						}
						else
						{
							TTY.read_pause=1;
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
							SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,CAPTFILE.filename);
							SetDlgItemText(hDlg,IDC_DEVICECOMBO,devicetypes[TTY.devicetype]);
							SetDlgItemText(hDlg,IDC_FILEMODECOMBO,captfiletypes[CAPTFILE.filetype]);
//							reset_oscilloscopes();
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RESETBUTTON,0);
							enable_buttons(hDlg);
						}
					}
					break;
				case IDC_CLOSE_REC:
					 TTY.read_pause=1;
					 if (CAPTFILE.file_action==FILE_WRITING)
					 {
	 					close_captfile();
						SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,"none");
					 }
					 enable_buttons(hDlg);
					break;
				case IDC_PLAY_ARCHIVE:
					if(CAPTFILE.filehandle==INVALID_HANDLE_VALUE) break;
					QueryPerformanceCounter((_LARGE_INTEGER *)&TIMING.readtimestamp);
					CAPTFILE.do_read=1;
					enable_buttons(hDlg);
					start_timer();
					break;
				case IDC_CH1:
				case IDC_CH2:
				case IDC_CH3:
				case IDC_CH4:
				case IDC_CH5:
				case IDC_CH6:
				case IDC_EXTEND: update_ports(hDlg,st);
					   sendcommand(CMD_SET_VINFO,VINFO_CHANNELS_MATRIX,(unsigned char) (st->chnmatrix&63));
					   InvalidateRect(ghWndDesign,NULL,TRUE);
					   break;
				case IDC_REC_ARCHIVE:
					close_captfile();
					strcpy(CAPTFILE.filename,GLOBAL.resourcepath);
					strcat(CAPTFILE.filename,"ARCHIVES\\*.arc");
					if (open_file_dlg(ghWndMain,CAPTFILE.filename, FT_ARCHIVE, OPEN_SAVE))
					{
						CAPTFILE.filehandle=create_captfile(CAPTFILE.filename);
						if(CAPTFILE.filehandle) 
						{    SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,CAPTFILE.filename);
							 CAPTFILE.do_write=1;
							 enable_buttons(hDlg);
						}
						else
						{  
							SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,"none");
							report_error("Could not Create Archive-File!");
						}
					}
					break;
				case IDC_CLOSE_ARCHIVE:
					strcpy(CAPTFILE.filename,"none");
					// if(CAPTFILE.filehandle==INVALID_HANDLE_VALUE) break;
					close_captfile();
					SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,"none");
					SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
					enable_buttons(hDlg);
					break;
				case IDC_APPLYOFFSET:
						GetDlgItemText(hDlg, IDC_OFFSET, strfloat, 20);
						CAPTFILE.offset = (int)((float)atof(strfloat) * (float) PACKETSPERSECOND);
						get_session_length();
						st->session_pos(TIMING.packetcounter);
					break;
				case IDC_RESOLUTION:
					{ long l = GetDlgItemInt(hDlg, IDC_RESOLUTION, NULL,0);
					if ((l>2)&&(l<80000000))
							st->resolution=(float) l;
					}
					break;

				
			}	
			break;
			//return(TRUE);

		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return(TRUE);
		case WM_SIZE:
		case WM_MOVE:  
				update_toolbox_position(hDlg);
				return(TRUE);


	}
    return FALSE;
}




EEGOBJ::EEGOBJ(int num) : BASE_CL()	
	{

		inports = 0;
 		outports = 7;
		scrolling=0;

		chnmatrix=63;
		resolution=1024.0f;
		update_ports(NULL,this);

	}

  	  void EEGOBJ::session_start(void) 
	  {	
		  TTY.read_pause=1;
		  if((CAPTFILE.filehandle!=INVALID_HANDLE_VALUE)&& (CAPTFILE.file_action==FILE_READING))
		  {
				QueryPerformanceCounter((_LARGE_INTEGER *)&TIMING.readtimestamp);
				CAPTFILE.do_read=1;

		  }
		  else if (TTY.CONNECTED)
		  { 
			  if ((CAPTFILE.filehandle==INVALID_HANDLE_VALUE)||(CAPTFILE.file_action==FILE_WRITING))
			     TTY.read_pause=0;
	  		  sendcommand(CMD_SET_VINFO, VINFO_RUNEEG ,1);

		  }
		  if (ghWndToolbox==hDlg)  enable_buttons(hDlg);

		
	  }
  	  void EEGOBJ::session_stop(void) 
	  {	
		    sendcommand(CMD_SET_VINFO, VINFO_RUNEEG ,0);
			CAPTFILE.do_read=0;TTY.read_pause=1; //CAPTFILE.do_write=0;
			if (hDlg==ghWndToolbox) enable_buttons(hDlg);
	  }
  	  void EEGOBJ::session_reset(void) 
	  {	
		    sendcommand(CMD_SET_VINFO, VINFO_RUNEEG ,0);
			CAPTFILE.do_read=0;CAPTFILE.do_write=0;TTY.read_pause=1;
			if (hDlg==ghWndToolbox) enable_buttons(hDlg);
			if(CAPTFILE.filehandle!=INVALID_HANDLE_VALUE) SetFilePointer(CAPTFILE.filehandle,CAPTFILE.data_begin,NULL,FILE_BEGIN);

	  }
  	  void EEGOBJ::session_pos(long pos) 
	  {	
			if(CAPTFILE.filehandle==INVALID_HANDLE_VALUE) return;
			pos-=CAPTFILE.offset;
			if (pos<0) pos=0;
			if (pos>CAPTFILE.length) pos=CAPTFILE.length;
			SetFilePointer(CAPTFILE.filehandle,CAPTFILE.data_begin+pos*TTY.bytes_per_packet,NULL,FILE_BEGIN);
	  } 

	  long EEGOBJ::session_length(void) 
	  {
			if ((CAPTFILE.filehandle!=INVALID_HANDLE_VALUE) && (CAPTFILE.file_action==FILE_READING))
				return(CAPTFILE.offset+CAPTFILE.length); else return(0);
	  }

	  void EEGOBJ::make_dialog(void) 
	  {  
		  	  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EEGBOX, ghWndStatusbox, (DLGPROC)EEGDlgHandler));
	  }

  	  void EEGOBJ::load(HANDLE hFile) 
	  {	
		  
			load_object_basics(this);
			load_property("chnmatrix", P_INT, &chnmatrix);
			load_property("resolution", P_FLOAT, &resolution);
			
			update_ports(NULL,this);
			sendcommand(CMD_SET_VINFO,VINFO_CHANNELS_MATRIX,(unsigned char) (chnmatrix&63));
						
	  }

	  void EEGOBJ::save(HANDLE hFile) 
	  {	  
			save_object_basics(hFile, this);
			save_property(hFile, "chnmatrix", P_INT, &chnmatrix);
			save_property(hFile,"resolution", P_FLOAT, &resolution);
			

	  }

	  void EEGOBJ::work(void) 
	  {
		int i,x;

        switch (TTY.devicetype)
		{
			case DEV_PENDANT3:
				pass_values(0,((float) PACKET.buffer[0])-4096);
		    	pass_values(1,((float) PACKET.buffer[1])-4096);
		    	pass_values(2,(float) (PACKET.buffer[2]));  // valid indicator
		    	pass_values(3,(float) (PACKET.switches));  

				break;
			case DEV_NIA:
			    for (i=0,x=0;i<6;i++)
			    {
				  if (chnmatrix & (1<<i))
				  {
					pass_values(x,(float) PACKET.buffer[i] * (out_ports[x].out_max-out_ports[x].out_min) / resolution - 16840000.0f / resolution);
									//NIA Normierung: Shiften auf Nulllinie 
				  }
			    }
				break;
			default:  // all other devices
			  for (i=0,x=0;i<6;i++)
			  {
				if (chnmatrix & (1<<i))
				{
					pass_values(x,(float) PACKET.buffer[i] * (out_ports[x].out_max-out_ports[x].out_min) / resolution + out_ports[x].out_min);
					x++;
				}
			  }

			  pass_values(x,(float)PACKET.switches);

			  if (chnmatrix&128)
			  {
				x++;
				pass_values(x, (float)(!(PACKET.switches&1)));
				x++;
				pass_values(x, (float)(!(PACKET.switches&2)));
				x++;
				pass_values(x, (float)(!(PACKET.switches&4)));
				x++;
				pass_values(x, (float)(!(PACKET.switches&8)));
			  }
			  break;

		}


		if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox) && (!scrolling) && (CAPTFILE.do_read)) 
		{
			DWORD x= SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_CURRENT);
			x=x*1000/CAPTFILE.length/TTY.bytes_per_packet;
			SetScrollPos(GetDlgItem(hDlg, IDC_ARCHIVE_POSBAR), SB_CTL, x, 1);
		}
	  }

EEGOBJ::~EEGOBJ()
	  {
	//	BreakDownCommPort();
	    close_captfile();
	  }  



