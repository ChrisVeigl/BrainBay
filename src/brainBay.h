/* ----------------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software
  OpenSource Application for realtime BioSignalProcessing & HCI with OpenEEG hardware. 
			   
  Author: Chris Veigl, contact: chris@shifz.org
  
  Co-Authors:
		 Jeremy Wilkerson (Modules: AND, OR, NOT, WAV, CORRELATION, EVALUATOR)
		 Lester John (Module MATLAB-transfer)
		 Stephan Gerhard (QDS parser)
		 Franz Stobl ( NIA support )

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
  

  MODULE:  brainbay.h

  Main Header File, 
  including prototypes for the multimedia and filter functions,
  declaration of important data structures, global variables,
  functions and Object-Identifiers

  
 -------------------------------------------------------------------------------------*/


#if !defined(MYEEG_H)
#define MYEEG_H


#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>
#include <math.h>
#include <GL/gl.h>					// OpenGL32 Library
#include <GL/glu.h>					// GLu32 Library
#include <SDL/SDL.h>				// SDL Library
extern "C" { 
#include "fidlib.h"				    // Filter Library, uses .c-conventions
} 


#include "resource.h"
#include "base.h"



//
// GLOBAL CONSTANTS
//



//  Available Objects, add new Objects here :
//
#define OB_EEG           0
#define OB_MIDI          1
#define OB_FFT           2
#define OB_THRESHOLD     3
#define OB_FILTER        4
#define OB_PARTICLE      5
#define OB_OSCI          6
#define OB_TRANSLATE     7
#define OB_SIGNAL        8
#define OB_MAGNITUDE     9
#define OB_AND          10
#define OB_WAV          11
#define OB_OR           12
#define OB_NOT          13
#define OB_TCP_RECEIVER 14
#define OB_DOKU         15
#define OB_EVAL		    16
#define OB_AVI		    17
#define OB_AVERAGE	    18
#define OB_CORR		    19
#define OB_EDF_READER   20
#define OB_EDF_WRITER   21
#define OB_TCP_SENDER   22
#define OB_COMPARE	    23
#define OB_BALLGAME     24
#define OB_MIXER4       25
#define OB_MOUSE        26
#define OB_ERPDETECT    27
#define OB_COM_WRITER   28
#define OB_CAM          29
#define OB_INTEGRATE    30
#define OB_DEBOUNCE     31
#define OB_SAMPLE_HOLD  32
#define OB_CONSTANT     33
#define OB_MATLAB       34
#define OB_COUNTER      35
#define OB_SKINDIALOG   36
#define OB_FILE_WRITER  37
#define OB_DEVIATION    38
#define OB_MCIPLAYER    39
#define OB_KEYSTRIKE    40
#define OB_PEAKDETECT   41
#define OB_SPELLER	    42
#define OB_MARTINI	    43
#define OB_FILE_READER	44
#define OB_PORT_IO		45
#define OB_ARRAY3600    46
#define OB_COMREADER    47
#define OB_NEUROBIT     48
#define OB_MIN          49
#define OB_MAX          50
#define OB_ROUND        51
#define OB_DIFFERENTIATE 52
#define OB_DELAY        53
#define OB_LIMITER      54
#define OB_EMOTIV		55
#define OB_FLOATVECTOR	56
#define OB_VECTORFLOAT	57
#define OB_DISPLAYVECTOR 58
#define OB_BUFFER       59
#define OB_GANGLION     60
#define OB_SESSIONTIME  61
#define OB_SESSIONMANAGER 62
#define OB_KEYCAPTURE   63
#define OB_BUTTON       64
#define OB_EVAL_EXPRTK  65
#define OB_SHADOW       66
#define OB_VOLUME       67
#define OB_OSC_SENDER   68
#define OB_BIOSEMI		69
#define OB_BRAINFLOW	70

#define OBJECT_COUNT 	71





