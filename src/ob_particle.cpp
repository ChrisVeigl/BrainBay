/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_PARTICLE.CPP:  contains functions for the Particle-Animation-Object

  The PARTICLE-Object draws into the Animation Window, which uses OpenGl.
  create_AnimationWindow:  created the Animation window 
			and inits the OpenGl- Rendering Context.
  DrawGLParticles: draws the particles of the current Object
  update_particledialog: show the setting in the toolbox window
  ParticleDlgHandler: processes the events for the toolbox window
  AnimationWndHandler: processes the events for the Animation window

  Contributors:  many thanks go to Jeff Molofee (NeHe) for his great OGL-tutorial
				 Web Site: nehe.gamedev.net

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
 

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_particle.h"

extern GLuint	texture[1];					// Storage For Our Particle Texture
struct PARTICLEPARAMETERStruct PARTICLEPARAMETER[PARTICLE_PARAMS]=
{ 
{"Number of Particles",1.0f,500.0f },
{"Generation Interval",0.0f,100.0f },
{"Slowdown",10.0f,3000.0f },
{"Color",0.0f,127.0f },
{"X-Position",-3.0f,3.0f },
{"Y-Position",-3.0f,3.0f },
{"Z-Position",-50.0f,10.0f },
{"X-Speed",-300.0f,300.0f },
{"Y-Speed",-300.0f,300.0f },
{"Z-Speed",-300.0f,300.0f },
{"X-Gravity",-5.0f,5.0f },
{"Y-Gravity",-5.0f,5.0f },
{"Z-Gravity",-5.0f,5.0f },
{"Life-Span",0.0f,10.0f },
{"Randomizer",0.0f,10.0f }
};




int create_AnimationWindow(void)
{
	HDC hDC;
	PIXELFORMATDESCRIPTOR pfd;
	int nPixelFormat;	
	BOOL	fullscreen=FALSE;			// Fullscreen Flag Set To Fullscreen Mode By Default
	RECT rc;


    if (!(ghWndAnimation= CreateWindow( "AnimationClass", "Animation", 
		WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME |WS_VISIBLE, GLOBAL.anim_left, GLOBAL.anim_top,GLOBAL.anim_right-GLOBAL.anim_left,GLOBAL.anim_bottom-GLOBAL.anim_top, ghWndMain, NULL, hInst, NULL)))
	{ 
		//SDL_Quit();
		report_error("Could not create Animation Window");
	    return FALSE;
	}
	ShowWindow(ghWndAnimation,1);
	UpdateWindow(ghWndAnimation);	
	//InvalidateRect(ghWndAnimation,NULL,TRUE);
	
	memset(&pfd, 0, sizeof(pfd));    
	pfd.nSize      = sizeof(pfd);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	// Set pixel format
	
	hDC = GetDC(ghWndAnimation);
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, nPixelFormat, &pfd);
	
	GLRC_Animation = wglCreateContext(hDC);
	wglMakeCurrent(hDC, GLRC_Animation);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f,0.0f,0.0f,0.0f);					// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
	glEnable(GL_BLEND);									// Enable Blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Type Of Blending To Perform
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				// Really Nice Point Smoothing
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
		report_error("Couldn't load Particle-Bitmap");
		return FALSE;									// If Texture Didn't Load Return FALSE
	}
	glBindTexture(GL_TEXTURE_2D,texture[0]);			// Select Our Texture
	GetClientRect(ghWndAnimation, &rc);
	Size_GL(ghWndAnimation, GLRC_Animation,0);					// Set Up Our Perspective GL Screen
	ReleaseDC(ghWndAnimation, hDC);
	DRAW.particles=1;
	return (TRUE);

}


