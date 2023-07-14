/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_FILTER.CPP:  contains assisting functions for the Filter-Object
  Authors: Jim Peters, Chris Veigl


  This module calls into the fid-lib filter library by Jim Peters to provide
  filter functionalities. The Filter-Type and order can be selected.
  A brief preview of the filter's frequency-response is shown in the 
  filter-toolbox-window. For a better display check out Jim Peters fiview-tool:
  http://uazu.net/fiview

  do_filt_design: Initialises a new filter using the init-String 
      (for example LpBe for a low-pass - bessel filter, see fiview-documentation)
	  and additional filter-parameters (like order, from-, to-frequency)
  update_filterdialog:  enables/disables init-parameters according to the filter-type
  FilterBoxDlgHandler: processes events for the filter-toolbox window.

  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.


-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_filter.h"

struct FILTERTYPEStruct     FILTERTYPE[FILTERTYPES]=
{{"BandStop Resonator","BsRe/",1},
{"LowPass Bessel","LpBe",1},
{"HighPass Bessel","HpBe",1},
{"BandPass Bessel","BpBe",2},
{"BandStop Bessel","BsBe",2},
{"LowPass Butterworth","LpBu",1},
{"HighPass Butterworth","HpBu",1},
{"BandPass Butterworth","BpBu",2},
{"BandStop Butterworth","BsBu",2}}; 


/*    // I had problems with Chebyshev Filters (filter setup crashed) ..
	
"LowPass Chebyshev","LpCh"
"HighPass Chebyshev","HpCh" 
"BandPass Chebyshev","BpCh"
"BandStop Chebyshev", "BsCh"
	
*/




int test_filterparams(int type,int p0,float p1,float p2)
{
	if ((type<0)||(type>=FILTERTYPES)) return FALSE;
	if (p0<1) return FALSE;
	if ((p1<0)||(p1>=(PACKETSPERSECOND/2))) return FALSE;
	if ((p2<0)||(p2>=(PACKETSPERSECOND/2))) return FALSE;

	switch (type)
	{
		case 0: if (p0>=100) return FALSE; 
			    break; 
  		case 1:
		case 2:
		case 3:  
		case 4: if (p0>=11) return FALSE; 
			    break;
		case 5:
		case 6: 
		case 7: if (p0>=60) return FALSE; 
			    break;
		case 8: if (p0>=30) return FALSE;
	}

	if ((type==3) || (type==7))
	{	if (p2<=p1) return FALSE;}

	return (TRUE);
}

FidFilter * do_filt_design(HWND hDlg, int ftype)
{
	char sztemp[30];
	char szorder[5];
	int p0;
	float p1,p2;

    BOOL trans;

	p0=GetDlgItemInt(hDlg, IDC_FILTERPAR0, &trans, 0); if (!trans) p0=0;

	GetDlgItemText(hDlg,IDC_FILTERPAR1,sztemp,sizeof(sztemp)); 
	sscanf(sztemp,"%f",&p1);
	GetDlgItemText(hDlg,IDC_FILTERPAR2,sztemp,sizeof(sztemp)); 
	sscanf(sztemp,"%f",&p2);

	if (test_filterparams(ftype,p0,p1,p2)) 
	{
	    strcpy(sztemp,FILTERTYPE[ftype].init);
	    GetDlgItemText(hDlg, IDC_FILTERPAR0,  szorder, sizeof(szorder));
		strcat(sztemp,szorder);
		write_logfile("filterpars: %s,%i,%f,%f ", sztemp,PACKETSPERSECOND,p1,p2);
        return (fid_design(sztemp, PACKETSPERSECOND, p1,p2,0,0));
	}
	return (NULL);
}

void update_filterdialog(HWND hDlg, int filternum)
{
	if (filternum>=0) 
	{
					switch (FILTERTYPE[filternum].param)  {
					case 1 : SetDlgItemText(hDlg, IDC_PAR0CAPT, "Filter Order");
							 SetDlgItemText(hDlg, IDC_PAR1CAPT, "Freq (Hz)");
							 SetDlgItemText(hDlg, IDC_PAR2CAPT, "");
							 EnableWindow(GetDlgItem(hDlg, IDC_FILTERPAR2), FALSE);
							break;
					case 2 :SetDlgItemText(hDlg, IDC_PAR0CAPT, "Filter Order");
							SetDlgItemText(hDlg, IDC_PAR1CAPT, "from (Hz)");
							SetDlgItemText(hDlg, IDC_PAR2CAPT, "to (Hz)");
							EnableWindow(GetDlgItem(hDlg, IDC_FILTERPAR2), TRUE);
						break;
							}
	}

}






/*-----------------------------------------------------------------------------

FUNCTION: CALLBACK FilterboxDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )

PURPOSE: Handles Messages for Filteredit Dialog

PARAMETERS:
    hDlg - Dialog window handle

-----------------------------------------------------------------------------*/

