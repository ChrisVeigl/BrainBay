/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_DIFFERENTIATE.CPP
  Author:  Chris Veigl


  This Object outputs arithmetic difference of the current sample value to the previous sample value (current-previous) 

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_differentiate.h"

DIFFERENTIATEOBJ::DIFFERENTIATEOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	width=80;
	strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
	previous=INVALID_VALUE;
}
	
void DIFFERENTIATEOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
}

void DIFFERENTIATEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
}

	
void DIFFERENTIATEOBJ::incoming_data(int port, float value)
{
	if ((previous!=INVALID_VALUE) && (value!=INVALID_VALUE))
		difference = value-previous;
	previous=value;
}
	
void DIFFERENTIATEOBJ::work(void)
{
	pass_values(0, difference);
}

DIFFERENTIATEOBJ::~DIFFERENTIATEOBJ() {}

