/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
			   
  Author: Chris Veigl, contact: chris@shifz.org
  
  Co-Authors:
		 Jeremy Wilkerson (Modules: AND, OR, NOT, WAV, CORRELATION, EVALUATOR)
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
#define VINFO_PROTOCOL_SUBNUMBER 9 	// view sub-version of protocol 21 (read only)


  char devicetypes[][40]   = {"ModularEEG P2","ModularEEG P3","1 Channel Raw Data","MonolithEEG P21","SmartBrainGames 4Chn","1Chn of 8bit values", "Pendant EEG v3", "QDS NFB 256", "NIA USB HDI Ver 1.4","IBVA 4-chn","SBT 2 Channel BT", "OpenBCI 8 Channels", "OPI TrueSense Exploration Kit", "OpenBCI 16 Channels", "Neurosky MindWave","\0"};
  int  AMOUNT_TO_READ   []  = {     68,               66,               8           ,  21 ,            25   ,                  4   ,                  25,              40         ,    6 ,                  16 ,             24           ,         66       ,        152                    ,          114            ,      48      };
  int  BYTES_PER_PACKET []  = {     17,               11,               2           ,  7  ,             5   ,                  1   ,                  5,               20         ,    6 ,                  16 ,             6            ,         33       ,        1                      ,          57             ,      8       };



char captfiletypes[][40] = {"Text (Debug Mode)","Integer Values","\0"};

char * szBaud[] = {"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600", "\0"};  // Combobox - Items for Baudrate
DWORD   BaudTable[] =  {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 0 } ;  // Constants for Baudrate Setting

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
  x = auxiliary channel byte
  a - f = 10-bit samples from ADC channels 0 - 5
  - = unused, must be zero
 
  There are 8 auxiliary channels that are transmitted in sequence.
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
	
}

/********************************************************************

  16 bit Raw binary Data Format:
   
	 1 channel, given in two byte (low byte first)

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

  SBT 4chn - Smart Brain Technologies / Pocket Neurobics A3 Protocol

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

void parse_byte_SBT4(unsigned char actbyte)
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

  SBT 2chn BT- Smart Brain Technologies 

  3 bytes data chn 0 (MSB first, 2's complement)  
  3 bytes data chn 1 (MSB first, 2's complement)  
  1 status byte

 **********************************************************************/