LRESULT CALLBACK FilterboxDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	int t; // ,z;
	char newname[25],sztemp[30];
	static int acttype;
	static int dinit=FALSE;
	static FidFilter * tempf=NULL,* newf=NULL;
    
	FILTEROBJ * st;
	
	st = (FILTEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_FILTER)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
				dinit=TRUE;
				SendMessage(GetDlgItem(hDlg, IDC_FILTERTYPECOMBO), CB_RESETCONTENT,0,0);
			    for (t = 0; t < FILTERTYPES; t++) 
					SendMessage( GetDlgItem(hDlg, IDC_FILTERTYPECOMBO), CB_ADDSTRING, 0,  (LPARAM) (LPSTR) FILTERTYPE[t].tname) ;

				SetDlgItemText(hDlg,IDC_FILTERTYPECOMBO, FILTERTYPE[st->filtertype].tname);
				SetDlgItemText(hDlg,IDC_FILTERNEWNAME, st->name);
				SetDlgItemInt(hDlg,IDC_FROMFREQ, st->dispfrom,0);
				SetDlgItemInt(hDlg,IDC_TOFREQ, st->dispto,0);
				SetDlgItemInt(hDlg,IDC_FILTERPAR0, st->par0,0);
				sprintf(sztemp,"%.5f",st->par1);
				SetDlgItemText(hDlg,IDC_FILTERPAR1, sztemp);
				sprintf(sztemp,"%.5f",st->par2);
				SetDlgItemText(hDlg,IDC_FILTERPAR2, sztemp);
				acttype=st->filtertype;
				dinit=FALSE;
				newf=do_filt_design(hDlg,acttype);
				if (newf) tempf=newf;
				update_filterdialog(hDlg,st->filtertype);				
				return TRUE;
		case WM_CLOSE:		
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			
			case IDC_FILTERTYPECOMBO:
				  acttype=SendMessage( GetDlgItem(hDlg, IDC_FILTERTYPECOMBO), CB_GETCURSEL, 0, 0 ) ;
				  update_filterdialog(hDlg,acttype);
			case IDC_FILTERPAR0:
			case IDC_FILTERPAR1:
			case IDC_FILTERPAR2:
			case IDC_FROMFREQ:
			case IDC_TOFREQ:
				if (!dinit)
				{
					newf=do_filt_design(hDlg,acttype);
					if (newf) tempf=newf;
 					InvalidateRect(hDlg,NULL,TRUE);
				}
				break;
			case IDC_FILTERSTORE:
				if (newf)
				{
					GetDlgItemText(hDlg, IDC_FILTERNEWNAME,newname,sizeof(newname));
				
					st->filtertype=acttype;
					st->par0=GetDlgItemInt(hDlg,IDC_FILTERPAR0, NULL, 0);
					GetDlgItemText(hDlg,IDC_FILTERPAR1,sztemp,sizeof(sztemp)); 
					sscanf(sztemp,"%f",&st->par1);
					GetDlgItemText(hDlg,IDC_FILTERPAR2,sztemp,sizeof(sztemp));
					st->dispfrom=GetDlgItemInt(hDlg, IDC_FROMFREQ, NULL, 0);
					st->dispto=GetDlgItemInt(hDlg, IDC_TOFREQ, NULL, 0);
					sscanf(sztemp,"%f",&st->par2);
					strcpy(st->name,newname);

					st->filt=do_filt_design(hDlg, acttype);
					st->run= fid_run_new(st->filt, &(st->funcp));
					if (st->fbuf!=NULL)
					{
						fid_run_freebuf(st->fbuf);
   						st->fbuf=fid_run_newbuf(st->run);
					}
				}
				break;
				}
				return(TRUE);
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;
				RECT rect;
				HPEN	 tpen;
				HBRUSH	 tbrush;
				int height;
				int f1,f2;
				float fstep,val,x;

				hdc = BeginPaint (hDlg, &ps);
				GetClientRect(hDlg, &rect);
				tpen    = CreatePen (PS_SOLID,1,0);
				SelectObject (hdc, tpen);
				tbrush  = CreateSolidBrush(RGB(240,240,240));
				SelectObject(hdc,tbrush);
				rect.top+=80;
				rect.bottom -= 18;
				height= rect.bottom-rect.top;
				Rectangle(hdc,rect.left,rect.top-1,rect.right,rect.bottom+20);
				Rectangle(hdc,rect.left,rect.top-1,rect.right,rect.bottom);
				Rectangle(hdc,rect.left,rect.bottom-(int)(height/1.3),rect.right,rect.bottom);
				
				DeleteObject(tbrush);
				DeleteObject(tpen);

				tpen = CreatePen (PS_SOLID,1,RGB(0,100,0));
				SelectObject (hdc, tpen);
				f1=GetDlgItemInt(hDlg, IDC_FROMFREQ, NULL, 0);
				f2=GetDlgItemInt(hDlg, IDC_TOFREQ, NULL, 0);
				fstep=(float)(f2-f1)/(rect.right-rect.left);
				MoveToEx(hdc,rect.left+1,rect.bottom-(int)(height*fid_response(tempf, (float)f1/256.0)/1.3),NULL);
				for (t=rect.left; t<rect.right; t++)
				{ 
					MoveToEx(hdc,1+t,rect.bottom,NULL);
					LineTo(hdc,1+t,rect.bottom-(int)(height*fid_response(tempf, (((float)f1+fstep*(t-rect.left))/PACKETSPERSECOND))/1.3));
				}
				SelectObject(hdc, DRAW.scaleFont);
				wsprintf(sztemp,"1.0"); 
				ExtTextOut( hdc, rect.left+2,rect.top+(int)(height*0.2308), 0, &rect,sztemp, strlen(sztemp), NULL ) ;
				val=(f2-f1)/10.0f;
				fstep=((rect.right-25)-rect.left)/10.0f;
				for (t=0; t<=10; t++)
				{ 
					x=f1+val*t;
					wsprintf(sztemp,"%d.%d",(int)x,(int)(x*10)%10); 
					ExtTextOut( hdc, rect.left+2+(int)(fstep*t),rect.bottom+2, 0, &rect,sztemp, strlen(sztemp), NULL ) ;
				}
				DeleteObject(tpen);
				EndPaint(hDlg, &ps );
				}
				break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
    return FALSE;
}