#define OBJNAMES "EEG","MIDI","FFT","THRESHOLD","FILTER","PARTICLES","OSCI", \
				 "TRANSLATE","SIGNAL","MAGNITUDE","AND","SOUND","OR","NOT", \
				 "TCP-RECEIVE","INFO","EVALUATOR", "AVI","AVERAGER","CORRELATION", \
				 "EDF-READER","EDF-WRITER","TCP-SENDER","COMPARE","BALLGAME",  \
				 "MIXER", "MOUSE", "ERP-DETECT", "COM-WRITER", "CAMERA", "INTEGRATE", \
				 "DEBOUNCE", "SAMPLE_HOLD", "CONSTANT", "MATLAB","COUNTER", \
				 "SKINDIALOG", "FILEWRITE", "DEVIATION", "MEDIAPLAYER", "KEYSTRIKE", \
				 "PEAKDETECT", "SPELLER", "MARTINI", "FILEREAD", "PORT_IO", \
				 "ARRAY-3600", "COMREADER", "NEUROBIT", "MIN", "MAX", "ROUND", \
				 "DIFFERENTIATE", "DELAY", "LIMITER", "EMOTIV", "FLOAT_VECTOR", \
				 "VECTOR_FLOAT", "DISPLAY_VECTOR", "VECTORBUFFER", "GANGLION", \
				 "SESSIONTIME", "SESSIONMANAGER", "KEYCAPTURE", "BUTTON", "EVALUATOR EXPR-TK", \
				 "SHADOW", "VOLUME", "OSC-SENDER", "BIOSEMI", "BRAINFLOW"
//
// use the main menu handler in brainbay.cpp 
// to call the 'create_object'-function (located in in gloabals.cpp)
//
//  brainbay-Objects need the following methods (copy them from other ob_.cpp files):
//	  void make_dialog(void);     displays a toolbox Window for settings
//	  void load(HANDLE hFile);    loads objects properties from config File
//	  void save(HANDLE hFile);    saves objects properties to config File
//	  void incoming_data(int port, float value);   new data arrived at port
//	  void work(void);            called at sampling frequency
//    + Constructor and Destructor-functions.



 

// Filetypes for open and save-dialogs
#define FT_HARMONIC      0
#define FT_ARCHIVE       1
#define FT_CONFIGURATION 2
#define FT_PALETTE       3
#define FT_WAV           4
#define FT_EDF           5
#define FT_AVI           6
#define FT_ERP           7
#define FT_TXT           8
#define FT_BMP           9
#define FT_DIC          10
#define FT_NB4          11
#define FT_NB_ARCHIVE   12
#define FT_MCI          13
#define FT_GANGLION_ARCHIVE 14
#define FT_BF_ARCHIVE   15


#define MAX_COMPORT				150
#define DEFAULT_PORT            0
#define DEF_BAUDRATE            CBR_57600
#define MAX_WRITE_BUFFER        1024
#define MAX_READ_BUFFER         8192
#define READ_TIMEOUT            1000
#define PURGE_FLAGS             PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR 


#define FILE_WRITING 1
#define FILE_READING 2

#define OPEN_SAVE 1
#define OPEN_LOAD 2

#define FILE_TEXTMODE  0
#define FILE_INTMODE   1

#define P_INT     1
#define P_FLOAT   2
#define P_STRING  3
#define P_END    99

#define DEV_MODEEG_P2 0
#define DEV_MODEEG_P3 1
#define DEV_RAW 2
#define DEV_MONOLITHEEG_P21 3
#define DEV_SBT4 4
#define DEV_RAW8BIT 5
#define DEV_PENDANT3 6
#define DEV_QDS 7
#define DEV_NIA 8
#define DEV_IBVA 9
#define DEV_SBT2 10
#define DEV_OPENBCI8 11
#define DEV_OPI_EXPLORATION 12
#define DEV_OPENBCI16 13
#define DEV_NEUROSKY 14


#define MAX_TEMPSTRING    512
#define MAX_LOADSTRING    512

#define MAX_MIDIPORTS       16
#define DEF_MIDIPORT		0
#define MAX_MIDITONES       20
#define MAX_NAMELEN			100
#define MAX_HARMONICTONES   256

#define FILTERTYPES	  9
#define PASSTYPES	  2

