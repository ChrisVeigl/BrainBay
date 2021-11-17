/* -----------------------------------------------------------------------------
   BrainBay  -  OpenSource Biofeedback Software
  
  MODULE: DIALOGS.CPP: this Module provides global accessible Dialog - Functions.

  select_color: display a color-select box and returns the selected color
  display_toolbox: creates the dialog-handler for the current object
  close_toolbox: closes the toolbox
  update_toolbox_position: saves the current position of the toolbox-window
  get_scrollpos: determines the position of a scrollbar
  color_button: fills a dialog item with a specified color
  AboutDlgHandler: Handler for the Info-Box
         
 --------------------------------------------------------------------------------------*/
#include "brainBay.h"


COLORREF select_color(HWND hwnd, COLORREF initial)
{
    CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
	COLORREF g_rgbSelect = initial; //RGB(128, 128, 128);
    static COLORREF g_rgbCustom[16] = {0};

    cc.Flags = CC_RGBINIT | CC_FULLOPEN  | CC_ANYCOLOR;
    cc.hwndOwner = hwnd;
    cc.rgbResult = g_rgbSelect;
    cc.lpCustColors = g_rgbCustom;

    if(ChooseColor(&cc))
    {
        g_rgbSelect = cc.rgbResult;
    }
	return g_rgbSelect;
}

void update_toolbox_position(HWND hDlg)
{
	WINDOWPLACEMENT  wndpl;
	if (ghWndToolbox!=NULL)
	{
		GetWindowPlacement(ghWndToolbox, &wndpl);
		GLOBAL.tool_top=wndpl.rcNormalPosition.top;
		GLOBAL.tool_left=wndpl.rcNormalPosition.left;
		GLOBAL.tool_right=wndpl.rcNormalPosition.right;
		GLOBAL.tool_bottom=wndpl.rcNormalPosition.bottom;
	}
}

void display_toolbox(HWND hDlg)
{
	int x,y,w,h;
	WINDOWPLACEMENT  wndpl;

	if (hDlg!=NULL)
	{
		ghWndToolbox=hDlg;
		ShowWindow(ghWndToolbox,FALSE);

		GetWindowPlacement(hDlg, &wndpl);
		x = GLOBAL.tool_left;
		y = GLOBAL.tool_top;
		w=wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left;
		h=wndpl.rcNormalPosition.bottom-wndpl.rcNormalPosition.top;

		SetWindowPos(ghWndToolbox, ghWndMain, x,y,w,h,SWP_NOACTIVATE|SWP_SHOWWINDOW);
		UpdateWindow(ghWndToolbox);
		ShowWindow(ghWndToolbox,TRUE);
		update_toolbox_position(hDlg);
		SetActiveWindow(ghWndMain);
		SetFocus(ghWndMain);
	}
}

			
void close_toolbox( void )
{
	if (ghWndToolbox!=NULL) SendMessage(ghWndToolbox,WM_CLOSE,0,0);
	ghWndToolbox=NULL;	actobject=NULL; actconnect=NULL; GLOBAL.showtoolbox=-1;
	InvalidateRect(ghWndDesign,NULL,TRUE);
}






int get_scrollpos(WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = (int)LOWORD(wParam);
	int nPos = (short int)HIWORD(wParam);
	int nNewPos; 	
	SCROLLINFO si = {sizeof(SCROLLINFO), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0,0};

	GetScrollInfo ((HWND)lParam,SB_CTL, &si);
	nNewPos = si.nPos;
	switch (nScrollCode)
	{
	  case 0:	nNewPos = nNewPos - 1;break;
	  case 1:	nNewPos = nNewPos + 1;break;
	  case 2:	nNewPos = nNewPos - 5;break;
	  case 3:	nNewPos = nNewPos + 5;break;
	  case SB_THUMBTRACK:
	  case SB_THUMBPOSITION:
				nNewPos = nPos;	break;
	}
	if (nNewPos<si.nMin) nNewPos=si.nMin;
	if (nNewPos>si.nMax) nNewPos=si.nMax;
	si.fMask = SIF_POS;  si.nPos = nNewPos;
	SetScrollInfo ((HWND)lParam,SB_CTL, &si, TRUE);
	return(nNewPos);
}


void color_button(HWND hWnd, COLORREF newcolor)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	HPEN	 tpen;
	HBRUSH	 tbrush;
    hdc = BeginPaint (hWnd, &ps);
    GetClientRect(hWnd, &rect);
	tpen    = CreatePen (PS_SOLID,1,0);
    SelectObject (hdc, tpen);
	tbrush  = CreateSolidBrush(newcolor);
	SelectObject(hdc,tbrush);
    Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);
	DeleteObject(tbrush);
	DeleteObject(tpen);
	EndPaint(hWnd, &ps );
}


LRESULT CALLBACK AboutDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


void update_colors(COLORREF * cols, int s, int e)
{
	int t;
	double r,g,b;
	if (e>s) 
	{
		  r=(double)(GetRValue(cols[e])-GetRValue(cols[s]))/(double)(e-s);
		  g=(double)(GetGValue(cols[e])-GetGValue(cols[s]))/(double)(e-s);
		  b=(double)(GetBValue(cols[e])-GetBValue(cols[s]))/(double)(e-s);

  	  	  for (t=s+1;t<e;t++)  cols[t]=	RGB(GetRValue(cols[s])+r*(t-s),GetGValue(cols[s])+g*(t-s),
										GetBValue(cols[s])+b*(t-s));

	} 
	
}



