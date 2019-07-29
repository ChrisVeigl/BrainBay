/*
	Neurobit device Application Programming Interface (API).

	Identifiers of configuration parameters (constant and variable)
	for data acquisition devices by Neurobit Systems.
	
	IMPORTANT! Not all available parameters may be listed here.
	Not all of those parameters have to be available in every device.
	Use NdEnumParams() function to retrieve up-to-date list of
	parameters for given device at runtime.

	ANSI C system independent source.

	Copyright (c) by Neurobit Systems, 2010-2017

--------------------------------------------------------------------------*/

#ifndef __PARAM_H__
#define __PARAM_H__

/* Maximum number of parameters */
#define ND_MAX_PARAMS 0x100u

/* Offset of numeric id. space for channel parameters */
#define ND_PAR_CH (ND_MAX_PARAMS/2)

/* Parameter identifiers.
	Parameters common for all devices are marked with an asterisk before comment.
	(However, they should not be considered a minimum required implementation.) */
enum {
	/* General parameters */
	ND_PAR_LINK = 0,         /**Link type / LOCAL */
	ND_PAR_CHAN_NUM,         /**Number of channels */
	ND_PAR_IND_SUPPORT,      /**Support of indicators */
	ND_PAR_ADDR,             /* Device address / LOCAL */
	ND_PAR_POW_FREQ,         /* Power mains frequency */
	ND_PAR_SILENT,           /* Silent mode */
	/* <- new general parameters can be added here */
	
	ND_GEN_PAR_NUM,          /* Have to be at the end of general parameter list */

	/* Channel parameters */
	ND_PAR_CH_NAME = ND_PAR_CH, /**Channel name */
	ND_PAR_CH_EN,            /**Channel enable (appears in all multi-channel devices) */
	ND_PAR_CH_DIR,           /**Channel direction: 0-"Input", 1-"Output" */
	ND_PAR_CH_RESOL,         /**ADC or DCA resolution */
	ND_PAR_CH_LABEL,         /**Channel label / may be LOCAL */
	ND_PAR_CH_TRANSDUCER,    /**Sensor or transducer info / may be LOCAL */
	ND_PAR_CH_PROF,          /* Channel profile (aggregates several settings for some types of signals) */
	ND_PAR_CH_RANGE_MAX,     /**Measurement range maximum */
	ND_PAR_CH_RANGE_MIN,     /**Measurement range minimum */
	ND_PAR_CH_SR,            /**Sample rate */
	ND_PAR_CH_BAND_FL,       /**Absolute lower limit of frequency band */
	ND_PAR_CH_BAND_FU,       /**Relative upper limit of frequency band */
	ND_PAR_CH_FUNC,          /* Channel function */
	ND_PAR_CH_CHAR,          /* Frequency characteristic */
	ND_PAR_CH_POW_FT,        /* Power interference filter */
	ND_PAR_CH_SUM_DISC,      /* Sum disconnected */
	ND_PAR_CH_TEST_SR,       /* Sampling rate for input circuit continuity test */
	ND_PAR_CH_TEST_RANGE,    /* Measurement range for input circuit continuity test */
	ND_PAR_CH_REF,           /* Connection of "-" input to common reference electrode */
	
	/* Added for new NO architecture: */
	ND_PAR_CH_CAP_CON,       /* Channel connection to EEG cap */
	/* <- new channel parameters can be added here */

	ND_CHAN_PAR_END          /* Have to be at the end of general parameter list */
};

/* Number of channel parameters */
#define ND_CHAN_PAR_NUM (ND_CHAN_PAR_END-ND_PAR_CH)

/* Notes:
	* ND_PAR_CH_EN parameter appears in all multi-channel devices. It influences
		accessibility of all other PAR_CH_*. If it is disabled, it is suggested
		to treat rest of the channel parameters as hidden (or at least disabled)
		(although their flags in NDPARAM structure remain unchanged).
	* Parameter ND_PAR_CH_PROF has special meaning. It is suggested to disable
		changes of all channel parameters after ND_PAR_CH_PROF when it is not set to
		0 ("User defined"). Note: this parameter may not exist in some devices.
	* In general, sets of PAR_CH_* parameters for individual channels can be
		different.
	* Digital samples are always signed and scaled according to ND_PAR_CH_RANGE_MAX,
		irrespective of ND_PAR_CH_RANGE_MIN (assuming that absolute value of the first
		parameter is greater or equal the second one). */

#endif /* __PARAM_H__ */
