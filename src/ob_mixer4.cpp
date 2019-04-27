/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_MIXER4.CPP:  contains functions for the 4 Channel Mixer-Object
  Autohr: Chris Veigl

  The Mixer-Object can mix 4 input Signals into one output signal.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_mixer4.h"


void update_scrollpos(HWND hDlg, MIXER4OBJ * st)
{
	char sztemp[20];
		SetScrollPos(GetDlgItem(hDlg,IDC_CHN1BAR), SB_CTL,(int)(st->chn1vol*10.0f),TRUE);
		sprintf(sztemp,"%.1f",st->chn1vol);
		SetDlgItemText(hDlg, IDC_CHN1,sztemp);
		SetScrollPos(GetDlgItem(hDlg,IDC_CHN2BAR), SB_CTL,(int)(st->chn2vol*10.0f),TRUE);
		sprintf(sztemp,"%.1f",st->chn2vol);
		SetDlgItemText(hDlg, IDC_CHN2,sztemp);
		SetScrollPos(GetDlgItem(hDlg,IDC_CHN3BAR), SB_CTL,(int)(st->chn3vol*10.0f),TRUE);
		sprintf(sztemp,"%.1f",st->chn3vol);
		SetDlgItemText(hDlg, IDC_CHN3,sztemp);
		SetScrollPos(GetDlgItem(hDlg,IDC_CHN4BAR), SB_CTL,(int)(st->chn4vol*10.0f),TRUE);
		sprintf(sztemp,"%.1f",st->chn4vol);
		SetDlgItemText(hDlg, IDC_CHN4,sztemp);
}


LRESULT CALLBACK Mixer4DlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	char sztemp[20];
	MIXER4OBJ * st;
	
	st = (MIXER4OBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_MIXER4)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				SCROLLINFO lpsi;
				
				lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				
				lpsi.nMin=0; lpsi.nMax=5000;
				SetScrollInfo(GetDlgItem(hDlg,IDC_CHN1BAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_CHN2BAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_CHN3BAR),SB_CTL,&lpsi,TRUE);
				SetScrollInfo(GetDlgItem(hDlg,IDC_CHN4BAR),SB_CTL,&lpsi,TRUE);

				switch (st->invmode)
				{
					case 0: CheckDlgButton(hDlg,IDC_INVIGNORE,TRUE); break;
					case 1: CheckDlgButton(hDlg,IDC_INVAND,TRUE); break;
					case 2: CheckDlgButton(hDlg,IDC_INVOR,TRUE); break;
				}
				update_scrollpos(hDlg,st);

			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_SOLO1:
				    st->chn1vol=100.0f; st->chn2vol=0.0f;st->chn3vol=0.0f;st->chn4vol=0.0f;
					update_scrollpos(hDlg,st);
				break;
			case IDC_SOLO2:
				    st->chn2vol=100.0f; st->chn1vol=0.0f;st->chn3vol=0.0f;st->chn4vol=0.0f;
					update_scrollpos(hDlg,st);
				break;
			case IDC_SOLO3:
				    st->chn3vol=100.0f; st->chn1vol=0.0f;st->chn2vol=0.0f;st->chn4vol=0.0f;
					update_scrollpos(hDlg,st);
				break;
			case IDC_SOLO4:
				    st->chn4vol=100.0f; st->chn1vol=0.0f;st->chn2vol=0.0f;st->chn3vol=0.0f;
					update_scrollpos(hDlg,st);
				break;
			case IDC_INVIGNORE:
				    st->invmode=0;
				break;
			case IDC_INVAND:
				    st->invmode=1;
				break;
			case IDC_INVOR:
				    st->invmode=2;
				break;
			}
			return TRUE;
		case WM_HSCROLL:
		{
			int nNewPos; 
			nNewPos=get_scrollpos(wParam, lParam);
		    {
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CHN1BAR))  
			  {   
				  sprintf(sztemp,"%.2f",nNewPos/10.0f);
				  SetDlgItemText(hDlg, IDC_CHN1,sztemp);
			      st->chn1vol=(float)nNewPos/10.0f;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CHN2BAR))  
			  {   
				  sprintf(sztemp,"%.2f",nNewPos/10.0f);
				  SetDlgItemText(hDlg, IDC_CHN2,sztemp);
			      st->chn2vol=(float)nNewPos/10.0f;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CHN3BAR))  
			  {   
				  sprintf(sztemp,"%.2f",nNewPos/10.0f);
				  SetDlgItemText(hDlg, IDC_CHN3,sztemp);
			      st->chn3vol=(float)nNewPos/10.0f;
			  }
			  if (lParam == (long) GetDlgItem(hDlg,IDC_CHN4BAR))  
			  {   
				  sprintf(sztemp,"%.2f",nNewPos/10.0f);
				  SetDlgItemText(hDlg, IDC_CHN4,sztemp);
			      st->chn4vol=(float)nNewPos/10.0f;
			  }
		  
			}
		
		} break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}



