/*
	Neurobit device Application Programming Interface (API).
	
	General services of access to device parameters.

	ANSI C system independent source.

	Copyright (c) by Neurobit Systems, 2010

--------------------------------------------------------------------------*/

#ifndef __DEV_INFO_H__
#define __DEV_INFO_H__

#include "common.h"

/* Type of parameter and option numeric identifier */
typedef word NDPARAMID;
typedef unsigned int NDOPTID;

/* Type identifiers of parameter values */
enum {
	ND_T_BOOL,
	ND_T_INT,
	ND_T_FLOAT,
	ND_T_TEXT,

	ND_BASE_TYPES,	/* Number of base types. Have to be at the end! */
	
	ND_T_LIST=0x80 /* Flag of list of base type values */
};

/* Parameter or option value */
typedef union {
	int i;
	bool b;
	float f;
	char *t;
} NDVAL;

/* Range of ND_T_INT parameter (without ND_T_LIST) */
struct NdIntRange {
	int min;
	int max;
};

/* Range of ND_T_FLOAT parameter (without ND_T_LIST) */
struct NdFloatRange {
	float min;
	float max;
};

/* Option of a list */
typedef struct {
	/* Numerical identifier of the option.
		Should remain the same in all software versions. */
	NDOPTID id;

	/* Option value */
	NDVAL val;

	/* Help for the option (or NULL) */
	const char* help;
} NDOPT;

/* Parameter domain */
typedef union {
	/* Maximum text length [characters] (without ND_T_LIST) */
	unsigned int tlen;

	/* Lists for ND_T_LIST */
	const NDOPT *opts;

	/* For parameters without ND_T_LIST: */
	/* Trimmed range for ND_T_INT parameter */
	struct NdIntRange irange;
	
	/* Trimmed range for ND_T_FLOAT parameter */
	struct NdFloatRange frange;
} NDDOMAIN;

/* Set value type */
typedef union {
	/* Option id specified for ND_T_LIST */
	NDOPTID opt; 

	/* Value specified without ND_T_LIST */
	NDVAL val; 
} NDSETVAL;

/* Default value id */
#define NDDEFAULT NDSETVAL

/* Type of structure returning both parameter value and (for ND_T_LIST)
	numeric id. of selected option */
typedef struct {
	/* Option id specified for ND_T_LIST */
	NDOPTID opt;

	/* Value specified when ND_T_LIST is not set */
	NDVAL val;
	
	/* Value type */
	byte type;
} NDGETVAL;

/* Parameter flags */
/* Read only parameter: cannot be edited. 
	IMPORTANT! It is intended only for constant parameters not mapped locally or 
	remotely into a variable. */
#define ND_PF_RDONLY 0x01
/* Hidden parameter: should not be displayed */
#define ND_PF_HIDDEN 0x02

/* Delimiter of parameter dependency chain */
#define ND_PAR_NONE 0xffff

/* Delimiter of option list */
#define ND_OPT_NONE 0xffff

/* Special flag added to a parameter id on a dependency list to mark that 
	a change of a switching parameter should not set default value of the 
	depending parameter, if possible (if it still belongs to the new domain).
	Note: it cannot appear for dependency on channel profile. */
#define ND_DEP_DONT_SET_DEF_FL 0x8000

/* Parameter description structure.
	It collects all parameters of multi-channel data acquisition/output unit.
	Parameters can be constant or variable,	including dependencies on other
	parameters. */
typedef struct {
	/* Numeric identifier of the parameter */
	NDPARAMID id;
	
	/* Name of parameter (for rendering) */
	const char* name;
	
	/* Name of group of parameters (or NULL); for possible rendering only. */
	const char* group;

	/* Value type */
	byte type;
	
	/* Access flags */
	byte flags;

	/* Parameter domain */
	NDDOMAIN domain;

	/* Physical unit of the parameter (or NULL) */
	const char *unit;

	/* Default value */
	NDDEFAULT def;

	/* Array of parameters depending on this parameter (or NULL) */
	const NDPARAMID *dep;

	/* Help for the parameter (or NULL) */
	const char* help;
} NDPARAM;

/* Notes:
	* Flag ND_PF_RDONLY is intended only for constant parameters not mapped locally 
		or remotely into a variable. 
	* For parameters with ND_PF_RDONLY and without ND_T_LIST, domain is ignored 
		and can be zero-filled.
	* For parameters with ND_T_BOOL and without ND_T_LIST, domain is not used and 
		can be zero-filled.
	* Parameter with not empty dependency list and without ND_T_LIST cannot be
		ND_T_FLOAT or ND_T_TEXT.
	* It is assumed that a parameter with set both ND_PF_HIDDEN and ND_PF_RDONLY 
		flags is not mapped into a user interface (configuration window). 
	* Parameters with set ND_T_LIST and single item option on the list can be
		considered as read-only.
*/

#endif /* __DEV_INFO_H__ */
