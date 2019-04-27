/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SPELLER.CPP:  contains functions for the SPELLER-Generator-Object
  Author: Chris Veigl

  The SPELLER-Object provides an Arkanoid-like SPELLER.
  The position of the bar is controlled by the input-port value.
  the drawing in done in a seperate window using GDI-functions

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_speller.h"
#include <wingdi.h> 


#define START_WORDS 20
#define START_DYNAMIC 400


#define EXTEND_CHARACTERS 1
#define FINAL_CHARACTERS 2
#define EXTEND_WORD 3
#define DICTIONARY 4

#define MAX_SUGGEST 10

void add_dict(SPELLEROBJ * st,int type,char * tag, char * data)
{
	st->dict[st->entries].type=type;st->dict[st->entries].count=0;
	strcpy(st->dict[st->entries].tag,tag);strcpy(st->dict[st->entries].data,data);
	st->entries++;
	st->dict[st->entries].type=0;
	st->dict[st->entries].count=0;
}

void init_dict( SPELLEROBJ * st )
{
	st->entries=0;

	add_dict(st,2,"Wörter ->","\3");
	add_dict(st,1,"<_G.,!?","< \2.,!?");
	add_dict(st,1,"enis","enis");
	add_dict(st,1,"ratd","ratd");
	add_dict(st,1,"hulc","hulc");
	add_dict(st,1,"gmobp","gmobp");
	add_dict(st,1,"vwfkz","vwfkz");
	add_dict(st,1,"jyxqäöü","jyxqäöü");
	add_dict(st,1,"012345678","0123456789");
	//add_dict(st,2,"Text senden","\1");
	add_dict(st,1,"F12345678","F12345678");
	st->dict[(st->entries)-1].data[1]=10;
	st->dict[(st->entries)-1].data[2]=11;
	st->dict[(st->entries)-1].data[3]=12;
	st->dict[(st->entries)-1].data[4]=13;
	st->dict[(st->entries)-1].data[5]=14;
	st->dict[(st->entries)-1].data[6]=15;
	st->dict[(st->entries)-1].data[7]=16;
	st->dict[(st->entries)-1].data[8]=17;

	add_dict(st,2,"Wörterbücher","\4");
	


	st->dict[START_WORDS].type=0;st->dict[START_WORDS].count=0;
	st->wordcount=0;
}


void send_key(unsigned char d)
{

	if ((d>=10) && (d<=17))
	{
	
	  keybd_event(VK_F1+d-10, 0 , 0 ,0  ); 
	  keybd_event(VK_F1+d-10, 0 , KEYEVENTF_KEYUP,0 ); 
	}
	else
	{
	  if ((d>='A')&&(d<='Z'))  keybd_event(16,MapVirtualKey(16,0) , 0 ,0  ); //KEYEVENTF_EXTENDEDKEY,0); 					  
	  if ((d>='a')&&(d<='z'))  d=d-'a'+'A';

	  keybd_event(d,MapVirtualKey(d,0) , 0 ,0  ); //KEYEVENTF_EXTENDEDKEY,0); 					  
	  keybd_event(d,MapVirtualKey(d,0) , KEYEVENTF_KEYUP,0 ); //|KEYEVENTF_EXTENDEDKEY,0); 
	  
	  keybd_event(16,MapVirtualKey(16,0) , KEYEVENTF_KEYUP,0 ); //|KEYEVENTF_EXTENDEDKEY,0); 
	}
}

void send_word( char * str )
{
	int i;

	for (i=0;i<(int)strlen(str);i++)
		send_key(str[i]);
}