int DrawGLParticles(GLvoid)										// Here's Where We Do All The Drawing
{
	float min;
	int s;
	unsigned int loop,particles;
	PARTICLEOBJ * st;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();										// Reset The ModelView Matrix

	for (s=0;s<GLOBAL.objects;s++)
	{ 
	  if (objects[s]->type==OB_PARTICLE)
	  {
		st=(PARTICLEOBJ *) objects[s];
		min=1000.0f;
        particles=(unsigned int)st->get_paramvalue(0);
		if (particles>MAX_PARTICLES) particles=0;           // mask out INVALID_VALUE 
		for (loop=0;loop<particles;loop++)					// Loop Through All The Particles
		{
			if (st->particle[loop].active)							// If The Particle Is Active
			{
				float x=st->particle[loop].x;						// Grab Our Particle X Position
				float y=st->particle[loop].y;						// Grab Our Particle Y Position
				float z=st->particle[loop].z;					    // Particle Z Pos

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
				glColor4f(st->particle[loop].r,st->particle[loop].g,st->particle[loop].b,st->particle[loop].life);

				glBegin(GL_TRIANGLE_STRIP);						// Build Quad From A Triangle Strip
				  glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z); // Top Right
				  glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); // Top Left
				  glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
				  glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
				glEnd();										// Done Building Triangle Strip

				st->particle[loop].x+=st->particle[loop].xi/st->get_paramvalue(2);// Move On The X Axis By X Speed
				st->particle[loop].y+=st->particle[loop].yi/st->get_paramvalue(2);// Move On The Y Axis By Y Speed
				st->particle[loop].z+=st->particle[loop].zi/st->get_paramvalue(2);// Move On The Z Axis By Z Speed

				st->particle[loop].xi+=st->particle[loop].xg;			// Take Pull On X Axis Into Account
				st->particle[loop].yi+=st->particle[loop].yg;			// Take Pull On Y Axis Into Account
				st->particle[loop].zi+=st->particle[loop].zg;			// Take Pull On Z Axis Into Account

				if (st->particle[loop].life>0.0f)					// If Particle Is Burned Out
					st->particle[loop].life-=FADING;		// Reduce Particles Life By 'Fade'
				
				if (st->particle[loop].life<min) { min=st->particle[loop].life;
		                      st->oldest_particle=loop; }
	
			}
		}
	  }
    }
	return TRUE;											// Everything Went OK
}


void update_particledialog(HWND hDlg, PARTICLEOBJ * st)
{
	static int updating=FALSE;
	char sztemp[25];
	
	SendDlgItemMessage(hDlg, IDC_PARAMETERCOMBO, CB_SETCURSEL, st->act_parameter, 0L ) ;
	SendDlgItemMessage(hDlg, IDC_REMOTECOMBO, CB_SETCURSEL, st->remote[st->act_parameter], 0L ) ;

	SetScrollPos(GetDlgItem(hDlg,IDC_VALUEBAR), SB_CTL,(int)st->value[st->act_parameter],TRUE);		

	sprintf(sztemp,"%.2f",st->get_paramvalue(st->act_parameter));
    SetDlgItemText(hDlg, IDC_VALUE,sztemp);

	sprintf(sztemp,"%.2f",st->min[st->act_parameter]); //PARTICLEPARAMETER[st->act_parameter].min);
	SetDlgItemText(hDlg,IDC_MINLABEL,sztemp);
	sprintf(sztemp,"%.2f",st->max[st->act_parameter]); //PARTICLEPARAMETER[st->act_parameter].max);
	SetDlgItemText(hDlg,IDC_MAXLABEL,sztemp);
	
	SetDlgItemText(hDlg, IDC_PALETTEFILE,st->palettefile);
	CheckDlgButton(hDlg, IDC_PARTICLEMUTE, st->mute);
	
}


/*-----------------------------------------------------------------------------

FUNCTION: CALLBACK ParticleDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )

PURPOSE: Handles Messages for Particle-Stream Dialog

PARAMETERS:
    hDlg - Dialog window handle

-----------------------------------------------------------------------------*/


