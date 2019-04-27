/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_MCI.CPP:  definitions for the Multimedia-Player-Object
  Author: Chris Veigl

  This Object can open standard Multimedia files and play them in an extra window

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_mci.h"
											


LRESULT CALLBACK MCIDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szFileName[MAX_PATH];
	MCIOBJ * st;
	HANDLE test=0;
	
	st = (MCIOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MCIPLAYER)) return(FALSE);

	switch( message ) 
	{
	case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_MCIFILE, st->mcifile);
			SCROLLINFO lpsi;
		    lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=4; lpsi.nMax=5000;
			SetScrollInfo(GetDlgItem(hDlg,IDC_SPEEDUPDATEBAR),SB_CTL,&lpsi, TRUE);
			SetScrollPos(GetDlgItem(hDlg,IDC_SPEEDUPDATEBAR), SB_CTL,st->upd_speed, TRUE);
			SetDlgItemInt(hDlg, IDC_UPDATESPEED, st->upd_speed, FALSE);
			SetDlgItemInt(hDlg, IDC_POS_CENTER, st->pos_center, FALSE);
			CheckDlgButton(hDlg, IDC_PLAY_ONCE, st->play_once);
			CheckDlgButton(hDlg, IDC_AUTOSTART, st->autostart);
		return TRUE;
        
	case WM_CLOSE:
		 EndDialog(hDlg, LOWORD(wParam));
		break;
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		  case IDC_MCIFILE:
				GetDlgItemText(hDlg, IDC_MCIFILE, st->mcifile,255); 
			break;
		  case IDC_OPEN:
			  if (!(strcmp(st->mcifile,"none")))
			  {
				strcpy(szFileName,GLOBAL.resourcepath);
				strcat(szFileName,"MOVIES\\*.avi");
			  } else strcpy (szFileName,st->mcifile);

			if (open_file_dlg(hDlg, szFileName, FT_MCI, OPEN_LOAD)) 
			{

				st->playing=FALSE;
				strcpy(st->mcifile,szFileName);
				SetDlgItemText(hDlg, IDC_MCIFILE, st->mcifile); 
			}
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		  case IDC_PLAY_ONCE:
			  st->play_once=IsDlgButtonChecked(hDlg,IDC_PLAY_ONCE);
			  break;
		  case IDC_AUTOSTART:
			  st->autostart=IsDlgButtonChecked(hDlg,IDC_AUTOSTART);
			  break;
		    case IDC_LOAD:
				if (st->m_video) {	MCIWndStop(st->m_video); 	MCIWndDestroy(st->m_video); }

				char tempfilename[255];
				strcpy(tempfilename,st->mcifile);
				test=CreateFile(tempfilename, GENERIC_READ , 0, NULL, OPEN_EXISTING, 0,NULL);
				if (test==INVALID_HANDLE_VALUE)
				{
					strcpy(tempfilename,GLOBAL.resourcepath);
					strcat(tempfilename,st->mcifile);
				}
				else CloseHandle(test);

				st->m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME|MCIWNDF_NOERRORDLG,tempfilename);
				//	st-> m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME,st->mcifile);

				if (!st->m_video)  report_error ("Cannot open MCI File");
				else
				{
					RECT prc;

					MCIWndSetZoom(st->m_video,100);
					MCIWndGetSource(st->m_video,&prc);
					MCIWndSetTimeFormat(st->m_video ,"ms");
					MCIWndSetActiveTimer(st->m_video,500);
					if ((!strstr(st->mcifile,".mp3")) && (!strstr(st->mcifile,".wav"))) 
						SetWindowPos(st->m_video,HWND_TOPMOST,st->left,st->top,st->left+prc.right-prc.left,st->top+prc.bottom-prc.top,SWP_SHOWWINDOW);
					else ShowWindow(st->m_video,FALSE);
					
				}
				break;
			case IDC_MCIPLAY:
					
				if (st->m_video)
				{
					// MCIWndSetSpeed(st->m_video,st->speed);
					MCIWndPlay(st->m_video);
					st->playing=TRUE;
				}
		 		break;
				case IDC_MCISTOP:
					if (st->m_video) {	MCIWndStop(st->m_video);}  //	MCIWndDestroy(st->m_video); }
					st->playing=FALSE;
					break;

				case IDC_MCIPLUS:
					st->speed+=10;
					// if (st->m_video) { 
					//	MCIWndSetSpeed(st->m_video,(unsigned int ) st->speed); //MCIWndStep(st->m_video,2); 
						//MCIWndPlay(st->m_video); 
					//}
					break;

				case IDC_MCIMINUS:
					st->speed-=10;
					// 	if (st->m_video) 	{  MCIWndSetSpeed(st->m_video,st->speed); 		//MCIWndStep(st->m_video,2); 
						//MCIWndPlay(st->m_video);
					//	}
					break;
				case IDC_POS_CENTER:
					st->pos_center=GetDlgItemInt(hDlg,IDC_POS_CENTER,0,FALSE);
					break;

		}
		break;
	
		case WM_HSCROLL:
		{
			int nNewPos; 
			nNewPos = get_scrollpos(wParam,lParam);
			if (lParam == (long) GetDlgItem(hDlg,IDC_SPEEDUPDATEBAR))  
			{
					SetDlgItemInt(hDlg, IDC_UPDATESPEED, nNewPos, TRUE);
                    st->upd_speed=nNewPos;
			}
			break;
		}

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;

	}
   return FALSE;
}