void parse_byte_SBT2(unsigned char actbyte)
{
	static int syncpos=0;
    switch (syncpos) 
	{
		  case 0: PACKET.buffer[0]= ((unsigned int)actbyte) << 16;
				  break;
		  case 1: PACKET.buffer[0]+= ((unsigned int)actbyte) << 8; 
				  break;
		  case 2: PACKET.buffer[0]+=actbyte; 
				  break;
		  case 3: PACKET.buffer[1]=  ((unsigned int)actbyte) << 16;
				  break;
		  case 4: PACKET.buffer[2]+= ((unsigned int)actbyte) << 8; 
				  break;
		  case 5: PACKET.buffer[3]+=actbyte; 
				  break;
		  case 6: PACKET.aux=actbyte;
				  if (PACKET.buffer[0] & (1<<23)) PACKET.buffer[0]-=(1<<23);
				  else PACKET.buffer[0]+=(1<<23);
				  if (PACKET.buffer[1] & (1<<23)) PACKET.buffer[1]-=(1<<23);
				  else PACKET.buffer[1]+=(1<<23);
				  process_packets();
				  break;
	}		  
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
    Byte 2      3rd Sync byte    = 0xcc
    Byte 3      Unknown meaning  = 0x00
    Byte 4/5    Channel 1 data
    Byte 6/7    Channel 2 data
    Byte 8/9    Channel 3 data (only used in some devices)
    Byte 10/11  Channel 4 data (only used in some devices)
    Byte 12/13  Channel 5 data (only used in some devices)
    Byte 14/15  Channel 6 data (only used in some devices)
    Byte 16/17  Channel 7 data (only used in some devices)
    Byte 18/19  Channel 8 data (only used in some devices)

    LSB is sent first, then MSB
    Data is 16 bits in 2's complement (15-bits + sign)

 **********************************************************************/


void parse_byte_QDS(unsigned char actbyte)
{
	static short buff=0;

    switch (PACKET.readstate) 
	{
		  case 0: if (actbyte==204) PACKET.readstate++;  break;
		  case 1: if (actbyte==51)  PACKET.readstate++;  break;
		  case 2: if (actbyte==204) PACKET.readstate++;  break;
		  case 3: PACKET.extract_pos=0; PACKET.readstate++; break;
		  case 4: if ((PACKET.extract_pos & 1) == 0) {
						buff =  actbyte;
					     PACKET.buffer[PACKET.extract_pos>>1]=buff;
				  }
			      else {
					  buff =  actbyte*256;
					  PACKET.buffer[PACKET.extract_pos>>1]+=buff+32768;
				  }
				  PACKET.extract_pos++;
				  if (PACKET.extract_pos>=16)
				  { 
					 PACKET.readstate=0;
		  	  	     process_packets();
				  }
		  		  break;

		  default: PACKET.readstate=0;
		}		
}

/********************************************************************

    Package the data according to the "IBVA 2 Channel" protocol

********************************************************************/
void parse_byte_IBVA(unsigned char actbyte)
{
	static int chn_index=0;
	static int digit=3;
	int val;

	if (actbyte == 0x0d) { digit=3;chn_index=0;}
	else if (((actbyte >='0') && (actbyte <='9')) || ((actbyte >='a') && (actbyte <='f')))
	{
		if (actbyte <='9') val=actbyte-'0'; else val=actbyte-'a'+10;
		if (digit==3) {
			if (actbyte=='b') {chn_index=5;val=0;digit++;}
			PACKET.buffer[chn_index]=0;
		}
		digit--;
		PACKET.buffer[chn_index]+= val << (digit*4);
		if (digit==0) {
//			PACKET.buffer[chn_index]-=0x200;
			digit=3; chn_index++;
			if (chn_index ==4) { process_packets(); chn_index=0; }
		}
	}
}


/********************************************************************

  Packet Format for OCZ NIA:
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
			PACKET.buffer[0]=actbyte ;
			PACKET.readstate++; 
			break;
		case 1:
			PACKET.buffer[0]+=actbyte*256 ;
			PACKET.readstate++; 
			break;
		case 2:
			PACKET.buffer[0]+=actbyte * 65536;
			PACKET.readstate++; 
		//	PACKET.readstate=0;     // 1 channel version
		//	process_packets();      // 1 channel version
			break;
		case 3:
			PACKET.buffer[1]=actbyte ;
			PACKET.readstate++; 
			break;
		case 4:
			PACKET.buffer[1]+=actbyte * 256;
			PACKET.readstate++; 
			break;
		case 5:
			PACKET.buffer[1]+=actbyte * 65536;
			PACKET.readstate=0; 
			process_packets();
			break;
		default: PACKET.readstate=0;

	}

}

/********************************************************************

  Packet Parser for OpenBCI:

  https://github.com/OpenBCI/OpenBCI/wiki/Data-Format-for-OpenBCI-V3

  3-byte signed integers are stored in 'big endian' format, MSB first.
  The 3-byte values are converted to 4-byte signed longs during the unpacking.

  Accelerometer data are 2-byte signed ints.  Also converted to signed longs.

  Start Indicator: 0xA0
  Frame number   : 1 byte  (advances on each packet)
  Channel N data : 3 bytes per channel (repeats for ch 1-8, or ch 1-16)
  ...
  A_channel X    : 2 bytes (note these may also be instead, optional user data,
  A_channel Y    : 2 bytes  replacing accelerometer values.)
  A_channel Z    : 2 bytes
  End Indcator:    0xC0
 **********************************************************************/

void parse_byte_OPENBCI(unsigned char actbyte, int channelsInPacket)
{
	static int bytecounter=0;
	static int channelcounter=0;
	static int tempval=0;
	static unsigned char framenumber=0;

	switch (PACKET.readstate) {

		// To better sync up when lost, look for two byte sequence.  It has happened
		// previously, that when data contains lots of A0's, sync could not reestablish.
		//
		case 0: if (actbyte == 0xC0)			// look for end indicator
					PACKET.readstate++;
				break;

		case 1:	if (actbyte == 0xA0)		    // look for start indicator next
					PACKET.readstate++;
				else
					PACKET.readstate = 0;
				break;

		case 2:	if (actbyte != framenumber) {
					GLOBAL.syncloss++;
					// but go ahead and parse it anyway, 
				}
				framenumber = actbyte + 1;		// next expected frame number
				bytecounter=0;
				channelcounter=0;
				tempval=0;
				PACKET.readstate++;
				break;

		case 3: // get channel values 
				tempval |= (((unsigned int)actbyte) << (16 - (bytecounter*8)));		// big endian
//				tempval |= (((unsigned int)actbyte) << (bytecounter*8));	// little endian
				bytecounter++;
				if (bytecounter==3) {
					if ((tempval & 0x00800000) > 0) {
						tempval |= 0xFF000000;
					} else {
						tempval &= 0x00FFFFFF;
					}
					PACKET.buffer[channelcounter] = tempval;
					channelcounter++;
					if (channelcounter==channelsInPacket) {  // all channels arrived !
						PACKET.readstate++;
						bytecounter=0;
						tempval=0;
					}
					else { bytecounter=0; tempval=0; }
				}
				break;

		case 4: // get accelerometer XYZ
				tempval |= (((unsigned int)actbyte) << (8 - (bytecounter*8)));		// big endian
//				tempval |= (((unsigned int)actbyte) << (bytecounter*8));	// little endian
				bytecounter++;
				if (bytecounter==2) {
					if ((tempval & 0x00008000) > 0) {
						tempval |= 0xFFFF0000;
					} else {
						tempval &= 0x0000FFFF;
					}  
					PACKET.buffer[channelcounter]=tempval;
					channelcounter++;
					if (channelcounter==(channelsInPacket+3)) {  // all channels arrived !
						PACKET.readstate++;
						bytecounter=0;
						channelcounter=0;
						tempval=0;
					}
					else { bytecounter=0; tempval=0; }
				}
				break;

		case 5: if (actbyte == 0xC0)     // if correct end delimiter found:
				{
					process_packets();   // call message pump
					PACKET.readstate = 1;
				}
				else
				{
					GLOBAL.syncloss++;
					PACKET.readstate = 0;	// resync
				}
				break;

		default: PACKET.readstate=0;  // resync
	}
}






/********************************************************************

  OPI Exploraton kit parser

 **********************************************************************/



void parse_byte_OPI(unsigned char actbyte)
{
	static int test=0;
	static int packetlen=0;
	static unsigned char buffer[512];
	static int actpos =0;
	static int checksum, runningsum=0;
	static short tempval=0;
	static unsigned char pdn,misc;

    switch (PACKET.readstate) 
	{
		  case 0: if (actbyte==0x33) PACKET.readstate++; 
				  break;
		  case 1: if (actbyte==0x33)  PACKET.readstate++; 
				  else PACKET.readstate=0;
				  break;
		  case 2: packetlen=actbyte<<8;
				  PACKET.readstate++; 
				  break;
		  case 3: packetlen+=actbyte;
				  if (packetlen == 0x92) { actpos=0; runningsum=0; PACKET.readstate++; }
				  else PACKET.readstate=0;
				  break;
	      case 4: buffer[actpos++]=actbyte;
				  runningsum+=actbyte;
				  if (actpos == packetlen) PACKET.readstate++;
				  break;
		  case 5: checksum=actbyte<<8;
				  PACKET.readstate++;
				  break;
		  case 6: checksum+=actbyte;
				  if (checksum==runningsum)
				  {
						unsigned int d;
					    pdn = (unsigned char) buffer[8];  // paired device number
						misc =(unsigned char) buffer[9];  // "misc data": Bit 7 (MSB) 1: 62 ADC data bytes 0: 64 ADC data bytes; 
												          // Bits 6-4: wireless data code (usually 1) 
														  // Bit 0: battery state (1: V sensor > 3.15V )

						PACKET.buffer[1]=  buffer[10+128]<<8;  //  * 1.13f - 46.8f  // -> deg Celisus 

						d= buffer[10+128+1]<<8;
						if (d < 32768) PACKET.buffer[2]= 32768+d; else PACKET.buffer[2]=d-32768;
						d= buffer[10+128+2]<<8;
						if (d < 32768) PACKET.buffer[3]= 32768+d; else PACKET.buffer[3]=d-32768;
						d= buffer[10+128+4]<<8;
						if (d < 32768) PACKET.buffer[4]= 32768+d; else PACKET.buffer[4]=d-32768;

						for (int i=0; i< 64; i++)
						{

							d= (buffer[10+i*2] << 8) | (buffer[11+i*2]);
							if (d < 32768) PACKET.buffer[0]= 32768+d;
							else PACKET.buffer[0]=d-32768;
							PACKET.buffer[0] &= 0xfffc;
							process_packets();
						}
				  }
				  PACKET.readstate=0;
				  break;
		  default: PACKET.readstate=0;
		}		
}



/********************************************************************

  NEUROSKY 1chn - Neuosky Mindset/Mindwave & compatible protocol

Neurosky packet protocol

The raw packet is sent 512 times every second and the analyzed packet is sent out once every second. 

Raw Packet 
Byte:Example // [CODE] Explanation 
[ 0]:0xAA // [SYNC] 
[ 1]:0xAA // [SYNC] 
[ 2]:0x04 // [PLENGTH] (payload length) of 4 bytes 
[ 3]:0x80 // [RAW_WAVE_VALUE] Single Big-endian 16 bit two's compliment signed value 
[ 4]:0x02 // [VLENGTH] 
[ 5]:0x00 // Raw Value High Byte 
[ 6]:0x05 // Raw Value Low Byte 
[ 7]:0x78 // [CHKSUM] 

Analyzed Packet 
Byte:Example // [CODE] Explanation 
[ 0]:0xAA // [SYNC] 
[ 1]:0xAA // [SYNC] 
[ 2]:0x20 // [PLENGTH] (payload length) of 32 bytes 
[ 3]:0x02 // [POOR_SIGNAL] Quality 
[ 4]:0x00 // No poor signal detected (0/200) 
[ 5]:0x83 // [ASIC_EEG_POWER_INT] 
[ 6]:0x18 // [VLENGTH] 24 bytes 
[ 7]:0x00 // (1/3) Begin Delta bytes 
[ 8]:0x00 // (2/3) 
[ 9]:0x94 // (3/3) End Delta bytes 
[10]:0x00 // (1/3) Begin Theta bytes  
[11]:0x00 // (2/3) 
[12]:0x42 // (3/3) End Theta bytes 
[13]:0x00 // (1/3) Begin Low-alpha bytes 
[14]:0x00 // (2/3) 
[15]:0x0B // (3/3) End Low-alpha bytes 
[16]:0x00 // (1/3) Begin High-alpha bytes 
[17]:0x00 // (2/3) 
[18]:0x64 // (3/3) End High-alpha bytes 
[19]:0x00 // (1/3) Begin Low-beta bytes 
[20]:0x00 // (2/3) 
[21]:0x4D // (3/3) End Low-beta bytes 
[22]:0x00 // (1/3) Begin High-beta bytes 
[23]:0x00 // (2/3) 
[24]:0x3D // (3/3) End High-beta bytes 
[25]:0x00 // (1/3) Begin Low-gamma bytes 
[26]:0x00 // (2/3) 
[27]:0x07 // (3/3) End Low-gamma bytes 
[28]:0x00 // (1/3) Begin Mid-gamma bytes 
[29]:0x00 // (2/3) 
[30]:0x05 // (3/3) End Mid-gamma bytes 
[31]:0x04 // [ATTENTION] 
[32]:0x0D // eSense Attention level of 13 eSense Attention level of 13 
[33]:0x05 // [MEDITATION] 
[34]:0x3D // eSense Meditation level of 61 eSense Meditation level of 61 
[35]:0x34 // [CHKSUM] (1's comp inverse of 8-bit Payload sum of 0x) 
 **********************************************************************/

void parse_byte_Neurosky(unsigned char actbyte)
{
	static int pos=0;
    static short tmpval=0;

	switch (pos) 
	{
		  case 0: if (actbyte==0xAA) pos++;
				  break;
		  case 1: if (actbyte==0xAA) pos++;
				  break;
  		  case 2: if (actbyte==0x04) pos++;        // raw packet
				  else if (actbyte==0x20) pos=10;  // analysed packet
				  break;
  		  case 3: if (actbyte==0x80) pos++;
				  break;
  		  case 4: if (actbyte==0x02) pos++;
				  break;

		  case 5: tmpval=actbyte<<8;
			      pos++;
				  break;

		  case 6: tmpval |= actbyte;
			      PACKET.buffer[0]=32767+tmpval; 
			      pos++;
				  break;

		  case 7: process_packets();
				  pos=0;  // next packet 
				  break;

		  case 10: pos=0; 
			      // TBD: parse analysed packet
			      // process_packets();
				  break;


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
			case DEV_MONOLITHEEG_P21: parse_byte_P21(actbyte); break;
			case DEV_SBT2:      parse_byte_SBT2(actbyte); break;
			case DEV_SBT4:      parse_byte_SBT4(actbyte); break;
			case DEV_RAW8BIT:   parse_byte_raw_8bit(actbyte); break;
			case DEV_PENDANT3:  parse_byte_PendantV3(actbyte); break;
			case DEV_QDS:       parse_byte_QDS(actbyte); break;
			case DEV_NIA:		parse_byte_NIA(actbyte); break;
			case DEV_IBVA:		parse_byte_IBVA(actbyte); break;
			case DEV_OPENBCI8:	parse_byte_OPENBCI(actbyte, 8); break;
			case DEV_OPENBCI16:	parse_byte_OPENBCI(actbyte, 16); break;
			case DEV_OPI_EXPLORATION: parse_byte_OPI(actbyte); break;
			case DEV_NEUROSKY:  parse_byte_Neurosky(actbyte); break;
		}
	}
	return;
}



