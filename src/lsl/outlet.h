#pragma once
#include "./common.h"
#include "types.h"

/// @file outlet.h Stream outlet functions

/** @defgroup outlet The lsl_outlet object
 *
 * This object represents an outlet sending data to all connected inlets.
 *
 * The data is pushed sample-by-sample or chunk-by-chunk into the outlet, and can consist of single-
 * or multichannel data, regular or irregular sampling rate, with uniform value types (integers,
 * floats, doubles, strings).
 *
 * Streams can have arbitrary XML meta-data (akin to a file header).
 * By creating an outlet the stream is made visible to a collection of computers (defined by the
 * network settings/layout) where one can subscribe to it by creating an inlet.
 * @{
 */

/**
 * Establish a new stream outlet. This makes the stream discoverable.
 * @param info The stream information to use for creating this stream.
 * Stays constant over the lifetime of the outlet.
 * @note the outlet makes a copy of the streaminfo object upon construction (so the old info should
 * still be destroyed.)
 * @param chunk_size Optionally the desired chunk granularity (in samples) for transmission.
 * If specified as 0, each push operation yields one chunk.
 * Stream recipients can have this setting bypassed.
 * @param max_buffered Optionally the maximum amount of data to buffer (in seconds if there is a
 * nominal  sampling rate, otherwise x100 in samples). A good default is 360, which corresponds to 6
 * minutes of data. Note that, for high-bandwidth data you will almost certainly want to use a lower
 * value here to avoid  running out of RAM.
 * @return A newly created lsl_outlet handle or NULL in the event that an error occurred.
 */
extern LIBLSL_C_API lsl_outlet lsl_create_outlet(lsl_streaminfo info, int32_t chunk_size, int32_t max_buffered);
/** @copydoc lsl_create_outlet()
 * @param flags An integer that is the result of bitwise OR'ing one or more options from
 * #lsl_transport_options_t together (e.g., #transp_bufsize_samples|#transp_bufsize_thousandths)
 */
extern LIBLSL_C_API lsl_outlet lsl_create_outlet_ex(
	lsl_streaminfo info, int32_t chunk_size, int32_t max_buffered, lsl_transport_options_t flags);

/**
 * Destroy an outlet.
 * The outlet will no longer be discoverable after destruction and all connected inlets will stop
 * delivering data.
 */
extern LIBLSL_C_API void lsl_destroy_outlet(lsl_outlet out);

/** Push a pointer to some values as a sample into the outlet.
 * Handles type checking & conversion.
 * @param out The lsl_outlet object through which to push the data.
 * @param data A pointer to values to push. The number of values pointed to must be no less than the
 * number of channels in the sample.
 * #lsl_local_clock(); if omitted, the current time is used.
 * @return Error code of the operation or lsl_no_error if successful (usually attributed to the
 * wrong data type).
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_sample_f(lsl_outlet out, const float *data);
extern LIBLSL_C_API int32_t lsl_push_sample_d(lsl_outlet out, const double *data);
extern LIBLSL_C_API int32_t lsl_push_sample_l(lsl_outlet out, const int64_t *data);
extern LIBLSL_C_API int32_t lsl_push_sample_i(lsl_outlet out, const int32_t *data);
extern LIBLSL_C_API int32_t lsl_push_sample_s(lsl_outlet out, const int16_t *data);
extern LIBLSL_C_API int32_t lsl_push_sample_c(lsl_outlet out, const char *data);
extern LIBLSL_C_API int32_t lsl_push_sample_str(lsl_outlet out, const char **data);
extern LIBLSL_C_API int32_t lsl_push_sample_v(lsl_outlet out, const void *data);
/// @}
/** @copydoc lsl_push_sample_f
 * @param timestamp Optionally the capture time of the sample, in agreement with
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_sample_ft(lsl_outlet out, const float *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_dt(lsl_outlet out, const double *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_lt(lsl_outlet out, const int64_t *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_it(lsl_outlet out, const int32_t *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_st(lsl_outlet out, const int16_t *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_ct(lsl_outlet out, const char *data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_strt(lsl_outlet out, const char **data, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_sample_vt(lsl_outlet out, const void *data, double timestamp);
/// @}
/** @copydoc lsl_push_sample_ft
 * @param pushthrough Whether to push the sample through to the receivers instead of buffering it
 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
 * precedence over the pushthrough flag.
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_sample_ftp(lsl_outlet out, const float *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_dtp(lsl_outlet out, const double *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_ltp(lsl_outlet out, const int64_t *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_itp(lsl_outlet out, const int32_t *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_stp(lsl_outlet out, const int16_t *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_ctp(lsl_outlet out, const char *data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_strtp(lsl_outlet out, const char **data, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_sample_vtp(lsl_outlet out, const void *data, double timestamp, int32_t pushthrough);
///@}

/** @copybrief lsl_push_sample_ftp
 * @see lsl_push_sample_ftp
 * @param out The lsl_outlet object through which to push the data.
 * @param data A pointer to values to push. The number of values pointed to must be no less than the
 * number of channels in the sample.
 * @param lengths A pointer the number of elements to push for each channel (string lengths).
 */
