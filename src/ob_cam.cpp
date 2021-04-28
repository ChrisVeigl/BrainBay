/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_CAM.CPP:  contains functions for Webcam functions
  Author: Chris Veigl

  The Camera-Object can connect to an installed webcam an display the live-
  video in a window. A Face-Detection is performed and the Position of the Nose
  and the Chin are presented at the objects-output ports.
  
  This Module uses the Intel OpenCV Library for Computer Vision, and source
  code from the sample-projects describingf the use of the HaarClassifier-Cascade
  and the Lukas Kanade Optical Flow Algorithm.
  see details about Intels OpenCV: http://opencv.org/  


-----------------------------------------------------------------------------*/
// #define DEBUG_OUTPUT


#include "brainbay.h"
#include "ob_cam.h"
#include <stdio.h>
#include <ctype.h>

#if _MSC_VER < 1900
#include <videoinput\include\videoInput.h>  
#include <opencv2\opencv.hpp>
#define cimg_plugin1 "CImg\plugins\cimgIPL.h"
#include "CImg\CImg.h"

	#define OPENCV_LIB_VER "242"
	#if defined(_DEBUG)
	#define OPENCV_LIB_SUFFIX "d.lib"
	#else
	#define OPENCV_LIB_SUFFIX ".lib"
	#endif

	#pragma comment(lib, "opencv_core" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "opencv_highgui" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "opencv_objdetect" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "opencv_imgproc" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "opencv_video" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "opencv_features2d" OPENCV_LIB_VER OPENCV_LIB_SUFFIX)
	#pragma comment(lib, "zlib")
	#pragma comment(lib, "libjasper")
	#pragma comment(lib, "libpng")
	#pragma comment(lib, "libtiff")
	#pragma comment(lib, "libjpeg") 
	#pragma comment(lib, "comctl32")
	#pragma comment(lib, "Vfw32")

	static CvVideoWriter* video_writer = 0;
	videoInput* VI;

#endif


CAMOBJ * camobj = 0;

int framerate=0;
int MAX_COUNT = 2;
int c_detect=0;
int cam_present=0;
int update_rate=65;
int cur_rate=0;
float save_dist,dist_error,dist_threshold;
float save_angle,angle_error,angle_threshold;
int autorestore;
int time_to_restore=0;
float PT1_xpos,	PT1_ypos, PT2_xpos=0.5f, PT2_ypos=0.6f;
int init_flag=0;
int count = 0, save_count=0,mcount, ccount;
HANDLE mutex;

#define MODE_VIDEOFILE_IDLE 0
#define MODE_VIDEOFILE_READING 1
#define MODE_VIDEOFILE_WRITING 2

#define STATUS_IDLE 0
#define STATUS_COULD_NOT_INIT 1
#define STATUS_THREAD_RUNNING 2

#define CAM_CONNECT_TIMEOUT 5000
#define ERR_THRESHOLD 500




//////////////  C - facetracking functions  /////////////////////////////////////////

int facetracker_work(void );
int facetracker_init(void );
int facetracker_exit(void );
int detect_face (void);
int check_jitter(int mode);

int win_size = 11;
int paintcnt = 0;
int paintperiod = 5;
HWND drawing_window = 0;

static float x_move = 0, y_move = 0, x_click = 0, y_click = 0;
static float x_moved = 0, y_moved = 0, x_clicked = 0, y_clicked = 0;


char* status = 0;
int need_to_init = 1;
int flags = 0;
int initstatus = STATUS_IDLE;

#if _MSC_VER < 1900

//////////////  C - global variables for facetracking, used from thread  ////////////

#define LK_NUM_POINTS 2
const char* cascade_name = "ComputerVision\\haarcascade_frontalface_alt.xml";


static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
CvCapture* capture = 0;
IplImage* image = 0, * grey = 0, * prev_grey = 0,
* pyramid = 0, * prev_pyramid = 0, * swap_temp = 0,
* frame = 0, * frame_copy = 0;
CvPoint2D32f* points[2] = { 0,0 }, * swap_points = 0;



//VidFormat vidFmt = {320, 240, 30.0 };

unsigned char* frame_buffer =0 ;


DWORD dwCamStatId;
HANDLE CAMTHREAD=0;
HANDLE CamExitEvent=0;

MSG msg;