LRESULT CALLBACK COLORDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static COLORREF c;
	static int t,x=0;
	static COLORREF cols[128];
	static int startband=0,endband=127,actcol=0;
	static char szFileName[MAX_PATH];
	static char tmppos[30];
	

	switch( message ) 
	{
	case WM_INITDIALOG:
		{
			SCROLLINFO lpsi;

			startband=0;
			endband=127;
			for (t=0;t<128;t++) cols[t]=RGB(0,0,150);

			SetDlgItemInt(hDlg, IDC_STARTBAND, startband,0); 
			SetDlgItemInt(hDlg, IDC_ENDBAND, endband,0); 

			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=0; lpsi.nMax=255;
			SetScrollInfo(GetDlgItem(hDlg,IDC_FROMREDBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_FROMREDBAR), SB_CTL, GetRValue(cols[startband]), 1);
			SetScrollInfo(GetDlgItem(hDlg,IDC_FROMGREENBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_FROMGREENBAR), SB_CTL, GetGValue(cols[startband]), 1);
			SetScrollInfo(GetDlgItem(hDlg,IDC_FROMBLUEBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_FROMBLUEBAR), SB_CTL, GetBValue(cols[startband]), 1);
			SetScrollInfo(GetDlgItem(hDlg,IDC_TOREDBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_TOREDBAR), SB_CTL, GetRValue(cols[endband]), 1);
			SetScrollInfo(GetDlgItem(hDlg,IDC_TOGREENBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_TOGREENBAR), SB_CTL, GetGValue(cols[endband]), 1);
			SetScrollInfo(GetDlgItem(hDlg,IDC_TOBLUEBAR),SB_CTL,&lpsi,TRUE);
 			SetScrollPos(GetDlgItem(hDlg, IDC_TOBLUEBAR), SB_CTL, GetBValue(cols[endband]), 1);

		}
		return TRUE;
        
	case WM_CLOSE:
		 EndDialog(hDlg, LOWORD(wParam));
		break;
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{ 
		case IDC_STARTBAND:
			startband=GetDlgItemInt(hDlg, IDC_STARTBAND, 0,0);
			if (startband>127) { startband=127;SetDlgItemInt(hDlg, IDC_ENDBAND, startband,0);  }
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_ENDBAND:
			endband=GetDlgItemInt(hDlg, IDC_ENDBAND, 0,0);
			if (endband>127) {endband=127; SetDlgItemInt(hDlg, IDC_ENDBAND, endband,0);  }
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_LOADPAL:
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PALETTES\\*.pal");
			if (open_file_dlg(hDlg, szFileName, FT_PALETTE, OPEN_LOAD)) 
			{
			  char tmp[256],*p1,*p2,diff=0;
			  strcpy(tmp,GLOBAL.resourcepath);
			  strcat(tmp,"PALETTES\\"); 
			  for (p1=tmp,p2=szFileName;(*p1) && (*p2) && (!diff);p1++,p2++) 
				  if (tolower(*p1)!=tolower(*p2)) diff=1;
			  if (diff||(strlen(tmp)>strlen(szFileName)))
				report("Please use Palletes-subfolder of brainbay application to load/store palette files");
			  else
			    if (!load_from_file(szFileName, &(cols), sizeof(cols)))
				  report_error("Could not load Palette");
			  InvalidateRect(hDlg,NULL,FALSE);
			}
			break;
		case IDC_SAVEPAL:
			strcpy(szFileName,GLOBAL.resourcepath);
			strcat(szFileName,"PALETTES\\*.pal");
			if (open_file_dlg(hDlg, szFileName, FT_PALETTE, OPEN_SAVE)) 
			{
			  char tmp[256],*p1,*p2,diff=0;
			  strcpy(tmp,GLOBAL.resourcepath);
			  strcat(tmp,"PALETTES\\"); 
			  for (p1=tmp,p2=szFileName;(*p1) && (*p2) && (!diff);p1++,p2++) 
				  if (tolower(*p1)!=tolower(*p2)) diff=1;
			  if (diff||(strlen(tmp)>strlen(szFileName)))
				report("Please use Palletes-subfolder of brainbay application to load/store palette files");
			  else
			  if (!save_to_file(szFileName, &(cols), sizeof(cols)))
				report_error("Could not save Palette");
			  InvalidateRect(hDlg,NULL,FALSE);
			}
			break;
		case IDC_FFTCOLOR1:
			c=select_color(hDlg,cols[startband]);
			cols[startband]=c;
			update_colors(cols,startband, endband);
			SetScrollPos(GetDlgItem(hDlg, IDC_FROMREDBAR), SB_CTL, GetRValue(cols[startband]), 1);
			SetScrollPos(GetDlgItem(hDlg, IDC_FROMGREENBAR), SB_CTL, GetGValue(cols[startband]), 1);
			SetScrollPos(GetDlgItem(hDlg, IDC_FROMBLUEBAR), SB_CTL, GetBValue(cols[startband]), 1);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		case IDC_FFTCOLOR2:
			c=select_color(hDlg,cols[endband]);
			cols[endband]=c;
			update_colors(cols,startband, endband);
			SetScrollPos(GetDlgItem(hDlg, IDC_TOREDBAR), SB_CTL, GetRValue(cols[endband]), 1);
			SetScrollPos(GetDlgItem(hDlg, IDC_TOGREENBAR), SB_CTL, GetGValue(cols[endband]), 1);
			SetScrollPos(GetDlgItem(hDlg, IDC_TOBLUEBAR), SB_CTL, GetBValue(cols[endband]), 1);
			InvalidateRect(hDlg,NULL,FALSE);
			break;
		}
		break;

		case WM_HSCROLL:
		{
			int nNewPos; 
			if ((nNewPos=get_scrollpos(wParam,lParam))>=0)
			{   
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FROMREDBAR)) 
			   cols[startband]=RGB(nNewPos,GetGValue(cols[startband]),GetBValue(cols[startband]));
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FROMGREENBAR)) 
			   cols[startband]=RGB(GetRValue(cols[startband]),nNewPos,GetBValue(cols[startband]));
			  if (lParam == (long) GetDlgItem(hDlg,IDC_FROMBLUEBAR)) 
			   cols[startband]=RGB(GetRValue(cols[startband]),GetGValue(cols[startband]),nNewPos);
			

			  if (lParam == (long) GetDlgItem(hDlg,IDC_TOREDBAR)) 
			    cols[endband]=RGB(nNewPos,GetGValue(cols[endband]),GetBValue(cols[endband]));
			  if (lParam == (long) GetDlgItem(hDlg,IDC_TOGREENBAR)) 
			    cols[endband]=RGB(GetRValue(cols[endband]),nNewPos,GetBValue(cols[endband]));
			  if (lParam == (long) GetDlgItem(hDlg,IDC_TOBLUEBAR)) 
			    cols[endband]=RGB(GetRValue(cols[endband]),GetGValue(cols[endband]),nNewPos);
			
			  update_colors(cols,startband,endband);
			  InvalidateRect(hDlg,NULL,FALSE); 
	
			}
		
		}
		break;

		case WM_MOUSEMOVE:
			if (HIWORD(lParam)<160)
			{ 
				  x = int((LOWORD(lParam)-10)/5); 
				  if (x<0) x=0; if (x>127) x=127;
				  actcol=x;
				  sprintf(tmppos,"%d",x);
				  SetDlgItemText(hDlg,IDC_POS,tmppos);
		  			InvalidateRect(hDlg,NULL,FALSE);

			}
			break;
		case WM_LBUTTONDOWN:
			  sprintf(tmppos,"%d",x);
			  startband=x;
	  		  SetDlgItemText(hDlg, IDC_STARTBAND, tmppos);
  				SetScrollPos(GetDlgItem(hDlg, IDC_FROMREDBAR), SB_CTL, GetRValue(cols[startband]), 1);
				SetScrollPos(GetDlgItem(hDlg, IDC_FROMGREENBAR), SB_CTL, GetGValue(cols[startband]), 1);
				SetScrollPos(GetDlgItem(hDlg, IDC_FROMBLUEBAR), SB_CTL, GetBValue(cols[startband]), 1);

			break;
		case WM_RBUTTONDOWN:
			  sprintf(tmppos,"%d",x);
			  endband=x;
	  		  SetDlgItemText(hDlg, IDC_ENDBAND, tmppos);
	 			SetScrollPos(GetDlgItem(hDlg, IDC_TOREDBAR), SB_CTL, GetRValue(cols[endband]), 1);
				SetScrollPos(GetDlgItem(hDlg, IDC_TOGREENBAR), SB_CTL, GetGValue(cols[endband]), 1);
				SetScrollPos(GetDlgItem(hDlg, IDC_TOBLUEBAR), SB_CTL, GetBValue(cols[endband]), 1);

			break;
		case WM_ACTIVATE:
		case WM_KILLFOCUS:
	//	case WM_SETFOCUS:
			InvalidateRect(hDlg,NULL,FALSE);
		break;
		case WM_PAINT:
			{  
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rect;
			HPEN	 tpen;
			HBRUSH	 tbrush;
			hdc = BeginPaint (hDlg, &ps);
			GetClientRect(hDlg, &rect);
				tpen    = CreatePen (PS_SOLID,1,0);
				SelectObject (hdc, tpen);
				MoveToEx(hdc,9,4,NULL);
				LineTo(hdc,650,4);
				LineTo(hdc,650,int(rect.bottom/1.6)+1);
				LineTo(hdc,9,int(rect.bottom/1.6)+1);
				LineTo(hdc,9,4);
//				Rectangle(hdc,9,4,650,int(rect.bottom/1.6)+1);
				DeleteObject(tpen);

				tpen    = CreatePen (PS_SOLID,1,255);
				SelectObject (hdc, tpen);
				MoveToEx(hdc,10+cols[startband]*5,4,NULL);
				LineTo(hdc,10+cols[startband]*5,int(rect.bottom/1.6));
				MoveToEx(hdc,10+cols[endband]*5,4,NULL);
				LineTo(hdc,10+cols[endband]*5,int(rect.bottom/1.6));
				DeleteObject(tpen);

			for (t=0;t<128;t++)
			{
				tbrush  = CreateSolidBrush(cols[t]);
				tpen    = CreatePen (PS_SOLID,1,cols[t]);
				SelectObject (hdc, tpen);
				SelectObject(hdc,tbrush);
				Rectangle(hdc,10+t*5,5,15+t*5,int(rect.bottom/1.6));
				DeleteObject(tbrush);
				DeleteObject(tpen);
			}
			EndPaint(hDlg, &ps );
			color_button(GetDlgItem(hDlg,IDC_FFTCOLOR1),cols[startband]);
			color_button(GetDlgItem(hDlg,IDC_FFTCOLOR2),cols[endband]);
			color_button(GetDlgItem(hDlg,IDC_FFTCOLOR3),cols[actcol]);
			}
		break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;

	}
   return FALSE;
}


void update_harmonic( HWND hCtrlWnd)
{
	int t,index;
    char szdata[5];

	SendMessage(hCtrlWnd, LB_RESETCONTENT, 0, 0);
	for (t=0;t<LOADSCALE.len;t++) 
	{
		wsprintf(szdata, "%d", LOADSCALE.tones[t]);
		index=SendMessage(hCtrlWnd, LB_ADDSTRING, 0,(LPARAM) szdata);
		SendMessage(hCtrlWnd, LB_SETITEMDATA, (WPARAM)index, (LPARAM)LOADSCALE.tones[t]);
	}

}

void apply_harmonic(HWND hDlg)
{ 
  int t;

  for (t=0;t<LOADSCALE.len;t++)
	  LOADSCALE.tones[t]=SendDlgItemMessage(hDlg, IDC_HARMONICLIST, LB_GETITEMDATA, (WPARAM)t, 0);
}


