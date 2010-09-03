/*
	Neurobit device Application Programming Interface (API).

	Copyright (c) by Neurobit Systems, 2010

	ANSI C source. System dependent parts written for Microsoft Windows.

--------------------------------------------------------------------------*/

#ifndef __API_H__
#define __API_H__

#include "common.h"
#include "dev_info.h"

#include <windows.h>

/* Constants for parameter space (argument space of NdEnumParams() and
	NdSetDefaults()) */

/* General device parameters (not associated with any channel) */
#define ND_GEN_PARAMS (-1)
/* All device parameters. Only for NdSetDefaults(). */
#define ND_ALL_PARAMS (-2)
/* Spaces >=0 correspond to individual channels. */

/* No context constant (can be an argument of NdSetDevConfig()). */
#define ND_NO_CONTEXT (-1)

/* Measurement modes for NdStartMeasurement function */
enum {
	/* Normal measurement */
	ND_MEASURE_NORMAL,
	/* Special mode: input circuit continuity / electrode impedance test.
		Not all devices support this mode. */
	ND_MEASURE_TEST
};

/*
* Types and constants for callback functions.
*/

/* Type of a signal sample in sample packet. It is left justified, i.e. for
	N-bit ADC only N most significant bits have the meaning. */
typedef long NdPackSample;

/* Structure of an individual channel info for current data packet passed on to
	NdProcSamples() function. */
typedef struct {
	/* Number of samples from the given channel in current data packet. */
	word num;
	/* Pointer to an array of the channel samples in the data packet.
		NULL for disabled channels. */
	const NdPackSample *samps;
	/* State of signal in channel (it may include info about some artifacts). */
	word sig_st;
	/* Sampling phase mask.
		This field can be used to rearrange samples from different channels into
		sampling phase order. It can be IGNORED in most applications.
		A channel contains a sample when !((phase+1) & mask). */
	word mask;
} NdPackChan;

/* Constants for device state indicators */

/* Device state indicators available for an user (see NdUserInd() function).
	Typically there are shown e.g. on status bar (and not generate message window).  */
enum {
	/* Link state has changed. See additional data word for details. */
	ND_IND_LINK,
	/* Battery state has changed. Battery state	is passed on by additional word. */
	ND_IND_BAT,
	/* State of measurement pause has changed.
		Used in devices having built-in pause feature. */
	ND_IND_PAUSE,
	/* Signal state has changed. Further info is passed on by additional word:
		the lower byte contains artifact type and the upper byte contains channel
		index (0 for SUM, 1 for A, 2 for B  and so on).
		(Signal state for each channel is also passed on along with signal samples
		to NdProcSamples() function.) */
	ND_IND_SIGNAL,
	
	/* Number of general state indicators. HAVE TO be at the end. */
	ND_IND_NUM
};

/* Link states passed on with ND_IND_LINK. */
#define ND_LINK_DISC 0x00  /* Link disconnected */
#define ND_LINK_CON  0x01  /* Link connected */
#define ND_LINK_ERR  0x02
	/* Temporary flag (no NdUserInd call when error condition
		disappears). Can be rendered e.g. as a "LED" color changed to orange
		for a short period of time. */

/* Battery states */
#define ND_BAT_FLAT  0x00
#define ND_BAT_WEAK  0x01
#define ND_BAT_OK    0x02
#define ND_BAT_FRESH 0x03

#define ND_PAUSE_OFF 0
#define ND_PAUSE_ON  1

/* Signal states */
#define ND_SIG_OK 0
#define ND_SIG_LACK 1
#define ND_SIG_OVERRANGE 2
#define ND_SIG_LOSS 3

/* Codes returned by Str2Par function */
enum {
	ND_S2P_OK,
	ND_S2P_INV_TYPE,
	ND_S2P_INV_RANGE,
	ND_S2P_NOT_ALLOWED,  /* Parameter change is not allowed (it is read only or currently disabled) */
	ND_S2P_INV_PARAM,

	/* Number of error types; HAVE TO be at the end. */
	ND_S2P_ERR_NUM
};

/*-------------------------------------------------------------------------*/

#ifdef NEUROBIT_DRIVER

/*
* Device context
*/

/* Set pointer to array of names of devices supported by API and return
	their number. (Additionally the array is terminated with NULL pointer.) */
word NdEnumDevices(const char* const ** devs);

/* Open (and select as current) a new context for given device type.
	The function sets default parameter values.
	It returns device context id (>=0); on error (e.g. too many opened contexts
	or unknown device) it returns negative number and ealier context (if any)
	remains selected. */
int NdOpenDevContext(const char *dev_name);

/* Select a given device context (opened earlier).
	All further parameter manipulations relate to that context until next
	call of NdSelectDevContext() (or NdOpenDevContext()).
	It returns the context id (>=0); on error it returns negative number
	and ealier context (if any) remains selected. */