int import_words(SPELLEROBJ * st, LPCTSTR szFileName)
{
    
    HANDLE hFile;
    BOOL bSuccess = FALSE;
	unsigned char buffer [2];
	DWORD dwRead=1;
	char actword[100],c;
	int actpos=0;
	int wordflag=0;
	int wordnum;

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return FALSE;    
   
	 wordnum=START_WORDS;
	 while (dwRead)
	 {
		 if (!ReadFile(hFile, (unsigned char *) buffer , 1, &dwRead, NULL))  return FALSE;
		 c=buffer[0];
		 if (!wordflag)
		 {  
			 if (((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z'))||(c=='ä')||(c=='ä')||(c=='ö')||(c=='ü')||(c=='Ä')||(c=='Ö')||(c=='Ü')||(c=='ß'))
			 wordflag=1;
			 actpos=0;
		 }
		 
		 if (wordflag)
		 {
			 if (((c<'a')||(c>'z'))&&((c<'A')||(c>'Z'))&&(c!='ä')&&(c!='ä')&&(c!='ö')&&(c!='ü')&&(c!='Ä')&&(c!='Ö')&&(c!='Ü')&&(c!='ß'))
			 {
				 actword[actpos]=0;

				 //report(actword);

				st->dict[wordnum].type=EXTEND_WORD;st->dict[wordnum].count=0;
				strcpy(st->dict[wordnum].tag,actword);
				strcpy(st->dict[wordnum].data,actword);
				wordnum++;	
				st->dict[wordnum].type=0;st->dict[wordnum].count=0;

				wordflag=0;
			 } 
			 else actword[actpos++]=c;
		 }
	 }

	 st->wordcount=wordnum-START_WORDS;

	 for (actpos=0;actpos<8;actpos++)	st->suggest[actpos]=START_WORDS+actpos;
	 st->suggestions=8;

     CloseHandle(hFile);
	 return TRUE;
}

char upstr(char c)
{
	if ((c>='a')&&(c<='z')) return (c-'a'+'A');
	if (c=='ä') return( 'Ä');
	if (c=='ö') return( 'Ö');
	if (c=='ü') return( 'Ü');
	return(c);
}


void draw_speller(SPELLEROBJ * st)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char szdata[400];
	RECT rect;
	HBRUSH actbrush;
	int ballx,bally,i;

	
	GetClientRect(st->displayWnd, &rect);
	hdc = BeginPaint (st->displayWnd, &ps);


	//ballx=(int) ((rect.right/100.0f)*st->xpos);
	//bally=(int) ((rect.bottom/100.0f)*st->ypos);

	ballx=(int)st->xpos;
	bally=(int)st->ypos;
	
	actbrush=CreateSolidBrush(RGB(180,0,0));
    
   	SelectObject (hdc, DRAW.pen_white);		
   	SelectObject (hdc, DRAW.brush_ltgreen);
	Rectangle(hdc,0,0,rect.right,rect.bottom);

    SelectObject(hdc, DRAW.mediumFont);
	SetTextColor(hdc,RGB(255,0,0));

	SelectObject (hdc, DRAW.pen_red);	
	
    sprintf(szdata, "->  %s",st->word);  // st->selstart,st->selend,st->suggestions,
	DrawText(hdc, szdata, -1, &rect, DT_CENTER | DT_WORDBREAK );

	// sprintf(szdata, "PAD: %d SEL: %d",st->paddle,st->select); 
    // ExtTextOut( hdc, 10,10, 0, &rect,szdata, strlen(szdata), NULL ) ;

	for (i=0;i<st->selections;i++)
	{
		if ((st->dict[st->selstart+i].type==EXTEND_CHARACTERS) 
			||(st->dict[st->selstart+i].type==FINAL_CHARACTERS)
			||(st->dict[st->selstart+i].type==DICTIONARY))
			strcpy(szdata, st->dict[st->selstart+i].tag);
		else strcpy(szdata,"inv");
		
		if (i==st->select) { SetTextColor(hdc,RGB(255,0,0)); SetBkColor(hdc,RGB(255,255,0)); }
		else { SetTextColor(hdc,RGB(255,0,0)); SetBkColor(hdc,RGB(30,30,30)); }

		ExtTextOut( hdc, 100,80+i*30, 0, &rect,szdata, strlen(szdata), NULL ) ; 

		/*
		switch (i)
		{
			case 0:	ExtTextOut( hdc, 150,50, 0, &rect,szdata, strlen(szdata), NULL ) ; break;
			case 1:	ExtTextOut( hdc, 250,100, 0, &rect,szdata, strlen(szdata), NULL ) ; break;
			case 2:	ExtTextOut( hdc, 50,100, 0, &rect,szdata, strlen(szdata), NULL ) ; break;
			case 3:	ExtTextOut( hdc, 150,150, 0, &rect,szdata, strlen(szdata), NULL ) ; break;
		}
		*/
	}

	if (st->dictfile[0])
	{
		char fn[256];

		SelectObject(hdc, DRAW.scaleFont);
		SetTextColor(hdc,RGB(255,255,255));
		SetBkColor(hdc,RGB(0,0,50));
		reduce_filepath(fn,st->dictfile);
		sprintf(szdata,"Wörterbuch: %s, %d words.",fn, st->wordcount);
		ExtTextOut( hdc, 360, 70, 0, &rect,szdata, strlen(szdata), NULL );

		//ExtTextOut( hdc, 360, 80, 0, &rect,st->fn, strlen(st->fn), NULL );

	}
	if (st->suggestions)
	{
		SelectObject(hdc, st->sugfont);
		SetTextColor(hdc,RGB(255,255,0));
		SetBkColor(hdc,RGB(40,40,0));

		for (i=0;(i<st->suggestions);i++)
		{
			strcpy(szdata, st->dict[st->suggest[i]].tag);
			ExtTextOut( hdc, 360,100+i*20, 0, &rect,szdata, strlen(szdata), NULL );
		}
		if (i==MAX_SUGGEST)
		{
			strcpy(szdata, "(...)");
			ExtTextOut( hdc, 360,100+i*20, 0, &rect,szdata, strlen(szdata), NULL );
		}

	}
   	SelectObject (hdc, DRAW.brush_orange);
//	Rectangle(hdc,ballx,bally,ballx+20,bally+20);

   
	DeleteObject(actbrush);
	EndPaint( st->displayWnd, &ps );
}



	
LRESULT CALLBACK SpellerDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	SPELLEROBJ * st;
	char szFileName [256];
	
	st = (SPELLEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_SPELLER)) return(FALSE);

	switch( message )
	{
		case WM_INITDIALOG:
			
			SetDlgItemInt(hDlg, IDC_SWITCHTIME,st->speed,0);
			SetDlgItemInt(hDlg, IDC_IDLETIME,st->idle,0);
			SetDlgItemInt(hDlg, IDC_PRESSTIME,st->press,0);
			SetDlgItemInt(hDlg, IDC_WORDCOUNT,st->wordcount,0);

			SetDlgItemText(hDlg, IDC_WORDFILE,st->wordfile);
			SetDlgItemText(hDlg,IDC_DICTFILE,st->dictfile);

			CheckDlgButton(hDlg, IDC_AUTOLEARN, st->autolearn);
			CheckDlgButton(hDlg, IDC_DIRECTSEND, st->directsend);

			SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) "Automatic Switching") ;
			SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) "Paddle + Joystick-Button") ;
			SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_ADDSTRING, 0,(LPARAM) "Joystick (2-Directions)") ;
			SendDlgItemMessage( hDlg, IDC_MODECOMBO, CB_SETCURSEL, st->mode, 0L ) ;
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ 
			
				case IDC_AUTOLEARN:
					st->autolearn=IsDlgButtonChecked(hDlg,IDC_AUTOLEARN);
					break;
				case IDC_DIRECTSEND:
					st->directsend=IsDlgButtonChecked(hDlg,IDC_DIRECTSEND);
					break;
				case IDC_SWITCHTIME:
					st->speed=GetDlgItemInt(hDlg,IDC_SWITCHTIME,NULL,0);
					break;
				case IDC_IDLETIME:
					st->idle=GetDlgItemInt(hDlg,IDC_IDLETIME,NULL,0);
					break;
				case IDC_PRESSTIME:
					st->press=GetDlgItemInt(hDlg,IDC_PRESSTIME,NULL,0);
					break;
				case IDC_DICTFILE:
					GetDlgItemText(hDlg,IDC_DICTFILE,st->dictfile,256);
					break;
				case IDC_LEARN:
					GetDlgItemText(hDlg, IDC_NEWWORDS,st->word,sizeof(st->word));
					st->learn_words();
					SetDlgItemText(hDlg, IDC_NEWWORDS,"");
					strcpy(st->word,"");
					SetDlgItemInt(hDlg, IDC_WORDCOUNT,st->wordcount,0);
					break;
				case IDC_IMPORTLIST:
					  strcpy(szFileName,GLOBAL.resourcepath);
 				      strcat(szFileName,"dictionary\\*.txt");
					  if (open_file_dlg(hDlg, szFileName, FT_TXT, OPEN_LOAD)) 
					  {
						if (!import_words(st, szFileName))
							report_error("Could not import Word list");
						else 
						{
							strcpy(st->wordfile,szFileName);
							SetDlgItemText(hDlg, IDC_WORDFILE,st->wordfile);
							SetDlgItemInt(hDlg, IDC_WORDCOUNT,st->wordcount,0);
						}

					  } else report_error("Could not open File");
					break;
				case IDC_LOADDICT:
					
						strcpy(szFileName,GLOBAL.resourcepath);
 						strcat(szFileName,"dictionary\\*.dic");
						if (open_file_dlg(hDlg, szFileName, FT_DIC, OPEN_LOAD)) 
							st->load_dictionary(szFileName);
					break;

				case IDC_SAVEDICT:

					if (strlen(st->dictfile)>=7) strcpy(szFileName,st->dictfile);
						else
						{
							strcpy(szFileName,GLOBAL.resourcepath);
 							strcat(szFileName,"dictionary\\*.dic");
						}

						if (open_file_dlg(hDlg, szFileName, FT_DIC, OPEN_SAVE)) 
						    st->save_dictionary(szFileName);
					
					break;
				case IDC_CLEARDICT:
					 init_dict(st);
					 SetDlgItemInt(hDlg, IDC_WORDCOUNT,st->wordcount,0);
					 break;

				case IDC_EXPORTLIST: 
					break;


				case IDC_MODECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
						st->mode=SendMessage(GetDlgItem(hDlg, IDC_MODECOMBO), CB_GETCURSEL , 0, 0);
						switch (st->mode)
						{
							case 0: st->selections=st->entries; break;
							case 1: st->selections=st->entries; break;
							case 2: st->selections=st->entries; break;
						}
				break;

			}
			return TRUE;
			
		
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		case WM_PAINT:
//			color_button(GetDlgItem(hDlg,IDC_SELECTCOLOR),st->color);
		break;
	}
    return FALSE;
}