void update_captfile_guibuttons(HWND hDlg)
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

int sendP21Command(unsigned char cmd, unsigned char data1, unsigned char data2)
{
	if ((!TTY.BIDIRECT) || (TTY.COMDEV==INVALID_HANDLE_VALUE) || (!TTY.CONNECTED)) return(0);
	{
		write_to_comport(cmd);
		write_to_comport(data1);
		write_to_comport(data2);
   	    PACKET.readstate=0;
	}
	return(1);
}

void update_p21state(void)
{
	if (TTY.devicetype!=DEV_MONOLITHEEG_P21) return;
	sendP21Command(CMD_SET_VINFO, VINFO_PROTOCOL_NUMBER, 21);
    // sendP21Command(CMD_SET_VINFO,VINFO_CHANNELS_MATRIX,(unsigned char) (st->chnmatrix&63));

	switch(PACKETSPERSECOND)
	{
		case 256:  sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 0);break;
		case 300:  sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 1);break;
		case 512:  sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 2);break;
		case 600:  sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 3);break;
		case 1024: sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 4);break;
		case 1200: sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 5);break;
		default:   sendP21Command(CMD_SET_VINFO, VINFO_SAMPLE_RATE, 0);break;

	}
	switch(TTY.BAUDRATE)
	{
		case 57600:   sendP21Command(CMD_SET_VINFO, VINFO_BAUD_RATE, 0);break;
		case 115200:  sendP21Command(CMD_SET_VINFO, VINFO_BAUD_RATE, 1);break;
		case 230400:  sendP21Command(CMD_SET_VINFO, VINFO_BAUD_RATE, 2);break;
	}
}


