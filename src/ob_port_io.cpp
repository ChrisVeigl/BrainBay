/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_PORTIO.CPP
  Authors: Craig Peacock, Chris Veigl


  This Object performs calls to the Beyond Logic Port Talk I/O Port Driver
  by Craig Peacock (http://www.beyondlogic.org) to allow I/O access to
  the Parallel Port. In the context of BrainBay, this object could be used 
  to control external events with Biosignal Parameters. 

  For reuse of the PortTalk driver source pt_ioctl.c refer to the README.txt
  document in ./portalk  

  
-------------------------------------------------------------------------------------*/
  
#include "brainBay.h"
#include "ob_port_io.h"  
#include "porttalk/pt_ioctl.c" 


void update_bitpositions(HWND hDlg, unsigned char portval)
{
	CheckDlgButton(hDlg, IDC_PD7, (portval & 128 ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD6, (portval & 64  ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD5, (portval & 32  ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD4, (portval & 16  ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD3, (portval & 8   ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD2, (portval & 4   ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD1, (portval & 2   ? TRUE : FALSE));
	CheckDlgButton(hDlg, IDC_PD0, (portval & 1   ? TRUE : FALSE));
}

LRESULT CALLBACK PortDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	PORTOBJ * st;
	char tmp[20];
	
	st = (PORTOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_PORT_IO)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_RESETCONTENT,0,0);
			    SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) " LPT1 (378h)") ;
				SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) " LPT2 (278h)") ;
				SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) " LPT3 (3BCh)") ;
				if (st->portaddress==0x378) SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_SETCURSEL, 0, 0L ) ;
				if (st->portaddress==0x278) SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_SETCURSEL, 1, 0L ) ;
				if (st->portaddress==0x3bc) SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_SETCURSEL, 2, 0L ) ;

				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_RESETCONTENT,0,0);
				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) "no periodic updates") ;
				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) "trigger updates") ;
				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) "bitwise updates") ;
				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_ADDSTRING, 0,  (LPARAM) (LPSTR) "meter updates") ;
				SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_SETCURSEL, st->triggermode, 0L ) ;

				SetDlgItemInt(hDlg,IDC_VAL0, st->val0,0);
				SetDlgItemInt(hDlg,IDC_VAL1, st->val1,0);
				sprintf(tmp,"%04X",st->portaddress);
				SetDlgItemText(hDlg,IDC_PORTADDRESS, tmp);

				update_bitpositions(hDlg, st->portval);
				
				break;		

		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_PORTADDRESSCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
				     switch (SendDlgItemMessage(hDlg, IDC_PORTADDRESSCOMBO, CB_GETCURSEL, 0, 0 ))
					 {
						case 0: st->portaddress=0x378; break;
						case 1: st->portaddress=0x278; break;
						case 2: st->portaddress=0x3bc; break;
					 }
					 sprintf(tmp,"%04X",st->portaddress);
		 			 SetDlgItemText(hDlg,IDC_PORTADDRESS, tmp);
					}
				  break;

				case IDC_PORTMODECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					  st->triggermode= SendDlgItemMessage(hDlg, IDC_PORTMODECOMBO, CB_GETCURSEL, 0, 0 );
					  
					break;
				case IDC_VAL0:
					 st->val0=GetDlgItemInt(hDlg,IDC_VAL0,NULL,0);
					break;
				case IDC_VAL1:
					 st->val1=GetDlgItemInt(hDlg,IDC_VAL1,NULL,0);
					break;

				case IDC_PD7:
					st->portval^=128; 
					outportb (st->portaddress,st->portval);
					break;
				case IDC_PD6:
					st->portval^=64;
					outportb (st->portaddress,st->portval);
					break;
				case IDC_PD5:
					st->portval^=32;
					outportb (st->portaddress,st->portval);
                    break;
				case IDC_PD4:
					st->portval^=16;
					outportb (st->portaddress,st->portval);
                    break;
				case IDC_PD3:
					st->portval^=8;
					outportb (st->portaddress,st->portval);
                    break;
				case IDC_PD2:
					st->portval^=4;
					outportb (st->portaddress,st->portval);
                    break;
				case IDC_PD1:
					st->portval^=2;
					outportb (st->portaddress,st->portval);
                    break;
				case IDC_PD0:
					st->portval^=1;
					outportb (st->portaddress,st->portval);
                    break;

				case IDC_WRITEVAL0:
					outportb (st->portaddress,st->val0);
					st->portval=st->val0;
					update_bitpositions(hDlg, st->portval);
					break;
				case IDC_WRITEVAL1:
					outportb (st->portaddress,st->val1);
					st->portval=st->val1;
					update_bitpositions(hDlg, st->portval);
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