LRESULT CALLBACK SpellerWndHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	int t;
	SPELLEROBJ * st;
	

	st=NULL;
	for (t=0;(t<GLOBAL.objects)&&(st==NULL);t++)
		if (objects[t]->type==OB_SPELLER)
		{	st=(SPELLEROBJ *)objects[t];
		    if (st->displayWnd!=hWnd) st=NULL;
		}

	if (st==NULL) return DefWindowProc( hWnd, message, wParam, lParam );
	
	switch( message ) 
	{	case WM_DESTROY:
		 break;
		case WM_KEYDOWN:
			  SendMessage(ghWndMain, message,wParam,lParam);
			break;
		case WM_MOUSEACTIVATE:
		  close_toolbox();
		  actobject=st;
		  SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
		  InvalidateRect(ghWndDesign,NULL,TRUE);
			break;
		case WM_SIZE: 
		case WM_MOVE:
			{
			WINDOWPLACEMENT  wndpl;
			GetWindowPlacement(st->displayWnd, &wndpl);
			st->top=wndpl.rcNormalPosition.top;
			st->left=wndpl.rcNormalPosition.left;
			st->right=wndpl.rcNormalPosition.right;
			st->bottom=wndpl.rcNormalPosition.bottom;
			}
			InvalidateRect(hWnd,NULL,TRUE);
			break;
		case WM_PAINT:
			draw_speller(st);
  	    	break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
    } 
    return 0;
}






