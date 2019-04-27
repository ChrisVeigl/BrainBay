 /* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: NIA.CPP:  contains functions for the NIA-Interface
  Author: Franz Strobl 2010, contact: fraxas@hotmail.de

  The NIA Object provides Interfacing to the NIA OCZ-Hardware with 
  EEG-Transducer. The NIA device is connected to any USB-Port. All 
  USB-Ports are scanned for a device from Vendor 0x1234 (OCZ NIA). If one 
  or two devices are found they will be registered and can deliver 
  Data to channel 1 and 2 of the eeg-object. 
  - NIA delivers Data with 4kHz (every 250usec. one value) each 3 or 4 samples 
  are transferred within one packet. This program evluates only the first 
  sample of one packet, because this seems to be enough for accurate signaling.
  - Changes were done for normalizing the input data (24-Bit original are 
  converted to 16-Bit which are evaluated)
  - Further the Sampling Rate is set to 1000 samples/second according Hardware.
  Archiving was also adapted for NIA Data Recording.
  - com-Port and Baud Rate are not considered using NIA.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "windows.h"
#include "brainBay.h"
#include "ob_eeg.h"

#define NIABYTECOUNT 3			// Count of bytes to be evaluated: 24-Bit (3Byte) per 1 Sample!

RAWINPUTDEVICE Rid[2];
RAWINPUTDEVICELIST RawInputDeviceList[20];
HANDLE	NIAREADERTHREAD;

HANDLE m_hNIA[2];
int	m_nCountNIA=0;

BOOL       fNIAThreadDone = FALSE;

int oldIndex_NIA1=0;
int oldIndex_NIA2=0;
int oldTiming=0;

// 
// DisconnectNIA sollte ‰hnlich BreakDownCommPort die NIA Schnittstelle abkoppeln und schlieﬂen
//

BOOL DisconnectNIA(void) {
	UINT ui;
	UINT ret;
	UINT i;
	write_logfile("Disconnecting NIA");
	ret=GetRegisteredRawInputDevices(NULL,&ui,sizeof(RAWINPUTDEVICE));			// get size
	if (ret<=sizeof(Rid))														// if okay, get structures
		GetRegisteredRawInputDevices(Rid, &ui, sizeof(RAWINPUTDEVICE));

	if(ret!=(UINT)-1){
		for (i=0; i<ui; i++) {
			Rid[i].dwFlags = RIDEV_REMOVE;		// this should disable the HID device
			Rid[i].hwndTarget = NULL ;
//			RawInputDeviceList[i].hDevice = NULL;

			if (RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
				report_error("Could not close NIA");
		}
	}
	ret=GetRegisteredRawInputDevices(Rid, &ui, sizeof(RAWINPUTDEVICE));
	if (ret !=0)
		write_logfile("Could not disconnect NIA");

	return TRUE;
}

//
// ConnectNIA, without MFC 
// NIA Device is attached as USB HID and read via GetRawInputData 
//

BOOL ConnectNIA(HWND hDlg) {
	UINT ui;
	UINT ret;
	ret=GetRawInputDeviceList(NULL,&ui,0);
	UINT i;
	int sav_pause;

	write_logfile("Connecting NIA");

	sav_pause=TTY.read_pause;
    TTY.read_pause=1;

	ret=GetRegisteredRawInputDevices(NULL,&ui,sizeof(RAWINPUTDEVICE));			// get size
	if (ret<=sizeof(Rid))														// if okay, get structures
		GetRegisteredRawInputDevices(Rid, &ui, sizeof(RAWINPUTDEVICE));
	if (ui>0) {
		write_logfile("NIA already connected!");
		TTY.read_pause=sav_pause;
		return (TRUE);
	}

	m_hNIA[0]=NULL;
	m_hNIA[1]=NULL;

	ui=20;
	ret=GetRawInputDeviceList(&RawInputDeviceList[0],&ui,sizeof(RAWINPUTDEVICELIST));			
	if(ret!=(UINT)-1)
	{
		RID_DEVICE_INFO dev_info ;
		TCHAR strName[300];

		m_nCountNIA = 0;

		for(i=0;i<ret;i++)
		{
			if(i>=20) break;

			dev_info.cbSize=sizeof(RID_DEVICE_INFO);
			ui=sizeof(RID_DEVICE_INFO);
			GetRawInputDeviceInfo(RawInputDeviceList[i].hDevice,RIDI_DEVICEINFO,&dev_info,&ui);
			ui=300;
			GetRawInputDeviceInfo(RawInputDeviceList[i].hDevice,RIDI_DEVICENAME,strName,&ui);

			if(dev_info.hid.dwVendorId==0x1234 /*&& dev_info.hid.dwProductId==0*/)
			{
				if(m_nCountNIA>=2) break;		// nur an dieser Stelle vorbereitet!
				m_hNIA[m_nCountNIA]=RawInputDeviceList[i].hDevice;
				Rid[m_nCountNIA].usUsagePage = 0xFF00; 
				Rid[m_nCountNIA].usUsage = 0xFF01; 
				Rid[m_nCountNIA].dwFlags = RIDEV_INPUTSINK;	// this enables the caller to receive the input even when the caller is not in the foreground. Note that hwndTarget must be specified.
				Rid[m_nCountNIA].hwndTarget = ghWndMain ;		//  directs Data to main Thread !!

				m_nCountNIA++;					// ToDo: f¸r 2 NIA Devices 2! Channels in Amplifier??? (not tested yet!)
			}
		}
	} else {
		TTY.read_pause=sav_pause;
		return (FALSE) ;
	}
	if(m_nCountNIA>0)
	{
		if (RegisterRawInputDevices(Rid, m_nCountNIA, sizeof(RAWINPUTDEVICE)) == FALSE) {
				write_logfile("Could not register NIA");
				report_error("Could not register NIA");
				TTY.read_pause=sav_pause;
				return FALSE; 
		} else {
			if (hDlg!=NULL) {
  			  SetDlgItemText( hDlg, IDC_PORTCOMBO, "none") ;
			  TTY.read_pause=sav_pause;
			  return TRUE;
			}
		}
	}

	TTY.read_pause=sav_pause;
	return FALSE;  
}