#define KEY_UP 38
#define KEY_DOWN 40
#define KEY_LEFT 37
#define KEY_RIGHT 39
#define KEY_ENTER 13
#define KEY_DELETE 46
#define KEY_BACKSPACE 8
#define KEY_CTRL 17
#define KEY_C 67
#define KEY_V 86
#define KEY_F5 116
#define KEY_F6 117
#define KEY_F7 118
#define KEY_F8 119

#define DEF_ZOOM    100
#define DEF_STARTBAND 1
#define DEF_ENDBAND  40
#define DEF_ALIGN     0

#define LEN_PIXELBUFFER  	500

#define CON_HEIGHT   15
#define CON_START    25
#define CON_MAGNETIC 10

#define  DDC_PI       (3.14159265358979323846)

//
// GLOBAL VARIABLES
//


extern HINSTANCE	 hInst;				// aktuelle Instanz
extern HACCEL        ghAccel;

extern HWND          ghWndMain;
extern HWND          ghWndStatusbox;
extern HWND          ghWndSettings;
extern HWND          ghWndDesign;
extern HWND          ghWndToolbox;
extern HWND          ghWndAnimation;
extern HGLRC		 GLRC_Animation;

extern int PACKETSPERSECOND;

extern char   midi_instnames[256][30];
extern char   captfiletypes[10][40];
extern char   devicetypes[20][40];
extern char   objnames[OBJECT_COUNT][20];
extern char   dimensions[10][10];
extern int    BYTES_PER_PACKET[20];
extern int    AMOUNT_TO_READ[20];
extern int    fft_bin_values[10];
extern char * szBaud[];
extern DWORD  BaudTable[];

extern struct TTYStruct			   TTY;
extern struct GLOBALStruct         GLOBAL;
extern struct PACKETStruct		   PACKET;
extern struct CAPTFILEStruct	   CAPTFILE;
extern struct DRAWStruct		   DRAW;
extern struct MIDIPORTStruct       MIDIPORTS[MAX_MIDIPORTS];
extern struct FILTERTYPEStruct	   FILTERTYPE[FILTERTYPES];
extern struct PASSTYPEStruct	   PASSTYPE[PASSTYPES];
extern struct SCALEStruct          LOADSCALE;
extern struct TIMINGStruct         TIMING;

//
//    DATA STRUCTURES
//


typedef struct GLOBALStruct
{
	unsigned int	 channels;
	int  objects;
	int  objstate;
	int  actcon;
	int  drawfromobj;
   
	int  midiports;
	int  temp;
	int  top,left,right,bottom;
	int	 anim_top,anim_left,anim_right,anim_bottom;
	int	 tool_top,tool_left,tool_right,tool_bottom;
	int	 design_top,design_left,design_right,design_bottom;
	int  fx,tx;
	int  fy,ty;

	int loading;
	int read_tcp;
	int packet_parsed;
	int running;
	int minimized;
	int session_sliding;
	int locksession;
	int pressed_key;
	int statusWindowHeight;
	int statusWindowHeightWithPlayer;
	int statusWindowMargin;
	int statusWindowMarginWithPlayer;

	int fly;
	int run_exception;

	int showdesign;
	int showtoolbox;
	int hidestatus;
	int startup;
	int startdesign;
	int autorun;
	int use_cv_capture;
	int os_version;

	int neurobit_available;
	int emotiv_available;
	int biosemi_available;
	int ganglion_available;
	int brainflow_available;

	int ganglion_bledongle;

	int P3ALC1;
	int P3ALC2;

	int dialog_interval;
	int draw_interval;
	LPARAM main_maximized;

	WORD actcolumn;
	char configbuffer[50000];
	char nextconfigname[256];
	char resourcepath[256];
	char configfile[256];
	char emotivpath[256];
	char startdesignpath[256];
	char ganglionhubpath[256];
	char gangliondevicename[100];
	char neurobit_device[100];

	long session_length;
	long session_start;
	long session_end;
	int	 session_loop;
	int syncloss;

	int add_archivetime;
	float addtime;
} GLOBALStruct;