int cameraSelection = 0;
int xResolution=320;
int yResolution=240;
size_t viWidth;
size_t viHeight;
size_t viSize;
#define FIRST_IMAGE_TIMEOUT 400   // * 10 ms   = 4 sec
#define ERR_THRESHOLD 500
#define GAIN 2


namespace ci=cimg_library;

ci::CImg<unsigned char> *cimg=0;
//window
ci::CImgDisplay* main_disp=0; //


void on_mouse( int event, int x, int y, int flags, void* param )
{
	
}



int allocate_resources(void)
{

	#ifdef DEBUG_OUTPUT
		 write_logfile("FaceTrackerLK C++ module: now connecting to cam\n" );
	#endif

//----------------------------------------------
	if (!VI) VI= new(videoInput); 
	int numDev=VI->listDevices(false);
	write_logfile("we want to set up %d !\n",cameraSelection);

   //	for (int i=0;i<numDev;i++)
   //	{
   //		write_logfile("Device %d: name=%s height=%d\n",i,VI->getDeviceName(i),VI->getHeight(i));
   //	}
	

	if((numDev<=cameraSelection) || (!strcmp(VI->getDeviceName(cameraSelection),"VDP Source")) || (!strcmp(VI->getDeviceName(cameraSelection),"vfwwdm32.dll")))
	{
		write_logfile("FaceTrackerLK C++ module: ERROR: desired Webcam not available\n");
		return(0);  
	}

	//
	write_logfile("\nFaceTrackerLK C++ module: Opening: %s with desired resolution %dx%d\n", VI->getDeviceName(cameraSelection),xResolution,yResolution);

	VI->setVerbose(false);
	VI->setUseCallback(true);

	VI->setupDevice(cameraSelection, xResolution, yResolution);

	if(!VI->isDeviceSetup(cameraSelection))
	{
		write_logfile("FaceTrackerLK C++ module: ERROR: Could not connect to WebCamera\n");
		return(0);  
	}

	viWidth 	= VI->getWidth(cameraSelection);
	viHeight 	= VI->getHeight(cameraSelection);
	viSize	= VI->getSize(cameraSelection);

	#ifdef DEBUG_OUTPUT
			write_logfile("FaceTrackerLK C++ module: loading classifier cascade\n" );
	#endif

	char c_name[256];
	strcpy(c_name,GLOBAL.resourcepath);
	strcat(c_name,cascade_name);


	cascade = (CvHaarClassifierCascade*)cvLoad( c_name, 0, 0, 0 );
	if( !cascade )  
	{		
		write_logfile("FaceTrackerLK C++ module: ERROR: Could not load classifier cascade %s\n",c_name ); 
		return(0); 
	}

	storage = cvCreateMemStorage(0);

	#ifdef DEBUG_OUTPUT
			write_logfile("FaceTrackerLK C++ module: query first frame\n" );
	#endif
	//----------------------------------------------
	//Wait for the camera to be ready
	#ifdef DEBUG_OUTPUT
		write_logfile("FaceTrackerLK C++ module: Waiting for the camera to be ready ....");
	#endif

    int inittimeout=0;
	while (!VI->isFrameNew(cameraSelection) && (inittimeout++<FIRST_IMAGE_TIMEOUT))
	{
		Sleep(10);
	}	

	if(inittimeout==FIRST_IMAGE_TIMEOUT)
	{		
		write_logfile("FaceTrackerLK C++ module: ERROR: Could not acquire image\n" ); 
		return(0); 
	}
			
	#ifdef DEBUG_OUTPUT
		printf ("FaceTrackerLK C++ module: Camera is ready.\n");
	#endif
	//ALLOCATE here the IplImage
	frame=cvCreateImage(cvSize(viWidth, viHeight), IPL_DEPTH_8U, 3);
	//get bytes
	VI->getPixels(cameraSelection, (unsigned char*) frame->imageData, false, true );
	cvFlip(frame, NULL, 1);
	//----------------------------------------------
  	cvWaitKey(1);

	//frame_copy=cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 3 ); 
	//image = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 3 );
		
	grey = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );

	prev_grey = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );
	pyramid = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 ); 
	prev_pyramid = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );

	points[0] = (CvPoint2D32f*)cvAlloc(LK_NUM_POINTS*sizeof(points[0][0]));
	points[1] = (CvPoint2D32f*)cvAlloc(LK_NUM_POINTS*sizeof(points[0][0]));

	points[1][1].x=0;
	points[1][1].y=0;

	status = (char*)cvAlloc(LK_NUM_POINTS);
	mutex= CreateMutex( NULL, FALSE, NULL ); 

	//3 channel CImg
	cimg=new ci::CImg<unsigned char>(static_cast<unsigned int>(viWidth),static_cast<unsigned int>(viHeight),1,3);
	return(1);
}