LRESULT CALLBACK SCALEDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static int t,port=DEF_MIDIPORT,midichn=11,instrument=12;
	static char szFileName[MAX_PATH];
	static int oldtone=0;


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;

				strcpy(LOADSCALE.name,"Chromatic1-127");
				for (t=1;t<128;t++)  LOADSCALE.tones[t-1]=t; 
				LOADSCALE.len=127;
				SetDlgItemText(hDlg, IDC_HARMONICNAME, LOADSCALE.name);

				for (t=0;t<255;t++) 
					SendDlgItemMessage(hDlg, IDC_MIDIINSTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) midi_instnames[t]) ;
				SendDlgItemMessage( hDlg, IDC_MIDIINSTCOMBO, CB_SETCURSEL, instrument, 0L ) ;

 			    for (t = 0; t < GLOBAL.midiports; t++) 
				 if(MIDIPORTS[t].midiout)
				   SendDlgItemMessage(hDlg, IDC_MIDIPORTCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) MIDIPORTS[t].portname ) ;
				SetDlgItemText(hDlg, IDC_MIDIPORTCOMBO, MIDIPORTS[port].portname);

				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=1; lpsi.nMax=127;
				SetScrollInfo(GetDlgItem(hDlg,IDC_TONEBAR),SB_CTL,&lpsi,TRUE);
 			    SetScrollPos(GetDlgItem(hDlg, IDC_TONEBAR), SB_CTL, 64, 1);
			    SetDlgItemInt(hDlg, IDC_ACTTONE, 64,0);
			    SetDlgItemInt(hDlg, IDC_MIDICHN, midichn,0);
				
				midi_Instrument(&(MIDIPORTS[port].midiout),midichn,instrument);
				update_harmonic(GetDlgItem(hDlg, IDC_HARMONICLIST));

			}
			return TRUE;
	
		case WM_CLOSE:
				midi_NoteOff(&(MIDIPORTS[port].midiout), midichn,oldtone);
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

			case IDC_MIDIPORTCOMBO:
 				 if (HIWORD(wParam)==CBN_SELCHANGE)
				    port=get_listed_midiport(SendMessage(GetDlgItem(hDlg, IDC_MIDIPORTCOMBO), CB_GETCURSEL , 0, 0));
					midi_Instrument(&(MIDIPORTS[port].midiout),midichn,instrument); 
				 break;
			case IDC_MIDIINSTCOMBO:
					instrument=SendMessage(GetDlgItem(hDlg, IDC_MIDIINSTCOMBO), CB_GETCURSEL , 0, 0);
					midi_Instrument(&(MIDIPORTS[port].midiout),midichn,instrument); 
				break;
			case IDC_MIDICHN:
					midichn=GetDlgItemInt(hDlg, IDC_MIDICHN, NULL, 0);
					midi_Instrument(&(MIDIPORTS[port].midiout),midichn,instrument);
				break;
			case IDC_DELTONE:
				{   int select;
					select=SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_GETCURSEL , 0, 0L ) ;
					if ((select>=0)&&(select<LOADSCALE.len))
					{
					  SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_DELETESTRING , (WPARAM) select, 0L ) ;
					  LOADSCALE.len--;
					  SetFocus(GetDlgItem(hDlg,IDC_HARMONICLIST));
					  SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_SETCURSEL , (WPARAM) select, 0L ) ;
					  apply_harmonic(hDlg);
					}
//					else  report("Nothing selected");
				}
				break;
			case IDC_HARMONICNAME:
				  GetDlgItemText(hDlg, IDC_HARMONICNAME, LOADSCALE.name, MAX_NAMELEN);
				  
				break;
			case IDC_ADDTONE:
				{  int index,dataint,select;
				   char szdata[5];

					if (LOADSCALE.len<MAX_HARMONICTONES-1)
					{
					  select=SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_GETCURSEL , 0, 0L )+1 ;
					  if ((select<1)||(select>LOADSCALE.len)) select=LOADSCALE.len;
					  GetDlgItemText(hDlg, IDC_ACTTONE, szdata, 4);
					  dataint=GetDlgItemInt(hDlg, IDC_ACTTONE, 0,0);
					  index=SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_INSERTSTRING , (WPARAM) select, (LPARAM) szdata) ;
					  SendDlgItemMessage(hDlg, IDC_HARMONICLIST, LB_SETITEMDATA, (WPARAM)index, (LPARAM)dataint);
					  LOADSCALE.len++;
					  SetFocus(GetDlgItem(hDlg,IDC_HARMONICLIST));
					  apply_harmonic(hDlg);
					} else report("List full");
				}
				break;
			case IDC_HARMONICCLEAR:
					SendDlgItemMessage(hDlg, IDC_HARMONICLIST, LB_RESETCONTENT, 0, 0);
					LOADSCALE.len=0;
					apply_harmonic(hDlg);
					break;
			case IDC_LOADHARMONIC:
				{
				strcpy(szFileName,GLOBAL.resourcepath);
 			    strcat(szFileName,"TONESCALES\\default.sc");
				 if (open_file_dlg(hDlg, szFileName, FT_HARMONIC, OPEN_LOAD)) 
				 {
	 			  char tmp[256],*p1,*p2,diff=0;
				  strcpy(tmp,GLOBAL.resourcepath);
				  strcat(tmp,"TONESCALES\\"); 
				  for (p1=tmp,p2=szFileName;(*p1) && (*p2) && (!diff);p1++,p2++) 
					  if (tolower(*p1)!=tolower(*p2)) diff=1;
				  if (diff||(strlen(tmp)>strlen(szFileName)))
					report("Please use Tonescales-subfolder of brainbay application to load/store palette files");
				  else
				  {

					if (!load_from_file(szFileName, &LOADSCALE, sizeof(struct SCALEStruct)))
						report_error("Could not load Harmonic Scale");
					else
					{
					    update_harmonic(GetDlgItem(hDlg, IDC_HARMONICLIST));
						SetDlgItemText(hDlg, IDC_HARMONICNAME, LOADSCALE.name);

					}
				  }
				 } else report_error("Could not load Harmonic Scale");

				}
				break;
			case IDC_SAVEHARMONIC:
				{
					char temp[260];
					strcpy(szFileName,GLOBAL.resourcepath);
					strcat(szFileName,"TONESCALES\\");
					GetDlgItemText(hDlg, IDC_HARMONICNAME, temp, MAX_PATH);
					strcat (szFileName,temp);
					strcat (szFileName,".sc");

					if (open_file_dlg(hDlg, szFileName, FT_HARMONIC, OPEN_SAVE))
					{
	 				  char tmp[256],*p1,*p2,diff=0;
					  strcpy(tmp,GLOBAL.resourcepath);
					  strcat(tmp,"TONESCALES\\"); 
					  for (p1=tmp,p2=szFileName;(*p1) && (*p2) && (!diff);p1++,p2++) 
						  if (tolower(*p1)!=tolower(*p2)) diff=1;
					  if (diff||(strlen(tmp)>strlen(szFileName)))
						report("Please use Tonescales-subfolder of brainbay application to load/store palette files");
					  else
					  {

						 if (!save_to_file(szFileName, &LOADSCALE, sizeof(struct SCALEStruct)))
							report_error("Could not save Scale");
					  }
					}
				}
				break;

            case IDC_HARMONICLIST:
                if (HIWORD(wParam)==LBN_SELCHANGE)
                {
					int dataint,sel;
					sel=SendDlgItemMessage( hDlg, IDC_HARMONICLIST, LB_GETCURSEL , 0, 0L ) ;
					dataint=SendDlgItemMessage(hDlg, IDC_HARMONICLIST, LB_GETITEMDATA, (WPARAM)sel, 0);
					midi_NoteOff(&(MIDIPORTS[port].midiout), midichn,oldtone);
					oldtone=dataint;
					midi_NoteOn(&(MIDIPORTS[port].midiout), midichn, dataint,127);
					
                }
				break;

			}
			return TRUE;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if ((nNewPos=get_scrollpos(wParam,lParam))>=0)
			{   
			  if (lParam == (long) GetDlgItem(hDlg,IDC_TONEBAR))  { SetDlgItemInt(hDlg, IDC_ACTTONE,nNewPos,0);
																	midi_NoteOff(&(MIDIPORTS[port].midiout), midichn,oldtone);
																	oldtone=nNewPos;
																	midi_NoteOn(&(MIDIPORTS[port].midiout), midichn,nNewPos,127);
																	apply_harmonic(hDlg);
																	}
			}
		
		}
		break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}