void setEEGDeviceDefaults(EEGOBJ * st)
{
	int i,numChannels=0;

	TTY.BIDIRECT=false;
	for (i=0;i<32;i++)
	{
	  	sprintf(st->out_ports[i].out_name,"Chn%d",i+1);
		sprintf(st->out_ports[i].out_desc,"EEG Channel%d",i+1);
		strcpy(st->out_ports[i].out_dim,"uV");
		st->out_ports[i].get_range=-1;
     	st->out_ports[i].out_min=-500.0f;
		st->out_ports[i].out_max=500.0f;
	}

	switch (TTY.devicetype)
	{	
		case DEV_RAW:
			numChannels=1;
			st->resolution=16;
			break;

		case DEV_RAW8BIT:
			numChannels=1;
			st->resolution=8;
			break;

		case DEV_OPI_EXPLORATION:
			numChannels=5;
			st->resolution=16;

			st->out_ports[0].get_range=-1;
			st->out_ports[0].out_min=-800.0f;
			st->out_ports[0].out_max=800.0f;

		  	sprintf(st->out_ports[1].out_name,"Temp");
			sprintf(st->out_ports[1].out_desc,"Temperature");
			strcpy(st->out_ports[1].out_dim,"degC");
			st->out_ports[1].get_range=-1;
			st->out_ports[1].out_min=0.0f;
			st->out_ports[1].out_max=50.0f;

		  	sprintf(st->out_ports[2].out_name,"AccX");
			sprintf(st->out_ports[2].out_desc,"Acceleration X");
			strcpy(st->out_ports[2].out_dim,"g");
			st->out_ports[2].get_range=-1;
			st->out_ports[2].out_min=-2.0f;
			st->out_ports[2].out_max=2.0f;

		  	sprintf(st->out_ports[3].out_name,"AccY");
			sprintf(st->out_ports[3].out_desc,"Acceleration Y");
			strcpy(st->out_ports[3].out_dim,"g");
			st->out_ports[3].get_range=-1;
			st->out_ports[3].out_min=-2.0f;
			st->out_ports[3].out_max=2.0f;

		  	sprintf(st->out_ports[4].out_name,"AccZ");
			sprintf(st->out_ports[4].out_desc,"Acceleration Z");
			strcpy(st->out_ports[4].out_dim,"g");
			st->out_ports[4].get_range=-1;
			st->out_ports[4].out_min=-2.0f;
			st->out_ports[4].out_max=2.0f;
			break;

		case DEV_PENDANT3:
			numChannels=4;
			st->resolution=13;
	 		sprintf(st->out_ports[2].out_name,"valid");
			sprintf(st->out_ports[2].out_desc,"Signal valid indication");
			strcpy(st->out_ports[2].out_dim,"none");
			st->out_ports[2].get_range=-1;
     		st->out_ports[2].out_min=0.0f;
			st->out_ports[2].out_max=100.0f;
	 		sprintf(st->out_ports[3].out_name,"bt1+2");
			sprintf(st->out_ports[3].out_desc,"Button 1+2 state");
			strcpy(st->out_ports[3].out_dim,"none");
			st->out_ports[3].get_range=-1;
     		st->out_ports[3].out_min=0.0f;
			st->out_ports[3].out_max=3.0f;
			break;

		case DEV_SBT4:
			numChannels=4;
			st->resolution=8;
			break;

		case DEV_QDS:
			numChannels=8;
			st->resolution=16;
			for (numChannels=0;numChannels<8;numChannels++) 
			{
     				st->out_ports[numChannels].out_min=-16485.0f;
					st->out_ports[numChannels].out_max=16485.0f;
			}
			break;

		case DEV_IBVA:
			st->resolution=10;
			numChannels=2;
			break;

		case DEV_NIA:
			st->resolution=24;
			numChannels=2;
			break;
		case DEV_SBT2:
			st->resolution=24;
			numChannels=2;
			break;

		case DEV_OPENBCI8:
			numChannels=8;
			TTY.BAUDRATE=115200;
			goto openbci;

		case DEV_OPENBCI16:
			numChannels=16;
			TTY.BAUDRATE=230400;  // Check with Joel


openbci:	// common section for OpenBCI devices

			// OpenBCI ADS1299 samples are 24 bits, so +- (2^23 - 1)  max signed integers
			//
			// From TI data sheet: (Volts/count) = 4.5 Volts / gain / (2^23 - 1);
			//  default gain for the board is 24x,
			//  that gives 4.5 / 24 / (2^23 - 1) = .0223 uV per count
			//
			// So full scale +- range of microvolts in 24 bits is 2^23 * .02235 = +-187485.388 uV.

#define OBCI_GAIN ((float)24)

		{
			float fullscale = (4.5e6 / OBCI_GAIN);

			for (i=0;i<numChannels;i++)
			{
     			st->out_ports[i].out_min= -(fullscale);
				st->out_ports[i].out_max= (fullscale);
			}

			st->resolution=24;
			update_samplingrate(250);

			i = numChannels;
			numChannels += 3;
			sprintf(st->out_ports[i].out_name,"AccX");
			sprintf(st->out_ports[i].out_desc,"Acceleration X");
			strcpy(st->out_ports[i].out_dim,"g");
			st->out_ports[i].out_min=-2.0f;
			st->out_ports[i].out_max=2.0f;

			i++;
			sprintf(st->out_ports[i].out_name,"AccY");
			sprintf(st->out_ports[i].out_desc,"Acceleration Y");
			strcpy(st->out_ports[i].out_dim,"g");
			st->out_ports[i].out_min=-2.0f;
			st->out_ports[i].out_max=2.0f;

			i++;
			sprintf(st->out_ports[i].out_name,"AccZ");
			sprintf(st->out_ports[i].out_desc,"Acceleration Z");
			strcpy(st->out_ports[i].out_dim,"g");
			st->out_ports[i].out_min=-2.0f;
			st->out_ports[i].out_max=2.0f;
		}

			break;

		case DEV_MONOLITHEEG_P21:
			 TTY.BIDIRECT=true;
			 update_p21state();
		case DEV_MODEEG_P2:
		case DEV_MODEEG_P3:
			st->resolution=10;
			sprintf(st->out_ports[6].out_name,"b1-b4");
			sprintf(st->out_ports[6].out_desc,"EEG Buttons");
			strcpy(st->out_ports[6].out_dim,"none");
			st->out_ports[6].get_range=-1;
			st->out_ports[6].out_min=0.0f;
			st->out_ports[6].out_max=15.0f;
			numChannels=7;
			break;
		case DEV_NEUROSKY:
			st->resolution=16;
			numChannels=1;
			break;
	}
	st->outports=numChannels;
	st->height=CON_START+st->outports*CON_HEIGHT+5;
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

			SetDlgItemText(hDlg,IDC_DEVICETYPE,devicetypes[TTY.devicetype]);

			for (t=0; captfiletypes[t][0]!=0;t++)
				SendDlgItemMessage( hDlg, IDC_FILEMODECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) captfiletypes[t]) ;
			SendDlgItemMessage( hDlg, IDC_FILEMODECOMBO, CB_SETCURSEL, CAPTFILE.filetype, 0L ) ;
	
			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=0; lpsi.nMax=1000;
			SetScrollInfo(GetDlgItem(hDlg,IDC_ARCHIVE_POSBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_ARCHIVE_POSBAR), SB_CTL, 0, 1);

			CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
			SetDlgItemText(hDlg, IDC_ARCHIVE_FILENAME,CAPTFILE.filename);

			sprintf(strfloat,"%.2f",(float)CAPTFILE.offset/(float)PACKETSPERSECOND);
			SetDlgItemText(hDlg,IDC_OFFSET,strfloat);
			SetDlgItemInt(hDlg,IDC_RESOLUTION,st->resolution,0);
			// Resolution field should really not be adjustable any more.
			EnableWindow(GetDlgItem(hDlg,IDC_RESOLUTION),FALSE);

			// only for IBVA:
			SetDlgItemText(hDlg, IDC_CUTOFF,"0.33");
			SetDlgItemText(hDlg, IDC_BATTERY,"0.00 V");
			//

			update_captfile_guibuttons(hDlg);
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
						update_captfile_guibuttons(hDlg);
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
						
						update_captfile_guibuttons(hDlg);
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
							} else {
								if (TTY.COMDEV!=INVALID_HANDLE_VALUE) 
									BreakDownCommPort();								
								TTY.CONNECTED=SetupCommPort(TTY.PORT);
								if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (TTY.devicetype==DEV_IBVA))
								{
									write_string_to_comport("SR 256\r");
								}
							}
							CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
						}
						update_captfile_guibuttons(hDlg);
					break;

				case IDC_BIDIRECT:   // currently not supported in GUI
						TTY.BIDIRECT=IsDlgButtonChecked(hDlg,IDC_BIDIRECT);
					break;
				case IDC_FLOW_CONTROL:  // currently not supported in GUI
					TTY.FLOW_CONTROL=IsDlgButtonChecked(hDlg,IDC_FLOW_CONTROL);
  				    if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
					{
					    BreakDownCommPort(); 
						TTY.CONNECTED=SetupCommPort(TTY.PORT);
						CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
					}
					break;

				// only for IBVA:
				case IDC_TESTBAT:
						if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (TTY.devicetype==DEV_IBVA))
						{
							write_to_comport(0x42);
							write_to_comport(0x4c);
							write_to_comport(0x0d);
						}
					break;
				case IDC_SETCUTOFF:
						if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (TTY.devicetype==DEV_IBVA))
						{
							char str[20];
							float fl;
							GetDlgItemText(hDlg,IDC_CUTOFF,(LPSTR) str,20);
							sscanf(str,"%f",&fl);
							sprintf(str,"FR %.2f\r",fl);
							write_string_to_comport(str);
						}
						break;
				// 

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
							if (strcmp(devicetypes[TTY.devicetype],CAPTFILE.devicetype))
								report("Warning: Archive File does not match device type");
							SetDlgItemText(hDlg,IDC_FILEMODECOMBO,captfiletypes[CAPTFILE.filetype]);