/*


EDF-HEADER RECORD 
8 ascii : version of this data format (0) 
80 ascii : local patient identification 
80 ascii : local recording identification 
8 ascii : startdate of recording (dd.mm.yy) 
8 ascii : starttime of recording (hh.mm.ss) 
8 ascii : number of bytes in header record 
44 ascii : reserved 
8 ascii : number of data records (-1 if unknown) 
8 ascii : duration of a data record, in seconds 
4 ascii : number of signals (ns) in data record 


// ns.. number of signals (channels)

ns * 16 ascii : labels (e.g. EEG FpzCz or Body temp) 
ns * 80 ascii : transducer types (e.g. AgAgCl electrode) 
ns * 8 ascii :  physical dimensions (e.g. uV or degreeC) 
ns * 8 ascii :  physical minimums (e.g. -500 or 34) 
ns * 8 ascii :  physical maximums (e.g. 500 or 40) 
ns * 8 ascii :  digital minimums (e.g. -2048) 
ns * 8 ascii :  digital maximums (e.g. 2047) 
ns * 80 ascii : prefiltering infos (e.g. HP:0.1Hz LP:75Hz) 
ns * 8 ascii :  nr of samples / segment
ns * 32 ascii : reserved 


EDF-DATA RECORD 
channel[1] * integer : first signal in the data record 
channel[2] * integer : second signal 
.. 
.. 
channel[ns] * integer : last signal 
*/

typedef struct EDFHEADER_PHYSICALStruct
{
char version[8];
char patient[80];
char recording[80];
char startdate[8];
char starttime[8];
char headerlength[8];
char reserved[44];
char records[8];
char duration[8];
char channels[4];
char end;
} EDFHEADER_PHYSICALStruct;

typedef struct EDFHEADERStruct
{
	char  patient[81];
	char  device[81];
	int	  channels;
	int   duration;
	int   samplespersegment;
	int   segments;
	int   samplingrate;
} EDFHEADERStruct;


#define samplebuflen 8192
 
typedef struct CHANNELStruct
{
    char label[17];
    char transducer[81];
    char prefiltering[81];
    char physdim[9];
	int  physmin;
	int  physmax;
	int  digmin;
	int  digmax;
	int  samples;
	short  buffer[samplebuflen];
} CHANNELStruct ;



typedef struct TTYStruct
{
    HANDLE		 COMDEV;
    int 		 PORT;
    DWORD		 BAUDRATE ;
	COMMTIMEOUTS TIMEOUTSORIG;
    COMMTIMEOUTS TIMEOUTSNEW;
	HANDLE		 READERTHREAD;
	HANDLE		 WRITERTHREAD;
	HANDLE		 OPITHREAD;
	HANDLE		 ThreadExitEvent;
	HANDLE		 writeMutex;
	BOOL		 CONNECTED;
	BOOL		 BIDIRECT;
	int          FLOW_CONTROL;
 	int          devicetype;
	int			 samplingrate;
    WORD		 read_pause;
	int          amount_to_read;
	int          bytes_per_packet;
	int			 amount_to_write;
    unsigned char readBuf[4096];
	unsigned char writeBuf[4096];
	LONGLONG packettime;
} TTYStruct;


typedef struct TIMINGStruct
{
	UINT     timerid;
	int      dialog_update;
	int      draw_update;
	int		 pause_timer;
	long     packetcounter;
	LONGLONG pcfreq; 
	LONGLONG acttime;    
	LONGLONG timestamp;    
	LONGLONG readtimestamp;
	LONGLONG packettime;
	unsigned int ppscounter;
	unsigned int actpps;
} TIMINGStruct;


typedef struct PACKETStruct
{
    unsigned char	  readstate;
	unsigned int 	  extract_pos;
	unsigned int 	  info;
	unsigned int 	  requestedinfo;
	unsigned int 	  chn;
    unsigned char     number;
	unsigned char     old_number;
	unsigned char     switches;
	unsigned char     aux;
	unsigned int      buffer[MAX_EEG_CHANNELS*2];
	unsigned int      tempbuf[MAX_EEG_CHANNELS*2];
} PACKETStruct;


typedef struct MIDIPORTStruct
{
	HMIDIOUT midiout;
	char portname[50];
} MIDIPORTStruct;