int NdSelectDevContext(word dc);

/* Close given device context and free associated resources.
	If communication with device was in progress, it is ceased.
	If given context is the current context, there will be no current context,
	even if there are still opened contexts (until one of them is selected with
	NdSelectDevContext()). */
void NdCloseDevContext(word dc);

/* Copy device configuration to storage buf of given size [B] for given
	context dc.
	That storage can be saved e.g. in file for future read with SetDevContext().
	The function returns number of bytes copied to storage; on error (e.g. too
	small buffer) it returns 0.
	An alternative function call with buf=NULL returns required storage size. */
dword NdGetDevConfig(word dc, void *buf, dword size);

/* Set device configuration given in storage buf of given size [B] (created
	earlier with NdGetDevConfig()) for given device context dc.
	If opened context was given, a device specified in storage have to be
	the same as in that context.
	If ND_NO_CONTEXT was given, new context is created and selected as a current one.
	The function returns device context, for which configuration was set, and on
	error (e.g. incompatible storage or cannot open new context) it returns
	negative number. */
int NdSetDevConfig(short dc, void *buf, dword size);

/*-------------------------------------------------------------------------*/

/*
* Device information services.
* They all work on current device context.
*/

/* Create device configuration window.
	Arguments are handles of application instance, owner (application) window
	and directory of Neurobit Driver runtime files.
	The function returns handle of device window, and on error it returns NULL. */
HWND NdCreateDevWindow(HINSTANCE hAppInstance, HWND hOwner, const char *runtimeDir);

/* Get pointer to device name for current context.
	If no context is opened, the function returns NULL. */
const char *NdGetDevName(void);

/* Get list of available parameters in the given space.
	The function sets parameter numeric identifiers in buffer params of given
	size [B]. They are ordered in array acoording to suggested order in
	configuration window of an application.
	The space can be ND_GEN_PARAMS for general device parameters or channel index
	(from 0) for given channel parameters. (Number of available channels can be
	retrieved with NdGetParam(ND_PAR_CHAN_NUM, 0, ...) function).
	The function returns number of parameters in buffer, or zero on error (buffer
	is too short, device context not opened).
	An alternative call with params=NULL & size=0 returns number of necessary
	NDPARAMID items in buffer. */
word NdEnumParams(short space, NDPARAMID *params, word size);

/* Return pointer to structure of complete description of given parameter par
	in channel chan or general device parameter (in the last case the argument
	chan is ignored).
	The parameter description includes its type and data necessary to render &
	read parameter, among other things. See definition of NDPARAM structure for
	details.
	REMARK! Parameter description may depend on other parameter values,
	thus after change of those superior parameter settings it may be necessary to
	get new description of depending parameter and render it again.
	The function returns NULL on error (parameter unknown or not available in
	current device and given space, device context not opened). */
const NDPARAM* NdParamInfo(NDPARAMID par, word chan);

/* Get value val of parameter par in channel chan or general device parameter
	(in the last case the argument chan is ignored).
	The function writes fields of NDGETVAL *val structure:
	- current parameter value in val field,
	- its type in type field,
	- numeric identifier (not index) of current option in opt field, for
		parameters with type&ND_T_LIST.
	The function returns non-zero on error (e.g. parameter unknown or not
	available in current device and given space). */
int NdGetParam(NDPARAMID par, word chan, NDGETVAL *val);

/* Set value val of parameter par in channel chan or general device parameter
	(in the last the argument chan is ignored).
	val should point to union giving new option identifier for parameter
	defined with ND_T_LIST or new parameter value without ND_T_LIST (required value
	type is given in corresponding NDPARAM structure).
	REMARK! If other parameters depend on the set parameter, their descriptions
	will be changed and their values will be set to defaults. Those parameter
	controls on the screen (or simply complete window) should be refreshed in
	this case.
	The function returns non-zero on error:
	- <0 if value is inconsistent with parameter domain,
	- >0 for other errors, e.g. parameter is constant, unknown or not
		available in current device and given space). */
int NdSetParam(NDPARAMID par, word chan, const NDSETVAL *val);

/* Get number of options of ND_T_LIST parameter.
	On error the function returns 0. */
word NdGetOptNum(NDPARAMID par, word chan);

/* Convert parameter pid in given parameter space to string *s.
	The function returns pointer to the resulting string, and on error it
	returns NULL. */
char* NdParam2Str(NDPARAMID pid, short space, char *s);

/* Set list of parameter options in string s and delimit it with additional '\0'.
	If tab is not NULL, array pointed by tab if filled out with pinters to
	individual option strings, and NULL pointer at the end.
	The function returns pointer to s on success and NULL it given parameter
	is not a list-type. */
