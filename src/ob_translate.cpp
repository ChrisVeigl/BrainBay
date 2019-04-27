/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_TRANSLATE.CPP:  contains functions for the Translation-Object
  Author: Chris Veigl

  This object is not finished yet. It shall provide mapping for signals
  Now, it can do only gaining and shifting of the signal.

  TranslateDlgHandler: processes events of the toolbox window


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_translate.h"

int top= 60;
int left=50;
int bottom=410;
int right= 470;

LRESULT CALLBACK TranslateDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	
	char sztemp[20];
	TRANSLATEOBJ * st;
	
	st = (TRANSLATEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_TRANSLATE)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			sprintf(sztemp,"%.2f",st->in_ports[0].in_min);
			SetDlgItemText(hDlg, IDC_IN_MIN, sztemp);
			sprintf(sztemp,"%.2f",st->in_ports[0].in_max);
			SetDlgItemText(hDlg, IDC_IN_MAX, sztemp);
			sprintf(sztemp,"%.2f",st->out_ports[0].out_min);
			SetDlgItemText(hDlg, IDC_OUT_MIN, sztemp);
			sprintf(sztemp,"%.2f",st->out_ports[0].out_max);
			SetDlgItemText(hDlg, IDC_OUT_MAX, sztemp);
			
			return TRUE;
		case WM_CLOSE:		
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_APPLY:
				st->in_ports[0].get_range=-1;
				st->out_ports[0].get_range=-1;
				GetDlgItemText(hDlg,IDC_IN_MIN,sztemp,sizeof(sztemp));
				sscanf(sztemp,"%f",&st->in_ports[0].in_min);
				GetDlgItemText(hDlg,IDC_IN_MAX,sztemp,sizeof(sztemp));
				sscanf(sztemp,"%f",&st->in_ports[0].in_max);
				GetDlgItemText(hDlg,IDC_OUT_MIN,sztemp,sizeof(sztemp));
				sscanf(sztemp,"%f",&st->out_ports[0].out_min);
				GetDlgItemText(hDlg,IDC_OUT_MAX,sztemp,sizeof(sztemp));
				sscanf(sztemp,"%f",&st->out_ports[0].out_max);
				break;

			}
			return TRUE;
		
    
	 	case WM_LBUTTONUP: st->setpoint=-1;
				break;
 	 	case WM_LBUTTONDOWN:
			{  int minpoint,i,z;
			   float actdist,mindist,actx,acty;

				  st->actmousex=(int)LOWORD(lParam);
				  st->actmousey=(int)HIWORD(lParam);
				  if ((st->actmousex<left)||(st->actmousex>right)||
					  (st->actmousey>bottom)||(st->actmousey<top)) break;
				  
				  mindist=10000;minpoint=-1;
				  actx=(float)(st->actmousex-left)/(right-left)*1023.0f;
				  acty=(float)(bottom-st->actmousey)/(bottom-top);

				  for (i=0;i<st->points;i++)
				  {
					  
				       actdist = (st->pointx[i]-actx)*(st->pointx[i]-actx)
						   + (st->pointy[i]-acty)*(st->pointy[i]-acty);
				
					   if (actdist<mindist) { mindist=actdist; minpoint=i; }

				  }

				  if (mindist<300) st->setpoint=minpoint; 
				  else 
				  { 
					 if (st->points>=MAX_TRANSLATIONPOINTS-1) break;
					
					 i=0;while ((st->pointx[i]<actx)&&(i<st->points)) i++;
					 for (z=st->points;z>i;z--)
					 {
					    st->pointx[z]=st->pointx[z-1];
					    st->pointy[z]=st->pointy[z-1];
					 }
					 st->pointx[z]=(int)actx;
					 st->pointy[z]=acty;
					 st->points++;
					 InvalidateRect(hDlg,NULL,TRUE);
					 st->setpoint=z;
				  }
			}

		case WM_MOUSEMOVE:
			{ 
				int i;
				st->actmousex=(int)LOWORD(lParam);
				st->actmousey=(int)HIWORD(lParam);
				  
				if (st->setpoint!=-1)
				{
					if ((st->actmousex<left)||(st->actmousex>right)||
					   (st->actmousey>bottom)||(st->actmousey<top))
					{
						if (st->points<=2) break;
						for (i=st->setpoint;i<st->points;i++)
						{
							st->pointx[i]=st->pointx[i+1];
							st->pointy[i]=st->pointy[i+1];
						}
						st->points--;
						st->pointx[0]=0;st->pointx[st->points-1]=1023;
						st->setpoint=-1;
					}
					else
					{
						st->pointx[st->setpoint]=(int)((st->actmousex-left)/(float)(right-left)*1023.0f);
						st->pointy[st->setpoint]=(float)(bottom-st->actmousey)/(bottom-top);
						if (st->setpoint==0) st->pointx[0]=0;
						if (st->setpoint==st->points-1) st->pointx[st->points-1]=1023;
					}
					st->calculate_map();
					sprintf(sztemp,"%.4f",((float)(st->pointx[st->setpoint])/1023*(st->in_ports[0].in_max-st->in_ports[0].in_min)+st->in_ports[0].in_min));
					SetDlgItemText(hDlg,IDC_ACT_INPUT,sztemp);

					sprintf(sztemp,"%.4f",st->pointy[st->setpoint]*(st->out_ports[0].out_max-st->out_ports[0].out_min)+st->out_ports[0].out_min);
					SetDlgItemText(hDlg,IDC_ACT_OUTPUT,sztemp);
 					InvalidateRect(hDlg,NULL,TRUE);
				}
			}
			break;


		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;
				RECT rect;
				HPEN tpen;
				HBRUSH tbrush;
				int i;
				float dx,dy;

				hdc = BeginPaint (hDlg, &ps);
				GetClientRect(hDlg, &rect);
				top=(int)rect.top+50;
				bottom=(int)rect.bottom-50;
				left=(int)rect.left+85;
				right=(int)rect.right-20;


				tpen    = CreatePen (PS_SOLID,3,50);
				SelectObject (hdc, tpen);
				SelectObject (hdc, DRAW.brush_ltorange);
				
				tbrush = CreateSolidBrush(PALETTERGB(240,240,240));
				SelectObject (hdc, tbrush);
				
				Rectangle(hdc,left,top,right,bottom);
				
				dx=(float)(right-left)/1023.0f;
				dy=(float)(bottom-top);

				DeleteObject(tpen);
				tpen = CreatePen (PS_SOLID,2,PALETTERGB(180,0,0));
				SelectObject (hdc, tpen);

				for (i=0;i<st->points-1;i++)
				{
					MoveToEx(hdc,left+ (int)((float)st->pointx[i]*dx),bottom - (int)(st->pointy[i]*dy),NULL);
					LineTo(hdc,left + (int)((float)st->pointx[i+1]*dx),bottom - (int)(st->pointy[i+1]*dy));
				}
				DeleteObject(tbrush);
				DeleteObject(tpen);
				EndPaint(hDlg, &ps );
			} 
			break;

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
			break;
		
	
	}
    return FALSE;
}