typedef struct CAPTFILEHEADERStruct
{
	char description[80];
	char filetype[40];
	char devicetype[40];
	char doc[95];
} CAPTFILEHEADERStruct;

typedef struct CAPTFILEStruct
{
	char    filename[MAX_PATH];
	char    devicetype[100];
    HANDLE  filehandle;
	int     filetype;
    int	    file_action;
	int     do_read;
	int     do_write;
	int		data_begin;
	long    start;
	long	length;
	long    offset;
} CAPTFILEStruct;


typedef struct FILTERTYPEStruct
{
	 char tname[30];
     char init[10];
	 int param;
} FILTERTYPEStruct;

typedef struct PASSTYPEStruct
{
	 char tname[30];
     char init[10];
} PASSTYPEStruct;

typedef struct SCALEStruct
{
	int  len;
	char name[MAX_NAMELEN];
	int  tones[MAX_HARMONICTONES];
} SCLAEStruct;

typedef struct DRAWStruct
{
	int		 particles;
	HPEN	 pen_blue,pen_white,pen_red,pen_ltblue;
	HBRUSH	 brush_blue,brush_white, brush_orange, brush_ltorange, brush_yellow, brush_ltgreen;
	
	HFONT	 scaleFont;
	HFONT    mediumFont;
	long	 scaleFontHeight;
} DRAWStruct;




//
//    FUNCTIONS 
//

//  
//  Init/Deinit Globals functions
//

void   register_classes (HINSTANCE);
void   GlobalInitialize( void );
void   GlobalCleanup( void );
int    write_to_comport ( unsigned char byte);
int    write_to_comport ( unsigned char * buf, int len);
void   write_string_to_comport ( char * s);
int    start_opi_pollthread (void);
int    stop_opi_pollthread (void);
void   init_system_time( void );
void   init_draw(void);
void   init_channels(void);
void   init_midi(void);
void   init_devicetype(void);
int    check_OS(void);
void   init_path(void);
void   update_p21state(void);

void	update_devicetype(void);
void	update_samplingrate(int);
void   create_logfile(void);
void   write_logfile(char *,...);

BOOL killProcess(char * name );




//    Timer-functions for playing the archive file 

void	CALLBACK TimerProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
void	stop_timer(void);
void	start_timer(void);
LONG    get_sliderpos(LONG samplepos);
void	set_session_pos(long pos);
void	get_session_length(void);
void	update_status_window(void);


//  Com-Port functions

void   ParseLocalInput(int);
void   process_packets(void);
void   init_comsettingsDlg( HWND hDlg );
BOOL   SetupCommPort( int );
BOOL   BreakDownCommPort( void );
DWORD  WINAPI ReaderProc( LPVOID );
DWORD  WINAPI WriterProc( LPVOID );
 


//	NIA functions

BOOL	ConnectNIA(HWND);
BOOL	DisconnectNIA(void);
int     ReadNIA(UINT, LONG );

//  Object Arrangement - functions

void   create_object(int);
void   free_object(int);
int    sort_objects(void);

void	update_dimensions(void);
void    link_object(BASE_CL *);
struct  LINKStruct * get_link (BASE_CL *,int);
int     count_inports(BASE_CL *);
int count_objects(int);

//    File-read and write functions

int		open_file_dlg(HWND hDlg, char * szFileName, int type, BOOL flag_save);
BOOL	load_from_file(LPCTSTR pszFileName, void * buffer, int size);
BOOL	save_to_file(LPCTSTR pszFileName, void * buffer, int size);
HANDLE	create_captfile(LPCTSTR);
int	open_captfile(LPCTSTR);
void	close_captfile(void );
void	write_captfile(unsigned char );
void	read_captfile(int);
BOOL	save_configfile(LPCTSTR);
BOOL	load_configfile(LPCTSTR);
BOOL	save_settings(void);
BOOL	load_settings(void);
void	save_property(HANDLE , char * ,int , void * );
int		load_next_config_buffer(HANDLE);
int		load_property(char * ,int , void * ); 
void	store_links(HANDLE,BASE_CL *);
void	load_object_basics(BASE_CL *);
void	save_object_basics(HANDLE , BASE_CL * );