LRESULT CALLBACK SETTINGSDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static int port=DEF_MIDIPORT;
	int t,wCount;
	char szBuffer[100];



	switch( message )
	{
		case WM_INITDIALOG:
			{
				SetDlgItemInt(hDlg, IDC_SAMPLINGRATE, PACKETSPERSECOND,0);

				for (t = 0; t < GLOBAL.midiports; t++) 
				{
					
				  SendDlgItemMessage(hDlg, IDC_MIDIPORTCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) MIDIPORTS[t].portname ) ;
				  if (MIDIPORTS[t].midiout) SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_ADDSTRING, 0, (LPARAM) (LPSTR) MIDIPORTS[t].portname) ;
				}
				SetDlgItemText(hDlg, IDC_MIDIPORTCOMBO, MIDIPORTS[port].portname);
				SetDlgItemText(hDlg, IDC_EMOTIV_PATH, GLOBAL.emotivpath);
				SetDlgItemText(hDlg, IDC_GANGLION_PATH, GLOBAL.ganglionhubpath);
				SetDlgItemText(hDlg, IDC_STARTDESIGN_PATH, GLOBAL.startdesignpath);

				for (wCount = 0; wCount < MAX_COMPORT; wCount++) 
				{
					wsprintf( szBuffer, "COM%d", wCount + 1 ) ;
					SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) szBuffer ) ;
				}
				if (TTY.PORT)
				   SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_SETCURSEL, (WPARAM) (TTY.PORT - 1), 0L ) ;
				else SetDlgItemText(hDlg, IDC_PORTCOMBO, "none");
			
				for (wCount = 0; BaudTable[wCount]; wCount++) 
				{
					SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) szBaud[wCount] ) ;
					if (BaudTable[wCount] == TTY.BAUDRATE) SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_SETCURSEL, (WPARAM) wCount, 0L ) ;
				}
				for (t=0; devicetypes[t][0]!=0;t++)
					SendDlgItemMessage( hDlg, IDC_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) devicetypes[t]) ;
				SendDlgItemMessage( hDlg, IDC_DEVICECOMBO, CB_SETCURSEL, TTY.devicetype, 0L ) ;

				SetDlgItemInt(hDlg, IDC_DIALOGINTERVAL,GLOBAL.dialog_interval,0);
				SetDlgItemInt(hDlg, IDC_DRAWINTERVAL,GLOBAL.draw_interval,0);

				CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
				CheckDlgButton(hDlg, IDC_STARTUP, GLOBAL.startup);
				CheckDlgButton(hDlg, IDC_STARTDESIGN, GLOBAL.startdesign);
				CheckDlgButton(hDlg, IDC_AUTORUN, GLOBAL.autorun);
				CheckDlgButton(hDlg, IDC_MINIMIZED, GLOBAL.minimized);
				CheckDlgButton(hDlg, IDC_LOCKSESSION, GLOBAL.locksession);
				CheckDlgButton(hDlg, IDC_USE_CVCAPTURE, GLOBAL.use_cv_capture);
				CheckDlgButton(hDlg, IDC_USE_VIDEOINPUT, !GLOBAL.use_cv_capture);
				CheckDlgButton(hDlg, IDC_ADDARCHIVETIME, GLOBAL.add_archivetime);

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				ghWndSettings=NULL;
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_MIDIPORTCOMBO:
				 if (HIWORD(wParam)==CBN_SELCHANGE)
				      port=SendMessage(GetDlgItem(hDlg, IDC_MIDIPORTCOMBO), CB_GETCURSEL , 0, 0);
					
				 	 break;
			case IDC_ENABLEMIDI:
					{
					  int t,f=0;
					  for (t=0;t<SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_GETCOUNT, 0,0);t++)
					  {
						SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_GETTEXT, t, (LPARAM) (LPSTR) szBuffer ) ;	
						if (!strcmp(MIDIPORTS[port].portname,szBuffer)) f=1;
					  }
	
					  if (!f) 
					  {
						if (!midi_open_port(&(MIDIPORTS[port].midiout),port))
						{
							wsprintf(szBuffer,"Cannot open Midiout Port %d: %s",port,MIDIPORTS[port].portname);
							MIDIPORTS[port].midiout=0;
							report_error(szBuffer);
						} else SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_ADDSTRING, 0, (LPARAM) (LPSTR) MIDIPORTS[port].portname) ;
						
					  } 
					} 
					break;

			case IDC_MIDIPORTLIST:
				if(HIWORD(wParam)==LBN_SELCHANGE)
				{
					int o=SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_GETCURSEL, 0, 0);

					SendDlgItemMessage(hDlg, IDC_MIDIPORTLIST, LB_DELETESTRING, o,0);
					midiOutClose(MIDIPORTS[get_listed_midiport(o)].midiout);
					MIDIPORTS[get_listed_midiport(o)].midiout=0;
				}
				break;
			case IDC_NEWSAMPLINGRATE:
				t=GetDlgItemInt(hDlg,IDC_SAMPLINGRATE,NULL,0);
				if ((t<1)) report_error("Please enter positive Value.");
				else update_samplingrate(t);
				SetDlgItemInt(ghWndStatusbox,IDC_SAMPLINGRATE,PACKETSPERSECOND,0);
				break;
			case IDC_STARTUP:
				 GLOBAL.startup= IsDlgButtonChecked(hDlg, IDC_STARTUP);
		 		 GLOBAL.tool_top=0;
				 GLOBAL.tool_left=0;					 
				 break;
			case IDC_STARTDESIGN:
				 GLOBAL.startdesign= IsDlgButtonChecked(hDlg, IDC_STARTDESIGN);
				 break;
			case IDC_ADDARCHIVETIME:
				 GLOBAL.add_archivetime= IsDlgButtonChecked(hDlg, IDC_ADDARCHIVETIME);
				 break;
			case IDC_AUTORUN:
				 GLOBAL.autorun= IsDlgButtonChecked(hDlg, IDC_AUTORUN);
				 break;
			case IDC_MINIMIZED:
				 GLOBAL.minimized= IsDlgButtonChecked(hDlg, IDC_MINIMIZED);
				break;
			case IDC_LOCKSESSION:
				 GLOBAL.locksession= IsDlgButtonChecked(hDlg, IDC_LOCKSESSION);
				 for (int i=0;i<GLOBAL.objects;i++)
					 if (objects[i]->displayWnd) {
						 SendMessage(objects[i]->displayWnd,WM_MOVE,0,0);
						 SendMessage(objects[i]->displayWnd,WM_MOUSEACTIVATE,0,0);
 						 InvalidateRect(objects[i]->displayWnd,NULL,TRUE);
					 }

					if (GLOBAL.locksession) 
					{  
							GLOBAL.showdesign=FALSE;
							ShowWindow(ghWndDesign,FALSE);
							SetDlgItemText(ghWndStatusbox,IDC_DESIGN,"Show Design");
						    ShowWindow(GetDlgItem(ghWndStatusbox,IDC_DESIGN), SW_HIDE);
					}
					else 
					{
							ShowWindow(GetDlgItem(ghWndStatusbox,IDC_DESIGN), SW_SHOW);
					}
				 break;
			case IDC_USE_CVCAPTURE:
			case IDC_USE_VIDEOINPUT:
				 GLOBAL.use_cv_capture= IsDlgButtonChecked(hDlg, IDC_USE_CVCAPTURE);
				break;
		
			case IDC_DIALOGINTERVAL:
				GLOBAL.dialog_interval=GetDlgItemInt(hDlg,IDC_DIALOGINTERVAL, NULL, 0);
				break;
			case IDC_DRAWINTERVAL:
				GLOBAL.draw_interval=GetDlgItemInt(hDlg,IDC_DRAWINTERVAL, NULL, 0);
				break;
			case IDC_EMOTIV_PATH:
				GetDlgItemText(hDlg,IDC_EMOTIV_PATH, GLOBAL.emotivpath, 255);
				break;
			case IDC_GANGLION_PATH:
				GetDlgItemText(hDlg,IDC_GANGLION_PATH, GLOBAL.ganglionhubpath, 255);
				break;
			case IDC_STARTDESIGN_PATH:
				GetDlgItemText(hDlg,IDC_STARTDESIGN_PATH, GLOBAL.startdesignpath, 255);
				break;
			case IDC_SAVESETTINGS:
				   if (!save_settings())  report_error("Could not save Settings");
			
//			case IDC_CLOSE:
 			    EndDialog(hDlg, LOWORD(wParam));
				ghWndSettings=NULL;
				break;			
			}
			return TRUE;
		
		case WM_SIZE:
		case WM_MOVE: // update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}


void update_dlg(HWND hDlg, struct LINKStruct * actconnect)
{
   char temp[50];

   /*
	if (actconnect->setdimension)
	{ 
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_DIMCOMBO),TRUE);
	}
	else
	{ 
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_DIMCOMBO),FALSE);
	}

  */
    SetDlgItemText(hDlg, IDC_DIMENSION, actconnect->dimension);

	sprintf(temp,"%.2f",actconnect->min);
	SetDlgItemText(hDlg, IDC_MIN, temp);

	sprintf(temp,"%.2f",actconnect->max);
	SetDlgItemText(hDlg, IDC_MAX, temp);
}

LRESULT CALLBACK CONNECTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	char temp[50],temp2[40];
 	
	if (actconnect==NULL) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				
				strcpy(temp,objects[actconnect->from_object]->tag);
				strcat(temp," (");
				strcpy(temp2,objects[actconnect->from_object]->out_ports[actconnect->from_port].out_name);
				if (!temp2[0]) wsprintf(temp2,"%d",actconnect->from_port+1);
				strcat(temp,temp2);
				strcat(temp,")");
				SetDlgItemText(hDlg, IDC_FROM, temp);

				strcpy(temp,objects[actconnect->to_object]->tag);
				strcat(temp," (");
				strcpy(temp2,objects[actconnect->to_object]->in_ports[actconnect->to_port].in_name);
				if (!temp2[0]) wsprintf(temp2,"%d",actconnect->to_port+1);
				strcat(temp,temp2);
				strcat(temp,")");
				SetDlgItemText(hDlg, IDC_TO, temp);

				SetDlgItemText(hDlg, IDC_DESC, objects[actconnect->from_object]->out_ports[actconnect->from_port].out_desc);

				update_dimensions();
				update_dlg(hDlg, actconnect);

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
/*		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

			}
			return TRUE;
*/		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}

void update_outportdlg(HWND hDlg)
{
	char temp[100];
	if (actobject->out_ports[actport].get_range==-1)
	{
		SetDlgItemText(hDlg, IDC_RANGEPORTCOMBO, "---");
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_DIMCOMBO),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_DESC),TRUE);
	}
	else
	{
		SetDlgItemInt(hDlg, IDC_RANGEPORTCOMBO, actobject->out_ports[actport].get_range+1,0);
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_DIMCOMBO),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_DESC),FALSE);
	}
	sprintf(temp,"%.2f",actobject->out_ports[actport].out_min);
	SetDlgItemText(hDlg, IDC_MIN, temp);

	sprintf(temp,"%.2f",actobject->out_ports[actport].out_max);
	SetDlgItemText(hDlg, IDC_MAX, temp);

	SetDlgItemText(hDlg, IDC_DESC, actobject->out_ports[actport].out_desc);
	SetDlgItemText(hDlg, IDC_DIMCOMBO, actobject->out_ports[actport].out_dim);
}