extern LIBLSL_C_API int32_t lsl_push_sample_buf(lsl_outlet out, const char **data, const uint32_t *lengths);
/** @copydoc lsl_push_sample_buf
 * @param timestamp @see lsl_push_sample_ftp */
extern LIBLSL_C_API int32_t lsl_push_sample_buft(lsl_outlet out, const char **data, const uint32_t *lengths, double timestamp);
/** @copydoc lsl_push_sample_buft
 * @param pushthrough @see lsl_push_sample_ftp */
extern LIBLSL_C_API int32_t lsl_push_sample_buftp(lsl_outlet out, const char **data, const uint32_t *lengths, double timestamp, int32_t pushthrough);

/** Push a chunk of multiplexed samples into the outlet. One timestamp per sample is provided.
 *
 * @attention Note that the provided buffer size is measured in channel values (e.g. floats) rather
 * than in samples.
 *
 * Handles type checking & conversion.
 * @param out The lsl_outlet object through which to push the data.
 * @param data A buffer of channel values holding the data for zero or more successive samples to
 * send.
 * @param data_elements The number of data values (of type T) in the data buffer. Must be a multiple
 * of the channel count.
 * @return Error code of the operation (usually attributed to the wrong data type).
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_f(lsl_outlet out, const float *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_d(lsl_outlet out, const double *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_l(lsl_outlet out, const int64_t *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_i(lsl_outlet out, const int32_t *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_s(lsl_outlet out, const int16_t *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_c(lsl_outlet out, const char *data, unsigned long data_elements);
extern LIBLSL_C_API int32_t lsl_push_chunk_str(lsl_outlet out, const char **data, unsigned long data_elements);
/// @}

/** @copydoc lsl_push_chunk_f
 * @param timestamp Optionally the capture time of the most recent sample, in agreement with
 * lsl_local_clock(); if omitted, the current time is used.
 * The time stamps of other samples are automatically derived based on the sampling rate of the
 * stream.
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_ft(lsl_outlet out, const float *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_dt(lsl_outlet out, const double *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_lt(lsl_outlet out, const int64_t *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_it(lsl_outlet out, const int32_t *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_st(lsl_outlet out, const int16_t *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_ct(lsl_outlet out, const char *data, unsigned long data_elements, double timestamp);
extern LIBLSL_C_API int32_t lsl_push_chunk_strt(lsl_outlet out, const char **data, unsigned long data_elements, double timestamp);
/// @}

/** @copydoc lsl_push_chunk_ft
 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
 * precedence over the pushthrough flag.
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_ftp(lsl_outlet out, const float *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_dtp(lsl_outlet out, const double *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_ltp(lsl_outlet out, const int64_t *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_itp(lsl_outlet out, const int32_t *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_stp(lsl_outlet out, const int16_t *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_ctp(lsl_outlet out, const char *data, unsigned long data_elements, double timestamp, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_strtp(lsl_outlet out, const char **data, unsigned long data_elements, double timestamp, int32_t pushthrough);
/// @}
/** @copydoc lsl_push_chunk_f
 * @param timestamps Buffer holding one time stamp for each sample in the data buffer.
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_ftn(lsl_outlet out, const float *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_dtn(lsl_outlet out, const double *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_ltn(lsl_outlet out, const int64_t *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_itn(lsl_outlet out, const int32_t *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_stn(lsl_outlet out, const int16_t *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_ctn(lsl_outlet out, const char *data, unsigned long data_elements, const double *timestamps);
extern LIBLSL_C_API int32_t lsl_push_chunk_strtn(lsl_outlet out, const char **data, unsigned long data_elements, const double *timestamps);
/// @}

/** @copydoc lsl_push_chunk_ftn
 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
 * precedence over the pushthrough flag.
 * @{
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_ftnp(lsl_outlet out, const float *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_dtnp(lsl_outlet out, const double *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_ltnp(lsl_outlet out, const int64_t *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_itnp(lsl_outlet out, const int32_t *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_stnp(lsl_outlet out, const int16_t *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_ctnp(lsl_outlet out, const char *data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
extern LIBLSL_C_API int32_t lsl_push_chunk_strtnp(lsl_outlet out, const char **data, unsigned long data_elements, const double *timestamps, int32_t pushthrough);
///@}

/** @copybrief lsl_push_chunk_ftp
 * @sa lsl_push_chunk_ftp
 * @param out The lsl_outlet object through which to push the data.
 * @param data An array of channel values holding the data to push.
 * @param lengths Pointer the number of elements to push for each value (string lengths) so that
 * `size(data[i])==lengths[i]`.
 * @param data_elements The number of data values in the data buffer.
 * Must be a multiple of the channel count.
 */
