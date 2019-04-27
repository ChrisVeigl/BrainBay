/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_VOLUME.CPP
  Author:  Chris Veigl


  This object sets the master volume to 0-100 according to the connected signal 

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_volume.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>


static IAudioEndpointVolume *g_pEndptVol = NULL;

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }

#define ERROR_CANCEL(hr)  \
              if (FAILED(hr)) {  \
                  MessageBox(hDlg, TEXT("The program will exit."),  \
                             TEXT("Fatal error"), MB_OK);  \
                  EndDialog(hDlg, TRUE); return TRUE; }

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

HRESULT hr = S_OK;

int init_audioInterface() {
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    //CAudioEndpointVolumeCallback EPVolEvents;

    CoInitialize(NULL);

   // hr = CoCreateGuid(&g_guidMyContext);
    EXIT_ON_ERROR(hr)

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    // Get default audio-rendering device.
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),
                           CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
    EXIT_ON_ERROR(hr)

   // hr = g_pEndptVol->RegisterControlChangeNotify(
   //                  (IAudioEndpointVolumeCallback*)&EPVolEvents);
    EXIT_ON_ERROR(hr)

    InitCommonControls();
	return(1);

Exit:
    if (FAILED(hr))
    {
        MessageBox(NULL, TEXT("Windows Vista or later required for audio voume control."),
                   TEXT("Error termination"), MB_OK);
    }
    if (pEnumerator != NULL)
    {
      //  g_pEndptVol->UnregisterControlChangeNotify(
        //            (IAudioEndpointVolumeCallback*)&EPVolEvents);
    }

    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(g_pEndptVol)
    CoUninitialize();
    return 0;
}


VOLUMEOBJ::VOLUMEOBJ(int num) : BASE_CL()
{
	outports = 0;
	inports = 1;
	strcpy(in_ports[0].in_name,"set");
	actvolume=INVALID_VALUE;
	if (init_audioInterface())
	{
		float fVolume;
	    int nVolume;
		hr = g_pEndptVol->GetMasterVolumeLevelScalar(&fVolume);
	    //ERROR_CANCEL(hr)
		nVolume = (int)(100*fVolume + 0.5);
		printf("Volume = %d\n",nVolume);
	}
}
	

//void VOLUMEOBJ::make_dialog(void)
//{
//	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ROUNDBOX, ghWndStatusbox, (DLGPROC)VolumeDlgHandler));
//}

void VOLUMEOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
}

void VOLUMEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
}

	
void VOLUMEOBJ::incoming_data(int port, float value)
{
	if (value != INVALID_VALUE) {
		actvolume=size_value(in_ports[0].in_min,in_ports[0].in_max,value,0.0f,100.0f,1);
	}
}
	
void VOLUMEOBJ::work(void)
{
	if (!TIMING.draw_update) 
	{
		if (actvolume!=INVALID_VALUE) {
			hr = g_pEndptVol->SetMasterVolumeLevelScalar(actvolume/100, NULL); 
		}
	}
}

VOLUMEOBJ::~VOLUMEOBJ() {
	SAFE_RELEASE(g_pEndptVol)
}