LRESULT CALLBACK OUTPORTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	char temp[100];
	int t;

	if ((actobject==NULL)||(actport==-1)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				
				strcpy(temp,actobject->tag);
				strcat(temp, " ( ");
				strcat(temp,actobject->out_ports[actport].out_name);
				strcat(temp, " )");

				SetDlgItemText(hDlg, IDC_CAPTION, temp);

				SetDlgItemText(hDlg, IDC_DESC, actobject->out_ports[actport].out_desc);

				for (t=0;dimensions[t][0];t++) 
					SendDlgItemMessage(hDlg, IDC_DIMCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) dimensions[t]) ;


				for (t=0;t<actobject->inports;t++)
				{
					sprintf(temp,"%d",t+1);
					SendDlgItemMessage(hDlg, IDC_RANGEPORTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) temp) ;
				}
				SendDlgItemMessage(hDlg, IDC_RANGEPORTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) "none (Set Range)") ;

				update_outportdlg(hDlg);

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

				case IDC_DESC:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
  					GetDlgItemText(hDlg, IDC_DESC, temp,sizeof(temp));
					strcpy(actobject->out_ports[actport].out_desc,temp);
					update_dimensions();
					update_outportdlg(hDlg);
				  }
				break;			
				case IDC_MIN:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
					GetDlgItemText(hDlg, IDC_MIN, temp,sizeof(temp));
					actobject->out_ports[actport].out_min=(float)atof(temp);
					update_dimensions();
					update_outportdlg(hDlg);
				  }
				break;			
				case IDC_MAX:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
  					GetDlgItemText(hDlg, IDC_MAX, temp,sizeof(temp));
					actobject->out_ports[actport].out_max=(float)atof(temp);
					update_dimensions();
					update_outportdlg(hDlg);
				  }
				break;			
				case IDC_DIMCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
					   strcpy(actobject->out_ports[actport].out_dim,dimensions[SendDlgItemMessage(hDlg, IDC_DIMCOMBO, CB_GETCURSEL, 0, 0 )]);
						update_dimensions();
						update_outportdlg(hDlg);
					} 
					else if (HIWORD(wParam)==CBN_KILLFOCUS) {
						GetDlgItemText(hDlg, IDC_DIMCOMBO, temp,sizeof(temp));
						strcpy(actobject->out_ports[actport].out_dim,temp);
						update_dimensions();
						update_outportdlg(hDlg);
					}
				break;		
				case IDC_RANGEPORTCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
						int r;
						r=SendDlgItemMessage(hDlg, IDC_RANGEPORTCOMBO, CB_GETCURSEL, 0, 0 );
						if (r>actobject->inports-1) r=-1;
					    actobject->out_ports[actport].get_range=r;
						update_dimensions();
						update_outportdlg(hDlg);
					}
				break;		

			}
			return TRUE;
		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}


void update_inportdlg(HWND hDlg)
{
	char temp[100];
	if (actobject->in_ports[actport].get_range==FALSE)
	{
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_DESC),TRUE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg,IDC_MIN),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_MAX),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_DESC),FALSE);
	}
	sprintf(temp,"%.2f",actobject->in_ports[actport].in_min);
	SetDlgItemText(hDlg, IDC_MIN, temp);

	sprintf(temp,"%.2f",actobject->in_ports[actport].in_max);
	SetDlgItemText(hDlg, IDC_MAX, temp);

	SetDlgItemText(hDlg, IDC_DESC, actobject->in_ports[actport].in_desc);
	CheckDlgButton(hDlg, IDC_GETRANGE,actobject->in_ports[actport].get_range);
}

LRESULT CALLBACK INPORTDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	char temp[100];

	if ((actobject==NULL)||(actport==-1)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				
				// MessageBox(ghWndMain,temp,"",MB_OK);

				strcpy(temp,actobject->tag);
				strcat(temp, " ( ");
				strcat(temp,actobject->in_ports[actport].in_name);
				strcat(temp, " )");

				SetDlgItemText(hDlg, IDC_CAPTION, temp);

				SetDlgItemText(hDlg, IDC_DESC, actobject->in_ports[actport].in_desc);

				SetDlgItemText(hDlg, IDC_DIMENSION, actobject->in_ports[actport].in_dim);
				
				sprintf(temp,"actport %d",actport);
				update_inportdlg(hDlg);

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

				case IDC_DESC:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
  					GetDlgItemText(hDlg, IDC_DESC, temp,sizeof(temp));
					strcpy(actobject->in_ports[actport].in_desc,temp);
					update_dimensions();
					update_inportdlg(hDlg);
				  }
				break;			
				case IDC_MIN:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
					GetDlgItemText(hDlg, IDC_MIN, temp,sizeof(temp));
					actobject->in_ports[actport].in_min=(float)atof(temp);
					update_dimensions();
					update_inportdlg(hDlg);
				  }
				break;			
				case IDC_MAX:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
  					GetDlgItemText(hDlg, IDC_MAX, temp,sizeof(temp));
					actobject->in_ports[actport].in_max=(float)atof(temp);
					update_dimensions();
					update_inportdlg(hDlg);
				  }
				break;			
				case IDC_GETRANGE:
					actobject->in_ports[actport].get_range=IsDlgButtonChecked(hDlg,IDC_GETRANGE);
					update_dimensions();
					update_inportdlg(hDlg);
				break;		

			}
			return TRUE;
		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}

LRESULT CALLBACK TAGDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{

	if (!actobject) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			{
				
				SetDlgItemText(hDlg, IDC_TYPE, objnames[actobject->type]);
				SetDlgItemText(hDlg, IDC_TAG, actobject->tag);

			}
			return TRUE;
	
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

				case IDC_TAG:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
  					GetDlgItemText(hDlg, IDC_TAG, actobject->tag,30);
					InvalidateRect(ghWndDesign,NULL,TRUE);
				  }
				break;			
				
			}
			return TRUE;
		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}

void update_statusinfo(void)
{
	char szdata[100];

	if (GLOBAL.running) 
	{
		wsprintf(szdata, "Session running,  %d Packets/sec (%d lost)",TIMING.actpps, GLOBAL.syncloss); 
		SetDlgItemText(ghWndStatusbox,IDC_STATUS,szdata);
	}
	else SetDlgItemText(ghWndStatusbox,IDC_STATUS,"Session paused");

	//sprintf(szdata, "%.1f", (float)TIMING.packetcounter/(float)PACKETSPERSECOND);

	if (GLOBAL.session_sliding<1)
	{
	 print_time(szdata,(float)TIMING.packetcounter/(float)PACKETSPERSECOND,1);
	 SetDlgItemText(ghWndStatusbox,IDC_TIME,szdata);
	}

	if (GLOBAL.session_length>0)
	{
		print_time(szdata,(float)GLOBAL.session_end/(float)PACKETSPERSECOND,0);
		SetDlgItemText(ghWndStatusbox,IDC_SESSLEN,szdata);

		print_time(szdata, (float)GLOBAL.session_start/(float)PACKETSPERSECOND,0);
		SetDlgItemText(ghWndStatusbox,IDC_SESSTART,szdata);

		if (GLOBAL.session_sliding<4) { GLOBAL.session_sliding=-1; SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,get_sliderpos(TIMING.packetcounter)); }
	}
}


void stop_session() {
	GLOBAL.fly=0;
	TTY.read_pause=1;
	GLOBAL.session_sliding=-1;
	stop_timer(); 							
	for (int t=0;t<GLOBAL.objects;t++) objects[t]->session_stop();
	SetDlgItemText(ghWndStatusbox,IDC_STATUS,"Session paused");
}

void run_session() {
	stop_timer();
	update_dimensions();
	GLOBAL.session_sliding=-1;
	for (int t=0;t<GLOBAL.objects;t++)  objects[t]->session_start();
	start_timer();
	SetDlgItemText(ghWndStatusbox,IDC_STATUS,"Session running");
}