// 
// ReadNIA wurde abgeleitet aus NIADlg.cpp, aber ohne MFC implementiert
// Das NIA Device wird mit GetRawInputData oder GetRawInputBuffer ausgelesen
//

int ReadNIA( UINT wParam, LONG lParam )
{
	int nNIA;
	int i;
	UINT dwSize=0;
	RAWINPUT* raw;
	static int ramp=0;														//first get size

	GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, NULL, &dwSize, 
					sizeof(RAWINPUTHEADER));



	LPBYTE lpb = new BYTE[dwSize+sizeof(RAWINPUTHEADER)];								
	if (lpb == NULL)											
	{
		write_logfile("Create NIA Buffer failed!");
		report_error("Create NIA Buffer failed!");
		return 0;
	} 
																//then get data

	if (GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, lpb, &dwSize, 
		 sizeof(RAWINPUTHEADER)) != dwSize )
		 write_logfile ("GetRawInputData doesn't return correct size !\n"); 

	raw = (RAWINPUT*)lpb;						


	if(m_hNIA[0]!=NULL && raw->header.hDevice==m_hNIA[0]) nNIA=1;
	else if(m_hNIA[1]!=NULL && raw->header.hDevice==m_hNIA[1]) nNIA=2;
	else nNIA=0;

	if(nNIA>0 && raw->header.dwType == RIM_TYPEHID && raw->data.hid.dwSizeHid==56) 
	{
		UINT ui;			
		RID_DEVICE_INFO dev_info;
		dev_info.cbSize=sizeof(RID_DEVICE_INFO);
		ui=sizeof(RID_DEVICE_INFO);
		GetRawInputDeviceInfo(raw->header.hDevice,RIDI_DEVICEINFO,&dev_info,&ui);

		#define MAX_SAMPLE_COUNT 16

		int nFixed;		// Bytes 49+50
		int nTiming;	// Timing-Info
		int nIndex;		// sequential Index(corresponding to 4kHz Sampling-Rate)
		int nSamples;	// number of Samples
		int nByteTrans; // number of Bytes to be transferred (2 Channels at one time!)

		nFixed=raw->data.hid.bRawData[49]+raw->data.hid.bRawData[50]*0x100;
		nTiming=raw->data.hid.bRawData[51]+raw->data.hid.bRawData[52]*0x100;
		nIndex=raw->data.hid.bRawData[53]+raw->data.hid.bRawData[54]*0x100;
		nSamples=raw->data.hid.bRawData[55];
		

/* copy channel data to TTY.readBuf (for archive functions) */

		nByteTrans = NIABYTECOUNT*2 ;						// take 1 Sample (3Bytes) /Channel 

		if ((!TTY.read_pause) && nNIA ==1) {

			if (oldIndex_NIA1 >0){							// check for lost samples 
	//			if (nIndex > (oldIndex+4)){
				if (nIndex != (oldIndex_NIA1+nSamples)%0xffff){
					 GLOBAL.syncloss++;
				}
			}
			oldIndex_NIA1 = nIndex;

			for (i=0; i<NIABYTECOUNT; i++)	
				TTY.readBuf[i] = (unsigned char) raw->data.hid.bRawData[1+i] ; 

			ramp+=100;
			ramp&=0x0fff;

			TTY.readBuf[i+3] = (unsigned char) (ramp&0xff); 
			TTY.readBuf[i+4] = (unsigned char) ((ramp>>8)&0xff); 
			TTY.readBuf[i+5] = (unsigned char) ((ramp>>16)&0xff); 

			ParseLocalInput(nByteTrans);					// evaluate always all channels(=2 or 6Bytes) with 1st sample; 
//			ParseLocalInput(NIABYTECOUNT);				

		} else if ((!TTY.read_pause) && nNIA ==2) {

			if (oldIndex_NIA2 >0){							// check for lost samples 
	//			if (nIndex > (oldIndex+4)){
				if (nIndex != (oldIndex_NIA2+nSamples)%0xffff){
					 GLOBAL.syncloss++;
				}
			}
			oldIndex_NIA2 = nIndex;

			for (i=0; i<NIABYTECOUNT; i++)	
				TTY.readBuf[i+3] = (unsigned char) raw->data.hid.bRawData[1+i] ; 
		}
	} else {
		write_logfile("NIA connection issue!");
		report_error("NIA connection issue!");
	}

	if(lpb!=NULL) delete[] lpb; 
	return 0L;
}