LRESULT CALLBACK ParticleDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
   PARTICLEOBJ * st;
	
	st = (PARTICLEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_PARTICLE)) return(FALSE);

   
	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				char sztemp[3];
				int t;

			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				

				lpsi.nMin=1; lpsi.nMax=1024;
				SetScrollInfo(GetDlgItem(hDlg,IDC_VALUEBAR),SB_CTL,&lpsi,TRUE);
				for (t=0;t<PARTICLE_PARAMS;t++)
					SendDlgItemMessage(hDlg, IDC_PARAMETERCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) PARTICLEPARAMETER[t].paramname ) ;

				SendDlgItemMessage(hDlg, IDC_REMOTECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "none" ) ;
				for (t=1;t<7;t++)
				{
					sprintf(sztemp,"%d",t);
					SendDlgItemMessage(hDlg, IDC_REMOTECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) sztemp ) ;
				}
				
				update_particledialog(hDlg,st);
			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {

			case IDC_PARAMETERCOMBO:
				if (HIWORD(wParam)==CBN_SELCHANGE)
				{
				st->act_parameter=SendDlgItemMessage(hDlg, IDC_PARAMETERCOMBO, CB_GETCURSEL, 0, 0 ) ;
				update_particledialog(hDlg,st);
				EnableWindow(GetDlgItem(hDlg, IDC_VALUEBAR),TRUE);
				}
				break;
			case IDC_PARTICLEMUTE:
					st->mute=IsDlgButtonChecked(hDlg, IDC_PARTICLEMUTE);
				break;

			case IDC_REMOTECOMBO:
				if (HIWORD(wParam)==CBN_SELCHANGE)
				{
				st->remote[st->act_parameter]=SendDlgItemMessage(hDlg, IDC_REMOTECOMBO, CB_GETCURSEL, 0, 0 ) ;
				//update_particledialog(hDlg,st);
				}
				break;
			case IDC_SETMIN:
				st->min[st->act_parameter]=st->get_paramvalue(st->act_parameter);
				st->value[st->act_parameter]=1.0f;
				//SetScrollPos(GetDlgItem(hDlg,IDC_VALUEBAR), SB_CTL,1,TRUE);		
				update_particledialog(hDlg,st);
				break;
			case IDC_SETMAX:
				st->max[st->act_parameter]=st->get_paramvalue(st->act_parameter);
				st->value[st->act_parameter]=1024.0f;
				//SetScrollPos(GetDlgItem(hDlg,IDC_VALUEBAR), SB_CTL,1024,TRUE);		
				update_particledialog(hDlg,st);
				break;
			case IDC_RESETMINMAX:
				st->min[st->act_parameter]=PARTICLEPARAMETER[st->act_parameter].min;
				st->max[st->act_parameter]=PARTICLEPARAMETER[st->act_parameter].max;
				update_particledialog(hDlg,st);
				break;
			case IDC_LOADPAL:
				{
				  char szFileName[100];
				  int t;

				  strcpy(szFileName,GLOBAL.resourcepath);
				  strcat(szFileName,"PALETTES\\*.pal");
				  if (open_file_dlg(hDlg, szFileName, FT_PALETTE, OPEN_LOAD)) 
				  {
				    if (!load_from_file(szFileName, &(st->cols), sizeof(st->cols)))
				   	  report_error("Could not load Color Palette");
				    else
					{
					  reduce_filepath(szFileName,szFileName);
					  strcpy(st->palettefile,szFileName);
					  SetDlgItemText(hDlg, IDC_PALETTEFILE, st->palettefile); 
					  for (t=0;t<128;t++)
					  {
							st->colors[t][0]=(float)((st->cols[t]&0xff)/256.0);
							st->colors[t][1]=(float)(((st->cols[t]>>8)&0xff)/256.0);
							st->colors[t][2]=(float)((st->cols[t]>>16)/256.0);
					  }

					}
				  }
				}
				break;


			}
			
			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if ((nNewPos=get_scrollpos(wParam, lParam))>=0)
		    {			  
			  if (lParam == (long) GetDlgItem(hDlg,IDC_VALUEBAR)) 
			  {
				  char sztemp[10];
				  st->value[st->act_parameter]=(float)nNewPos;
				  sprintf(sztemp,"%.2f",st->get_paramvalue(st->act_parameter));
				  SetDlgItemText(hDlg, IDC_VALUE,sztemp);
			  }

			}
		
		}
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;

		return TRUE;
	}
    return FALSE;
}



LRESULT CALLBACK AnimationWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC	hDC;
	int t;

	switch( message ) 
	{
	case WM_CLOSE:
			DRAW.particles=0;
			Shutdown_GL(GLRC_Animation);
			ghWndAnimation=NULL;
  			DefWindowProc( hWnd, message, wParam, lParam );
		break;

	case WM_KEYDOWN:
				SendMessage(ghWndMain, message,wParam,lParam);
	case WM_SIZE: 
			Size_GL(hWnd, GLRC_Animation,0);	
	case WM_MOVE:
			{
  				WINDOWPLACEMENT  wndpl;
				GetWindowPlacement(hWnd, &wndpl);

				if (GLOBAL.locksession) {
					wndpl.rcNormalPosition.top=GLOBAL.anim_top;
					wndpl.rcNormalPosition.left=GLOBAL.anim_left;
					wndpl.rcNormalPosition.right=GLOBAL.anim_right;
					wndpl.rcNormalPosition.bottom=GLOBAL.anim_bottom;
					SetWindowPlacement(hWnd, &wndpl);
 					SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE)&~WS_SIZEBOX);
				}
				else {
					GLOBAL.anim_top=wndpl.rcNormalPosition.top;
					GLOBAL.anim_left=wndpl.rcNormalPosition.left;
					GLOBAL.anim_right=wndpl.rcNormalPosition.right;
					GLOBAL.anim_bottom=wndpl.rcNormalPosition.bottom;
					SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_SIZEBOX);
				}
				InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
	case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			wglMakeCurrent(hDC, GLRC_Animation);
			DrawGLParticles();			//  Do All The Drawing
			SwapBuffers(hDC);
			wglMakeCurrent(0, 0);
			EndPaint(hWnd, &ps);
		break;
	default:
			return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}