//
//  Object Implementation
//


TRANSLATEOBJ::TRANSLATEOBJ(int num) : BASE_CL()	
{

	outports = 1;
	inports = 1;
	width=70;

	strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");

	points=2;
	pointx[0]=0;
	pointy[0]=0;
	pointx[1]=1023;
	pointy[1]=1;

	setpoint=-1;
	calculate_map();
	
}
	

void TRANSLATEOBJ::calculate_map(void) 
{  
	int t,i,dist,actpos;
	float stepx,stepy,actval;
	
	stepx=1.0f/1023.0f;
	

	for (t=0;t<points-1;t++)
	{
		actpos=pointx[t];
		dist=pointx[t+1]-pointx[t];	
		
		actval=pointy[t];
		stepy=(pointy[t+1]-pointy[t])/(float)dist;
		
		for (i=0;i<dist;i++)
		{
			map[actpos+i]=actval;
			actval+=stepy;
		}

	}
  
}
void TRANSLATEOBJ::make_dialog(void) 
{  
	  display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_TRANSLATEBOX, ghWndStatusbox, (DLGPROC)TranslateDlgHandler)); 
}

void TRANSLATEOBJ::load(HANDLE hFile) 
{
	int t;
	char sztemp[20];

	load_object_basics(this);
	load_property("points",P_INT,&points);
	for (t=0;t<points;t++)
	{
		wsprintf(sztemp,"pointx%d",t);
		load_property(sztemp,P_INT,&pointx[t]);
		wsprintf(sztemp,"pointy%d",t);
		load_property(sztemp,P_FLOAT,&pointy[t]);
	}
	calculate_map();
	
}
		
void TRANSLATEOBJ::save(HANDLE hFile) 
{	
	int t;
	char sztemp[20];

	save_object_basics(hFile, this);
  
  	save_property(hFile,"points",P_INT,&points);
	for (t=0;t<points;t++)
	{
		wsprintf(sztemp,"pointx%d",t);
		save_property(hFile,sztemp,P_INT,&pointx[t]);
		wsprintf(sztemp,"pointy%d",t);
		save_property(hFile,sztemp,P_FLOAT,&pointy[t]);
	}
}

void TRANSLATEOBJ::incoming_data(int port, float value) { input= value; }
	  
void TRANSLATEOBJ::work(void) 
{
	int index;

	if (input==INVALID_VALUE) {pass_values(0,INVALID_VALUE); return; }
	if (in_ports[0].in_max!=in_ports[0].in_min)
	 index=(int)((input-in_ports[0].in_min)/(in_ports[0].in_max-in_ports[0].in_min)*1023.0f);
	if (index<0) index=0;
	if(index>1023) index=1023;
	pass_values(0,map[index]*(out_ports[0].out_max-out_ports[0].out_min)+out_ports[0].out_min);
	
}


TRANSLATEOBJ::~TRANSLATEOBJ()
{
	// free object
}  