//							reset_oscilloscopes();
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RESETBUTTON,0);
							update_captfile_guibuttons(hDlg);
						    InvalidateRect(ghWndDesign,NULL,TRUE);
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
					 update_captfile_guibuttons(hDlg);
					break;
				case IDC_PLAY_ARCHIVE:
					if(CAPTFILE.filehandle==INVALID_HANDLE_VALUE) break;
					QueryPerformanceCounter((_LARGE_INTEGER *)&TIMING.readtimestamp);
					CAPTFILE.do_read=1;
					update_captfile_guibuttons(hDlg);
					start_timer();
					break;
				case IDC_REC_ARCHIVE:
					close_captfile();
					strcpy(CAPTFILE.filename,GLOBAL.resourcepath);
					strcat(CAPTFILE.filename,"ARCHIVES\\*.arc");
					if (open_file_dlg(ghWndMain,CAPTFILE.filename, FT_ARCHIVE, OPEN_SAVE))
					{
						CAPTFILE.filehandle=create_captfile(CAPTFILE.filename);
						if(CAPTFILE.filehandle!=INVALID_HANDLE_VALUE) 
						{    SetDlgItemText(hDlg,IDC_ARCHIVE_FILENAME,CAPTFILE.filename);
							 CAPTFILE.do_write=1;
							 update_captfile_guibuttons(hDlg);
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
					update_captfile_guibuttons(hDlg);
					break;
				case IDC_APPLYOFFSET:
						GetDlgItemText(hDlg, IDC_OFFSET, strfloat, 20);
						CAPTFILE.offset = (int)((float)atof(strfloat) * (float) PACKETSPERSECOND);
						get_session_length();
						st->session_pos(TIMING.packetcounter);
					break;
				case IDC_RESOLUTION:
					{   int temp = GetDlgItemInt(hDlg, IDC_RESOLUTION, NULL,0);
						if ((temp>4)&&(temp<30))
							st->resolution=temp;
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
		width=60;
		update_devicetype();
		setEEGDeviceDefaults(this);
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
	  		  if (TTY.devicetype==DEV_MONOLITHEEG_P21) sendP21Command(CMD_SET_VINFO, VINFO_RUNEEG ,1);
			  if (TTY.devicetype==DEV_SBT2) { write_to_comport(0x20); write_to_comport(0x00); }
			  if (TTY.devicetype==DEV_OPENBCI8
				  || TTY.devicetype==DEV_OPENBCI16) { write_to_comport('b'); }
			  if (TTY.devicetype==DEV_OPI_EXPLORATION) {  start_opi_pollthread();  }
		  }
		  if (ghWndToolbox==hDlg)  update_captfile_guibuttons(hDlg);

		
	  }
  	  void EEGOBJ::session_stop(void) 
	  {	
		    if (TTY.devicetype==DEV_MONOLITHEEG_P21) sendP21Command(CMD_SET_VINFO, VINFO_RUNEEG ,0);
			if (TTY.devicetype==DEV_SBT2) { write_to_comport(0x40); write_to_comport(0x00); }
			if (TTY.devicetype==DEV_OPENBCI8
				  || TTY.devicetype==DEV_OPENBCI16) { write_to_comport('s'); }
			if (TTY.devicetype==DEV_OPI_EXPLORATION) {  stop_opi_pollthread();  }

			CAPTFILE.do_read=0;TTY.read_pause=1; //CAPTFILE.do_write=0;
			if (hDlg==ghWndToolbox) update_captfile_guibuttons(hDlg);
	  }
  	  void EEGOBJ::session_reset(void) 
	  {	
		    if (TTY.devicetype==DEV_MONOLITHEEG_P21) sendP21Command(CMD_SET_VINFO, VINFO_RUNEEG ,0);
			CAPTFILE.do_read=0;CAPTFILE.do_write=0;TTY.read_pause=1;
			if (hDlg==ghWndToolbox) update_captfile_guibuttons(hDlg);
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
		  switch(TTY.devicetype) {
			  case DEV_IBVA:
		  	  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EEGBOX_IBVA, ghWndStatusbox, (DLGPROC)EEGDlgHandler));
				break;
			  case DEV_NIA:
		  	  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EEGBOX_NIA, ghWndStatusbox, (DLGPROC)EEGDlgHandler));
				break;
			  case DEV_SBT2:
		  	  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EEGBOX_SBT2, ghWndStatusbox, (DLGPROC)EEGDlgHandler));
				break;
			  default:
		  	  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EEGBOX_GENERIC, ghWndStatusbox, (DLGPROC)EEGDlgHandler));
		  }
	  }

  	  void EEGOBJ::load(HANDLE hFile) 
	  {	
		  float dummy;   // compatibility to older configurations
		  resolution=0;
			load_object_basics(this);
			load_property("resolution", P_FLOAT, &dummy);
			while (dummy>1) {resolution++; dummy/=2;}
			if (resolution<8) resolution=10;
			write_logfile("EEG LOAD: TTY.devicetype = %d, outports=%d",TTY.devicetype,outports);

			int desired_outports=7;

			switch (TTY.devicetype) {
				case DEV_RAW: 
				case DEV_RAW8BIT: 
					// Was set to '4' by mistake?  Creating a DEV_RAW* makes only 1 channel(!)
					// '4' breaks all the supplied con files which reference recorded ARCHIVE files,
					// and shows 3 extra 'phantom' outports.  So set back to '1' for now.
					//desired_outports=4;
					desired_outports=1;
				break;
				case DEV_OPI_EXPLORATION: 
					desired_outports=5;
				break;
				case DEV_NIA: 
					desired_outports=2;
					break;
				case DEV_SBT2: 
					desired_outports=2;
				break;
				case DEV_PENDANT3: 
				case DEV_SBT4: 
					desired_outports=4;
				break;
				case DEV_QDS: 
				case DEV_OPENBCI8:
					desired_outports=8+3;
				break;
				case DEV_OPENBCI16:
					desired_outports=16+3;
				break;
				case DEV_NEUROSKY:
					desired_outports=1;
				break;
				default:
					desired_outports=7;
				break;
			}
			if (outports!=desired_outports) 
			{
					//report ("Compatibility Issue with loaded design found: port selection for EEG element was deprecated - please check EEG element output port connections! ");
					outports=desired_outports;
			}
	  }

	  void EEGOBJ::save(HANDLE hFile) 
	  {	  
		  long l = (1<<resolution);  // compatibility to older configurations
		  float dummy= (float)l;
			save_object_basics(hFile, this);
			save_property(hFile, "resolution", P_FLOAT, &dummy);
	  }

	  void EEGOBJ::work(void) 
	  {
		int i,x;

        switch (TTY.devicetype)
		{
			case DEV_PENDANT3:
			    for (x=0;x<2;x++)
				   pass_values(x,(float) PACKET.buffer[x] * (out_ports[x].out_max-out_ports[x].out_min) / (float)(1<<resolution) + out_ports[x].out_min);
		    	pass_values(2,(float) (PACKET.buffer[2]));  // valid indicator
		    	pass_values(3,(float) (PACKET.switches));  
				break;

			case DEV_MODEEG_P2:
			case DEV_MODEEG_P3:
			case DEV_MONOLITHEEG_P21: 
				  for (x=0;x<6;x++)
						pass_values(x,(float) PACKET.buffer[x] * (out_ports[x].out_max-out_ports[x].out_min) / (float)(1<<resolution) + out_ports[x].out_min);
				  pass_values(x,(float)PACKET.switches);
				  break;

			case DEV_OPENBCI8:
				i = 8;
				goto openbci;

			case DEV_OPENBCI16:
				i = 16;
openbci:	{
				float uvpercount, maxcount, pbufferuv;
				int pbuffer;
				maxcount = (float)((1<<23) - 1);
				for (x=0;x<i;x++)
				{
					uvpercount = (float)(out_ports[x].out_max) / maxcount;
					// PACKET.buffer is unsigned int, but our values are signed.
					pbuffer = (int)PACKET.buffer[x];
					pbufferuv = (float)pbuffer * uvpercount;
					pass_values(x, pbufferuv);
					// if (x == 1)	write_logfile("uvpc %f, pbuf %d, pbufuv %f\n", uvpercount, pbuffer, pbufferuv);
				}
				pass_values(x,(float) ((int)PACKET.buffer[x]));  x++;
				pass_values(x,(float) ((int)PACKET.buffer[x]));  x++;
				pass_values(x,(float) ((int)PACKET.buffer[x]));  x++;
				break;
			}

			default:  // all other devices
			  for (x=0;x<outports;x++)
					pass_values(x, ((float) PACKET.buffer[x])  * (out_ports[x].out_max-out_ports[x].out_min) / (float)(1<<resolution) + out_ports[x].out_min);
			  break;
		}


		if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox))
		{
			if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
			{
				char str[15];
				if (TTY.devicetype==DEV_IBVA)
				{
					sprintf(str,"%.2f V",(float)PACKET.buffer[5]*16/1024);
					SetDlgItemText(hDlg, IDC_BATTERY, str);
				}
				if (TTY.devicetype==DEV_SBT2)
				{
					sprintf(str,"%d",PACKET.buffer[6]);
					SetDlgItemText(hDlg, IDC_SBT2STATUS, str);
				}
			}

			if((!scrolling) && (CAPTFILE.do_read)) 
			{
				DWORD x= SetFilePointer(CAPTFILE.filehandle,0,NULL,FILE_CURRENT);
				x=x*1000/CAPTFILE.length/TTY.bytes_per_packet;
				SetScrollPos(GetDlgItem(hDlg, IDC_ARCHIVE_POSBAR), SB_CTL, x, 1);
			}
		}
	  }

EEGOBJ::~EEGOBJ()
	  {
		if (TTY.devicetype==DEV_NIA) DisconnectNIA();
	    close_captfile();
	  }  