//
//  Object Implementation
//


PARTICLEOBJ::PARTICLEOBJ(int num) : BASE_CL()	
	  {
		inports  = 1;
		outports = 0;
		
		for (t=0;t<PARTICLE_PARAMS;t++)
		{
			value[t]=512.0f;
			min[t]=PARTICLEPARAMETER[t].min;
			max[t]=PARTICLEPARAMETER[t].max;
		}
			
		for (t=0;t<128;t++)
		{
			colors[t][0]=defcolors[t/10][0];
			colors[t][1]=defcolors[t/10][1];
			colors[t][2]=defcolors[t/10][2];
		}

		value[0]=200.0f;   //  Number of Particles
		value[1]=30.0f;    //  Production Interval
		value[2]=400.0f;   //  Slowdown
		value[13]=70.0f;   //  Live
		value[14]=100.0f;  //  Randomizer

		input1=0.0f;	input2=0.0f;	input3=0.0f;
		input4=0.0f;	input5=0.0f;	input6=0.0f;//512.0f;
		strcpy(palettefile,"none");

		for (t=0;t<15;t++) remote[t]=0;
		oldest_particle = 0;counter=0;act_parameter=0;mute=0;
		init_particlestream(); 
		if (!ghWndAnimation) 
		{
		  	if (!create_AnimationWindow()) report_error("Could not create Animation Window");
		}
		DRAW.particles=1;
		displayWnd=ghWndAnimation;
	 }

	  void PARTICLEOBJ::init_particlestream(void)
	  {
		int loop;

		for (loop=0;loop<MAX_PARTICLES;loop++)
		{
			particle[loop].active=TRUE;						// Make All The Particles Active
			particle[loop].life=2.0f;
			particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/128)][0];	// Select Red Rainbow Color
			particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/128)][1];	// Select Green Rainbow Color
			particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/128)][2];	// Select Blue Rainbow Color
			particle[loop].x=0.0f;	
			particle[loop].y=0.0f;	
			particle[loop].z=-10.0f;

			particle[loop].xi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On X Axis
			particle[loop].yi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
			particle[loop].zi=(float)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis

			particle[loop].xg=0.0f;	// Set Horizontal Pull
			particle[loop].yg=0.0f;	// Set Vertical Pull 
			particle[loop].zg=0.0f;	// Set Pull On Z Axis 
		}
	  } 

	  void PARTICLEOBJ::update_inports(void)
	  { 
		int i,z,m;
		m=-1;
		for (i=0;i<GLOBAL.objects;i++)
	  	  for (z=0;objects[i]->out[z].to_port!=-1;z++)
		    if ((objects[objects[i]->out[z].to_object]==this)&&(objects[i]->out[z].to_port>m)) m=objects[i]->out[z].to_port;
		inports=m+2;
		
		if (inports>6) inports=6;
		height=CON_START+inports*CON_HEIGHT+5;
		InvalidateRect(ghWndMain,NULL,TRUE);
	  }   

	  void PARTICLEOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_PARTICLEBOX, ghWndStatusbox, (DLGPROC)ParticleDlgHandler));
	  }
	  void PARTICLEOBJ::load(HANDLE hFile) 
	  {
		  load_object_basics(this);
		  load_property("mute",P_INT,&mute);
    	  load_property("palette-file",P_STRING,palettefile);
		  if (strcmp(palettefile,"none"))
		  {
			char szFileName[MAX_PATH] = "";
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PALETTES\\");
			strcat(szFileName,palettefile);
			if (!load_from_file(szFileName, cols, sizeof(cols) ))
			{    
				report_error(szFileName);
				report_error("Could not load Color Palette ");
			}
			else
			{
				for (t=0;t<128;t++)
				{
					colors[t][0]=(float)((cols[t]&0xff)/256.0);
					colors[t][1]=(float)(((cols[t]>>8)&0xff)/256.0);
					colors[t][2]=(float)((cols[t]>>16)/256.0);
				}

			}
		  } 

          for (t=0;t<PARTICLE_PARAMS;t++)
		  {
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-min");
			load_property(actpropname,P_FLOAT,&min[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-max");
			load_property(actpropname,P_FLOAT,&max[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-value");
			load_property(actpropname,P_FLOAT,&value[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-remote");
			load_property(actpropname,P_INT,&remote[t]);
			
		  }
			if (GLOBAL.locksession) {
	 			SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
				//SetWindowLong(displayWnd, GWL_STYLE, 0);
			} else { SetWindowLong(displayWnd, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }

	  }
		
	  void PARTICLEOBJ::save(HANDLE hFile) 
	  {	  
		  char actpropname[100];

		  save_object_basics(hFile, this);
		  save_property(hFile,"mute",P_INT,&mute);
    	  save_property(hFile,"palette-file",P_STRING,palettefile);
          for (t=0;t<PARTICLE_PARAMS;t++)
		  {
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-min");
			save_property(hFile,actpropname,P_FLOAT,&min[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-max");
			save_property(hFile,actpropname,P_FLOAT,&max[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-value");
			save_property(hFile,actpropname,P_FLOAT,&value[t]);
			strcpy(actpropname,PARTICLEPARAMETER[t].paramname);
			strcat(actpropname,"-remote");
			save_property(hFile,actpropname,P_INT,&remote[t]);

		  }

	  }

	  void PARTICLEOBJ::incoming_data(int port, float value) 
	  {	float * fp = &input1;
		fp+=port; 
		if (value!=INVALID_VALUE) *fp = (value-in_ports[port].in_min)/(in_ports[port].in_max-in_ports[port].in_min)*1024.0f;
		else *fp=INVALID_VALUE;
	  }

	  float PARTICLEOBJ::get_paramvalue(int param)
	  {
		if (value[param]==INVALID_VALUE) return(INVALID_VALUE);
		if (value[param]<in_ports[param].in_min) value[param]=in_ports[param].in_min;
		if (value[param]>1024) value[param]=1024;
		return( min[param] +  value[param]/1024.0f * (max[param]-min[param]));
	  }

	  void PARTICLEOBJ::work(void) 
	  {
	    float * fp =&input1;
		int f=0;

		if (GLOBAL.fly) return;

		for (t=0;t<6;t++)
			for (i=0;i<PARTICLE_PARAMS;i++)
				if (remote[i]==t+1)
				{
				    if (*(fp+t)!=INVALID_VALUE) value[i]=*(fp+t);
					else {value[i]=INVALID_VALUE;f=1;}
					if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox) &&(act_parameter==i)) 
					{
						char sztemp[25];
						if (value[i]!=INVALID_VALUE) 
						{ 
							sprintf(sztemp,"%.2f",get_paramvalue(i));
							SetDlgItemText(hDlg, IDC_VALUE,sztemp);
							EnableWindow(GetDlgItem(hDlg, IDC_VALUEBAR),TRUE); SetScrollPos(GetDlgItem(hDlg, IDC_VALUEBAR), SB_CTL, (int)value[i], 1); 
						}
						else { SetDlgItemText(hDlg, IDC_VALUE, "none"); EnableWindow(GetDlgItem(hDlg, IDC_VALUEBAR),FALSE); }
					}
				}

		counter++;
		if ((counter>get_paramvalue(1))&& (!mute)) 
		{  
			int replace,colindex;
			counter=0; 
			if (!f)
			{
			replace=oldest_particle;
				
			colindex=(int)get_paramvalue(3);if (colindex==INVALID_VALUE) colindex=0;
			particle[replace].r=colors[colindex][0];			// Select Red From Color Table
			particle[replace].g=colors[colindex][1];			// Select Green From Color Table
			particle[replace].b=colors[colindex][2];			// Select Blue From Color Table
			particle[replace].x=get_paramvalue(4);        //   Position
			particle[replace].y=get_paramvalue(5);
			particle[replace].z=get_paramvalue(6);
			particle[replace].xi=get_paramvalue(7)			//  Speed + Randomizer
				+(float)((rand()%50-25.0f)*get_paramvalue(14));
			particle[replace].yi=get_paramvalue(8) 
				+ (float)((rand()%50-25.0f)*get_paramvalue(14));
			particle[replace].zi=get_paramvalue(9)
				+ (float)((rand()%50-25.0f)*get_paramvalue(14));
			particle[replace].xg=get_paramvalue(10);
			particle[replace].yg=get_paramvalue(11);				// Gravity
			particle[replace].zg=get_paramvalue(12);
			particle[replace].life=get_paramvalue(13); 				// Give It New Life

			}
		}
	
	  }

PARTICLEOBJ::~PARTICLEOBJ()
	  {
			if ((count_objects(OB_PARTICLE)==1) && ghWndAnimation) SendMessage(ghWndAnimation,WM_CLOSE,0,0);	// free_object
	  }  

