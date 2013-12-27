/******************************************************************************/
/*                                                                            */
/*                          IoExample for PortTalk V2.1                       */
/*                        Version 2.1, 12th January 2002                      */
/*                          http://www.beyondlogic.org                        */
/*                                                                            */
/* Copyright © 2002 Craig Peacock. Craig.Peacock@beyondlogic.org              */
/* Any publication or distribution of this code in source form is prohibited  */
/* without prior written permission of the copyright holder. This source code */
/* is provided "as is", without any guarantee made as to its suitability or   */
/* fitness for any particular use. Permission is herby granted to modify or   */
/* enhance this sample code to produce a derivative program which may only be */
/* distributed in compiled object form only.                                  */
/******************************************************************************/

#include <winioctl.h>


#define PORTTALK_TYPE 40000 /* 32768-65535 are reserved for customers */

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

#define IOCTL_IOPM_RESTRICT_ALL_ACCESS \
    CTL_CODE(PORTTALK_TYPE, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IOPM_ALLOW_EXCUSIVE_ACCESS \
    CTL_CODE(PORTTALK_TYPE, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_IOPM \
    CTL_CODE(PORTTALK_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ENABLE_IOPM_ON_PROCESSID \
    CTL_CODE(PORTTALK_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_READ_PORT_UCHAR \
    CTL_CODE(PORTTALK_TYPE, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_WRITE_PORT_UCHAR \
    CTL_CODE(PORTTALK_TYPE, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)

//void report_error( char * Message );
char errormsg[1024];

unsigned char OpenPortTalk(void);
void ClosePortTalk(void);

void outportb(unsigned short PortAddress, unsigned char byte);
unsigned char inportb(unsigned short PortAddress);

void InstallPortTalkDriver(void);
unsigned char StartPortTalkDriver(void);

#define    inp(PortAddress)         inportb(PortAddress)
#define    outp(PortAddress, Value) outportb(PortAddress, Value)

HANDLE PortTalk_Handle=INVALID_HANDLE_VALUE;        /* Handle for PortTalk Driver */

void outportb(unsigned short PortAddress, unsigned char byte)
{
    unsigned int error;
    DWORD BytesReturned;        
    unsigned char Buffer[3];
    unsigned short * pBuffer;
    pBuffer = (unsigned short *)&Buffer[0];
    *pBuffer = PortAddress;
    Buffer[2] = byte;

    error = DeviceIoControl(PortTalk_Handle,
                            IOCTL_WRITE_PORT_UCHAR,
                            &Buffer,
                            3, 
                            NULL,
                            0,
                            &BytesReturned,
                            NULL);

    //if (!error) {sprintf(errormsg,"Error occured during outportb while talking to PortTalk driver %d\n",GetLastError());
	//report_error(errormsg); } 
}

unsigned char inportb(unsigned short PortAddress)
{
    unsigned int error;
    DWORD BytesReturned;
    unsigned char Buffer[3];
    unsigned short * pBuffer;
    pBuffer = (unsigned short *)&Buffer;
    *pBuffer = PortAddress;

    error = DeviceIoControl(PortTalk_Handle,
                            IOCTL_READ_PORT_UCHAR,
                            &Buffer,
                            2,
                            &Buffer,
                            1,
                            &BytesReturned,
                            NULL);

    //if (!error) {sprintf(errormsg,"Error occured during inportb while talking to PortTalk driver %d\n",GetLastError());
    //				report_error(errormsg); }
    return(Buffer[0]);
}

unsigned char OpenPortTalk(void)
{
	if (PortTalk_Handle!=INVALID_HANDLE_VALUE) return 0;
    /* Open PortTalk Driver. If we cannot open it, try installing and starting it */
    PortTalk_Handle = CreateFile("\\\\.\\PortTalk", 
                                 GENERIC_READ, 
                                 0, 
                                 NULL,
                                 OPEN_EXISTING, 
                                 FILE_ATTRIBUTE_NORMAL, 
                                 NULL);

    if(PortTalk_Handle == INVALID_HANDLE_VALUE) {
            /* Start or Install PortTalk Driver */
            StartPortTalkDriver();
            /* Then try to open once more, before failing */
            PortTalk_Handle = CreateFile("\\\\.\\PortTalk", 
                                         GENERIC_READ, 
                                         0, 
                                         NULL,
                                         OPEN_EXISTING, 
                                         FILE_ATTRIBUTE_NORMAL, 
                                         NULL);
               
            if(PortTalk_Handle == INVALID_HANDLE_VALUE) {
                    report("PortTalk: Couldn't access PortTalk Driver, Please ensure driver is loaded.");
                    return -1;
            }
    }
	return 0;
}

void ClosePortTalk(void)
{
	if (PortTalk_Handle != INVALID_HANDLE_VALUE)
       CloseHandle(PortTalk_Handle);
	PortTalk_Handle=INVALID_HANDLE_VALUE;
}

unsigned char StartPortTalkDriver(void)
{
    SC_HANDLE  SchSCManager;
    SC_HANDLE  schService;
    BOOL       ret;
    DWORD      err;

    /* Open Handle to Service Control Manager */
    SchSCManager = OpenSCManager (NULL,                        /* machine (NULL == local) */
                                  NULL,                        /* database (NULL == default) */
                                  SC_MANAGER_ALL_ACCESS);      /* access required */
                         
    if (SchSCManager == NULL)
      if (GetLastError() == ERROR_ACCESS_DENIED) {
         /* We do not have enough rights to open the SCM, therefore we must */
         /* be a poor user with only user rights. */
         report_error("PortTalk: You do not have (admin-)rights to access the Service Control Manager, PorTalk could not be installed.");
         
         return(0);
         }

    do {
         /* Open a Handle to the PortTalk Service Database */
         schService = OpenService(SchSCManager,         /* handle to service control manager database */
                                  "PortTalk",           /* pointer to name of service to start */
                                  SERVICE_ALL_ACCESS);  /* type of access to service */

         if (schService == NULL)
            switch (GetLastError()){
                case ERROR_ACCESS_DENIED:
                        report("PortTalk: You do not have rights to the PortTalk service database");
                        return(0);
                case ERROR_INVALID_NAME:
                        report("PortTalk: The specified service name is invalid.");
                        return(0);
                case ERROR_SERVICE_DOES_NOT_EXIST:
                        report("PortTalk: The PortTalk driver does not exist. Installing driver.");
                        
                        InstallPortTalkDriver();
                        break;
            }
         } while (schService == NULL);

    /* Start the PortTalk Driver. Errors will occur here if PortTalk.SYS file doesn't exist */
    
    ret = StartService (schService,    /* service identifier */
                        0,             /* number of arguments */
                        NULL);         /* pointer to arguments */
                    
    if (ret) ;// report("PortTalk: The PortTalk driver has been successfully started.");
    else {
        err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING)
          report("PortTalk: The PortTalk driver is already running.");
        else {
          report_error("PortTalk: Unknown error while starting PortTalk driver service.");
          report("PortTalk: Does PortTalk.SYS exist in your \\System32\\Drivers Directory?");
          return(0);
        }
    }

    /* Close handle to Service Control Manager */
    CloseServiceHandle (schService);
    return(TRUE);
}

void InstallPortTalkDriver(void)
{
    SC_HANDLE  SchSCManager;
    SC_HANDLE  schService;
    DWORD      err;
    CHAR         DriverFileName[80];

    /* Get Current Directory. Assumes PortTalk.SYS driver is in this directory.    */
    /* Doesn't detect if file exists, nor if file is on removable media - if this  */
    /* is the case then when windows next boots, the driver will fail to load and  */
    /* a error entry is made in the event viewer to reflect this */

    /* Get System Directory. This should be something like c:\windows\system32 or  */
    /* c:\winnt\system32 with a Maximum Character lenght of 20. As we have a       */
    /* buffer of 80 bytes and a string of 24 bytes to append, we can go for a max  */
    /* of 55 bytes */

    if (!GetSystemDirectory(DriverFileName, 55))
        {
         report_error("PortTalk: Failed to get System Directory. Is System Directory Path > 55 Characters?\n");
         report("PortTalk: Please manually copy driver to your system32/driver directory.\n");
        }

    /* Append our Driver Name */
    lstrcat(DriverFileName,"\\Drivers\\PortTalk.sys");
    sprintf(errormsg,"PortTalk: Copying driver to %s\n",DriverFileName);
	report(errormsg);

    /* Copy Driver to System32/drivers directory. This fails if the file doesn't exist. */

    if (!CopyFile("PortTalk.sys", DriverFileName, FALSE))
        {
         report_error("PortTalk: Failed to copy driver");
         report("PortTalk: Please manually copy driver to your system32/driver directory.\n");
        }

    /* Open Handle to Service Control Manager */
    SchSCManager = OpenSCManager (NULL,                   /* machine (NULL == local) */
                                  NULL,                   /* database (NULL == default) */
                                  SC_MANAGER_ALL_ACCESS); /* access required */

    /* Create Service/Driver - This adds the appropriate registry keys in */
    /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services - It doesn't  */
    /* care if the driver exists, or if the path is correct.              */

    schService = CreateService (SchSCManager,                      /* SCManager database */
                                "PortTalk",                        /* name of service */
                                "PortTalk",                        /* name to display */
                                SERVICE_ALL_ACCESS,                /* desired access */
                                SERVICE_KERNEL_DRIVER,             /* service type */
                                SERVICE_DEMAND_START,              /* start type */
                                SERVICE_ERROR_NORMAL,              /* error control type */
                                "System32\\Drivers\\PortTalk.sys", /* service's binary */
                                NULL,                              /* no load ordering group */
                                NULL,                              /* no tag identifier */
                                NULL,                              /* no dependencies */
                                NULL,                              /* LocalSystem account */
                                NULL                               /* no password */
                                );

    if (schService == NULL) {
         err = GetLastError();
         if (err == ERROR_SERVICE_EXISTS)
               report("PortTalk: Driver already exists. No action taken.");
         else  report_error("PortTalk: Unknown error while creating Service.");    
    }
    else report("PortTalk: Driver successfully installed.");

    /* Close Handle to Service Control Manager */
    CloseServiceHandle (schService);
}