//
//  Object Implementation
//


FILTEROBJ::FILTEROBJ(int num) : BASE_CL()	
	  {
	    outports = 1;
		inports = 1;
		strcpy(in_ports[0].in_name,"in");
		strcpy(out_ports[0].out_name,"out");

		filtertype=3;
		par0=8;
		par1=8;
		par2=12;
		dispfrom=4;
		dispto=15;
		filt= fid_design("BpBe8", PACKETSPERSECOND, 8, 12, 0, 0);
		run= fid_run_new(filt, &(funcp));
		fbuf=fid_run_newbuf(run);
		strcpy(name,"Alpha Bessel");
	  }
	void FILTEROBJ::session_start(void)
	{
		if (fbuf==NULL)	// fid_run_freebuf(fbuf);
		fbuf=fid_run_newbuf(run);
	}
	void FILTEROBJ::session_reset(void)
	{
		if (fbuf!=NULL)  fid_run_freebuf(fbuf);
		fbuf=fid_run_newbuf(run);
	}
	void FILTEROBJ::session_pos(long pos)
	{
		if (fbuf!=NULL)	 fid_run_freebuf(fbuf);
		fbuf=fid_run_newbuf(run);
	}

	  void FILTEROBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_FILTERBOX, ghWndStatusbox, (DLGPROC)FilterboxDlgHandler));
	  }
	  
	  void FILTEROBJ::load(HANDLE hFile) 
	  {	
		char sztemp[30];
		char szorder[5];

		  load_object_basics(this);
		  load_property("name",P_STRING,name);
		  load_property("type",P_INT,&filtertype);
		  load_property("order",P_INT,&par0);
		  load_property("display-from",P_INT,&dispfrom);
		  load_property("display-to",P_INT,&dispto);
		  load_property("par1",P_FLOAT,&par1);
		  load_property("par2",P_FLOAT,&par2);
		
		  if (fbuf!=NULL) fid_run_freebuf(fbuf);
		  if (filt!=NULL) fid_run_free(filt);
 		  strcpy(sztemp,FILTERTYPE[filtertype].init);
		  wsprintf(szorder,"%d",par0);
		  strcat(sztemp,szorder);			
		  filt=fid_design(sztemp, PACKETSPERSECOND, par1, par2, 0, 0);
		  run= fid_run_new(filt, &(funcp));
		  fbuf=fid_run_newbuf(run);
	  }

	  void FILTEROBJ::save(HANDLE hFile) 
	  {	  
		  save_object_basics(hFile, this);
	  	  save_property(hFile,"name",P_STRING,name);
		  save_property(hFile,"type",P_INT,&filtertype);
		  save_property(hFile,"display-from",P_INT,&dispfrom);
		  save_property(hFile,"display-to",P_INT,&dispto);
		  save_property(hFile,"order",P_INT,&par0);
		  save_property(hFile,"par1",P_FLOAT,&par1);
		  save_property(hFile,"par2",P_FLOAT,&par2);

	  }

	  void FILTEROBJ::incoming_data(int port, float value)  {	input=value; }

	  void FILTEROBJ::work(void) 
	  {  float x; 

		 x=(float)(funcp(fbuf,(double)input));
//		 if ((filtertype==2)||(filtertype==3)||(filtertype==6)||(filtertype==7)) x+=512.0f;
	     pass_values(0,x); 
	  }


FILTEROBJ::~FILTEROBJ()
	  {
			if (fbuf!=NULL)	 fid_run_freebuf(fbuf);
			if (filt!=NULL)  fid_run_free(filt);
	  }  