PORTOBJ::PORTOBJ(int num) : BASE_CL()
{
	outports = 0;
	inports = 9;
	strcpy(in_ports[0].in_name,"trigger");
	strcpy(in_ports[1].in_name,"D0");
	strcpy(in_ports[2].in_name,"D1");
	strcpy(in_ports[3].in_name,"D2");
	strcpy(in_ports[4].in_name,"D3");
	strcpy(in_ports[5].in_name,"D4");
	strcpy(in_ports[6].in_name,"D5");
	strcpy(in_ports[7].in_name,"D6");
	strcpy(in_ports[8].in_name,"D7");

	triggermode=0;
	old_trigger=trigger=INVALID_VALUE;
	val0=0;val1=255;
	portval=0;

	portaddress=0x378;
	OpenPortTalk();
	
}

	
void PORTOBJ::make_dialog(void) 
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_PORTBOX, ghWndStatusbox, (DLGPROC)PortDlgHandler));
}

void PORTOBJ::load(HANDLE hFile) 
{  
	load_object_basics(this);
	load_property("triggermode",P_INT,&triggermode);
	load_property("val0",P_INT,&val0);
	load_property("val1",P_INT,&val1);
	load_property("portaddress",P_INT,&portaddress);
	load_property("portval",P_INT,&portval);
	
}

void PORTOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"triggermode",P_INT,&triggermode);
	save_property(hFile,"val0",P_INT,&val0);
	save_property(hFile,"val1",P_INT,&val1);
	save_property(hFile,"portaddress",P_INT,&portaddress);
	save_property(hFile,"portval",P_INT,&portval);
	
}
	
void PORTOBJ::incoming_data(int port, float value)
{
	
	if (port==0) trigger=value;
	if (triggermode==2)
	{
		if (port==1) {if (value!=INVALID_VALUE) portval|=1; else portval&=~(unsigned char)1;}
		if (port==2) {if (value!=INVALID_VALUE) portval|=2; else portval&=~(unsigned char)2;}
		if (port==3) {if (value!=INVALID_VALUE) portval|=4; else portval&=~(unsigned char)4;}
		if (port==4) {if (value!=INVALID_VALUE) portval|=8; else portval&=~(unsigned char)8;}
		if (port==5) {if (value!=INVALID_VALUE) portval|=16; else portval&=~(unsigned char)16;}
		if (port==6) {if (value!=INVALID_VALUE) portval|=32; else portval&=~(unsigned char)32;}
		if (port==7) {if (value!=INVALID_VALUE) portval|=64; else portval&=~(unsigned char)64;}
		if (port==8) {if (value!=INVALID_VALUE) portval|=128; else portval&=~(unsigned char)128;}
	}
	

}
	
void PORTOBJ::work(void)
{
	
	if ((triggermode==1)&&(old_trigger!=trigger))
	{
		if (trigger!=INVALID_VALUE) portval=val1;
	    else portval=val0;
		outportb(portaddress,portval);
		old_trigger=trigger; 
	}
	else if (triggermode == 2)
	{
		outportb(portaddress,portval);
	}
	else if (triggermode ==3)
	{
		int s_trigger;

		if (trigger!=INVALID_VALUE)
		{
		  s_trigger= (int) size_value(in_ports[0].in_min,in_ports[0].in_max,trigger,0.0f,255.0f,1);
		  portval=0;

		  if (s_trigger>0)    portval+=1;
		  if (s_trigger>=32)  portval+=2;
		  if (s_trigger>=64)  portval+=4;
		  if (s_trigger>=96)  portval+=8;
		  if (s_trigger>=128) portval+=16;
		  if (s_trigger>=160) portval+=32;
		  if (s_trigger>=192) portval+=64;
		  if (s_trigger>=224) portval+=128;
		} 
		else portval=val0;
		
		outportb(portaddress,portval);

	}

	if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
	{
		if (triggermode!=0)
			update_bitpositions(hDlg, portval);
			
	}

	
	// pass_values(0, value);
}
	
PORTOBJ::~PORTOBJ() 
{   
	int t,i=0;
	for (t=0;t<GLOBAL.objects;t++) if (objects[t]->type==OB_PORT_IO) i++;
	if (i==1) ClosePortTalk();

}