//
//  Object Implementation
//


SPELLEROBJ::SPELLEROBJ(int num) : BASE_CL()
	  {
		outports = 1;
		inports = 2;
		strcpy(in_ports[0].in_name,"joystick");
		strcpy(in_ports[1].in_name,"paddle");
		width=80;

		select=-1;selstart=0;selbegin=0;selend=4;os=0;
		
		waitres=0;idletime=0;presstime=0; switchtime=0; 
		wordcount=0;
		
		input=15;oldinput=15;
		xpos=200.0f;ypos=100.0f;
		suggestions=0;selections=4;
		sugchars=0; delchars=0;
		upchar=0;directsend=0;
		autolearn=0;

		strcpy(fn,"none");
		strcpy(word,"");
		strcpy(lastword,"");
		strcpy(wordfile,"");
		strcpy(dictfile,"none");
		enter=0;
		input=0.0f; input2=0.0f;
		paddle=0;oldpaddle=0;
		left=500;right=800;top=50;bottom=400;
		
		speed=300;press=300;idle=500; 
		mode=0;

		if (!(sugfont = CreateFont(-MulDiv(12, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial")))
			report_error("Font creation failed!");

		init_dict(this);
        
		if(!(displayWnd=CreateWindow("Speller_Class", "Speller", WS_CLIPSIBLINGS| WS_CHILD | WS_CAPTION | WS_THICKFRAME ,left, top, right-left, bottom-top, ghWndMain, NULL, hInst, NULL)))
		    report_error("can't create SPELLER Window");
		else { SetForegroundWindow(displayWnd); ShowWindow( displayWnd, TRUE ); UpdateWindow( displayWnd ); }
		InvalidateRect(displayWnd, NULL, TRUE);
	  }
	  void SPELLEROBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SPELLERBOX, ghWndStatusbox, (DLGPROC)SpellerDlgHandler));
	  }
	  void SPELLEROBJ::load(HANDLE hFile) 
	  {
		  load_object_basics(this);
		  load_property("top",P_INT,&top);
		  load_property("left",P_INT,&left);
		  load_property("right",P_INT,&right);
		  load_property("bottom",P_INT,&bottom);
		  load_property("speed",P_INT,&speed);
		  load_property("press",P_INT,&press);
		  load_property("idle",P_INT,&idle);
		  load_property("mode",P_INT,&mode);
		  load_property("selections",P_INT,&selections);
		  load_property("wordfile",P_STRING,wordfile);
		  load_property("dictfile",P_STRING,dictfile);
		  load_property("autolearn",P_INT,&autolearn);
		  load_property("directsend",P_INT,&directsend);

		  if (strlen(dictfile)>6) load_dictionary(dictfile);
		  MoveWindow(displayWnd,left,top,right-left,bottom-top,TRUE);
	  }
		
	  void SPELLEROBJ::save(HANDLE hFile) 
	  {	  
	 	  save_object_basics(hFile, this);
  		  save_property(hFile,"top",P_INT,&top);
		  save_property(hFile,"left",P_INT,&left);
		  save_property(hFile,"right",P_INT,&right);
		  save_property(hFile,"bottom",P_INT,&bottom);
		  save_property(hFile,"speed",P_INT,&speed);
		  save_property(hFile,"press",P_INT,&press);
		  save_property(hFile,"idle",P_INT,&idle);
		  save_property(hFile,"mode",P_INT,&mode);
		  save_property(hFile,"selections",P_INT,&selections);
		  save_property(hFile,"wordfile",P_STRING,wordfile);
		  save_property(hFile,"dictfile",P_STRING,dictfile);
		  save_property(hFile,"autolearn",P_INT,&autolearn);
		  save_property(hFile,"directsend",P_INT,&directsend);

	  }


	  void SPELLEROBJ::incoming_data(int port, float value)
      {
        if (port==0) input = value;
		if (port==1) input2 = value;
		
      }

	  void SPELLEROBJ::make_suggestions(void)
	  {
		  int i,u,f,x;

			suggestions=0;
		    f=1;
			for (x=strlen(word);(x>0)&&f;)
			{ 
			   x--;
			   if (( (word[x]==' ') || (word[x]=='.')) && (word[x+1]))
			   { f=0; x++; } 
			}

			if (strlen(word)-x>0)
			{
			  for (i=START_WORDS;(i<START_WORDS+wordcount)&&(suggestions<MAX_SUGGEST);i++)
			  {
				f=1;
				for (u=0;(u+x<(int)strlen(word))&&f;u++)
					if (upstr( word[u+x]) != upstr (dict[i].tag[u]) ) f=0;
				
				if (f) { suggest[suggestions++]=i; sugchars=u; }
			  }
			}

			if (strlen(word) && (word[strlen(word)-1]==' ')||(word[strlen(word)-1]=='.')) 
			{ 	
			  for (u=0;(u<MAX_SUGGEST)&&(u<wordcount);u++) suggest[suggestions++]=START_WORDS+u;
			  sugchars=0;
			} 
	  }

	  void SPELLEROBJ::get_suggestions(void)
	  {
		int i;

		if (!suggestions) return;
		for (i=0;i<suggestions;i++)
		{
				strcpy(dict[START_DYNAMIC+i].data, dict[suggest[i]].tag);
				strcpy(dict[START_DYNAMIC+i].tag,  dict[suggest[i]].tag);
				dict[START_DYNAMIC+i].type=FINAL_CHARACTERS;
		}

		selstart=START_DYNAMIC;selbegin=START_DYNAMIC;
		selend=START_DYNAMIC+i-1;
		selections=i;
		select=-1;os=0;
		delchars=sugchars;
	  }

  	  void SPELLEROBJ::get_dictionary(void)
	  {
		int i=START_DYNAMIC;

		
		strcpy(dict[i].data, "allgemein.dic");
		strcpy(dict[i].tag,  "allgemein.dic");
		dict[i].type=DICTIONARY; i++;

		strcpy(dict[i].data, "test.dic");
		strcpy(dict[i].tag,  "test.dic");
		dict[i].type=DICTIONARY; i++;

		strcpy(dict[i].data, "home.dic");
		strcpy(dict[i].tag,  "home.dic");
		dict[i].type=DICTIONARY; i++;

		
		selstart=START_DYNAMIC;selbegin=START_DYNAMIC;
		selend=i-1;
		selections=i-START_DYNAMIC;
		select=-1;os=0;
	
	  }

	  void SPELLEROBJ::learn_words()
	  {
		  int x,i,u,f,a=0;
		  struct dictstruct tmp;
		  char actword[50];

		  for (x=0;x<=(int)strlen(word);x++)
		  { 
			if ( ((word[x]>='a') && (word[x]<='z')) || ((word[x]>='A') && (word[x]<='Z'))
				|| (word[x]=='ä') || (word[x]=='ö') || (word[x]=='ü') || (word[x]=='Ä') || (word[x]=='Ö') || (word[x]=='Ü'))
			{
				actword[a++]=word[x];
				actword[a]=0;
			}
			else
			{ 
				if (a>2) 
				{
					f=0;
					for (i=START_WORDS;(i<START_WORDS+wordcount) && (!f);i++)
						if (!strcmp(actword,dict[i].tag)) f=1; // word found
					
					if (!f) 
					{
						
						//report(actword);

						dict[START_WORDS+wordcount].type=EXTEND_WORD;
						dict[START_WORDS+wordcount].count=1;
						strcpy(dict[START_WORDS+wordcount].tag,actword);
						strcpy(dict[START_WORDS+wordcount].data,actword);
						
						wordcount++;	
						dict[START_WORDS+wordcount].type=0;
						dict[START_WORDS+wordcount].count=0;
						
					}
					else
					{
						i--;
						SetDlgItemText(ghWndStatusbox,IDC_TEST,dict[i].tag);
						dict[i].count++;
						for (u=i;u>START_WORDS;u--)
						{
							if (dict[u].count>dict[u-1].count)
							{
								memcpy(&tmp,&(dict[u]),sizeof(tmp));
								memcpy(&(dict[u]),&(dict[u-1]),sizeof(tmp));
								memcpy(&(dict[u-1]),&tmp,sizeof(tmp));
							}
						}
					}
				
				}
				a=0;
				actword[a]=0;
			} 
		  }
		  	
	  }

	  void SPELLEROBJ::load_dictionary(char * szFileName)
	  {
		  DWORD dwRead;
		  HANDLE hFile;

		  hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		  if (hFile!=INVALID_HANDLE_VALUE) 
		  {
			  if (ReadFile(hFile, (unsigned char *) &dict , sizeof(dict), &dwRead, NULL))
			  {		
				strcpy(dictfile,szFileName);
				for (wordcount=START_WORDS;( wordcount< START_DYNAMIC ) && dict[START_WORDS+wordcount].type==EXTEND_WORD ; wordcount++) ;
				
				if (ghWndToolbox==hDlg) 
				{
					SetDlgItemText(hDlg, IDC_DICTFILE,dictfile);
					SetDlgItemInt(hDlg, IDC_WORDCOUNT,wordcount,0);
				}
				InvalidateRect(displayWnd,NULL,TRUE);
			  }
			 
			  CloseHandle (hFile);  
			  
		  }
		  
		}

	  void SPELLEROBJ::save_dictionary(char * szFileName)
	  {
		  DWORD dwWritten;
		  HANDLE hFile;

			hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			 if (hFile!=INVALID_HANDLE_VALUE) 
			 {
			  if (WriteFile(hFile, (unsigned char *) &dict , sizeof(dict), &dwWritten, NULL))
			  {		
				strcpy(dictfile,szFileName);
				if (ghWndToolbox==hDlg) SetDlgItemText(hDlg,IDC_DICTFILE,dictfile);
			  }
			  
			  CloseHandle (hFile);
			 }
			 
	  }


	  void SPELLEROBJ::work(void) 
	  {
	    int i;


		if (mode==0)
		{
  		  enter=0;
		  if (((input!=63) && (oldinput==63)) || (input!=INVALID_VALUE) && (oldinput==INVALID_VALUE))
		  {
			enter=1;
			switchtime=0;
		  }
		  oldinput=input;

		  if (switchtime++> speed)
		  {

			switchtime=0;
			select++; 
			if (select>=selections) 
			{
			   select=0; 
			   selstart=0; selbegin=0; 
			   selections=entries; selend=selections; 
			   delchars=0; 

			   //selstart+=selections; 
			   //if (selstart>selend) selstart=0;
			}
			if (displayWnd) InvalidateRect(displayWnd,NULL,FALSE);

		  }
		}

		if (mode==1)
		{
			enter=0;
			
			if (waitres) 
			{
				if (input!=(int)oldinput)  waitres=0;
			}
			else
			{


				oldpaddle=paddle;
				paddle = ((int) size_value(in_ports[1].in_min,in_ports[1].in_max,input2, 16.0f ,0.0f,0));

				
				if (paddle!= oldpaddle)
				{ 
					select = paddle  % (selections+1) -1 ; 
					presstime=0;
				}
		
				if (select==-1) idletime++;
				else { idletime=0; presstime++;}
				

				if (idletime>idle)
				{
					idletime=0; select=-1; 
					oldpaddle=paddle+1;
					if (selstart+selections<selend) selstart+=selections;
					
					// if (selstart>selend)
					else                      
					{ selstart=0; selbegin=0; selections=entries; selend=selections; 
					  delchars=0; }
					InvalidateRect(displayWnd,NULL,FALSE);
				}

				if (input!=15)  { get_suggestions(); waitres=1; oldinput=input; idletime=0; presstime=0; select=-1; }

				if (presstime>press) {presstime=0; enter=1; idletime=0; }

				if (displayWnd && (select!=os)) InvalidateRect(displayWnd,NULL,FALSE);
				os=select;
			
			}

		}

		if (mode==2)
		{
			if (waitres) 
			{
				if (input==15)  waitres=0;
			}
			else
			{
				if (input==15) idletime++; else idletime=0;

				if (idletime>idle)
				{
					idletime=0; select=-1; 
					if (selstart+selections<selend) selstart+=selections;
					
					// if (selstart>selend)
					else                      
					{ selstart=0; selbegin=0; selections=entries; selend=selections; 
					  delchars=0; }
					InvalidateRect(displayWnd,NULL,FALSE);
				}

				if ((input!=oldinput) || (input==15)) presstime=0;
				else
				{ 
					if (++presstime>press)
					{

						if (input==14) 	{ selstart=0; selbegin=0; selections=entries; selend=selections; }

						if (input==13)  get_suggestions();

						if (input==7) {  select--; if (select<0) select=selections-1; enter=2; }
						if (input==11)  {  select++; if (select==selections) select=-1; enter=2; }
			
						presstime=1;
					}
				}

				if ((enter==2) && (idletime>180) && (select>=0)) {enter=1; waitres=1;}

				if (displayWnd && (select!=os)) InvalidateRect(displayWnd,NULL,FALSE);
				os=select;
			}
		    oldinput=input;

		}



		if (enter==1)
		{
			enter=0;

			switch( dict[selstart+select].type)
			{
			  
			  case FINAL_CHARACTERS:
				{
				 
 				  char c;
				  int u,keep=0;

				  c=dict[selstart+select].data[0];
				
				  switch (c)
				  {

					case '<': if (strlen(word)>0) word[strlen(word)-1]=0; 
							  if (directsend)  send_key(8);
								break;
					case 1:   if (!directsend)  send_word(word);
							  if (autolearn) learn_words();
							  strcpy(word,"");
							break;

					case 2:   for (u=0;u<entries;u++) 
							  {
								  if (dict[u].data[0]>20)   // not a command 
								  {
									up_chars(dict[u].data,dict[u].data);
									up_chars(dict[u].tag,dict[u].tag);
								  }
							  }
							  upchar=1; break;

					case 3:  get_suggestions(); 
							 InvalidateRect(displayWnd,NULL,FALSE);
							 keep=1;
							 //waitres=1; oldinput=input; 
							 idletime=0; presstime=0; switchtime=0;
							 //select=-1; 
							 break;
					
					case 4:  get_dictionary(); 
							 InvalidateRect(displayWnd,NULL,FALSE);
							 keep=1;
							 idletime=0; presstime=0; switchtime=0;
							 break;

					default:  
						if (delchars)
						{ 
						  word[strlen(word)-delchars]=0;
						  strcat(word,dict[selstart+select].data);
						  strcat(word," ");
						  if (directsend)
						  {
							send_word(dict[selstart+select].data);
							send_key(' ');
						  }
						  delchars=0;
						}
						else 
						{
							strcat(word,dict[selstart+select].data);
							if (directsend) send_word(dict[selstart+select].data);
							if (strlen(dict[selstart+select].data)>1)
							{
								strcat(word," ");
								if (directsend)  send_key(' ');
							}

							if ((dict[selstart+select].data[0]>20)&&(upchar))
							{
								upchar=0;
								for (u=0;u<entries;u++) 
								{
									if (dict[u].data[0]>20)
									{
									  low_chars(dict[u].data,dict[u].data);
									  low_chars(dict[u].tag,dict[u].tag);
									}
								}

							}
						}
				  }
				
				  if (!keep)
				  {
				    select=-1;
				    selstart=0;selbegin=0;selections=entries;selend=selections;

					make_suggestions();
				    
				  }
				}
			    break;

			case EXTEND_CHARACTERS:
				i=0;
				while (dict[selstart+select].data[i])
				{
					dict[START_DYNAMIC+i].data[0]=dict[selstart+select].data[i];
					dict[START_DYNAMIC+i].data[1]=0;

					dict[START_DYNAMIC+i].tag[0]=dict[selstart+select].tag[i];
					dict[START_DYNAMIC+i].tag[1]=0;

					dict[START_DYNAMIC+i].type=FINAL_CHARACTERS;
					i++;
				}
				selstart=START_DYNAMIC;selbegin=START_DYNAMIC;
				selend=START_DYNAMIC+i-1;
				selections=i;
				select=-1;
				break;


			case DICTIONARY:
					strcpy(fn,GLOBAL.resourcepath);
 					strcat(fn,"dictionary\\");
					strcat(fn,dict[selstart+select].data);
					load_dictionary(fn);
					make_suggestions();
					select=-1;
				    selstart=0;selbegin=0;selections=entries;selend=selections;
				    
				break;

			}

			if (displayWnd) InvalidateRect(displayWnd,NULL,FALSE);
		}
		
		

	  }


SPELLEROBJ::~SPELLEROBJ()
	  {
		if  (displayWnd!=NULL){ DestroyWindow(displayWnd); displayWnd=NULL; }
	  }  

