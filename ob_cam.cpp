/* -----------------------------------------------------------------------------

  BrainBay  Version 1.7, GPL 2003-2010, contact: chris@shifz.org
  
  MODULE: OB_CAM.CPP:  contains functions for Webcam functions
  Author: Chris Veigl

  The Camera-Object can connect to an installed webcam an display the live-
  video in a window. A Face-Detection is performed and the Position of the Nose and 
  the Chin are presented at the objects-output ports.
  
  This Module uses the Intel OpenCV Library for Computer Vision, and source
  code from the sample-projects describingf the use of the HaarClassifier-Cascade
  and the Lukas Kanade Optical Flow Algorithm.
  see details about Intels OpenCV: http://www.intel.com/research/mrl/research/opencv
  

  
-----------------------------------------------------------------------------*/

#include "brainbay.h"
#include "ob_cam.h"
#include "cv.h"
#include "cvcam.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#include "videoInput.h"  

videoInput VI; 
static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
static CvVideoWriter * video_writer =0;
static IplImage* frame = 0, *frame_copy = 0;
unsigned char* frame_buffer =0 ;
CvPoint pt1, pt2, ptd;

CAMOBJ * camobj;

int c_detect=0;
int cam_present=0;
int update_rate=65;
int cur_rate=0;

//VidFormat vidFmt = {320, 240, 30.0 };
const char* cascade_name =
    "haarcascade_frontalface_alt.xml";


static IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0,
         *save_grey = 0, *save_pyramid=0, *swap_temp;
static CvCapture* capture = 0;

int win_size = 11;
int MAX_COUNT = 2;

static CvPoint2D32f* points[2] = {0,0}, *save_points=0, *swap_points;
int pt_mode[2];
int save_pt_mode[2];

float save_dist,dist_error,dist_threshold;
float save_angle,angle_error,angle_threshold;
int threshold_time;

float PT1_xpos,	PT1_ypos, PT2_xpos=0.5f, PT2_ypos=0.6f;
char* status = 0;
int count = 0, save_count=0,mcount, ccount;
int need_to_init = 0;
int night_mode = 0;
int flags = 0, saved=0;
int statuscount=0;
float x_move=0,y_move=0,x_click=0,y_click=0;
int add_remove_pt = 0;
int autorestore;
int time_to_restore=0;
int init_flag=0;
CvPoint pt;

DWORD dwCamStatId;
HANDLE CAMTHREAD=0;
HANDLE CamExitEvent=0;
long a_t,l_t;

int lk_work(CAMOBJ * );
void on_mouse( int event, int x, int y, int flags, void* param );


