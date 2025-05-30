#pragma once

//! @file common.h Global constants for liblsl

#if defined(LIBLSL_FFI)
// Skip any typedefs that might confuse a FFI header parser, e.g. cffi
#elif defined(_MSC_VER) && _MSC_VER < 1600
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
#define __func__ __FUNCTION__
#endif

/// LIBLSL_C_API expands function attributes needed for the linker
#if defined(LIBLSL_STATIC) || defined(LIBLSL_FFI)
#define LIBLSL_C_API
#elif defined _WIN32 || defined __CYGWIN__
#if defined LIBLSL_EXPORTS
#define LIBLSL_C_API __declspec(dllexport)
#else
#define LIBLSL_C_API __declspec(dllimport)
#ifndef LSLNOAUTOLINK
#pragma comment(lib, "lsl.lib")
#endif
#endif
#pragma warning(disable : 4275)
#else // Linux / OS X
#define LIBLSL_C_API __attribute__((visibility("default")))
#endif

/// Constant to indicate that a stream has variable sampling rate.
#define LSL_IRREGULAR_RATE 0.0

/**
 * Constant to indicate that a sample has the next successive time stamp.
 *
 * This is an optional optimization to transmit less data per sample.
 * The stamp is then deduced from the preceding one according to the stream's
 * sampling rate (in the case of an irregular rate, the same time stamp as
 * before will is assumed). */
#define LSL_DEDUCED_TIMESTAMP -1.0

/// A very large time value (ca. 1 year); can be used in timeouts.
#define LSL_FOREVER 32000000.0

/**
 * Constant to indicate that there is no preference about how a data stream
 * shall be chunked for transmission.
 * (can be used for the chunking parameters in the inlet or the outlet).
 */
#define LSL_NO_PREFERENCE 0

/// Data format of a channel (each transmitted sample holds an array of channels), 4 bytes wide
typedef enum {
	/** For up to 24-bit precision measurements in the appropriate physical unit (e.g., microvolts).
	 * Integers from -16777216 to 16777216 are represented accurately. */
	cft_float32 = 1,
	/** For universal numeric data as long as permitted by network & disk budget.
	 * The largest representable integer is 53-bit. */
	cft_double64 = 2,
	/** For variable-length ASCII strings or data blobs, such as video frames, complex event
	   descriptions, etc. */
	cft_string = 3,
	/** For high-rate digitized formats that require 32-bit precision.
	 * Depends critically on meta-data to represent meaningful units.
	 * Useful for application event codes or other coded data. */
	cft_int32 = 4,
	/** For very high rate signals (40Khz+) or consumer-grade audio.
	 * For professional audio float is recommended. */
	cft_int16 = 5,
	/// For binary signals or other coded data. Not recommended for encoding string data.
	cft_int8 = 6,
	/** 64 bit integers. Support for this type is not yet exposed in all languages.
	 * Also, some builds of liblsl will not be able to send or receive data of this type. */
	cft_int64 = 7,
	/// Can not be transmitted.
	cft_undefined = 0,

	// prevent compilers from assuming an instance fits in a single byte
	_cft_maxval = 0x7f000000
} lsl_channel_format_t;

// Abort compilation if lsl_channel_format_t isn't exactly 4 bytes wide
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(lsl_channel_format_t) == 4, "lsl_channel_format_t size breaks the LSL ABI");
#elif defined(__cplusplus) && __cplusplus >= 201103L
static_assert (sizeof(lsl_channel_format_t) == 4, "lsl_channel_format_t size breaks the LSL ABI");
#elif !defined(LIBLSL_FFI)
static char _lsl_channel_format_size_check[1 - 2*!(sizeof(lsl_channel_format_t)==4)];
#endif

