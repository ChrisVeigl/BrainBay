Welcome to BrainBay !
---------------------

BrainBay is an open source bio- and neurofeedback application. It also offers
some features for the creation of alternative Human-Computer-Interfaces (HCIs) 
such as facetracking via webcam, EMG signal pattern recognition or mouse-/keyboard control.

All source code is licensed under GPL, for copyright information 
please have a look at ReadMe_License.txt

*New:* **BrainBay User Forum** available, see: https://brainbay.iphpbb3.com/


Version Info
------------
 
* The current release version is 2.8  (2023-07-14)

* Recent changes for this release:
  * support for new Brainflow version, serveral new compatible EEG amplifiers!
  
* Not-so-recent changes in recent releases worth mentioning:
  * improved element captions
  * New elements Shadow and Volume allow control of volume and desktop transparency
  * New element OSC-Sender for transfer of live values to other applications via Open Sound Control / UDP
  * Logical elements And, Or and Not got extended modes, increasing flexibility
  * Threshold element improved, increasing flexibility
  * Improved interface and handling for Neurobit devices
  * Support for Neurosky devices
  * Cursorkey integration for Oscilloscope and Threshold Windows
  * Oscilloscope features snapshots for training reporting
  * SessionManager and Sessiontime elements for menu-based selection of design

Link to Release and other Infos
-------------------------------

* Download latest release: https://github.com/ChrisVeigl/BrainBay/releases

* The release contains an installer for windows. In addition, the 'bin' folder of the github source repo contains an up-to-date executable file (BrainBay.exe)
  The 'bin' folder also contains the user- and developer manuals (.pdf).

* If you want to use BrainBay with the OpenBCI Ganglion, install the OpenBCIHub fist - have a look at:
  - http://docs.openbci.com/OpenBCI%20Software/01-OpenBCI_GUI#the-openbci-gui-installing-the-openbci-gui-as-a-standalone-application-install-openbci_gui-on-windows
  
* Useful info if you want to run BrainBay under Linux:
  - http://www.autodidacts.io/use-openbci-with-brainbay-on-ubuntu-linux-and-wine/
  - see lso new Linux installation tips below!

* Other related infos:
  - https://sites.google.com/site/biofeedbackpages/brainbay-openbci
  - http://openbci.com/forum/index.php?p=/discussion/90/brainbay-install-neurofeedback-tutorial
  - http://eeghacker.blogspot.co.at/2013/11/brainbay-eeg-visualization-software.html

* If you want to modify or extend the software, the Visual Studio 2010
  project files and all source modules are located in the 'src' folder.


## Installing and Using BrainBay under Linux:

* Install Wine (Windows-Emulator):
  `sudo apt install winehq`
  If this does not work: see installation guide for your OS, e.g. https://wiki.winehq.org/Debian
* Install BrainBay:
  doubleclick `BrainBay_Setup.exe`, extract the files to a folder
* Determint the Serial interface:
  Connect the EEG/serial device, use `dmesg -w` to display the device file (e.g. /dev/ttyUSB0) 
* Configure the serial interface as COM Port in Wine:
  `WINEPREFIX=~/.wine wine regedit`
  Add Item `COM1` with correct string value for eh device file, e.g. `/dev/ttyUSB0` (respectively the actual device name for the serial port) under `HKEY_LOCAL_MACHINE\Software\Wine\Ports`
* Activate configuration:
  `wineserver -k`
* To get audio (midi) working:
  `sudo apt install timidity`
* Start Brainbay with midi support:
  `timidity -iA -Os -B2,8 &`
  `wine brainbay.exe`
  


Credits
-------

* Jim Peters for his marvellous filter-works (http://uazu.net/fiview/)
* Jeremy Wilkerson for programming some BrainBay Objects and MinGW/WINE support
* Jeff Molofee (NeHe) for this great OpenGl-tutorial (http://nehe.gamedev.net)
* Don Cross for a well working fft-routine (http://www.intersrv.com/~dcross/fft.html)
* AllenD for showing how to do a thread-oriented Com-Handler in Win32
* Craig Peacock for the the PortTalk I/O-driver, see http://www.beyondlogic.org 
* Marcin Kocikowski for the Neurobit Optima-4 Device and support
* Raymond Nguyen for the vector port additions
* Franz Strobl for the OCZ NIA support
* Stephan Gerhard for the QDS parser
* William Croft (OpenBCI) for various fixes, enhancements and blog posts
* Elliot Mebane for valuable feedback and hints for improvements
* Jan-Hendrik Franz for Wine installation tips

Donate
------

If you like BrainBay and want to support its development, please consider a donation:
http://www.shifz.org/brainbay/donate.htm

Further Information
-------------------

* Find documentation on the project hompage http://brainbay.lo-res.org
and in the user and developer manuals.

* I recommend having a look at the OpenEEG site http://openeeg.sf.net,
the OpenBCI project: http://www.openbci.org and the Neurobit devices http://www.neurobitsystems.com/

* If you want to share your BrainBay design files for neurofeedback or biofeedback
protocols, I would be happy to include them in the release.


Enjoy :-)

Chris Veigl
contact: chris@shifz.org