LRESULT CALLBACK StatusDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	WINDOWPLACEMENT  wndpl;
	int t;
	char str[21];
	HWND hTrack_Bar;
	float fl;

	switch( message )
	{
		case WM_INITDIALOG:
				GetWindowPlacement(ghWndMain, &wndpl);
				SetWindowPos(hDlg, HWND_BOTTOM, wndpl.rcNormalPosition.left+4, wndpl.rcNormalPosition.bottom-40, 
				wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left-8,35, SWP_NOACTIVATE|SWP_NOZORDER);
				
				hTrack_Bar=GetDlgItem(hDlg,IDC_SESSIONPOS);
				SendMessage(hTrack_Bar,TBM_SETRANGE,TRUE,MAKELONG(0, 1000));
				SendMessage(hTrack_Bar,TBM_SETPOS,TRUE,(LONG)0);

				SetDlgItemText(hDlg,IDC_TIME," 00:00:00.000");
				SetDlgItemText(hDlg, IDC_JUMPPOS, "00:00:00");
				SetDlgItemText(hDlg, IDC_SESSLEN, "00:00:00");
				SetDlgItemText(hDlg, IDC_SESSTART, "00:00:00");
				SetDlgItemText (hDlg, IDC_STATUS, "BrainBay ready.");

				SendMessage(hTrack_Bar,TBM_SETSELSTART,TRUE,0);
				SendMessage(hTrack_Bar,TBM_SETSELEND,TRUE,1000);
				CheckDlgButton(hDlg,IDC_SESSIONLOOP,GLOBAL.session_loop);

				return TRUE;

		case WM_MOUSEACTIVATE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		  SetFocus(ghWndMain);
		  SetActiveWindow(ghWndMain);
		  // SendMessage(ghWndMain,WM_MOUSEACTIVATE,wParam,lParam);
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_RESETBUTTON:
						stop_session();
						TIMING.ppscounter=0;
						TIMING.packetcounter=0;
						SetDlgItemText(hDlg,IDC_TIME,"0.0");
						GLOBAL.session_sliding=-1;
						SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,0);
						SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,1000);
						SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_SETPOS,TRUE,(LONG)(0));
						GLOBAL.session_start=0;
						GLOBAL.session_end=GLOBAL.session_length;
						update_statusinfo();
						for (t=0;t<GLOBAL.objects;t++) objects[t]->session_reset();
						GLOBAL.syncloss=0;
						if (GLOBAL.session_length>0) set_session_pos(0);
					    SetFocus(ghWndMain);
					break;

				case IDC_RUNSESSION:
						run_session();
						SetFocus(ghWndMain);
					break;
				case IDC_STOPSESSION:
						stop_session();

						if (lParam==1) {   // sent from sessiontime, must be asynchronous ...
							char configfilename[MAX_PATH];
							close_toolbox();
							strcpy(configfilename,GLOBAL.resourcepath); 
							strcat(configfilename,"CONFIGURATIONS\\");
							strcat(configfilename,GLOBAL.nextconfigname);
							strcat(configfilename,".con");
							printf("trying to load configfile: %s\n",configfilename);
							if (!load_configfile(configfilename)) 
								report_error("Could not load Config File");
							else sort_objects();					  
						}
						SetFocus(ghWndMain);
					break;

				case IDC_ENDSESSION:
						GLOBAL.fly=0;
						TTY.read_pause=1;
						GLOBAL.session_sliding=-1;
						stop_timer(); 							
						for (t=0;t<GLOBAL.objects;t++) objects[t]->session_stop();
						for (t=0;t<GLOBAL.objects;t++) objects[t]->session_end();
						Sleep(100);
						char configfilename[MAX_PATH];
						close_toolbox();
						strcpy(configfilename,GLOBAL.resourcepath); 
						strcat(configfilename,"CONFIGURATIONS\\");
						strcat(configfilename,GLOBAL.startdesignpath);
						strcat(configfilename,".con");
						// printf("trying to load configfile: %s\n",configfilename);
						if (load_configfile(configfilename)) 
							sort_objects();					  
					    SetFocus(ghWndMain);
					break;
				case IDC_JUMP:
						stop_session();
						GetDlgItemText(hDlg, IDC_JUMPPOS, str, 20);
						fl = get_time(str) * (float) PACKETSPERSECOND;
						if (GLOBAL.session_length>0) set_session_pos((long)fl);
						SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,get_sliderpos(TIMING.packetcounter));
					    SetFocus(ghWndMain);
						break;
				case IDC_SESSIONLOOP:
						GLOBAL.session_loop=IsDlgButtonChecked(hDlg, IDC_SESSIONLOOP);
					    SetFocus(ghWndMain);
						break;
				case IDC_GOSTART:
					{
						char tmp[20];
						print_time(tmp,(float)TIMING.packetcounter/(float)PACKETSPERSECOND,0);
						//SetDlgItemText(ghWndStatusbox,IDC_TIME,tmp);
						SetDlgItemText(hDlg, IDC_JUMPPOS,tmp);
				
						//if (GLOBAL.session_length>0) set_session_pos(GLOBAL.session_start);
						//SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,get_sliderpos(TIMING.packetcounter));
					}
					break;
				case IDC_SETSTART:
					{
 						int pos=get_sliderpos(TIMING.packetcounter);
						SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,pos);
						GLOBAL.session_start=TIMING.packetcounter;
						update_statusinfo();
					}
					break;
				case IDC_SETEND:
					{
 						int pos = get_sliderpos(TIMING.packetcounter);
						SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,pos);
						GLOBAL.session_end=TIMING.packetcounter;
						update_statusinfo();
					}
					break;
				case IDC_FLY:
					if ((GLOBAL.session_length>0)&&(GLOBAL.session_start!=GLOBAL.session_end))
					{
							if (TIMING.packetcounter>=GLOBAL.session_end) 
								set_session_pos(GLOBAL.session_start);
							GLOBAL.fly=1;
							if (!GLOBAL.running) SendMessage (hDlg,WM_COMMAND,IDC_RUNSESSION,0);
					}
					break;
				case IDC_DESIGN:
						if (GLOBAL.showdesign) 
						{  
							GLOBAL.showdesign=FALSE;
							ShowWindow(ghWndDesign,FALSE);
							SetDlgItemText(ghWndStatusbox,IDC_DESIGN,"Show Design");
						}
						else 
						{
							GLOBAL.showdesign=TRUE;
							ShowWindow(ghWndDesign,TRUE);
						    SetWindowPos(ghWndDesign,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
							SetDlgItemText(ghWndStatusbox,IDC_DESIGN,"Hide Design"); 
						}
					    SetFocus(ghWndMain);
					break;
				case IDC_HIDE:
						if (GLOBAL.hidestatus) 
						{  
							GLOBAL.hidestatus=FALSE;
							ShowWindow(ghWndStatusbox,TRUE);
						}
						else 
						{
							GLOBAL.hidestatus=TRUE;
							ShowWindow(ghWndStatusbox,FALSE);
						}
					    SetFocus(ghWndMain);
					break;
				case IDC_SAMPLINGRATE:
					if (HIWORD(wParam)==256)
                     if (ghWndSettings==NULL) ghWndSettings=CreateDialog(hInst, (LPCTSTR)IDD_SETTINGSBOX, ghWndStatusbox, (DLGPROC)SETTINGSDlgHandler);
						 else SetForegroundWindow(ghWndSettings);
					break;
			}
			return TRUE;

		case WM_NOTIFY:
			if ((LOWORD(wParam)==IDC_SESSIONPOS) && ( ((LPNMHDR)(lParam))->code == NM_RELEASEDCAPTURE + 4))
			{
				int pos=SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_GETPOS,0,0);
				GLOBAL.session_sliding++;		
				if (GLOBAL.session_sliding>8) 
				{
					char tmp[20];
					print_time(tmp,(float)pos/1000.0f*(float)GLOBAL.session_length/(float)PACKETSPERSECOND,0);
					SetDlgItemText(ghWndStatusbox,IDC_TIME,tmp);
					//SetDlgItemText(hDlg, IDC_JUMPPOS,tmp);
					//SetDlgItemInt(hDlg, IDC_JUMPPOS,(long)((float)pos/1000.0f*(float)GLOBAL.session_length/(float)PACKETSPERSECOND),0);
				}
		
			}
			if ((LOWORD(wParam)==IDC_SESSIONPOS) && ( ((LPNMHDR)(lParam))->code == NM_RELEASEDCAPTURE )  )
			{
				int pos=SendMessage(GetDlgItem(hDlg,IDC_SESSIONPOS),TBM_GETPOS,0,0);
				GLOBAL.session_sliding=0;
				set_session_pos((long)((float)pos/1000.0f*(float)GLOBAL.session_length));
				stop_session();
			}
		return(TRUE);



	}
    return FALSE;
}




int find_min_to_line(int x, int y,int x1, int y1, int x2, int y2)
{
	int min_dist=1000,dist;

	int t;
	float dx,dy,ax,ay;

	#define ST 200

	dx=(float)(x2-x1)/ST;  // test 200 Points of the Line
	dy=(float)(y2-y1)/ST;
	for (t=0;t<ST;t++)
	{
		ax=x1+dx*t;
		ay=y1+dy*t;
		dist=(int) (sqrt((ax-x)*(ax-x)+(ay-y)*(ay-y)));
		if (dist<min_dist) min_dist=dist;
	}
	return(min_dist);

}