extern LIBLSL_C_API int32_t lsl_push_chunk_buf(lsl_outlet out, const char **data, const uint32_t *lengths, unsigned long data_elements);

/** @copydoc lsl_push_chunk_buf @sa lsl_push_chunk_ftp @sa lsl_push_chunk_buf
 * @param timestamp Optionally the capture time of the most recent sample, in agreement with
 * lsl_local_clock(); if omitted, the current time is used.
 * The time stamps of other samples are automatically derived based on the sampling rate of the
 * stream. */
extern LIBLSL_C_API int32_t lsl_push_chunk_buft(lsl_outlet out, const char **data, const uint32_t *lengths, unsigned long data_elements, double timestamp);

/** @copydoc lsl_push_chunk_buft @sa lsl_push_chunk_ftp @sa lsl_push_chunk_buf
 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
 * precedence over the pushthrough flag. */
extern LIBLSL_C_API int32_t lsl_push_chunk_buftp(lsl_outlet out, const char **data, const uint32_t *lengths, unsigned long data_elements, double timestamp, int32_t pushthrough);

/** @copydoc lsl_push_chunk_buf @sa lsl_push_chunk_ftp @sa lsl_push_chunk_buf
 * @param timestamps Buffer holding one time stamp for each sample in the data buffer. */
extern LIBLSL_C_API int32_t lsl_push_chunk_buftn(lsl_outlet out, const char **data, const uint32_t *lengths, unsigned long data_elements, const double *timestamps);

/** @copydoc lsl_push_chunk_buftn @sa lsl_push_chunk_ftp @sa lsl_push_chunk_buf
 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
 * precedence over the pushthrough flag. */
extern LIBLSL_C_API int32_t lsl_push_chunk_buftnp(lsl_outlet out, const char **data, const uint32_t *lengths, unsigned long data_elements, const double *timestamps, int32_t pushthrough);

/**
* Check whether consumers are currently registered.
* While it does not hurt, there is technically no reason to push samples if there is no consumer.
*/
extern LIBLSL_C_API int32_t lsl_have_consumers(lsl_outlet out);

/**
* Wait until some consumer shows up (without wasting resources).
* @return True if the wait was successful, false if the timeout expired.
*/
extern LIBLSL_C_API int32_t lsl_wait_for_consumers(lsl_outlet out, double timeout);

/**
 * Retrieve a handle to the stream info provided by this outlet.
 * This is what was used to create the stream (and also has the Additional Network Information
 * fields assigned).
 * @return A copy of the streaminfo of the outlet or NULL in the event that an error occurred.
 * @note It is the user's responsibility to destroy it when it is no longer needed.
 * @sa lsl_destroy_string()
 */
extern LIBLSL_C_API lsl_streaminfo lsl_get_info(lsl_outlet out);

///@}