DWORD WINAPI CamProc(LPVOID lpv)
{
	l_t=SDL_GetTicks();
    HANDLE hArray[1];
	DWORD dwRes;
	BOOL CamThreadDone=FALSE;
	MSG msg;
	int numDevices;
	hArray[0] = CamExitEvent;
	char fname[256];

 		
	  if (camobj->mode == 2)  // capture from file
	  { 
		  if ((strstr(camobj->videofilename,".avi")) && (!capture)) 
			  capture = cvCaptureFromAVI( camobj->videofilename );
		  if (!capture)  {
			  cam_present=-1;
			  write_logfile("ERROR: Could not open capture from file");
		  }
	  }
	  else  {  // capture from camera

		  if (GLOBAL.use_cv_capture)  {  // using opencv/cvcam  for capture
			  capture = cvCaptureFromCAM( 0 );
			  if (!capture) { 
				  cam_present=-1;
			  	  write_logfile("ERROR: Could not open cvcapture from camera");
				  return(0);
			  }
    		  frame = cvQueryFrame(capture);
		  } 

		  else  {   // using videoinput library for capture
		    numDevices = VI.listDevices();
	   	    write_logfile("%d video input devices found",numDevices);

		    if ((numDevices==0) || (!VI.setupDevice(0,320, 240, VI_COMPOSITE)))		    {
			  if (!GLOBAL.loading) close_toolbox();
 		   	  write_logfile("ERROR: Could not setup video input device");
			  cam_present=-1;
			  return(0);
		    }

	   	    write_logfile("opened video device: %s", VI.getDeviceName(0));
  		    VI.setUseCallback(false);
			frame_buffer = new unsigned char[VI.getSize(0)];
			frame = cvCreateImage( cvSize(VI.getWidth(0),VI.getHeight(0)),IPL_DEPTH_8U, 3 );
		  }
	  }

	  strcpy(fname,GLOBAL.resourcepath);
	  strcat(fname,cascade_name);
	  
	  cascade = (CvHaarClassifierCascade*)cvLoad( fname, 0, 0, 0 );
	  if( !cascade )  
	  {
		   	  SetDlgItemText(ghWndStatusbox, IDC_STATUS,"ERROR: Could not load face detection classifier cascade");
		   	  write_logfile("ERROR: Could not load face detection classifier cascade %s",fname);
			  cam_present=-2; return(0); 
	  } 
	  write_logfile("face detection classifier cascade loaded.");    
      storage = cvCreateMemStorage(0);

	  image = cvCreateImage( cvGetSize(frame), 8, 3 );
	  image->origin = frame->origin;

	  grey = cvCreateImage( cvGetSize(frame), 8, 1 );
	  prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
	  save_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
	  pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
	  prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
	  save_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );

	  points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
	  points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
	  save_points = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));

	  status = (char*)cvAlloc(MAX_COUNT);
	  for (int i=0;i<MAX_COUNT;i++) pt_mode[i]=0;
	  flags = 0;

   	  if (camobj->showlive)  {
		cvNamedWindow( "Camera", 1 );
	    cvMoveWindow("Camera",0,0);
//        cvSetMouseCallback( "Camera", on_mouse, 0 );
	  }


	  while (!CamThreadDone)      //   capture loop 
	  {
		  if(!GetMessage(&msg, NULL, 0, 0)) break;
		  TranslateMessage(&msg);	
		  DispatchMessage(&msg);	

		  if(camobj->mode ==2)	{  // capture from file
			frame = cvQueryFrame(capture);
			//	cvShowImage( "Camera", frame );			
		  }
		  else {   // capture from camera
		   
			//if (VI.isFrameNew(0))
			if (GLOBAL.use_cv_capture)	{      
				frame = cvQueryFrame(capture);
			}
			else {   // use videoinput library 
			  VI.getPixels(0, frame_buffer, false, true); 
			  frame->imageData=(char *)frame_buffer;
		    }
		  }

		  lk_work(camobj);   // perform face detection / lukas kanade algorithm
		  a_t=SDL_GetTicks();
		  cur_rate=a_t-l_t;
		  l_t=a_t;
		// Sleep(1);
			
		  dwRes = WaitForMultipleObjects(1, hArray, FALSE, 0);  // check for thread end request
          switch(dwRes) {
                case WAIT_OBJECT_0: 
                    CamThreadDone = TRUE;
		        case WAIT_TIMEOUT:   // timeouts are not reported
                    break;                       
                default:
                   // report_error("WaitForMultipleObjects(CamExitEvent) does not return");
                    break;
		  }
	  }

      write_logfile("cam thread closed, releasing resources");

  	  if (!GLOBAL.use_cv_capture) 
			VI.stopDevice(0);

	  if (frame) { 
//				cvReleaseImage( &frame ); 
				frame=0;   }
	  if (frame_buffer) 
			{ delete (frame_buffer); frame_buffer=0;}
	  if (image) 
			{ cvReleaseImage( &image ); image=0; }
	  if (grey) 
			{ cvReleaseImage( &grey ); grey=0; }
	  if (save_grey) 
			{ cvReleaseImage( &save_grey ); save_grey=0; }
	  if (prev_grey) 
			{ cvReleaseImage( &prev_grey ); prev_grey=0;}
	  if (pyramid) 
			{ cvReleaseImage( &pyramid ); pyramid=0; }
	  if (prev_pyramid) 
			{ cvReleaseImage( &prev_pyramid ); prev_pyramid=0; }
	  if (save_pyramid) 
			{ cvReleaseImage( &save_pyramid ); save_pyramid=0; }
	  if (capture) 
			{ cvReleaseCapture( &capture ); capture=0; }
	  if (storage) 
			{ cvReleaseMemStorage(&storage); storage=0; }
	  if (video_writer) 
			{ cvReleaseVideoWriter(&video_writer); video_writer=0; }
	  if (cascade) 
			{ cvFree((void**)&cascade); cascade=0; }

	  write_logfile("closing camera window");
			cvDestroyWindow("Camera");

	  return(1);
}