LRESULT CALLBACK DesignWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	static int sx=0,sy=0,PX,PY;
	SCROLLINFO si;
    ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(ghWndDesign, SB_HORZ, &si);
	PX=si.nPos;
	GetScrollInfo(ghWndDesign, SB_VERT, &si);
	PY=si.nPos;
	
	switch( message ) 
	{	
			case WM_MOUSEACTIVATE:
				SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
				InvalidateRect(hWnd,NULL,TRUE);
			break;
			case WM_MOUSEMOVE:

				if (GLOBAL.objstate==2)
				{
				  GLOBAL.tx = LOWORD(lParam)+PX; 
				  GLOBAL.ty= HIWORD(lParam)+PY; 
				  InvalidateRect(hWnd,NULL,TRUE);
				}
				
				// if (wParam==MK_LBUTTON)    // MK_CONTROL,MK_SHIFT
				if (GLOBAL.objstate==1)
				{
					if (actobject)
					{
						actobject->xPos = LOWORD(lParam)-actobject->width/2+sx+PX; 
						actobject->yPos = HIWORD(lParam)-CON_MAGNETIC+sy+PY; 
					}
					else { GLOBAL.objstate=0; GLOBAL.drawfromobj=-1; }
					InvalidateRect(hWnd,NULL,TRUE);
				}
				break;
			
			case WM_RBUTTONDOWN:
				{  
					int dx,dy,t,i;
					
		
					close_toolbox();
					actport=-1;
					if (GLOBAL.objstate==0)
					{
					  dx = LOWORD(lParam)+PX; 
					  dy = HIWORD(lParam)+PY; 
					  for( t=0; (t<GLOBAL.objects)&&(actobject==NULL);t++)
					  {
						 for (i=0;(i<objects[t]->outports)&&(actport==-1);i++)    // was an outport clicked ?
						 {
							if ((abs(dx - objects[t]->xPos - objects[t]->width)<CON_MAGNETIC)&&(abs(dy - (objects[t]->yPos+CON_START+i*CON_HEIGHT))<CON_MAGNETIC))
							{
								actobject=objects[t];			
								GLOBAL.showtoolbox=t;
								actport=i;
								display_toolbox(actobject->hDlg=CreateDialog(hInst, (LPCTSTR)IDD_OUTPORTBOX, ghWndStatusbox, (DLGPROC)OUTPORTDlgHandler));
								//InvalidateRect(hWnd,NULL,TRUE);
								break;
							}
						 }

						 for (i=0;(i<objects[t]->inports)&&(actport==-1);i++)    // was an inport clicked ?
						 {
							if ((abs(dx - objects[t]->xPos )<CON_MAGNETIC)&&(abs(dy - (objects[t]->yPos+CON_START+i*CON_HEIGHT))<CON_MAGNETIC))
							{
								actobject=objects[t];
								GLOBAL.showtoolbox=t;
								actport=i;
								display_toolbox(actobject->hDlg=CreateDialog(hInst, (LPCTSTR)IDD_INPORTBOX, ghWndStatusbox, (DLGPROC)INPORTDlgHandler));
								break;
							}
						 }

					     if ((actport==-1)&&(abs(dx - objects[t]->xPos - objects[t]->width/2)<objects[t]->width/2)&&(abs(dy - objects[t]->yPos - objects[t]->height/2)<objects[t]->height/2+CON_MAGNETIC))
						 {				// object right-clicked  -> activate its display window
							close_toolbox();
							actobject=objects[t];
							GLOBAL.showtoolbox=t;
							if (actobject)
							{
								actobject->make_dialog(); 
								if (actobject->displayWnd) 
									SetWindowPos(actobject->displayWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
							}
							
						 }

						 InvalidateRect(hWnd,NULL,TRUE);
					  }
					}
				}
				break;
			case WM_LBUTTONDOWN:
				{  
					int dx,dy,t,i,st,si,con_found=0,obj_found=0,min=1000,m;
					class BASE_CL * oldobject;

					oldobject=actobject;
					close_toolbox();
					if (GLOBAL.objstate==0)
					{
					  dx = LOWORD(lParam)+PX; 
					  dy = HIWORD(lParam)+PY;
					  if(oldobject)
					  
					  {
					    actobject=oldobject;
					    for (i=0;i<actobject->outports;i++)    
							                
						  if ((!con_found)&&(abs(dx - actobject->xPos - actobject->width)<CON_MAGNETIC)&&(abs(dy - (actobject->yPos+CON_START+i*CON_HEIGHT))<CON_MAGNETIC))
							{
							  int k;
							  GLOBAL.fx=actobject->xPos+actobject->width;
							  GLOBAL.fy=actobject->yPos+CON_START+i*CON_HEIGHT;
							  for (k=0;actobject->out[k].from_port!=-1;k++);
							  actobject->out[k+1].from_port=-1;
							  actobject->out[k].from_port=i;
							  for (t=0;(t<GLOBAL.objects)&&(objects[t]!=actobject);t++) ; 
							  actobject->out[k].from_object=t;
							  GLOBAL.tx=dx;
							  GLOBAL.ty=dy;
							  GLOBAL.actcon=k;
							  GLOBAL.objstate=2;
							  GLOBAL.drawfromobj=t;
							  con_found=1;
							  actconnect=NULL;
							}
					  }
					  if (!con_found)
					  {
					    for( t=0; t<GLOBAL.objects;t++)   // was an object clicked ?
						{

						  if ((abs(dx - objects[t]->xPos - objects[t]->width/2)<objects[t]->width/2)&&(abs(dy - objects[t]->yPos - objects[t]->height/2)<objects[t]->height/2)) 
						  {                            
							  

							obj_found=1;
							close_toolbox();
							actobject=objects[t];
							
							sx=actobject->xPos - (dx-actobject->width/2); 
							sy=actobject->yPos - (dy-CON_MAGNETIC); 
							GLOBAL.objstate=1;
							
							actconnect=NULL;
						  } 
						}
					  }
					  if ((!obj_found)&&(!con_found))     // was a connection clicked ?
					  { 
							close_toolbox();
							GLOBAL.objstate=0;
							min=1000;
							for(t=0;t<GLOBAL.objects;t++)
								for(i=0;objects[t]->out[i].to_port!=-1;i++)
								  if ((m=find_min_to_line(dx,dy,objects[t]->xPos+objects[t]->width,objects[t]->yPos+CON_START+objects[t]->out[i].from_port*CON_HEIGHT,
										objects[objects[t]->out[i].to_object]->xPos,objects[objects[t]->out[i].to_object]->yPos+CON_START+objects[t]->out[i].to_port*CON_HEIGHT))<min)
									{	min=m;	si=i;  st=t;	}
							
							if ((min<5)) 	              // yes:
							{
								
								actconnect=&(objects[st]->out[si]);
								display_toolbox(CreateDialog(hInst, (LPCTSTR)IDD_CONNECTBOX, ghWndStatusbox, (DLGPROC)CONNECTDlgHandler));

							}
					
					  } 

					  InvalidateRect(hWnd,NULL,TRUE);
					}
				}			
				break;

			case WM_LBUTTONDBLCLK:	
				if (actobject)
				{
					display_toolbox(actobject->hDlg=CreateDialog(hInst, (LPCTSTR)IDD_TAGBOX, ghWndStatusbox, (DLGPROC)TAGDlgHandler));
					InvalidateRect(hWnd,NULL,TRUE);
				}
				break;

			case WM_LBUTTONUP:
				{ 
				int connected=0;
				if( GLOBAL.objstate==2)        // dropping connection from an outport
				{
				  int t,i; 
				  struct LINKStruct * act;

				  for (t=0;t<GLOBAL.objects;t++)         // look for an inport
				  	for (i=0;i<objects[t]->inports;i++)
				 	  if ( (abs(GLOBAL.tx-objects[t]->xPos-CON_MAGNETIC/2)<CON_MAGNETIC) &&
					       (abs(GLOBAL.ty-(objects[t]->yPos+CON_START+i*CON_HEIGHT))<CON_MAGNETIC) &&
						   (t!=GLOBAL.drawfromobj)) 
					  {									// inport found -> link to port
						     act=&objects[GLOBAL.drawfromobj]->out[GLOBAL.actcon];

						     act->from_object=GLOBAL.drawfromobj;
						     act->to_object=t;
							 act->to_port=i;

							 act->min=objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_min;
							 act->max=objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_max;
							 strcpy(act->dimension,objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_dim);
							 strcpy(act->description,objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_desc);

							 if (objects[t]->in_ports[i].get_range)
							 {
								objects[t]->in_ports[i].in_min=act->min;
								objects[t]->in_ports[i].in_max=act->max;
								strcpy(objects[t]->in_ports[i].in_dim,objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_dim);
								strcpy(objects[t]->in_ports[i].in_desc,objects[GLOBAL.drawfromobj]->out_ports[act->from_port].out_desc);
							 }
							 objects[t]->update_inports();
							 							 
							 if (sort_objects())   connected=TRUE;
							 else
							 {
								 act->to_object=-1;
							     act->to_port=-1;
								 sort_objects();
							 }						 
							 update_dimensions();
							 t=GLOBAL.objects;break;
						}
									
				  if (!connected) objects[GLOBAL.drawfromobj]->out[GLOBAL.actcon].from_port=-1;
				 
				}
				GLOBAL.drawfromobj=-1;
				GLOBAL.objstate=0;
				InvalidateRect(hWnd,NULL,TRUE);
				}
				break;
		
		case WM_KEYDOWN:
			    if (wParam==KEY_DELETE )
				{
					int i,t,object_index;


					TIMING.pause_timer=1;
					if (actobject) //&&(actobject!=objects[0]))        // delete a whole object
					{
  		 		   	    write_logfile("deleting object: %s",objnames[actobject->type]);
						if (deviceobject==actobject) deviceobject=NULL;
						for (object_index=0;actobject!=objects[object_index];object_index++);

						//deleting all connections at this object's output ports
						//for array_data_ports
						for (i=MAX_CONNECTS-1;i>=0;i--){
							delete_connection(&objects[object_index]->out[i]);							
						}
						for(t=0;t<GLOBAL.objects;t++)						 
						  for (i=0;i<MAX_CONNECTS;i++)
						  {
							while (objects[t]->out[i].to_object==object_index)
							{
								memcpy(&objects[t]->out[i],&objects[t]->out[i+1],sizeof(LINKStruct)*(MAX_CONNECTS-i));
							    objects[t]->out[MAX_CONNECTS-1].to_object=-1;
								objects[t]->out[MAX_CONNECTS-1].from_port=-1;
								objects[t]->out[MAX_CONNECTS-1].to_port=-1;
							}
							if (objects[t]->out[i].to_object>object_index) 
								objects[t]->out[i].to_object--;
							if (objects[t]->out[i].from_object>object_index) 
								objects[t]->out[i].from_object--;
						  }
						close_toolbox();
						free_object(object_index);
						get_session_length();
						InvalidateRect(hWnd,NULL,TRUE);
					}
					else if (actconnect!=NULL)                      // delete a connection
					{
						int o,c;

					    o=actconnect->to_object;c=-1;
						for(;actconnect->to_port!=-1;actconnect++)
						  memcpy(actconnect,(void *)(actconnect+1),sizeof(struct LINKStruct));

						close_toolbox();
					    InvalidateRect(hWnd,NULL,TRUE);

					}
					for(t=0;t<GLOBAL.objects;t++) objects[t]->update_inports();
					update_dimensions();
					TIMING.pause_timer=0;
				}
 			    SetFocus(ghWndMain);
				break;
				
/*
SCROLLINFO { 
    UINT cbSize; 
    UINT fMask; 
    int  nMin; 
    int  nMax; 
    UINT nPage; 
    int  nPos; 
    int  nTrackPos; 
}   SCROLLINFO, *LPSCROLLINFO; 


The fMask member can be one or more of the following values.
SIF_PAGE
Copies the scroll page to the nPage member of the SCROLLINFO structure pointed to by lpsi.
SIF_POS
Copies the scroll position to the nPos member of the SCROLLINFO structure pointed to by lpsi.
SIF_RANGE
Copies the scroll range to the nMin and nMax members of the SCROLLINFO structure pointed to by lpsi.
SIF_TRACKPOS
Copies the current scroll box tracking position to the nTrackPos member of the SCROLLINFO structure pointed to by lpsi.

*/

		case WM_HSCROLL:
		    ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			switch (LOWORD(wParam))
			{
				case SB_THUMBPOSITION:
				case SB_THUMBTRACK:
				    si.fMask = SIF_TRACKPOS;
 				    if (GetScrollInfo(ghWndDesign, SB_HORZ, &si) )
					{			
						si.fMask = SIF_POS;
						si.nPos=si.nTrackPos;
						SetScrollInfo(ghWndDesign, SB_HORZ, &si,TRUE);
					}
					break;
				case SB_LINELEFT:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_HORZ, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos-10;
						SetScrollInfo(ghWndDesign, SB_HORZ, &si,TRUE);
					}
					break;
				case SB_LINERIGHT:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_HORZ, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos+10;
						SetScrollInfo(ghWndDesign, SB_HORZ, &si,TRUE);
					}
					break;
				case SB_PAGELEFT:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_HORZ, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos-100;
						SetScrollInfo(ghWndDesign, SB_HORZ, &si,TRUE);
					}
					break;
				case SB_PAGERIGHT:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_HORZ, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos+100;
						SetScrollInfo(ghWndDesign, SB_HORZ, &si,TRUE);
					}
					break;
			}
  		    InvalidateRect(hWnd,NULL,TRUE);
			break;
		case WM_VSCROLL:
		    ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			switch (LOWORD(wParam))
			{
				case SB_THUMBPOSITION:
				case SB_THUMBTRACK:
				    si.fMask = SIF_TRACKPOS;
 				    if (GetScrollInfo(ghWndDesign, SB_VERT, &si) )
					{			
						si.fMask = SIF_POS;
						si.nPos=si.nTrackPos;
						SetScrollInfo(ghWndDesign, SB_VERT, &si,TRUE);
					}
					break;
				case SB_LINEUP:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_VERT, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos-10;
						SetScrollInfo(ghWndDesign, SB_VERT, &si,TRUE);
					}
					break;
				case SB_LINEDOWN:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_VERT, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos+10;
						SetScrollInfo(ghWndDesign, SB_VERT, &si,TRUE);
					}
					break;
				case SB_PAGEUP:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_VERT, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos-100;
						SetScrollInfo(ghWndDesign, SB_VERT, &si,TRUE);
					}
					break;
				case SB_PAGEDOWN:
				    si.fMask = SIF_POS;
 				    if (GetScrollInfo(ghWndDesign, SB_VERT, &si) )
					{			
					    si.fMask = SIF_POS;
						si.nPos=si.nPos+100;
						SetScrollInfo(ghWndDesign, SB_VERT, &si,TRUE);
					}
					break;
			}
		    InvalidateRect(hWnd,NULL,TRUE);
			break;

		case WM_SIZE: 
			
		case WM_MOVE:
			{
  			  WINDOWPLACEMENT  wndpl;
			  GetWindowPlacement(hWnd, &wndpl);
			  GLOBAL.design_top=wndpl.rcNormalPosition.top;
			  GLOBAL.design_left=wndpl.rcNormalPosition.left;
			  GLOBAL.design_right=wndpl.rcNormalPosition.right;
			  GLOBAL.design_bottom=wndpl.rcNormalPosition.bottom;
			  InvalidateRect(hWnd,NULL,TRUE);
			}
			break;

		case WM_PAINT:
			if (!GLOBAL.loading)  draw_objects(hWnd);
            
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}