void free_resources(void )
{
	#ifdef DEBUG_OUTPUT
      write_logfile("FaceTrackerLK C++ module: releasing thread resources\n");
	#endif

	if (VI) 
	{
		VI->stopDevice(cameraSelection);
		delete VI; 
		VI=0;
	} 

	if (frame)   { cvReleaseImage( &frame ); frame=0;}			

	if (cascade) { cvFree((void**)&cascade); cascade =0;}
	if (storage) { cvReleaseMemStorage(&storage); storage=0; }
	if (grey)    { cvReleaseImage( &grey ); grey=0;}
	if (prev_grey) {cvReleaseImage( &prev_grey ); prev_grey=0;}
	if (pyramid) { cvReleaseImage( &pyramid ); pyramid=0;}
	if (prev_pyramid) {cvReleaseImage( &prev_pyramid ); prev_pyramid=0;}
	if (points[0]) {cvFree((void**) &points[0] ); points[0]=0;}
	if (points[1]) {cvFree((void**) &points[1] ); points[1]=0;}
	if (status) {cvFree((void**) &status ); status=0;}
	if (cimg) {delete cimg; cimg=0;};
	CloseHandle(mutex);

}

void create_paintwindow()
{
	main_disp= new ci::CImgDisplay(*cimg,"CImg");
	main_disp->move(main_disp->screen_width()-viWidth - 20, 30);	//main_disp->screen_height()

	#ifdef DEBUG_OUTPUT
		write_logfile("FaceTrackerLK C++ module: Drawing window ID: ");
	#endif
}

void resize_paintwindow(int x,int y,int w,int h)
{
	if (main_disp)
	{
		main_disp->move(x,y);
		main_disp->resize(w,h,1); 
	}
	#ifdef DEBUG_OUTPUT
		write_logfile("FaceTrackerLK C++ module: Resizing window ID: ");
	#endif
}

void destroy_paintwindow()
{
	if (main_disp) {delete main_disp; main_disp=0;}
}




/////////////////  Camera thread: polls picture and calls featuretracking ////////////
DWORD WINAPI CamProc(LPVOID lpv)
{
    HANDLE     hArray[1];
	DWORD dwRes;
	BOOL CamThreadDone=FALSE;
	hArray[0] = CamExitEvent;

	static int cnt =0;

	    if (!allocate_resources())
		{
		   write_logfile("C++: ERROR: Could not allocate resources\n" );
		   free_resources();
   	   	   initstatus=STATUS_COULD_NOT_INIT;
		   return (0);
		}

	    if (paintperiod > 0) 
		{
			#ifdef DEBUG_OUTPUT
			  write_logfile( "creating paint window ...\n");
			#endif
			create_paintwindow();
			resize_paintwindow(0,0,320,240);

		}

		#ifdef DEBUG_OUTPUT
		  write_logfile( "camthread is ready, cam connected ...\n");
		#endif

		need_to_init=1;
		flags = 0;
		initstatus=STATUS_THREAD_RUNNING;

		while (!CamThreadDone)  {

			#ifdef DEBUG_OUTPUT
				  write_logfile( "calling factracking worker ...\n");
			#endif

			facetracker_work();   // perform feature tracking

			#ifdef DEBUG_OUTPUT
				write_logfile("C++ right before callback %d,%d\n",  (int)x_move , (int)y_move);
			#endif

			if ( WaitForSingleObject( mutex, INFINITE ) == WAIT_OBJECT_0 )
			{
				if ((x_move<ERR_THRESHOLD)&&(x_move>-ERR_THRESHOLD)&&(y_move<ERR_THRESHOLD)&&(y_move>-ERR_THRESHOLD))
				{
					x_moved=-x_move;
					y_moved=-y_move;
					x_clicked=-x_click;
					y_clicked=-y_click;
				}
				else {
				#ifdef DEBUG_OUTPUT
					write_logfile("dropped out-of-bounds-movement\n");
				#endif
				}
				ReleaseMutex( mutex );
	        } 


			dwRes = WaitForMultipleObjects(1, hArray, FALSE, 0);
			switch(dwRes)  {
				case WAIT_OBJECT_0: 
					 CamThreadDone = TRUE;
					 #ifdef DEBUG_OUTPUT
					 write_logfile("camthread exit event received\n");
		 			 #endif
					 break;
				case WAIT_TIMEOUT:
					 #ifdef DEBUG_OUTPUT
					 write_logfile("camthread timed out\n");
		 			 #endif
					 break;                       
				default: break;
		}
	}

	free_resources();
	destroy_paintwindow();
 	return(1);
}


