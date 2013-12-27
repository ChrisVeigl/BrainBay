
BrainBay  Version 1.8, written by Chris Veigl under GPL, contact: chris@shifz.org

Objects-development - Information

The internal processing of the Signal and Channel Data in 'BrainBay' is very simple:
Each output port of an object can be connected to one or more input ports of other objects.
An input port can handle only one connection (all others are ignored).
The data flow consists of float-values ranging from 0 to 1024 ->  the objects can
be sure that the incoming data does not exceed these ranges.
The existing objects can be accessed by an array of pointers:

BASE_CL * objects[MAX_OBJECTS];  // array of existing objetcts, defined in GLOBALS.CPP

The number of objects is stored in GLOBAL.objects (you find the 'GLOBAL' structure in brainBay.h)

BASE_CL * actobject;  // the current object, which values are showed in the toolbox window


The new values are passed at sampling frequency (256 HZ) and the worker-methods of all
existing objects are called at that frequency.
You find the origin of the stream-values in the modules
OB_EEG.CPP and TTY.CPP (TTY contains the setting up of the com-port and the reader
thread, and OB_EEG contains the processing of the different firmware-protocols, the timing
of archive-file-reading) 
The function 'process_packets' passes filtered channel data to all objects connected to the EEG-Object.
It is called every time a valid packet has been recognized.

void process_packets(void)    // in OB_EEG.CPP
{
	unsigned int portnum;
	static double calc;
	int t;

	GLOBAL.packetspersecond++;
    	GLOBAL.packetcounter++;
	if ((GLOBAL.packetcounter&255)==0) GLOBAL.seconds++;
	for (portnum=0;objects[0]->out[portnum].to_port!=-1;portnum++)
	{
			calc=(double)PACKET.buffer[objects[0]->out[portnum].from_port];
			objects[0]->pass_values(objects[0]->out[portnum].from_port,(float) calc);
	}
	
	for (t=0;t<GLOBAL.objects;t++)
		 objects[t]->work();
	
} 

'pass_values' informs the connected objects that a new value has arrived on a specified port 
by calling the object's 'incoming_data'-method. 'pass_value' is defined in the Base Class.
BASE_CL inherits its properties and methods to all other objects. 
One property is the link-strtucture, which holds the connected object-numbers and ports. 


class BASE_CL    // in base.h
{
  public:
	HWND hDlg;     // dialog handle to the toolbox window, to modify the object's properties
	HBITMAP hbm;
	int xPos;
	int yPos;
	int type;      // Object-Type
	int inports;   // number of input port of the object
	int outports;  // number of output ports of the object
  
	LINKStruct  out[MAX_CONNECTS];   // links to other objects

	virtual ~BASE_CL (void) {}
	virtual void work (void) {}             // various worker methods for the different objects
	virtual void make_dialog (void) {}      //  displays the toolbox window
	virtual void load (HANDLE hFile) {}	//  load properties from configfile
	virtual void save (HANDLE hFile) {}	//  save properties to configfile
	virtual void incoming_data(int port, float value) {}    // tells the objects incomig data
	
	void pass_values (int port, float value)
	{
		LINKStruct * act_link;
		for (act_link=&(out[0]);act_link->to_port!=-1;act_link++)
			if (act_link->from_port==port)
				objects[act_link->to_object]->incoming_data(act_link->to_port, value);
	}
};


But these things you only need to understand when you want to alter or improve the whole application
or change this concept.

When you want to build new objects, there are only a few things you need:

* a new class derived from BASE_CL, with the properties for your object, 
  with methods for construction, incoming data, load, save, work and destruction (view the OB_*.H - files)
* a dialog handler to control the objects behaviour (view the OB_*.CPP - files)
* a bitmap for the objects appearance on screen (use the resource-editor) 
* a menu entry and the 'create_object'-call when the menu-point was selected 
  (view the MainWnd-Handler in brainbay.cpp) 