//     Dialog - functions

int	  get_scrollpos(WPARAM, LPARAM);
void  display_toolbox(HWND);
void  update_toolbox_position(HWND);
void  close_toolbox( void );
void  update_statusinfo( void );
void  reset_oscilloscopes(void);
void  add_to_listbox(HWND , int , char * );
void  color_button(HWND, COLORREF );
COLORREF  select_color(HWND, COLORREF);
int check_keys(void);



//     Math - functions

float size_value(float min,float max, float x, float to_min, float to_max, int clip);

//     String - functions (general)
 
void low_chars(char *, char *);
void up_chars(char *, char *);
void reduce_filepath(char * , char *);
void copy_string(char * , int , int , char * );
int  get_int(char * str, int pos, int * target);
int  get_float(char * , const char *, float *);
int  get_string(char * , const char *, char *);
void print_time(char * ,float, int);
float get_time(char *);

//     (edf) - functions

HANDLE open_edf_file(EDFHEADERStruct * , CHANNELStruct * ,  char * );
HANDLE create_edf_file(EDFHEADERStruct * , CHANNELStruct * , char * );
void edfheader_to_physical(EDFHEADERStruct * from, EDFHEADER_PHYSICALStruct * to);
void edfchannels_to_physical(CHANNELStruct * fromchn,char * to,int channels);
void generate_edf_header(char * to, EDFHEADERStruct * header,CHANNELStruct * channels);
void parse_edf_header(EDFHEADERStruct *, CHANNELStruct *,char *);
void update_header(HWND hDlg, EDFHEADERStruct * header);
void get_header(HWND hDlg, EDFHEADERStruct * header);
void update_channelcombo(HWND hDlg, CHANNELStruct * channel, int channels);
void update_channel(HWND hDlg, CHANNELStruct * channel, int actchn);
void get_channel(HWND hDlg, CHANNELStruct * channel, int actchn);
void reset_channel(CHANNELStruct * channel);
void reset_header (EDFHEADERStruct * header);



//    OpenGl- and Drawing functions

void	Size_GL(HWND, HGLRC, int);
void	Shutdown_GL(HGLRC);
int		LoadGLTextures(void);
void	draw_objects (HWND);
int		create_AnimationWindow(void);
SDL_Surface *	LoadBMP(char *);


//   Midi functions

int  midi_open_port(HMIDIOUT * , int);
void  midi_Message(HMIDIOUT * , int);
void midi_Instrument(HMIDIOUT *, int, int);
void midi_NoteOn(HMIDIOUT *, int , int, int);
void midi_NoteOff(HMIDIOUT *, int, int);
void midi_ControlChange(HMIDIOUT * , int , int , int );
void init_midi(void);
int  get_opened_midiport(int);
int  get_listed_midiport(int);
void mute_all_midi(void);
void midi_Pitch(HMIDIOUT * midiout, int chn, int wheel);
void midi_PitchRange(HMIDIOUT * midiout, int chn, int pitchrange);
void midi_Vol(HMIDIOUT * midiout, int chn, int vol);

//  Window callback handler 

LRESULT CALLBACK MainWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OsciWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK MeterWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK SessionManagerWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK ButtonWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK CounterWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK ShadowWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK BallgameWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK MartiniWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK SpellerWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK DesignWndHandler(HWND hWnd, UINT , WPARAM , LPARAM );
LRESULT CALLBACK StatusDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SETTINGSDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK CONNECTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK OUTPORTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK INPORTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK TAGDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );



//  Window callback handler for opengl-windows
LRESULT CALLBACK AVIWndHandler(HWND, UINT , WPARAM , LPARAM);
LRESULT CALLBACK FFTWndHandler(HWND, UINT , WPARAM , LPARAM);
LRESULT CALLBACK AnimationWndHandler(HWND, UINT , WPARAM , LPARAM);


// Report and Error functions
void report(char * Message);
void report_error( char * Message );
void critical_error( char * Message );

//for array_data_ports
void set_inports(BASE_CL *st, int num);
void set_outports(BASE_CL *st, int num);
void delete_connection(LINKStruct *myactconnect);

#endif