int facetracker_init( CAMOBJ * st )
{
	  int inittimeout=0;

	  initstatus=STATUS_COULD_NOT_INIT;
	  #ifdef DEBUG_OUTPUT
		  write_logfile("setting up camthread\n" );
	  #endif

  	  camobj=st;
  	  CamExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      if (CamExitEvent == NULL)	  { write_logfile("CreateEvent failed (CamThread exit event)\n"); return(0); }

	  CAMTHREAD =   CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) CamProc, (LPVOID) NULL, 0, &dwCamStatId);
	  if (CAMTHREAD == NULL) { write_logfile("CreateThread failed\n"); return(0); }
	
	  while ((initstatus==STATUS_COULD_NOT_INIT) && (inittimeout++<CAM_CONNECT_TIMEOUT))
	  {
		  cvWaitKey(1);
		  Sleep(1);
	  }
	  if (initstatus==STATUS_THREAD_RUNNING) return (1);
	  return(0);
}


int facetracker_exit(void)
{
    HANDLE hThreads[1];
    DWORD  dwRes;

	if (CAMTHREAD)	{

      hThreads[0] = CAMTHREAD;
      SetEvent(CamExitEvent);
      dwRes = WaitForMultipleObjects(1, hThreads, FALSE, 10000);

      switch(dwRes)       {

		case WAIT_OBJECT_0:
			#ifdef DEBUG_OUTPUT
				write_logfile("Thread returned.\n");
			#endif
			break;
		case WAIT_TIMEOUT:
			#ifdef DEBUG_OUTPUT
				write_logfile("Thread timed out.\n");
			#endif
    	     break;
	    default:
             write_logfile("C++: Camthread - unknown exit error\n");
             break;
      }

	  // reset thread exit event here
      ResetEvent(CamExitEvent);
      // write_logfile("closing down camthread.\n");
  	  CloseHandle(CamExitEvent);
	  CloseHandle(CAMTHREAD);
	  CAMTHREAD=0;
	  camobj=0;
	}

	#ifdef DEBUG_OUTPUT
	   write_logfile("Camera Module quit.\n");
	#endif
	return(1);
}

long distance_to_act_face(int nx,int ny)
{
	double dx,dy;

	dx=points[1][1].x-(double)nx;
	dy=points[1][1].y-(double)ny;

	return((long)sqrt(dx*dx+dy*dy));
}