MIXER4OBJ::MIXER4OBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 4;
	strcpy(out_ports[0].out_name,"out");
	input1=0;	input2=0;	input3=0;	input4=0;
	invmode=1;
	chn1vol=100.0f;	chn2vol=100.0f;	chn3vol=100.0f;	chn4vol=100.0f;
}
	
void MIXER4OBJ::make_dialog(void) 
{
  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_MIXER4BOX, ghWndStatusbox, (DLGPROC)Mixer4DlgHandler));

}

void MIXER4OBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("chn1vol",P_FLOAT,&chn1vol);
    load_property("chn2vol",P_FLOAT,&chn2vol);
    load_property("chn3vol",P_FLOAT,&chn3vol);
    load_property("chn4vol",P_FLOAT,&chn4vol);
	load_property("invmode",P_INT,&invmode);

}

void MIXER4OBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"chn1vol",P_FLOAT,&chn1vol);
    save_property(hFile,"chn2vol",P_FLOAT,&chn2vol);
    save_property(hFile,"chn3vol",P_FLOAT,&chn3vol);
    save_property(hFile,"chn4vol",P_FLOAT,&chn4vol);
	save_property(hFile,"invmode",P_INT,&invmode);
}
	
void MIXER4OBJ::incoming_data(int port, float value)
{
	switch (port) 
	{
		case 0: input1 = value; break;
		case 1: input2 = value; break;
		case 2: input3 = value; break;
		case 3: input4 = value; break;
	}
}
	
void MIXER4OBJ::work(void)
{
	int valid =0;
	float result=0;

	if (input1 != INVALID_VALUE)
	{  valid++;
	   result+=input1*chn1vol/100.0f;
	}
	if (input2 != INVALID_VALUE)
	{  valid++;
	   result+=input2*chn2vol/100.0f;
	}
	if (input3 != INVALID_VALUE)
	{  valid++;
	   result+=input3*chn3vol/100.0f;
	}
	if (input4 != INVALID_VALUE)
	{  valid++;
	   result+=input4*chn4vol/100.0f;
	}

	if (invmode==1)
		if ( ((chn1vol!=0) && (input1==INVALID_VALUE)) &&
			((chn2vol!=0) && (input2==INVALID_VALUE)) &&
			((chn3vol!=0) && (input3==INVALID_VALUE)) &&
			((chn4vol!=0) && (input4==INVALID_VALUE)) )  result=INVALID_VALUE;

	if (invmode==2)
		if ( ((chn1vol!=0) && (input1==INVALID_VALUE)) ||
			((chn2vol!=0) && (input2==INVALID_VALUE)) ||
			((chn3vol!=0) && (input3==INVALID_VALUE)) ||
			((chn4vol!=0) && (input4==INVALID_VALUE)) )  result=INVALID_VALUE;


//	if (valid) result/=valid;

	pass_values(0, result);
}
	
MIXER4OBJ::~MIXER4OBJ() {}