void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( event == CV_EVENT_LBUTTONDOWN )
    {
        pt = cvPoint(x,y);
        add_remove_pt = 1;
    }
}

int lk_init( CAMOBJ * st )
{
	  camobj=st;
	  cam_present=1;
      CamExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      if (CamExitEvent == NULL)
        report_error("CreateEvent failed (CamThread exit event)"); 
	  CAMTHREAD =   CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) CamProc, (LPVOID) ghWndMain, 0, &dwCamStatId);
      if (CAMTHREAD == NULL) report_error("CreateThread failed");
	  Sleep(100);
	  if (cam_present<0) { report_error("Camera initialisation problem"); }
	  return (1);
}


int lk_exit(void)
{
    HANDLE hThreads[1];
    DWORD  dwRes;


	if (CAMTHREAD)
	{

		hThreads[0] = CAMTHREAD;

		SetEvent(CamExitEvent);
		dwRes = WaitForMultipleObjects(1, hThreads, FALSE, 2000);
		switch(dwRes)
		{

			case WAIT_OBJECT_0:
			case WAIT_TIMEOUT:
				SetDlgItemText(ghWndStatusbox,IDC_STATUS,"Camera closed");
				//report("Camera Interface closed");
    		break;
	
			default:
			   report_error("CamThread: unknown exit error");
				break;
		}
		//
		// reset thread exit event here
		//
		ResetEvent(CamExitEvent);
		CloseHandle(CamExitEvent);
		CloseHandle(CAMTHREAD);
		Sleep(100);
		CAMTHREAD=0;

	}

	return(0);
}



int detect_face (void)
{
    // int scale = 1;
	CvSeq* faces;
    //IplImage* temp = cvCreateImage( cvSize(img->width/scale,img->height/scale), 8, 3 );
	//    int i;
    //cvPyrDown( img, temp, CV_GAUSSIAN_5x5 );


    if( image && cascade)
    {

	   cvClearMemStorage( storage );
  	   pt1.x=0; pt1.y=0; pt2.x=0; pt2.y=0;
		
	   faces = cvHaarDetectObjects( frame, cascade, storage,
                               1.1, 4, CV_HAAR_DO_CANNY_PRUNING , cvSize(70, 70) );

	   if (faces->total)
	   {
		    CvRect* r = (CvRect*)cvGetSeqElem( faces, 0); //i );
			pt1.x = r->x; // *scale;
			pt2.x = (r->x+r->width); //*scale;
			pt1.y = r->y; // *scale;
		    pt2.y = (r->y+r->height); //*scale;
			cvRectangle( image, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );

			ptd.x=pt1.x+(int)((pt2.x-pt1.x)* PT1_xpos);
			ptd.y=pt1.y+(int)((pt2.y-pt1.y)* PT1_ypos);
			points[1][0].x=(float)ptd.x;
			points[1][0].y=(float)(ptd.y);
			cvCircle( image, ptd, 4, CV_RGB(255,0,0), 2, 8,0);
			pt_mode[0]=1;

			ptd.x=pt1.x+(int)((pt2.x-pt1.x)*PT2_xpos);
			ptd.y=pt1.y+(int)((pt2.y-pt1.y)*PT2_ypos);
			points[1][1].x=(float)ptd.x;
			points[1][1].y=(float)(ptd.y);
			cvCircle( image, ptd, 4, CV_RGB(0,255,0), 2, 8,0);
			pt_mode[1]=0;


			points[0][0].x=points[1][0].x;
			points[0][0].y=points[1][0].y;
			points[0][1].x=points[1][1].x;
			points[0][1].y=points[1][1].y;

			return(1);
	   }
    }
	return(0);
}