- the 'load' and 'save' methods:
first I simply saved a copy of the whole object to disk, but then I realized that every little
change (eg. altering properties or creating new properties for the object) would outdate all previously
safed configuration-files, because the amount of memory changed and no of the following properties
would be on their places.
So i decided to change the configuration file-format to some kind of 'human-readable' - style,
(the properties are exported to and imported from a textfile). Using that method, old
configuration file can be used in most cases. But this method has also a disadvantage: for objects that
hold large amounts of data (like color palettes, tone scales, signal segments etc.) it makes no sense 
to store all this data in an alphanumerical (readable) format in the configuration file. Therefore, only
a filename is stored and all such data is kept in seperate files in the 'resource'-subdirectory. You can
consider this directory as a library which holds all palettes, harmonics, etc. The configuration file holds
only the name (not the path) of these files, assuming to find them in the 'resource'-subdirectory.

example: the 'load' method of the filter-object contains the following:

	  load_property("xpos",P_INT,&xPos);
	  load_property("ypos",P_INT,&yPos);
  	  load_property("name",P_STRING,name);
	  load_property("type",P_INT,&filtertype);
	  load_property("order",P_INT,&par0);
	  load_property("par1",P_INT,&par1);
	  load_property("par2",P_INT,&par2);
	  load_property("par3",P_INT,&par3);

in case of a filter, the initialisations like setting up the filter function and buffer have to be done here, too.
the 'save' method looks like this:

	  save_property(hFile,"xpos",P_INT,&xPos);
	  save_property(hFile,"ypos",P_INT,&yPos);
  	  save_property(hFile,"name",P_STRING,name);
	  save_property(hFile,"type",P_INT,&filtertype);
	  save_property(hFile,"order",P_INT,&par0);
	  save_property(hFile,"par1",P_INT,&par1);
	  save_property(hFile,"par2",P_INT,&par2);
	  save_property(hFile,"par3",P_INT,&par3);

The load and save methods of all objects are called when the user loads or saves a configuration file.
When saving, the above method produces the following entry in the configuration-file:

	next object=4
	xpos=107
	ypos=59
	name=Alpha Bessel
	type=9
	order=4
	par1=12
	par2=18
	par3=0
	linkport 0-3,0
	linkport 0-4,0
	end object




--------------------------------------------------------

Compiling/Linking-information for VC++, Win32-Debug:

The Projects uses the following additional libraries: 
  fidlib.lib : Jim Peter's filter library
  opengl32.lib, glu32.lib : openGL
  SDL.lib SDL_net.lib SDL_sound.lib : Simple Direct Media Layer 
  matheval.lib  modplug.lib vfw32.lib 
  cv.lib cvcam.lib cxcore.lib highgui.lib Open Computer Vision Library
  skinstyle.lib : skinnned dialog interface
  libeng.lib libmx.lib:  third Party commercial libraries for Matlab engine connection
  (if you want matlab engine support, define the constant MATLAB_RELEASE and add these
   two libs provided with matlab to your library path)

Preprocessor Options:
  WIN32,_DEBUG,_WINDOWS,_MBCS

Project Options:
  /nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"Debug/" /Fd"Debug/" /FD /GZ /c 

all Library Modules:
  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib opengl32.lib glu32.lib SDL.lib SDL_net.lib fidlib.lib matheval.lib SDL_sound.lib modplug.lib vfw32.lib glaux.lib cv.lib cvcam.lib cxcore.lib highgui.lib libeng.lib libmx.lib skinstyle.lib 



---------------------------------------------------------


Compiling/Linking-information for MinGW: (contributed by Jeremy Wilkerson)

MinGW has all the needed libraries.  It links with the native Windows libraries.  
Here are the steps to build the application:

 -use the make to compile and link just the main source files
 -use the make-all.bat file to compile and link everthing, including matheval library

the resource file can be compiled seperately using  windres -i brainBay.rc -o brainBayRes.o
following compile switches are used:  g++ -c -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS -DMINGW *.cpp

the -DMINGW define tells the preprocessor to bypass the skinstyle library for the skinned dialog interface
  because this is only supported by the MSVC++ compiler by now..


If mingw\bin is on your path, use the command 'make depends', then 'make'.  
The 'make depends' generates a file that tells make which header files each cpp file depends on, 
and it doesn't need to be executed again unless that information changes.


