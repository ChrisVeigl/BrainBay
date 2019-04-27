/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_COM_WRITER.CPP:  contains functions for the Com-Port Writer
  Author: Chris Veigl

  The Com-Port Writer Object can be used to write Bytes to an opened Com-Port.
  This will be usually a port where a bidirectional EEG-device like the
  MONOLITH-EEG is connected to. Using this Object, Command- and Data Bytes can be
  sent to the Device, for example to adjust the baud- or sampling rate.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_com_writer.h"


LRESULT CALLBACK ComWriterDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szdata[50];
	COM_WRITEROBJ * st;
	
	st = (COM_WRITEROBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_COM_WRITER)) return(FALSE);
    
	switch( message )
	{
		case WM_INITDIALOG:
	        CheckDlgButton(hDlg, IDC_TRIGGER, st->en_trigger);
			SetDlgItemInt(hDlg,IDC_COMMAND,st->command,0);
			SetDlgItemInt(hDlg,IDC_DATA1,st->data1,0);
			SetDlgItemInt(hDlg,IDC_DATA2,st->data2,0);
			if (TTY.COMDEV==INVALID_HANDLE_VALUE)
				SetDlgItemText(hDlg,IDC_STATUS,"COM-Port for EEG not available");
			else if (st->cnt==0) SetDlgItemText(hDlg,IDC_STATUS,"ready to send");
			else {	  sprintf(szdata,"%d frames written",st->cnt);
					  SetDlgItemText(hDlg,IDC_STATUS,szdata);		}

	        break;

		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_SENDCOMMAND:
					st->command=(unsigned char) GetDlgItemInt(hDlg,IDC_COMMAND,NULL,0);
					st->data1=(unsigned char) GetDlgItemInt(hDlg,IDC_DATA1,NULL,0);
					st->data2=(unsigned char) GetDlgItemInt(hDlg,IDC_DATA2,NULL,0);
					if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
					{
                       write_to_comport (st->command );
					   write_to_comport (st->data1 );
					   write_to_comport (st->data2 );

					   //if (!SetEvent(TTY.WriterEvent))
//					   if (WriterProc(NULL)) st->cnt++;
					   sprintf(szdata,"%d frames written",st->cnt);
 					   SetDlgItemText(hDlg,IDC_STATUS,szdata);				
					}
                    break;
				case IDC_TRIGGER:
					st->en_trigger=IsDlgButtonChecked(hDlg, IDC_TRIGGER);
					break;
				case IDC_RESET:
					st->cnt=0;
					if (TTY.COMDEV!=INVALID_HANDLE_VALUE) 
						SetDlgItemText(hDlg,IDC_STATUS,"0 frames written");				
					break;
            }
			return TRUE;
			break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
	}
	return FALSE;
}


COM_WRITEROBJ::COM_WRITEROBJ(int num) : BASE_CL()
{
	inports = 2;
	width=75;
	strcpy(in_ports[0].in_name,"trigger");
	strcpy(in_ports[1].in_name,"data2");
	
	input = INVALID_VALUE;
	trigger=INVALID_VALUE;

	input2 = INVALID_VALUE;
	trigger2=INVALID_VALUE;

	command=129; data1=0; data2=0; en_trigger=0; cnt=0;
	
}
	
void COM_WRITEROBJ::make_dialog(void) {display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_COM_WRITERBOX, ghWndStatusbox, (DLGPROC)ComWriterDlgHandler)); }

void COM_WRITEROBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("command",P_INT,&command);
	load_property("data1",P_INT,&data1);
	load_property("data2",P_INT,&data2);
	load_property("trigger",P_INT,&en_trigger);
}

void COM_WRITEROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"command",P_INT,&command);
	save_property(hFile,"data1",P_INT,&data1);
	save_property(hFile,"data2",P_INT,&data2);
	save_property(hFile,"trigger",P_INT,&en_trigger);
}
	
void COM_WRITEROBJ::incoming_data(int port, float value)
{
	if (port==0) input=value;
	if (port==1) input2=value;
}
	
void COM_WRITEROBJ::work(void)
{

	if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (en_trigger) && (input!=INVALID_VALUE) && (trigger==INVALID_VALUE))
	{
		write_to_comport (command );
		write_to_comport (data1 );
		write_to_comport (data2 );
		cnt++;
		if (hDlg==ghWndToolbox)
		{
			char szdata[50];
			sprintf(szdata,"%d frames written",cnt);
 			SetDlgItemText(hDlg,IDC_STATUS,szdata);
		}
	}
	trigger=input;


	if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (en_trigger) && (input2!=trigger2) && (input2!=INVALID_VALUE))
	{
		write_to_comport (command );
		write_to_comport (data1 );
		write_to_comport ((unsigned char) input2);
		cnt++;
		if (hDlg==ghWndToolbox)
		{
			char szdata[50];
			sprintf(szdata,"%d frames written",cnt);
 			SetDlgItemText(hDlg,IDC_STATUS,szdata);
		}
	}
	trigger2=input2;
	
}
	
COM_WRITEROBJ::~COM_WRITEROBJ() {}