int detect_face (void)
{
	CvSeq* faces;
    CvPoint pt1, pt2, nose, chin, best_nose, best_chin;
	long act_distance,best_distance=100000;
	int i;

	best_nose.x=0;best_nose.y=0;
	best_chin.x=0;best_chin.y=0;
    //cvPyrDown( frame, frame_copy, CV_GAUSSIAN_5x5 );
    //cvFlip( frame, frame_copy, 0 );

	   cvClearMemStorage( storage );

	  #ifdef DEBUG_OUTPUT
	     write_logfile("detecting face...\n");
       #endif		
	faces = cvHaarDetectObjects(grey, cascade, storage,
                            1.2, 2, CV_HAAR_DO_CANNY_PRUNING, cvSize(70, 70) );

	best_distance=1000000;

	for( i = 0; i < (faces ? faces->total : 0); i++ ) {
		#ifdef DEBUG_OUTPUT
 		    write_logfile("FaceTrackerLK C++ module:  face found!\n");
		#endif

		CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

		pt1.x = r->x;
		pt2.x = (r->x + r->width);

		pt1.y = (r->y);//frame->height-(r->y);
		pt2.y = (r->y + r->height);//frame->height - (r->y + r->height);

		chin.x=pt1.x+(int)((pt2.x-pt1.x)* 0.5f);
		chin.y=(pt1.y+(int)((pt2.y-pt1.y)* 0.95f));

		nose.x=chin.x;
		nose.y=(pt1.y+(int)((pt2.y-pt1.y)* 0.6f));

		if ((act_distance=distance_to_act_face(nose.x,nose.y)) < best_distance) 
		{ 
			best_distance=act_distance;
			best_nose.x=nose.x;
			best_nose.y=nose.y;
			best_chin.x=chin.x;
			best_chin.y=chin.y;
		}
	}

	if (i>0)
	{
		points[1][0].x=(float)best_chin.x;
		points[1][0].y=(float)best_chin.y;
		cvCircle( frame, best_chin, 4, CV_RGB(255,0,0), 2, 8,0);

		points[1][1].x=(float)best_nose.x;
		points[1][1].y=(float)best_nose.y;
		cvCircle( frame, best_nose, 4, CV_RGB(255,0,0), 2, 8,0);

		points[0][0].x=points[1][0].x;
		points[0][0].y=points[1][0].y;
		points[0][1].x=points[1][1].x;
		points[0][1].y=points[1][1].y;
		return (1);
	}
	return(0);
}

int check_jitter(int mode)
{
    static float orig_dist,orig_angle;
	float dist_error, angle_error;
	
	double c;
	double dx,dy;

	dx=points[1][1].x - points[1][0].x;
	dy=points[1][1].y - points[1][0].y;
    c=sqrt((double)(dx*dx+dy*dy));
	  
    if (c == 0.0) return (1);

	if (mode==1) {  // first calculation of distance and angle 
		  orig_dist=(float)c; dist_error=0.0f; 
		  orig_angle= (float) asin(dx/c) * 57.29577f; angle_error=0.0f;
		  return (0);
	}

	dist_error= (float) fabs(orig_dist-c);
	angle_error=  (float) fabs(orig_angle-asin(dx/c) * 57.29577);
	
	if ((dist_error>=60.0f) || (angle_error>=25.0f)) return (1);
	return(0);
}


int facetracker_work(void)
{
	double acttime;
	int t_display;
	static double old_displaytime=0.;

	acttime = (double)cvGetTickCount();

	#ifdef DEBUG_OUTPUT
		int t_thread;
		static double old_threadtime=0.;
		t_thread = (int)((acttime-old_threadtime)/((double)cvGetTickFrequency()*1000.));
		old_threadtime=acttime;
  		write_logfile("FacetrackerLK C++ module: Thread call (ms latency = %d) !\n", t_thread );
	#endif

	if(!VI->getPixels(cameraSelection, (unsigned char*) frame->imageData, false, true )) return 0;
	cvFlip(frame, NULL, 1);


	
	if (camobj->mode==MODE_VIDEOFILE_WRITING)
		{
		  if (!video_writer)
		       video_writer = cvCreateVideoWriter(camobj->videofilename,-1,15,cvGetSize(frame));
		  	
		  cvWriteFrame(video_writer,frame);
  		}


	cvCvtColor( frame, grey, CV_BGR2GRAY );

	if (need_to_init) {
		if (detect_face()) 	{
	  		need_to_init=0;

			cvFindCornerSubPix( grey, points[1], LK_NUM_POINTS,
			cvSize(win_size,win_size), cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER,1,1.0));
			//	cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
  
			cvCopy(grey,prev_grey,0 );
			cvCopy(pyramid,prev_pyramid,0 );

			points[0][0].x=points[1][0].x;
			points[0][0].y=points[1][0].y;
				
			check_jitter(1);
			flags = 0;				
		} 
		old_displaytime=0.;
	}        
	else  
	{
        cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
            points[0], points[1], LK_NUM_POINTS, cvSize(win_size,win_size), 5, status, 0,
            cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );

        flags |= CV_LKFLOW_PYR_A_READY;
         
		if ((!status[0] ) || (!status[1]))  
			need_to_init=1; 
		else 
		{
			//[0][x] is 'old', [1][x] is 'new'
			//[x][0] is Chin, [x][1] is Nose

			x_click = (points[1][0].x - points[0][0].x) * GAIN;//Chin
			y_click = (points[1][0].y - points[0][0].y) * GAIN;//Chin
			x_move = (points[1][1].x - points[0][1].x)  * GAIN;//Nose
			y_move = (points[1][1].y - points[0][1].y)  * GAIN;//Nose 
			//Yellow Point CHIN
			cvCircle( frame, cvPointFrom32f(points[1][0]), 4, CV_RGB(255,255,0), 2, 8,0);
			//Green Point NOSE
			cvCircle( frame, cvPointFrom32f(points[1][1]), 4, CV_RGB(0,210,0), 2, 8,0);
    			
			if (check_jitter(0))  need_to_init=1;
		}
	}

  	CV_SWAP( prev_grey, grey, swap_temp );
	CV_SWAP( prev_pyramid, pyramid, swap_temp );
	CV_SWAP( points[0], points[1], swap_points );	

	if (need_to_init)
	{
			x_move=0;
			y_move=0;
			x_click=0;
			y_click=0;
	}

	if (main_disp && (paintperiod>0))
	{
		t_display = (int)((acttime-old_displaytime)/((double)cvGetTickFrequency()*1000.));

		if (t_display>paintperiod)
		{
			old_displaytime=acttime;

			cimg->assign(frame); //Mat -> IplImage -> CImg

			if (!main_disp->is_closed() && main_disp->is_resized())
					main_disp->resize().display(*cimg);
			else
				main_disp->display(*cimg);
			//Get events....
			main_disp->wait(1);
		}
	}
	return(1);
}