char* NdParamList2Str(const NDPARAM *p, char *s, char **tab);

/* Convert given string to value of a parameter.
	The function returns non-zero on error (see S2P_* constants). */
int NdStr2Param(const char *s, NDPARAMID par, short space);

/* Set default parameters in given space of current device configuration
	structure. space can specify general device parameters (ND_GEN_PARAMS),
	parameters of a given channel (channel index>=0), or all device parameters
	(ND_ALL_PARAMS).
	On error the function returns non-zero.
	(Defaults are always set by NdOpenDevContext, thus calling NdSetDefaults
	at the beginning is not required.) */
int NdSetDefaults(short space);

/* Run service application (NeurobitServ.exe) for Neurobit devices (firmware
	upgrade etc.) An argument is a directory of Neurobit Driver runtime files.
	Device configuration for current context is passed on to the application.
	On error the function returns non-zero. */
int NdDevServices(const char *runtimeDir);

/*-------------------------------------------------------------------------*/

/*
* Interface functions of protocol state machine
*/

/* Communication protocol engine for all opened device contexts.
	This function HAVE TO be called in application main loop.
	The function returns 1, when communication with any device is
	in progress, and 0 otherwise. */
int NdProtocolEngine(void);

/* Start measurement in given device context.
	The function returns:
	0 - measurement started;
	2 - cannot initialize transport protocol;
	1 - connection with the device cannot be established;
	-1 - invalid device context or device state;
	-2 - specified mode is not supported for given device context. */
int NdStartMeasurement(word dc, word mode);

/* Stop measurement in given device context.
	The function returns:
	0 - processing instantly finished;
	1 - required waiting for end of processing;
	-1 - invalid device context or device state. */
int NdStopMeasurement(word dc);

/*------------------------------------------------------------------------*/

/*
* Application-specific functions called back by driver state machine
*/

/* Display to user a message msg in given device context dc.
	Text of a message starts with exclamation mark for errors. */
extern void (*NdUserMsg)(word dc, const char *msg);

/* Update specified indicator ind of measurement state in given device context
	dc at the level of user interface.
	data argument pass additional information specific to a given indicator. */
extern void (*NdUserInd)(word dc, int ind, word data);

/* Process received packet of signal samples in given device context dc.
	phase is a relative time index (modulo 64K) of the beggining of the sample
	package; the phase is incremented with each of the shortest sampling period
	in enabled channels.
	sum_st is a status of the common mode voltage; it is set to ND_SIG_OVERRANGE,
	if the common voltage exceeds acceptable range.
	chans is an array of the packet data for individual channels. See definition
	of structure ChanOutState for details. */
extern void (*NdProcSamples)(word dc, word phase, word sum_st, const NdPackChan *chans);

/*------------------------------------------------------------------------*/

#else /* NEUROBIT_DRIVER */

/*
* Types of pointers to interface functions listed above
*/

/* Device context */

typedef word (*TEnumDevices)(const char* const ** devs);
typedef int (*TOpenDevContext)(const char *dev_name);
typedef int (*TSelectDevContext)(word dc);
typedef void (*TCloseDevContext)(word dc);
typedef dword (*TGetDevConfig)(word dc, void *buf, dword size);
typedef int (*TSetDevConfig)(short dc, void *buf, dword size);

/* Device information services */

typedef HWND (*TCreateDevWindow)(HINSTANCE hAppInstance, HWND hOwner, const char *runtimeDir);
typedef const char* (*TGetDevName)(void);
typedef word (*TEnumParams)(short space, NDPARAMID *params, word size);
typedef const NDPARAM* (*TParamInfo)(NDPARAMID par, word chan);
typedef int (*TGetParam)(NDPARAMID par, word chan, NDGETVAL *val);
typedef int (*TSetParam)(NDPARAMID par, word chan, const NDSETVAL *val);
typedef word (*TGetOptNum)(NDPARAMID par, word chan);
typedef int (*TSetDefaults)(short space);
typedef int (*TDevServices)(const char *runtimeDir);

typedef char* (*TParam2Str)(NDPARAMID pid, short space, char *s);
typedef char* (*TParamList2Str)(const NDPARAM *p, char *s, char **tab);
typedef int (*TStr2Param)(const char *s, NDPARAMID par, short space);

/* Interface functions of protocol state machine */

typedef int (*TProtocolEngine)(void);
typedef int (*TStartMeasurement)(word dc, word mode);
typedef int (*TStopMeasurement)(word dc);

/* Application-specific functions called back by driver state machine */

typedef void (*TUserMsg)(word dc, const char *msg);
typedef void (*TUserInd)(word dc, int ind, word data);
typedef void (*TProcSamples)(word dc, word phase, word sum_st, const NdPackChan *chans);

#endif /* NEUROBIT_DRIVER */

#endif /* __API_H__ */