//
//  Object Implementation
//


MCIOBJ::MCIOBJ(int num) : BASE_CL()	
	  {
	    inports  = 5;
		outports = 0;
		strcpy(in_ports[0].in_name,"play");
		strcpy(in_ports[1].in_name,"vol");
		strcpy(in_ports[2].in_name,"speed");
		strcpy(in_ports[3].in_name,"step");
		strcpy(in_ports[4].in_name,"position");
	
		width=75;
		left=250;right=500;top=100;bottom=200;
		input=0;m_video=0;speed=100; volume=1000;
		play_once=FALSE;
		updatestep=0;step=0;
		volume=1000;
		upd_speed=200; 
		pos_center=0;pos_change=0;
		prevtime=0;
		play=INVALID_VALUE; playing=FALSE; autostart=FALSE; resetted=FALSE;
		strcpy(mcifile,"none");
		//displayWnd=create_AVI_Window(left,right,top,bottom,&GLRC);	
	  }


void MCIOBJ::session_stop(void)
{
		 if (m_video) 	MCIWndStop(m_video);
		 playing=FALSE;
}

void MCIOBJ::session_start(void)
{
		 if (m_video) MCIWndPlay(m_video);
		 play=TRUE; playing=TRUE;
}
										

void MCIOBJ::make_dialog(void)
{
		 display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MCIPROPBOX, ghWndStatusbox, (DLGPROC)MCIDlgHandler));
}

void MCIOBJ::load(HANDLE hFile) 
{	
		load_object_basics(this);
		load_property("mci-file",P_STRING,mcifile);
		load_property("wnd-top",P_INT,&top);
		load_property("wnd-bottom",P_INT,&bottom);
		load_property("wnd-left",P_INT,&left);
		load_property("wnd-right",P_INT,&right);
		load_property("upd_speed",P_INT,&upd_speed);
		load_property("pos_center",P_INT,&pos_center);
		load_property("play_once",P_INT,&play_once);
		load_property("autostart",P_INT,&autostart);

		if (strcmp(mcifile,"none"))
		{

				char tempfilename[255];
				strcpy(tempfilename,mcifile);
				HANDLE test=CreateFile(tempfilename, GENERIC_READ , 0, NULL, OPEN_EXISTING, 0,NULL);
				if (test==INVALID_HANDLE_VALUE)
				{
					strcpy(tempfilename,GLOBAL.resourcepath);
					strcat(tempfilename,mcifile);
				}
				else CloseHandle(test);
				m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME|MCIWNDF_NOERRORDLG,tempfilename);


//			m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME|MCIWNDF_NOMENU|MCIWNDF_NOPLAYBAR|MCIWNDF_NOERRORDLG,mcifile);
//			m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME|MCIWNDF_NOERRORDLG,mcifile);
//			m_video = MCIWndCreate(ghWndMain, hInst,WS_VISIBLE|WS_THICKFRAME,mcifile);
			if (!m_video)  report_error ("Cannot open MCI File");
			else
			{
				MCIWndSetTimeFormat(m_video ,"ms");
				MCIWndSetActiveTimer(m_video,500);
				if ((!strstr(mcifile,".mp3")) && (!strstr(mcifile,".wav"))) SetWindowPos(m_video,HWND_TOPMOST,left,top,right-left,bottom-top,SWP_SHOWWINDOW);
				else ShowWindow(m_video,FALSE);

				if (GLOBAL.locksession) {
	 				SetWindowLong(m_video, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE)&~WS_SIZEBOX);
					//SetWindowLong(displayWnd, GWL_STYLE, 0);
				} else { SetWindowLong(m_video, GWL_STYLE, GetWindowLong(displayWnd, GWL_STYLE) | WS_SIZEBOX); }
			}
		}
}