#else

int facetracker_init(CAMOBJ* st) { MessageBox(NULL, "The Camera/OpenCV Functions are only supported if BrainBay is compiled with VisualStudio 2010 (Platform Toolset V100).\n Please recompile Brainbay or use another binary version ...", "Error", MB_OK);  return (0); }
int facetracker_exit(void) { return (0); }

#endif


LRESULT CALLBACK CamDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	CAMOBJ * st;
	
	st = (CAMOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_CAM)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemInt(hDlg,IDC_CUR_RATE,0,0);
				SetDlgItemText(hDlg,IDC_CAMSTATUS,"Ready.");
				SetDlgItemInt(hDlg,IDC_ERROR_DIST,(int)dist_threshold,0);
				SetDlgItemInt(hDlg,IDC_ERROR_ANGLE,(int)angle_threshold,0);
				SetDlgItemInt(hDlg,IDC_THRESHOLD_TIME,paintperiod,0);
				SetDlgItemInt(hDlg,IDC_PT1X,(int)(PT1_xpos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT1Y,(int)(PT1_ypos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT2X,(int)(PT2_xpos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT2Y,(int)(PT2_ypos*100.0f),1);
				
				CheckDlgButton(hDlg,IDC_AUTORESTORE,autorestore);
				CheckDlgButton(hDlg,IDC_SHOWLIVE,st->showlive);
				CheckDlgButton(hDlg,IDC_ENABLE_TRACKING,st->enable_tracking);
				CheckDlgButton(hDlg,IDC_TRACKFACE,st->trackface);

				if (st->mode==MODE_VIDEOFILE_IDLE) CheckDlgButton(hDlg,IDC_NOARCHIVE,TRUE); else
					if (st->mode==MODE_VIDEOFILE_WRITING) CheckDlgButton(hDlg,IDC_RECORDARCHIVE,TRUE); else
						if (st->mode==MODE_VIDEOFILE_READING) CheckDlgButton(hDlg,IDC_PLAYARCHIVE,TRUE);
				SetDlgItemText(hDlg,IDC_ARCHIVEFILE,st->videofilename);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_INITCAM:
					facetracker_exit();
					facetracker_init(st);
					break;
				case IDC_EXITCAM:
					facetracker_exit();
                    break;
				case IDC_RESET:
		            count = 0;
					break;

				case IDC_NOARCHIVE:
					  facetracker_exit();
					  st->mode=MODE_VIDEOFILE_IDLE;
					  facetracker_init(st);
					break;
				case IDC_RECORDARCHIVE:
					  facetracker_exit();
					  st->mode=MODE_VIDEOFILE_WRITING;

					  if (!strcmp(st->videofilename,"none"))
					  {
						 strcpy(st->videofilename,GLOBAL.resourcepath); 
						 strcat(st->videofilename,"MOVIES\\*.avi");
					  }

					  if (!open_file_dlg(hDlg,st->videofilename, FT_AVI, OPEN_SAVE))
					     strcpy(st->videofilename,"none");
					  SetDlgItemText(hDlg, IDC_ARCHIVEFILE,st->videofilename);
					  facetracker_init(st);

					break;
				case IDC_PLAYARCHIVE:
					  facetracker_exit();
					  st->mode=MODE_VIDEOFILE_READING;
					  if (!strcmp(st->videofilename,"none"))
					  {
						 strcpy(st->videofilename,GLOBAL.resourcepath); 
						 strcat(st->videofilename,"MOVIES\\*.avi");
					  }
					  if (!open_file_dlg(hDlg,st->videofilename, FT_AVI, OPEN_LOAD))
					     strcpy(st->videofilename,"none");
					  SetDlgItemText(hDlg, IDC_ARCHIVEFILE,st->videofilename);
					  facetracker_init(st);

					break;

				case IDC_THRESHOLD_TIME:
					paintperiod=GetDlgItemInt(hDlg, IDC_THRESHOLD_TIME,0,0);
                    break;
            }
			return TRUE;
			break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}


CAMOBJ::CAMOBJ(int num) : BASE_CL()
{
	outports = 4;
	inports = 0;
	width=65;
	strcpy(out_ports[0].out_name,"Point1-X");
	strcpy(out_ports[1].out_name,"Point1-Y");
	strcpy(out_ports[2].out_name,"Point2-X");
	strcpy(out_ports[3].out_name,"Point2-Y");
	
	strcpy(videofilename,"none"); 
	interval=10;dist_threshold=80.0f;angle_threshold=20.0f;autorestore=TRUE;
	PT1_xpos=0.5f; PT1_ypos=0.99f;
	PT2_xpos=0.5f; PT2_ypos=0.6f;

	mode=MODE_VIDEOFILE_IDLE; enable_tracking=1; showlive=1; trackface=1;

	if (!facetracker_init(this)) GLOBAL.run_exception=1;
}
	
void CAMOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_CAMBOX, ghWndStatusbox, (DLGPROC)CamDlgHandler));
}

void CAMOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("paintperiod",P_INT,&paintperiod);
   load_property("mode",P_INT,&mode);
   load_property("archive",P_STRING,videofilename);
   if (trackface) MAX_COUNT=2; else MAX_COUNT=1;

}

void CAMOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"paintperiod",P_INT,&paintperiod);
	save_property(hFile,"mode",P_INT,&mode);
	save_property(hFile,"archive",P_STRING,videofilename);

}
	
void CAMOBJ::incoming_data(int port, float value)
{
}
	
void CAMOBJ::work(void)
{

	if ((hDlg==ghWndToolbox) && (!TIMING.dialog_update))
	{ 
		if (mode==MODE_VIDEOFILE_READING)  SetDlgItemText(hDlg,IDC_CAMSTATUS,"Reading from Videofile");
		else if (mode==MODE_VIDEOFILE_WRITING)  SetDlgItemText(hDlg,IDC_CAMSTATUS,"Writing to Videofile");
#if _MSC_VER < 1900
		else if (capture)  SetDlgItemText(hDlg,IDC_CAMSTATUS,"Displaying live images from Camera");
#endif
		else SetDlgItemText(hDlg,IDC_CAMSTATUS,"Waiting for Video Source");
	    SetDlgItemInt(hDlg,IDC_CUR_RATE,framerate,0);
	}

	if (trackface)
	{
		if ((!need_to_init)) //&&(init_flag>0))
		{
			  if ( WaitForSingleObject( mutex, INFINITE ) == WAIT_OBJECT_0 )
			  {
				pass_values(0, x_moved);
				pass_values(1, y_moved);
				pass_values(2, x_clicked);
				pass_values(3, y_clicked);
	            // Mutex wieder freigeben
	            ReleaseMutex( mutex );
			  }
        }
	}
	else
	{
			pass_values(0, x_move);
			pass_values(1, y_move);
	}

}

CAMOBJ::~CAMOBJ() { facetracker_exit(); }

