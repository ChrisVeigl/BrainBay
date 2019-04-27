/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: TTY.cpp:  contains functions for Com-opening, reading, and the reader thread

        SetupCommPort     - Opens the port for the first time
        WaitForThreads    - Sets the thread exit event and wait for worker
                            threads to exit
        BreakDownCommPort - Closes a connection to the comm port

  Contributors: AllenD

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include <commctrl.h>


#define NUM_EVENTHANDLES    2


//    Prototypes for functions called only within this file

DWORD WaitForThreads( DWORD );
DWORD WINAPI ReaderProc(LPVOID);

int c=0;

BOOL       fThreadDone = FALSE;
BOOL       fOpiThreadDone = FALSE;

/*-----------------------------------------------------------------------------

FUNCTION: SetupCommPort( int Port )

PURPOSE: Setup Communication Port with our settings

-----------------------------------------------------------------------------*/
BOOL SetupCommPort(int port)
{
    DWORD dwReadStatId;
    DWORD dwWriteStatId;
	int sav_port,sav_pause;
    DCB dcb = {0};
	char PORTNAME[10];

	if (!port) return(false);

	sav_port=TTY.PORT;
	sav_pause=TTY.read_pause;
    TTY.read_pause=1;
    
	BreakDownCommPort();

	TTY.PORT = port ;

	// set tty structure for the specified port
	
	sprintf(PORTNAME,"\\\\.\\COM%d",port);
	PACKET.readstate=0;
	PACKET.number=0;
	PACKET.old_number=0;
	PACKET.info=0;

    // open communication port handle
//    TTY.COMDEV = CreateFile( PORTNAME,GENERIC_READ | GENERIC_WRITE, 
//                  0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,0);

    TTY.COMDEV = CreateFile( PORTNAME,GENERIC_READ | GENERIC_WRITE, 
                  0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

    if (TTY.COMDEV == INVALID_HANDLE_VALUE) goto failed;
    
    if (!GetCommState(TTY.COMDEV, &dcb))      // get current DCB settings
	{ report_error("GetCommState");goto failed; }

   
    // update DCB rate, byte size, parity, and stop bits size
   
	dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = TTY.BAUDRATE;
    dcb.ByteSize = 8;
    
	dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
	dcb.EvtChar = '\0';

   
    // update flow control settings
   
    dcb.fDtrControl     =  DTR_CONTROL_ENABLE;
    dcb.fRtsControl     =  RTS_CONTROL_ENABLE;

	if (TTY.FLOW_CONTROL)
	{
      dcb.fOutxCtsFlow    = TRUE;
      dcb.fOutxDsrFlow    = TRUE;
	}
	else
	{
      dcb.fOutxCtsFlow    = FALSE;
      dcb.fOutxDsrFlow    = FALSE;
	}

    dcb.fDsrSensitivity = FALSE;;
    dcb.fOutX           = FALSE;
    dcb.fInX            = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.XonChar         = 0;
    dcb.XoffChar        = 0;
    dcb.XonLim          = 0;
    dcb.XoffLim         = 0;
    dcb.fParity = FALSE; //TRUE;

    if (!SetCommState(TTY.COMDEV, &dcb))     // set new state
	{ report_error("SetCommState failed"); goto failed;}
	
	// set comm buffer sizes
    if (!SetupComm(TTY.COMDEV, 1024, 1024))
	{  report_error("SetupComm failed");goto failed;}

    if (!EscapeCommFunction(TTY.COMDEV, SETDTR))        // raise DTR
	{  report_error("EscapeCommFunction failed");goto failed;}

	  SetCommMask (TTY.COMDEV, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

      
    // start the reader and writer threads

	fThreadDone = FALSE;

    TTY.READERTHREAD =  
		CreateThread( NULL, 1000, (LPTHREAD_START_ROUTINE) ReaderProc, 0, 0, &dwReadStatId);
    if (TTY.READERTHREAD == NULL)
	{ report_error("CreateThread failed"); goto failed;}

	TTY.WRITERTHREAD =
		CreateThread( NULL, 1000, (LPTHREAD_START_ROUTINE) WriterProc, 0, 0, &dwWriteStatId);
    if (TTY.WRITERTHREAD == NULL)
	{ report_error("CreateWriterThread failed"); goto failed;}

	write_logfile("COMPORT opened: %s", PORTNAME);
	TTY.amount_to_write=0;	
	TTY.read_pause=sav_pause;

    return TRUE;

failed:
		{  
			char sztemp[100];
 	        write_logfile("COMPORT open failed");
		
			if (!GLOBAL.loading)
			{
				sprintf(sztemp, "The Port COM%d is not available. Please select another Com-Port.",TTY.PORT);
				report_error(sztemp);
			}
		
			TTY.read_pause=sav_pause;
			TTY.PORT=sav_port;
			TTY.COMDEV=INVALID_HANDLE_VALUE;
			TTY.CONNECTED=FALSE;
			TTY.amount_to_write=0;
			return FALSE;
		}
}


/*-----------------------------------------------------------------------------

FUNCTION: ReaderProc(LPVOID)

PURPOSE: Thread function controls comm port reading
-----------------------------------------------------------------------------*/

DWORD WINAPI ReaderProc(LPVOID lpv)
{
    DWORD 	   dwRead;          // bytes actually read
    DWORD      dwBytesTransferred;           // result from WaitForSingleObject
    struct _COMSTAT status;
    unsigned long   etat;
	//

    while (!fThreadDone) 
	{

		// Wait for an event to occur for the port.
  	    //	WaitCommEvent (TTY.COMDEV, &dwCommModemStatus, 0);

		// Re-specify the set of events to be monitored for the port.
	    //	SetCommMask (TTY.COMDEV, EV_RXCHAR | EV_CTS | EV_DSR | EV_RING);
        if ((TTY.CONNECTED) && (!GLOBAL.loading))
		{
          dwBytesTransferred = 0;
          if (ClearCommError(TTY.COMDEV, &etat, &status))
            dwBytesTransferred = status.cbInQue;
   
		  if (dwBytesTransferred) 
	  	  {
 				dwRead=0;
				// Read the data from the serial port.
				ReadFile (TTY.COMDEV, TTY.readBuf, dwBytesTransferred, &dwRead, 0);
				// Display the data read.
				if ((dwRead)&&(!TTY.read_pause)) ParseLocalInput((int)dwRead);
		  }  else Sleep (1);
		}
	}
	write_logfile("COMPORT closed");    
    return 1;
}


/*-----------------------------------------------------------------------------

FUNCTION: WriterProc(LPVOID)

PURPOSE: Thread function controls comm port reading
-----------------------------------------------------------------------------*/

DWORD WINAPI WriterProc(LPVOID lpv)
{
    DWORD dwWritten;

    while (!fThreadDone) 
	{
        if ((TTY.CONNECTED)&&(TTY.amount_to_write>0))
		{
			if ( WaitForSingleObject( TTY.writeMutex, INFINITE ) == WAIT_OBJECT_0 )
			{
			    WriteFile(TTY.COMDEV, TTY.writeBuf, TTY.amount_to_write, &dwWritten, 0); 
				TTY.amount_to_write=0;
				ReleaseMutex( TTY.writeMutex );
			}
		} else Sleep (10);
	}
	write_logfile("COMPORT closed");    
    return 1;
}


DWORD WINAPI OpiProc(LPVOID lpv)
{
    DWORD dwWritten;

	unsigned char buf[8];
	buf[0]=0x33; buf[1]=0x33; buf[2]=0x00;
	buf[3]=0x02; buf[4]=0x10; buf[5]=0x00;
	buf[6]=0x00; buf[7]=0x10;

    while (!fThreadDone) 
	{
		if (TTY.CONNECTED)
		{
			write_to_comport (buf,8); 
			Sleep (50);  // send a request packet every 50 milliseconds
		}
	}
	return(0);
}


int start_opi_pollthread (void)
{
	DWORD dwOpiThreadStatId;

	unsigned char buf[8] = {0x33,0x33,0x00,0x02,0x20,0x23,0x00,0x43};   // request start module 
	write_to_comport (buf,8); 

	fOpiThreadDone=FALSE;
	TTY.OPITHREAD =
		CreateThread( NULL, 1000, (LPTHREAD_START_ROUTINE) OpiProc, 0, 0, &dwOpiThreadStatId);

	return(0);
}

int stop_opi_pollthread (void)
{
	unsigned char buf[8] = {0x33,0x33,0x00,0x02,0x20,0x22,0x00,0x42};   // request stop module 

	fOpiThreadDone=TRUE;
	Sleep(150);
	write_to_comport (buf,8); 
	return(0);
}


int write_to_comport ( unsigned char byte)
{
	if ( WaitForSingleObject( TTY.writeMutex, INFINITE ) == WAIT_OBJECT_0 )
	{
		TTY.writeBuf[TTY.amount_to_write]=byte;TTY.amount_to_write++;
	    ReleaseMutex( TTY.writeMutex );
    }
	return(1);
}

int write_to_comport ( unsigned char * buf, int len)
{
	if ( WaitForSingleObject( TTY.writeMutex, INFINITE ) == WAIT_OBJECT_0 )
	{
		for (int i=0; i<len;i++)
		{
			TTY.writeBuf[TTY.amount_to_write]=buf[i];
			TTY.amount_to_write++;
		}
	    ReleaseMutex( TTY.writeMutex );
    }
	return(1);
}


void write_string_to_comport ( char * s)
{
	for (unsigned int i=0; i< strlen(s); i++) 
		write_to_comport(s[i]);
}



/*-----------------------------------------------------------------------------

FUNCTION: WaitForThreads(DWORD)

PURPOSE: Waits a specified time for the worker threads to exit

PARAMETERS:
    dwTimeout - milliseconds to wait until timeout

RETURN:
    WAIT_OBJECT_0 - successful wait, threads are not running
    WAIT_TIMEOUT  - at least one thread is still running
    WAIT_FAILED   - failure in WaitForMultipleObjects

----------------------------------------------------------------------------*/
DWORD WaitForThreads(DWORD dwTimeout)
{	
    HANDLE hThreads[2];
    DWORD  dwRes;

    hThreads[0] = TTY.READERTHREAD;
//	hThreads[1] = TTY.WRITERTHREAD;
	
    
    //
    // set thread exit event here
    //
    SetEvent(TTY.ThreadExitEvent);
    write_logfile("COMPORT waitforthreads");

    dwRes = WaitForMultipleObjects(1, hThreads, TRUE, dwTimeout);
    switch(dwRes)
    {

        case WAIT_OBJECT_0:
        case WAIT_OBJECT_0 + 1: 
            dwRes = WAIT_OBJECT_0;
            break;

        case WAIT_TIMEOUT:
            
            if (WaitForSingleObject(TTY.READERTHREAD, 0) == WAIT_TIMEOUT)
                report_error("Reader Thread didn't exit");

            // if (WaitForSingleObject(TTY.WRITERTHREAD, 0) == WAIT_TIMEOUT)
            //    report_error("Writer Thread didn't exit");

	
	    default:
            report_error("WaitForTheads: unknown exit error");
            break;
    }

    //
    // reset thread exit event here
    //
    ResetEvent(TTY.ThreadExitEvent);

    return dwRes;
}

/*-----------------------------------------------------------------------------

FUNCTION: BreakDownCommPort

PURPOSE: Closes a connection to a comm port

COMMENTS: Waits for threads to exit,
          clears DTR, restores comm port timeouts, purges any i/o
          and closes all pertinent handles

-----------------------------------------------------------------------------*/
BOOL BreakDownCommPort()
{	
	int sav_pause = TTY.read_pause;

	Sleep(200);
	TTY.CONNECTED=FALSE;
	TTY.amount_to_write=0;
	if (TTY.COMDEV==INVALID_HANDLE_VALUE) return TRUE;
	TTY.read_pause=TRUE;
	fThreadDone = TRUE;
		
    if (!PurgeComm(TTY.COMDEV, PURGE_FLAGS))  report_error("PurgeComm failed..");
    if (!EscapeCommFunction(TTY.COMDEV, CLRDTR)) report_error("EscapeCommFunction failed");
    // if (!SetCommTimeouts(TTY.COMDEV,  &(TTY.TIMEOUTSORIG)))   report_error("SetCommTimeouts failed");

	CloseHandle(TTY.COMDEV);
	TTY.COMDEV=INVALID_HANDLE_VALUE;
    
	CloseHandle(TTY.READERTHREAD);
    //CloseHandle(TTY.WRITERTHREAD);
    
	TTY.read_pause=sav_pause;

    return TRUE;
}


