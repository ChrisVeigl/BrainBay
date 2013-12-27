
                Beyond Logic Port Talk I/O Port Driver
                      http://www.beyondlogic.org

The PortTalk driver combined with allowIO.exe, grants certain programs 
exclusive access to IO Ports on a Windows NT/2000/XP system.

Important information for Upgrading PortTalk from V1.x
______________________________________________________

When installing PortTalk V2.0 on machines with an older version 
of V1.x, the existing driver must be un-installed. Simply run
uninstall.exe with administrator privileges.

Installing PortTalk V2.0
________________________

The PortTalk usermode programs now come with a self installer. 
This is built into allowio.exe and ioexample.exe. When these
usermode programs are executed, a check is made if the PortTalk
driver is started. If the driver is not started it attempts to
install the driver and then start it.

Allowio - Grants programs access to IO Ports.
_____________________________________________

Usage : AllowIO <executable.exe> <Hex Addresse(s)> <Switches>

eg. Using allowio to grant access to IO ports 0x42, 0x43 and 0x61

 C:\porttalk\AllowIO>allowio 0x42 0x43 0x61 beep.exe
 Beyond Logic AllowIO.EXE
 Address 0x042 (IOPM Offset 0x08) has been granted access.
 Address 0x043 (IOPM Offset 0x08) has been granted access.
 Address 0x061 (IOPM Offset 0x0C) has been granted access.
 Executing beep.exe with a ProcessID of 1096
 PortTalk Device Driver has set IOPM for ProcessID 1096.

This is more secure than allowing all programs and processes access
to all IO ports.

If you don't know what addresses a certain program uses, you can
grant access to all ports.

 C:\porttalk>allowio beep /a
 BeyondLogic AllowIO
 Granting exclusive access to all I/O Ports
 Executing beep with a ProcessID of 524
 PortTalk Device Driver has set IOPM for ProcessID 524.

This is less secure. For example the program can talk to COM1 
and lock up your mouse.

Craig Peacock
13th January 2002