void add_to_listbox(HWND hDlg, int idc, char * str)
{
	char line[80];
	int x=0;

	while ((x<60)&&(str[x])&&(str[x]!=10)&&(str[x]!=13)) { line[x]=str[x]; x++; }
	line [x]=0;
	SendDlgItemMessage(hDlg,idc, LB_ADDSTRING, 0, (LPARAM) line);
	x=SendDlgItemMessage(hDlg,idc, LB_GETCOUNT, 0, 0);
	SendDlgItemMessage(hDlg,idc, LB_SETCURSEL, x-1, 0);
}

LONG get_sliderpos(LONG samplepos)
{
	    if (GLOBAL.session_length>0)
		  return ( (LONG)((float)samplepos/(float)GLOBAL.session_length*1000.0f));
		else return 0;
}

void update_status_window(void)
{
	WINDOWPLACEMENT  wndpl;
	int rm,lm,bm,bh;

	if (GLOBAL.session_length==0)  {
		lm=8;rm=16;
//		bm=48;bh=41;
		bm=GLOBAL.statusWindowMargin;
		bh=GLOBAL.statusWindowHeight;

	}
	else {
		lm=8;rm=16;
//		bm=83;bh=76;
		bm=GLOBAL.statusWindowMarginWithPlayer;
		bh=GLOBAL.statusWindowHeightWithPlayer;
	}

//	if (!GLOBAL.main_maximized)
	{
		RECT rc;
		GetWindowRect(ghWndMain, &rc); 

		// GetWindowPlacement(ghWndMain, &wndpl);
		SetWindowPos(ghWndMain,ghWndMain,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		if (GLOBAL.hidestatus)
//			SetWindowPos(ghWndStatusbox, ghWndMain, wndpl.rcNormalPosition.left+lm, wndpl.rcNormalPosition.bottom-bm, 
//             wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left-rm,bh, 0); //SWP_NOACTIVATE|SWP_NOZORDER);
			SetWindowPos(ghWndStatusbox, ghWndMain, rc.left+lm, rc.bottom-bm, rc.right-rc.left-rm,bh, 0); //SWP_NOACTIVATE|SWP_NOZORDER);
		else 
			 SetWindowPos(ghWndStatusbox, ghWndMain, rc.left+lm, rc.bottom-bm, rc.right-rc.left-rm,bh,SWP_SHOWWINDOW); //SWP_NOACTIVATE|SWP_NOZORDER);
//			 SetWindowPos(ghWndStatusbox, ghWndMain, wndpl.rcNormalPosition.left+lm, wndpl.rcNormalPosition.bottom-bm, 
//	         wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left-rm,bh,SWP_SHOWWINDOW); //SWP_NOACTIVATE|SWP_NOZORDER);
	}

/*	else	{
		if (GLOBAL.session_length==0)
  		     SetWindowPos(ghWndStatusbox, ghWndMain, 4, HIWORD(GLOBAL.main_maximized)+15, 
		               LOWORD(GLOBAL.main_maximized)-8,HIWORD(GLOBAL.main_maximized), 0);
		else SetWindowPos(ghWndStatusbox, ghWndMain, 4, HIWORD(GLOBAL.main_maximized)-80, 
		               LOWORD(GLOBAL.main_maximized)-8,HIWORD(GLOBAL.main_maximized), 0);
	}
	*/
}


void report(char * Message)
{
	write_logfile("INFO: %s",Message);
    if (!GLOBAL.loading) close_toolbox();
	ShowWindow(ghWndMain,FALSE);
	UpdateWindow(ghWndMain);
	write_logfile(Message);
    MessageBox(ghWndMain, Message, "BrainBay - Info", MB_OK|MB_SYSTEMMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
    UpdateWindow(ghWndMain);
    ShowWindow(ghWndMain,TRUE);
    UpdateWindow(ghWndMain);
}

void report_error(char * Message)
{
	write_logfile("ERROR: %s",Message);
    if (!GLOBAL.loading) close_toolbox();
	ShowWindow(ghWndMain,FALSE);
	UpdateWindow(ghWndMain);
    MessageBox(ghWndMain, Message, "BrainBay - Problem", MB_OK|MB_SYSTEMMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONWARNING);
	UpdateWindow(ghWndMain);
    ShowWindow(ghWndMain,TRUE);
    UpdateWindow(ghWndMain);
   //InvalidateRect(ghWndMain,NULL,TRUE);
}


void critical_error(char * Message)
{	
	write_logfile("CRITICAL ERROR: %s",Message);
    if (!GLOBAL.loading) close_toolbox();
	ShowWindow(ghWndMain,FALSE);
	UpdateWindow(ghWndMain);
    MessageBox(ghWndMain, Message, "BrainBay - Error", MB_OK|MB_SYSTEMMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONSTOP);
    ExitProcess(0);
}