void calc_distances(int mode)
{
	double c;
	double dx,dy;

	if (mode<2)
	{
      dx=points[1][1].x - points[1][0].x;
	  dy=points[1][1].y - points[1][0].y;
	}
	else
	{
      dx=points[1][0].x - PT1_xpos*100;
	  dy=points[1][0].y - PT1_ypos*100;
	}
      c=sqrt((double)(dx*dx+dy*dy));
	  
  	  if (c== 0.0) { dist_error=dist_threshold; return; }

	  if (mode==1) 
	  { 
		  save_dist=(float)c; dist_error=0.0f; 
		  save_angle= (float) asin(dx/c) * 57.29577f; angle_error=0.0f;
		  return;
	  }
	  if (mode<2)
	  {
		dist_error= (float) fabs(save_dist-c);
		angle_error=  (float) fabs(save_angle-asin(dx/c) * 57.29577);
		return;
	  }
	  dist_error= (float)c;
	  angle_error=0;
}

int lk_work(CAMOBJ * st)
{
        int i, k;
		float mx,my,cx,cy;

        if((!frame ) || ((st->mode==2) && (cam_present!=1)))
            return(1);

        cvCopy( frame, image, 0 );

		if (st->mode==1)
		{
		  if (!video_writer)
		       video_writer = cvCreateVideoWriter(st->videofilename,-1,15,cvGetSize(frame));
		  	
		  cvWriteFrame(video_writer,frame);
  		}


        if (st->enable_tracking)
		{
		cvCvtColor( frame, grey, CV_BGR2GRAY );

        if( night_mode )
            cvZero( image );

		if (need_to_init)
		{
		  need_to_init=0;
		  init_flag=0;

		  if (st->trackface)
		  {
			if (detect_face())
			{
				int x;

				count=2;

				cvFindCornerSubPix( grey, points[1], count,
					cvSize(win_size,win_size), cvSize(-1,-1),
					cvTermCriteria(CV_TERMCRIT_ITER,1,1.0));
//					cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
  
	            cvCopy(grey,save_grey,0 );
		        cvCopy(pyramid,save_pyramid,0 );
				cvCopy(grey,prev_grey,0 );
		        cvCopy(pyramid,prev_pyramid,0 );

			    for (x=0;x<count;x++)
				{
					save_points[x].x=points[1][x].x;
					save_points[x].y=points[1][x].y;
					points[0][x].x=points[1][x].x;
					points[0][x].y=points[1][x].y;
					save_pt_mode[x]=pt_mode[x];
				}
				calc_distances(1);
				save_count=count;
				add_remove_pt = 0;
	            flags = 0;
				time_to_restore=0;
				
			} 
		  }
		  else
		  {
			    save_points[0].x=PT1_xpos*100;
				save_points[0].y=PT1_ypos*100;
				points[0][0].x=PT1_xpos*100;
				points[0][0].y=PT1_ypos*100;
				save_pt_mode[0]=0;
				count=1;MAX_COUNT=1;
				calc_distances(1);

  				cvFindCornerSubPix( grey, points[1], 1,
					cvSize(win_size,win_size), cvSize(-1,-1),
					cvTermCriteria(CV_TERMCRIT_ITER,1,1.0));
	     
				cvCopy(grey,save_grey,0 );
		        cvCopy(pyramid,save_pyramid,0 );
				cvCopy(grey,prev_grey,0 );
		        cvCopy(pyramid,prev_pyramid,0 );
				
				save_count=1;
				add_remove_pt = 0;
	            flags = 0;
				//time_to_restore=0;
				
		  }

		}        

		if(count < MAX_COUNT) need_to_init=1;
		else
        {
			
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], count, cvSize(win_size,win_size), 5, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;

			mx=0;my=0;
			cx=0;cy=0;mcount=0;ccount=0;
            for( i = k = 0; i < count; i++ )
            {

                if( add_remove_pt )
                {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if( dx*dx + dy*dy <= 25 )
                    {
                        add_remove_pt = 0;
                        if (pt_mode[i]==1) {pt_mode[i]=0; continue;}
						pt_mode[i]=1;
                    }
                }
                
                if( !status[i] ) { need_to_init=1; status[i]=true; }
                    

				if (pt_mode[i]==1)
				{
					cx+= (points[0][i].x - points[1][i].x);
					cy+= (points[0][i].y - points[1][i].y);
					ccount++;
				}
				else
				{
					mx += (points[0][i].x - points[1][i].x);
					my += (points[0][i].y - points[1][i].y);
					mcount++;
				}
				
				points[1][k] = points[1][i];
				pt_mode[k++]=pt_mode[i];
				if (need_to_init)
				  cvCircle( image, cvPointFrom32f(points[1][i]), 4, CV_RGB(255,0,0), 2, 8,0);
				else if (pt_mode[i]==1)
                  cvCircle( image, cvPointFrom32f(points[1][i]), 4, CV_RGB(255,255,0), 2, 8,0);
				  else
				   cvCircle( image, cvPointFrom32f(points[1][i]), 4, CV_RGB(0,210,0), 2, 8,0);
            }
            count = k;
			if (k==MAX_COUNT)
			{
				if (init_flag>1)
				{
				x_move=mx/mcount;
				y_move=my/mcount;
				x_click=cx/ccount;
				y_click=cy/ccount;
				}
				if (st->trackface) calc_distances(0); else calc_distances(2);
				
				
				if ((autorestore)) // && (init_flag>5))
				{
				  if (st->trackface)
				  {
					if ((dist_error>=dist_threshold) || (angle_error>=angle_threshold))
						time_to_restore++;
					else time_to_restore=0;

					if (time_to_restore>threshold_time)
					{ need_to_init=1; time_to_restore=0; }
				  }
				  else
				  {
					if ((dist_error>=dist_threshold))
						time_to_restore++;
					else time_to_restore=0;

					if (time_to_restore>threshold_time)
					{ need_to_init=1; time_to_restore=0; }

				  }
				  
				}
				
					
			} 
        }

        if( add_remove_pt && count < MAX_COUNT )
        {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix( grey, points[1] + count - 1, 1,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            add_remove_pt = 0;
        }

	  }

	  CV_SWAP( prev_grey, grey, swap_temp );
	  CV_SWAP( prev_pyramid, pyramid, swap_temp );
	  CV_SWAP( points[0], points[1], swap_points );
		
	  if (init_flag<1000) init_flag++;

	  if (st->showlive) cvShowImage( "Camera", image );
	

	return(0);
}



