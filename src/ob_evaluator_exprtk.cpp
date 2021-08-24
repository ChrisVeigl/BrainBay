/* -----------------------------------------------------------------------------

  BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
Expression-Evaluator - Object
Provides an interface to the ExprTk - Mathematical Expression Library

Authors: Jeremy Wilkerson, Chris Veigl, Arash Partow (ExprTk), Janez Jere (glue ExprTk into this project)
Link to ExprTk - Mathematical Expression Library: http://www.partow.net/programming/exprtk/

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; See the
GNU General Public License for more details.


-----------------------------------------------------------------------------  */
// include exprtk first to hack minmax values
// define _SCL_SECURE_NO_WARNINGS to skip warnings in exprtk.hpp
// exprtk.hpp is a large file. so it is included only in this source file

#define _SCL_SECURE_NO_WARNINGS
#include "exprtk.hpp"

#include "brainBay.h"
#include "ob_evaluator_exprtk.h"
#include <string>

const int MAXEXPRLENGTH = 2560;
const int NUMINPUTS = 6;

class EVALEXPRTKOBJ : public BASE_CL
{
	float input[NUMINPUTS];

	std::string expr_str;
	bool valid;

	exprtk::symbol_table<float> symbol_table;
	exprtk::expression<float> exp;

	void setExpression(const char* str);
public:
	EVALEXPRTKOBJ(int num);
	void update_inports();
	void make_dialog();
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void incoming_data(int port, float value);
	void work();
	friend LRESULT CALLBACK EvalExptkDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

/**
factory function 
*/
void createEvaluatorExprtk(int global_num_objects, BASE_CL** actobject) 
{
	*actobject = new EVALEXPRTKOBJ(global_num_objects);
	(*actobject)->object_size=sizeof(EVALEXPRTKOBJ);
}

EVALEXPRTKOBJ::EVALEXPRTKOBJ(int num) : BASE_CL()
{
	valid = false;

	outports = 1;
	inports=1;
	width = 70;
	
	for (int i = 0; i < NUMINPUTS; i++) {
		char name[] = { 'A' + (char)i, 0 };

		input[i] = 0;
		strcpy(in_ports[i].in_name, name);
		symbol_table.add_variable(name, input[i]);
	}

	strcpy(out_ports[0].out_name,"out");
	symbol_table.add_constants();

	exp.register_symbol_table(symbol_table);

	exprtk::parser<float> parser;

	setExpression("A");
}

void EVALEXPRTKOBJ::update_inports()
{
	int i = std::min(count_inports(this), NUMINPUTS);

	if (i > inports) {
		inports = i;
	}

	height=CON_START+inports*CON_HEIGHT+5;
	InvalidateRect(ghWndMain,NULL,TRUE);
}

void EVALEXPRTKOBJ::make_dialog()
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EVALBOX_ML, ghWndStatusbox, (DLGPROC)EvalExptkDlgHandler));
}

static char* prop_name = "expression_exprtk";

// https://stackoverflow.com/questions/5343190/how-do-i-replace-all-instances-of-a-string-with-another-string
static std::string replace(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void EVALEXPRTKOBJ::load(HANDLE hFile) 
{
	char expr[MAXEXPRLENGTH];

	load_object_basics(this);
	load_property(prop_name,P_STRING,expr);
	std::string s = replace(expr, "\\n", "\r\n"); 
	setExpression(s.c_str());
}

void EVALEXPRTKOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	std::string s = replace(expr_str, "\r\n", "\\n");
	save_property(hFile, prop_name,P_STRING, (void*)s.c_str());
}

void EVALEXPRTKOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE) {
		input[port] = value;
	}
}

void EVALEXPRTKOBJ::setExpression(const char *str)
{
	expr_str.assign(str);
	exprtk::parser<float> parser;
	valid = parser.compile(expr_str, exp);

	if (!valid) {
		report_error("Unable to parse expression");
	}
}

void EVALEXPRTKOBJ::work()
{
	float result = valid ? exp.value() : INVALID_VALUE;
	pass_values(0, result);
}

LRESULT CALLBACK EvalExptkDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EVALEXPRTKOBJ * st = (EVALEXPRTKOBJ *) actobject;

	if (st==NULL || st->type!=OB_EVAL_EXPRTK) 
		return FALSE;

	switch( message ) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EVALEXPRESSION, st->expr_str.c_str());
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EVALAPPLY:
			char strExpr[MAXEXPRLENGTH+1];
			GetDlgItemText(hDlg, IDC_EVALEXPRESSION, strExpr, MAXEXPRLENGTH);
			st->setExpression(strExpr);
			strncpy(st->tag, st->expr_str.c_str(),12);			
			st->tag[12]='.';st->tag[13]='.';st->tag[14]=0;
			InvalidateRect(ghWndDesign,NULL,TRUE);
			break;
		}
		return TRUE;
	case WM_SIZE:
	case WM_MOVE:  
		update_toolbox_position(hDlg);
		break;
	}
	return FALSE;
}