/// Post-processing options for stream inlets.
typedef enum {
	/** No automatic post-processing; return the ground-truth time stamps for manual
	   post-processing. This is the default behavior of the inlet. */
	proc_none = 0,

	/** Perform automatic clock synchronization; equivalent to manually adding the time_correction()
	   value to the received time stamps. */
	proc_clocksync = 1,

	/** Remove jitter from time stamps.
	 *
	 * This will apply a smoothing algorithm to the received time stamps; the smoothing needs to see
	 * a minimum number of samples (30-120 seconds worst-case) until the remaining jitter is
	 * consistently below 1ms. */
	proc_dejitter = 2,

	/** Force the time-stamps to be monotonically ascending.
	 *
	 * Only makes sense if timestamps are dejittered. */
	proc_monotonize = 4,

	/** Post-processing is thread-safe (same inlet can be read from by multiple threads);
	 * uses somewhat more CPU. */
	proc_threadsafe = 8,

	/// The combination of all possible post-processing options.
	proc_ALL = 1 | 2 | 4 | 8,

	// prevent compilers from assuming an instance fits in a single byte
	_proc_maxval = 0x7f000000
} lsl_processing_options_t;

/// Possible error codes.
typedef enum {
	/// No error occurred
	lsl_no_error = 0,

	/// The operation failed due to a timeout.
	lsl_timeout_error = -1,

	/// The stream has been lost.
	lsl_lost_error = -2,

	/// An argument was incorrectly specified (e.g., wrong format or wrong length).
	lsl_argument_error = -3,

	/// Some other internal error has happened.
	lsl_internal_error = -4,

	// prevent compilers from assuming an instance fits in a single byte
	_lsl_error_code_maxval = 0x7f000000
} lsl_error_code_t;

/// Flags for outlet_ex and inlet_ex
typedef enum {
	/// Keep legacy behavior: max_buffered / max_buflen is in seconds; use asynch transfer.
	transp_default = 0,

	/// The supplied max_buf value is in samples.
	transp_bufsize_samples = 1,

	/// The supplied max_buf should be scaled by 0.001.
	transp_bufsize_thousandths = 2,

	// prevent compilers from assuming an instance fits in a single byte
	_lsl_transport_options_maxval = 0x7f000000
} lsl_transport_options_t;

/// Return an explanation for the last error
extern LIBLSL_C_API const char *lsl_last_error(void);

/**
 * LSL version the binary was compiled against
 *
 * Used either to check if the same version is used
 * (`if(lsl_protocol_version()!=LIBLSL_COMPILE_HEADER_VERSION`) …
 * or to require a certain set of features:
 * ```
 * #if LIBLSL_COMPILE_HEADER_VERSION > 113
 * do_stuff();
 * #endif
 * ```
 * */
#define LIBLSL_COMPILE_HEADER_VERSION 114

/**
 * Protocol version.
 *
 * The major version is `protocol_version() / 100;`
 * The minor version is `protocol_version() % 100;`
 *
 * Clients with different minor versions are protocol-compatible while clients
 * with different major versions will refuse to work together.
 */
extern LIBLSL_C_API int32_t lsl_protocol_version();

/**
 * Version of the liblsl library.
 *
 * The major version is `library_version() / 100;`
 * The minor version is `library_version() % 100;`
 */
extern LIBLSL_C_API int32_t lsl_library_version();

/**
 * Get a string containing library information.
 *
 * The format of the string shouldn't be used for anything important except giving a debugging
 * person a good idea which exact library version is used. */
extern LIBLSL_C_API const char *lsl_library_info(void);

/**
 * Obtain a local system time stamp in seconds.
 *
 * The resolution is better than a millisecond.
 * This reading can be used to assign time stamps to samples as they are being acquired.
 * If the "age" of a sample is known at a particular time (e.g., from USB transmission
 * delays), it can be used as an offset to lsl_local_clock() to obtain a better estimate of
 * when a sample was actually captured. See lsl_push_sample() for a use case.
 */
extern LIBLSL_C_API double lsl_local_clock();

/**
 * Deallocate a string that has been transferred to the application.
 *
 * Rarely used: the only use case is to deallocate the contents of
 * string-valued samples received from LSL in an application where
 * no free() method is available (e.g., in some scripting languages).
 */
extern LIBLSL_C_API void lsl_destroy_string(char *s);