LRESULT CALLBACK CamDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	CAMOBJ * st;
	int tmp;
	
	st = (CAMOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_CAM)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemInt(hDlg,IDC_UPDATERATE,update_rate,0);
				SetDlgItemText(hDlg,IDC_CAMSTATUS,"0");
				SetDlgItemInt(hDlg,IDC_ERROR_DIST,(int)dist_threshold,0);
				SetDlgItemInt(hDlg,IDC_ERROR_ANGLE,(int)angle_threshold,0);
				SetDlgItemInt(hDlg,IDC_THRESHOLD_TIME,threshold_time,0);
				SetDlgItemInt(hDlg,IDC_PT1X,(int)(PT1_xpos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT1Y,(int)(PT1_ypos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT2X,(int)(PT2_xpos*100.0f),1);
				SetDlgItemInt(hDlg,IDC_PT2Y,(int)(PT2_ypos*100.0f),1);
				
				CheckDlgButton(hDlg,IDC_AUTORESTORE,autorestore);
				CheckDlgButton(hDlg,IDC_SHOWLIVE,st->showlive);
				CheckDlgButton(hDlg,IDC_ENABLE_TRACKING,st->enable_tracking);
				CheckDlgButton(hDlg,IDC_TRACKFACE,st->trackface);

				if (st->mode==0) CheckDlgButton(hDlg,IDC_NOARCHIVE,TRUE); else
					if (st->mode==1) CheckDlgButton(hDlg,IDC_RECORDARCHIVE,TRUE); else
						if (st->mode==2) CheckDlgButton(hDlg,IDC_PLAYARCHIVE,TRUE);
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
					lk_exit();
					lk_init(st);
					break;
				case IDC_EXITCAM:
					lk_exit();
                    break;
				case IDC_RESET:
		            count = 0;
					break;

				case IDC_NOARCHIVE:
					  lk_exit();
					  st->mode=0;
					  lk_init(st);
					break;
				case IDC_RECORDARCHIVE:
					  lk_exit();
					  st->mode=1;

					  if (!strcmp(st->videofilename,"none"))
					  {
						 strcpy(st->videofilename,GLOBAL.resourcepath); 
						 strcat(st->videofilename,"MOVIES\\*.avi");
					  }

					  if (!open_file_dlg(hDlg,st->videofilename, FT_AVI, OPEN_SAVE))
					     strcpy(st->videofilename,"none");
					  SetDlgItemText(hDlg, IDC_ARCHIVEFILE,st->videofilename);
					  lk_init(st);

					break;
				case IDC_PLAYARCHIVE:
					  lk_exit();
					  st->mode=2;
					  if (!strcmp(st->videofilename,"none"))
					  {
						 strcpy(st->videofilename,GLOBAL.resourcepath); 
						 strcat(st->videofilename,"MOVIES\\*.avi");
					  }
					  if (!open_file_dlg(hDlg,st->videofilename, FT_AVI, OPEN_LOAD))
					     strcpy(st->videofilename,"none");
					  SetDlgItemText(hDlg, IDC_ARCHIVEFILE,st->videofilename);
					  lk_init(st);

					break;
				
				case IDC_SHOWLIVE:
					st->showlive=IsDlgButtonChecked(hDlg,IDC_SHOWLIVE);
					if (!st->showlive) cvDestroyWindow("Camera");
					else
					{ 
					  cvNamedWindow( "Camera", 1 );
					  // cvSetMouseCallback( "Camera", on_mouse, 0 );
					}
                    break;
				case IDC_UPDATERATE:
					tmp=GetDlgItemInt(hDlg,IDC_UPDATERATE,NULL,0);
					if ((tmp>10)&&(tmp<1000)) update_rate=tmp;
                    break;
				case IDC_AUTORESTORE:
					autorestore=IsDlgButtonChecked(hDlg,IDC_AUTORESTORE);
                    break;
				case IDC_TRACKFACE:
					st->trackface=IsDlgButtonChecked(hDlg,IDC_TRACKFACE);
					if (st->trackface) MAX_COUNT=2; else MAX_COUNT=1;
                    break;
				case IDC_ERROR_DIST:
					dist_threshold=(float)GetDlgItemInt(hDlg, IDC_ERROR_DIST,0,0);
                    break;
				case IDC_ERROR_ANGLE:
					angle_threshold=(float)GetDlgItemInt(hDlg, IDC_ERROR_ANGLE,0,0);
                    break;
				case IDC_THRESHOLD_TIME:
					threshold_time=GetDlgItemInt(hDlg, IDC_THRESHOLD_TIME,0,0);
                    break;
				case IDC_PT1X:
					PT1_xpos=(float) GetDlgItemInt(hDlg, IDC_PT1X,0,1) / 100.0f;
					need_to_init=1;
					break;
				case IDC_PT1Y:
					PT1_ypos=(float)GetDlgItemInt(hDlg, IDC_PT1Y,0,1)/ 100.0f;
					need_to_init=1;
					break;
				case IDC_PT2X:
					PT2_xpos=(float)GetDlgItemInt(hDlg, IDC_PT2X,0,1)/ 100.0f;
					need_to_init=1;
					break;
				case IDC_PT2Y:
					PT2_ypos=(float)GetDlgItemInt(hDlg, IDC_PT2Y,0,1)/ 100.0f;
					need_to_init=1;
					break;
//				case IDC_NIGHTMODE:
//		            night_mode ^= 1;
//					break;
				case IDC_AUTOINIT:
					 need_to_init=1;
					break;
				case IDC_ENABLE_TRACKING:
					st->enable_tracking=IsDlgButtonChecked(hDlg,IDC_ENABLE_TRACKING);
					break;
				case IDC_SETTINGS:
					int		  numDevices = VI.listDevices();

					VI.showSettingsWindow(0);
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

	mode=0; enable_tracking=1; showlive=1; trackface=1;

	if (!lk_init(this)) GLOBAL.run_exception=1;
}
	
void CAMOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_CAMBOX, ghWndStatusbox, (DLGPROC)CamDlgHandler));
}

void CAMOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("interval",P_INT,&interval);
   load_property("autorestore",P_INT,&autorestore);
   load_property("dist_threshold",P_FLOAT,&dist_threshold);
   load_property("angle_threshold",P_FLOAT,&angle_threshold);
   load_property("threshold_time",P_INT,&threshold_time);
   load_property("pt1_xpos",P_FLOAT,&PT1_xpos);
   load_property("pt1_ypos",P_FLOAT,&PT1_ypos);
   load_property("pt2_xpos",P_FLOAT,&PT2_xpos);
   load_property("pt2_ypos",P_FLOAT,&PT2_ypos);
   load_property("mode",P_INT,&mode);
   load_property("showlive",P_INT,&showlive);
   load_property("tracking",P_INT,&enable_tracking);
   load_property("trackface",P_INT,&trackface);
   load_property("archive",P_STRING,videofilename);

   if (!showlive) cvDestroyWindow("Camera");
   if (trackface) MAX_COUNT=2; else MAX_COUNT=1;

}

void CAMOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"interval",P_INT,&interval);
    save_property(hFile,"autorestore",P_INT,&autorestore);
    save_property(hFile,"dist_threshold",P_FLOAT,&dist_threshold);
	save_property(hFile,"angle_threshold",P_FLOAT,&angle_threshold);
	save_property(hFile,"threshold_time",P_INT,&threshold_time);
    save_property(hFile,"pt1_xpos",P_FLOAT,&PT1_xpos);
    save_property(hFile,"pt1_ypos",P_FLOAT,&PT1_ypos);
    save_property(hFile,"pt2_xpos",P_FLOAT,&PT2_xpos);
    save_property(hFile,"pt2_ypos",P_FLOAT,&PT2_ypos);
	save_property(hFile,"mode",P_INT,&mode);
	save_property(hFile,"showlive",P_INT,&showlive);
	save_property(hFile,"tracking",P_INT,&enable_tracking);
	save_property(hFile,"trackface",P_INT,&trackface);
	save_property(hFile,"archive",P_STRING,videofilename);

}
	
void CAMOBJ::incoming_data(int port, float value)
{
}
	
void CAMOBJ::work(void)
{

	char sztemp[255];

	if ((hDlg==ghWndToolbox) && (!TIMING.dialog_update))
	{ 
	  sprintf(sztemp,"Distance=%.2f, Angle=%.2f, Time=%d",dist_error,angle_error,time_to_restore);
	  SetDlgItemText(hDlg,IDC_CAMSTATUS,sztemp);
	  SetDlgItemInt(hDlg,IDC_CUR_RATE,cur_rate,0);
	}

	if (trackface)
	{
		if ((!need_to_init)&&(init_flag>0))
		{
			pass_values(0, x_move);
			pass_values(1, y_move);
			pass_values(2, x_click);
			pass_values(3, y_click);
		}

	}
	else
	{
			pass_values(0, x_move);
			pass_values(1, y_move);
	}

}

CAMOBJ::~CAMOBJ() { lk_exit(); }