void MCIOBJ::save(HANDLE hFile) 
{	
		WINDOWPLACEMENT  wndpl;

		if (m_video)
		{
			GetWindowPlacement(m_video, &wndpl);
			left=wndpl.rcNormalPosition.left;
			right=wndpl.rcNormalPosition.right;
			top=wndpl.rcNormalPosition.top;
			bottom=wndpl.rcNormalPosition.bottom;
		}
		

		save_object_basics(hFile,this);
		save_property(hFile,"mci-file",P_STRING,mcifile);
		save_property(hFile,"wnd-top",P_INT,&top);
		save_property(hFile,"wnd-bottom",P_INT,&bottom);
		save_property(hFile,"wnd-left",P_INT,&left);
		save_property(hFile,"wnd-right",P_INT,&right);
		save_property(hFile,"upd_speed",P_INT,&upd_speed);
		save_property(hFile,"pos_center",P_INT,&pos_center);
   	    save_property(hFile,"play_once",P_INT,&play_once);
		save_property(hFile,"autostart",P_INT,&autostart);


}

void MCIOBJ::incoming_data(int port, float value) 
{	
		switch (port)
		{
			case 0: play=(int)value; break;
			case 1: volume=(int)value;break;
			case 2: speed=(int)value; break;
			case 3: step=(int)value; break;
			case 4: pos_change=(int)value; break;
		}

}

void MCIOBJ::work(void) 
{
//		  InvalidateRect(displayWnd,NULL,FALSE);	

	if (GLOBAL.fly) return;
	if (!m_video) return;

	//if (MCIWndGetPosition(m_video)>=MCIWndGetLength(m_video)) playing=FALSE;

	if ((play!=INVALID_VALUE) && (m_video) && (!playing))
	{ 
		//write_logfile("MCI: start playing\n");
		if ((!play_once)||(resetted)) { MCIWndPlay(m_video); playing = TRUE; }
		resetted=FALSE;
	}

	if (play==INVALID_VALUE)
	{
		//write_logfile("MCI: INVALID play\n");
		resetted=TRUE;
		if ((m_video) && (playing))
		{	MCIWndStop(m_video); playing=FALSE; }
	}
	
	acttime=TIMING.acttime-prevtime;
    if (((float)acttime/(float)TIMING.pcfreq*1000)>(float)upd_speed)
	{  
		prevtime=TIMING.acttime;
		// if (actspeed!=speed) { actspeed=speed; MCIWndSetSpeed(m_video, speed); }	
		if (actvolume!=volume) { actvolume=volume;	MCIWndSetVolume(m_video, volume); }
		if (step!=0) {
			if	(MCIWndStep(m_video,step)!=0)
			{
				//write_logfile("MCI: step fails, rewind\n");
				if (step>0) MCIWndSeek(m_video,MCIWND_START);
				else MCIWndSeek(m_video,MCIWND_END);
			}
		} 
		else
		if (pos_change)
		{
			//write_logfile("MCI: pos_change\n");
			MCIWndSeek(m_video,pos_center+pos_change); pos_change=0;
		}
		else
		if (MCIWndGetPosition(m_video)>=MCIWndGetLength(m_video))
		{
			//write_logfile("MCI: position > length, restart\n");
			MCIWndStop(m_video);
			MCIWndSeek(m_video,MCIWND_START);
			MCIWndPlay(m_video);
		}
	}
	
}

MCIOBJ::~MCIOBJ()
	  {
		if (m_video) {	
			MCIWndStop(m_video); 	
			MCIWndClose(m_video); 
			Sleep(1000);
			MCIWndDestroy(m_video); }
	  }  





//  MCIWndHome(st->m_video);		
//	MCIWndResume(st->m_video);
//  MCIWndPause(st->m_video);
//	MCIWndStop(st->m_video);
//	MCIWndDestroy(st->m_video);


//  MCIWndSetVolume(st->m_video, m_Volume);
//  MCIWndSeek(st->m_video,SeekPos);
//  MCIWndPlay(st->m_video);
//	MCIWndSetZoom(st->m_video,150);

//  Length=MCIWndGetLength(st->m_video);
//  SetWindowPos(hWnd,HWND_TOPMOST,re.bottom,re.left,re.right,re.top,SWP_SHOWWINDOW);
//  SeekPos=MCIWndGetPosition(st->m_video);
//  if(SeekPos==Length)
//	MCIWndSetSpeed(st->m_video,1000);
