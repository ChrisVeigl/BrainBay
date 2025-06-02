#pragma once
#include "common.h"
#include "types.h"


/// @file inlet.h Stream inlet functions

/** @defgroup lsl_inlet The lsl_inlet object
 * @{
 */

/**
 * Construct a new stream inlet from a resolved stream info.
 * @param info A resolved stream info object (as coming from one of the resolver functions).
 * @note The inlet makes a copy of the info object at its construction.
 * @note The stream_inlet may also be constructed with a fully-specified stream_info, if the desired
 * channel format and count is already known up-front, but this is  strongly discouraged and should
 * only ever be done if there is no time to resolve the  stream up-front (e.g., due to limitations
 * in the client program).
 * @param max_buflen Optionally the maximum amount of data to buffer (in seconds if there is a
 * nominal sampling rate, otherwise x100 in samples).
 *
 * Recording applications want to use a fairly large buffer size here, while real-time applications
 * would only buffer as much as they need to perform their next calculation.
 *
 * A good default is 360, which corresponds to 6 minutes of data.
 * @param max_chunklen Optionally the maximum size, in samples, at which chunks are transmitted.
 * If specified as 0, the chunk sizes preferred by the sender are used.
 * Recording applications can use a generous size here (leaving it to the network how  to pack
 * things), while real-time applications may want a finer (perhaps 1-sample) granularity.
 * @param recover Try to silently recover lost streams that are recoverable (=those that that have a
 * source_id set).
 *
 * It is generally a good idea to enable this, unless the application wants to act in a special way
 * when a data provider has temporarily crashed.
 *
 * If recover is 0 or the stream is not recoverable, most outlet functions will return an
 * #lsl_lost_error if the stream's source is lost.
 * @return A newly created lsl_inlet handle or NULL in the event that an error occurred.
 */
extern LIBLSL_C_API lsl_inlet lsl_create_inlet(lsl_streaminfo info, int32_t max_buflen, int32_t max_chunklen, int32_t recover);
/** @copydoc lsl_create_inlet()
 * @param flags An integer that is the result of bitwise OR'ing one or more options from
 * #lsl_transport_options_t together (e.g., #transp_bufsize_samples)
 */
extern LIBLSL_C_API lsl_inlet lsl_create_inlet_ex(lsl_streaminfo info, int32_t max_buflen,
	int32_t max_chunklen, int32_t recover, lsl_transport_options_t flags);

/**
* Destructor.
* The inlet will automatically disconnect if destroyed.
*/
extern LIBLSL_C_API void lsl_destroy_inlet(lsl_inlet in);

/**
 * Retrieve the complete information of the given stream, including the extended description.
 * Can be invoked at any time of the stream's lifetime.
 * @param in The lsl_inlet object to act on.
 * @param timeout Timeout of the operation. Use LSL_FOREVER to effectively disable it.
 * @param[out] ec Error code: if nonzero, can be either lsl_timeout_error (if the timeout has
 * expired) or #lsl_lost_error (if the stream source has been lost).
 * @return A copy of the full streaminfo of the inlet or NULL in the event that an error happened.
 * @note It is the user's responsibility to destroy it when it is no longer needed.
 */
extern LIBLSL_C_API lsl_streaminfo lsl_get_fullinfo(lsl_inlet in, double timeout, int32_t *ec);

/**
 * Subscribe to the data stream.
 *
 * All samples pushed in at the other end from this moment onwards will be queued and
 * eventually be delivered in response to pull_sample() calls.
 * Pulling a sample without some preceding lsl_open_stream() is permitted (the stream will then be
 * opened implicitly).
 * @param in The lsl_inlet object to act on.
 * @param timeout Optional timeout of the operation. Use LSL_FOREVER to effectively disable it.
 * @param[out] ec Error code: if nonzero, can be either #lsl_timeout_error (if the timeout has
 * expired) or lsl_lost_error (if the stream source has been lost).
 */
extern LIBLSL_C_API void lsl_open_stream(lsl_inlet in, double timeout, int32_t *ec);

/**
* Drop the current data stream.
*
* All samples that are still buffered or in flight will be dropped and transmission
* and buffering of data for this inlet will be stopped. If an application stops being
* interested in data from a source (temporarily or not) but keeps the outlet alive,
* it should call lsl_close_stream() to not waste unnecessary system and network
* resources.
*/
extern LIBLSL_C_API void lsl_close_stream(lsl_inlet in);

/**
 * @brief Retrieve an estimated time correction offset for the given stream.
 *
 * The first call to this function takes several milliseconds until a reliable first estimate is
 * obtained. Subsequent calls are instantaneous (and rely on periodic background updates).
 *
 * On a well-behaved network, the precision of these estimates should be below 1 ms (empirically it
 * is within +/-0.2 ms).
 *
 * To get a measure of whether the network is well-behaved, use #lsl_time_correction_ex and check
 * uncertainty (which maps to round-trip-time). 0.2 ms is typical of wired networks.
 *
 * 2 ms is typical of wireless networks. The number can be much higher on poor networks.
 *
 * @param in The lsl_inlet object to act on.
 * @param timeout Timeout to acquire the first time-correction estimate.
 * Use LSL_FOREVER to defuse the timeout.
 * @param[out] ec Error code: if nonzero, can be either #lsl_timeout_error (if the timeout has
 * expired) or lsl_lost_error (if the stream source has been lost).
 * @return The time correction estimate.
 * This is the number that needs to be added to a time stamp that was remotely generated via
 * lsl_local_clock() to map it into the local clock domain of this machine.
 */
extern LIBLSL_C_API double lsl_time_correction(lsl_inlet in, double timeout, int32_t *ec);
/** @copydoc lsl_time_correction()
 * @param remote_time The current time of the remote computer that was used to generate this
 * time_correction.
 * If desired, the client can fit time_correction vs remote_time to improve the real-time
 * time_correction further.
 * @param uncertainty The maximum uncertainty of the given time correction.
 */
extern LIBLSL_C_API double lsl_time_correction_ex(lsl_inlet in, double *remote_time, double *uncertainty, double timeout, int32_t *ec);


/**
 * Set post-processing flags to use.
 *
 * By default, the inlet performs NO post-processing and returns the ground-truth time stamps, which
 * can then be manually synchronized using time_correction(), and then smoothed/dejittered if
 * desired.
 *
 * This function allows automating these two and possibly more operations.
 * @warning When you enable this, you will no longer receive or be able to recover the original time
 * stamps.
 * @param in The lsl_inlet object to act on.
 * @param flags An integer that is the result of bitwise OR'ing one or more options from
 * #lsl_processing_options_t together (e.g., #proc_clocksync|#proc_dejitter);
 * a good setting is to use #proc_ALL.
 * @return The error code: if nonzero, can be #lsl_argument_error if an unknown flag was passed in.
 */
extern LIBLSL_C_API int32_t lsl_set_postprocessing(lsl_inlet in, uint32_t flags);


/* === Pulling a sample from the inlet === */

/**
 * Pull a sample from the inlet and read it into a pointer to values.
 * Handles type checking & conversion.
 * @param in The #lsl_inlet object to act on.
 * @param[out] buffer A pointer to hold the resulting values.
 * @param buffer_elements The number of samples allocated in the buffer.
 * @attention It is the responsibility of the user to allocate enough memory.
 * @param timeout The timeout for this operation, if any.
 * Use #LSL_FOREVER to effectively disable it. It is also permitted to use 0.0 here;
 * in this case a sample is only returned if one is currently buffered.
 * @param[out] ec Error code: can be either no error or #lsl_lost_error
 * (if the stream source has been lost).<br>
 * @note If the timeout expires before a new sample was received the function returns 0.0;
 * ec is *not* set to #lsl_timeout_error (because this case is not considered an error condition).
 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
 * available. To remap this time stamp to the local clock, add the value returned by
 * lsl_time_correction() to it.
 * @{
 */
extern LIBLSL_C_API double lsl_pull_sample_f(lsl_inlet in, float *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_d(lsl_inlet in, double *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_l(lsl_inlet in, int64_t *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_i(lsl_inlet in, int32_t *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_s(lsl_inlet in, int16_t *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_c(lsl_inlet in, char *buffer, int32_t buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API double lsl_pull_sample_str(lsl_inlet in, char **buffer, int32_t buffer_elements, double timeout, int32_t *ec);
///@}

/** @copydoc lsl_pull_sample_f
 * These strings may contains 0's, therefore the lengths are read into the buffer_lengths array.
 * @param buffer_lengths
 * A pointer to an array that holds the resulting lengths for each returned binary string.*/
extern LIBLSL_C_API double lsl_pull_sample_buf(lsl_inlet in, char **buffer, uint32_t *buffer_lengths, int32_t buffer_elements, double timeout, int32_t *ec);

/**
 * Pull a sample from the inlet and read it into a custom struct or buffer.
 *
 * Overall size checking but no type checking or conversion are done.
 * Do not use for variable-size/string-formatted streams.
 * @param in The #lsl_inlet object to act on.
 * @param[out] buffer A pointer to hold the resulting values.
 * @param buffer_bytes Length of the array held by buffer in bytes, not items
 * @param timeout The timeout for this operation, if any.
 * Use #LSL_FOREVER to effectively disable it. It is also permitted to use 0.0 here;
 * in this case a sample is only returned if one is currently buffered.
 * @param[out] ec Error code: can be either no error or #lsl_lost_error
 * (if the stream source has been lost).<br>
 * @note If the timeout expires before a new sample was received the function returns 0.0;
 * ec is *not* set to #lsl_timeout_error (because this case is not considered an error condition).
 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
 * available. To remap this time stamp to the local clock, add the value returned by
 * lsl_time_correction() to it.
 */
extern LIBLSL_C_API double lsl_pull_sample_v(lsl_inlet in, void *buffer, int32_t buffer_bytes, double timeout, int32_t *ec);

/**
 * Pull a chunk of data from the inlet and read it into a buffer.
 *
 * Handles type checking & conversion.
 *
 * @attention Note that the provided data buffer size is measured in channel values (e.g. floats)
 * rather than in samples.
 * @param in The lsl_inlet object to act on.
 * @param[out] data_buffer A pointer to a buffer of data values where the results shall be stored.
 * @param[out] timestamp_buffer A pointer to a double buffer where time stamps shall be stored.
 *
 * If this is NULL, no time stamps will be returned.
 * @param data_buffer_elements The size of the data buffer, in channel data elements (of type T).
 * Must be a multiple of the stream's channel count.
 * @param timestamp_buffer_elements The size of the timestamp buffer.
 *
 * If a timestamp buffer is provided then this must correspond to the same number of samples as
 * data_buffer_elements.
 * @param timeout The timeout for this operation, if any.
 *
 * When the timeout expires, the function may return before the entire buffer is filled.
 * The default value of 0.0 will retrieve only data available for immediate pickup.
 * @param[out] ec Error code: can be either no error or #lsl_lost_error (if the stream source has
 * been lost).
 * @note if the timeout expires before a new sample was received the function returns 0.0;
 * ec is *not* set to #lsl_timeout_error (because this case is not considered an error condition).
 * @return data_elements_written Number of channel data elements written to the data buffer.
 * @{
 */
extern LIBLSL_C_API unsigned long lsl_pull_chunk_f(lsl_inlet in, float *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_d(lsl_inlet in, double *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_l(lsl_inlet in, int64_t *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_i(lsl_inlet in, int32_t *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_s(lsl_inlet in, int16_t *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_c(lsl_inlet in, char *data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);
extern LIBLSL_C_API unsigned long lsl_pull_chunk_str(lsl_inlet in, char **data_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);

///@}

/**
 * Pull a chunk of data from the inlet and read it into an array of binary strings.
 *
 * These strings may contains 0's, therefore the lengths are read into the lengths_buffer array.
 * Handles type checking & conversion.
 * IMPORTANT: Note that the provided data buffer size is measured in channel values (e.g., floats)
 * rather than in samples.
 * @param in The lsl_inlet object to act on.
 * @param[out] data_buffer A pointer to a buffer of data values where the results shall be stored.
 * @param[out] lengths_buffer A pointer to an array that holds the resulting lengths for each
 * returned binary string.
 * @param timestamp_buffer A pointer to a buffer of timestamp values where time stamps shall be
 * stored. If this is NULL, no time stamps will be returned.
 * @param data_buffer_elements The size of the data buffer, in channel data elements (of type T).
 * Must be a multiple of the stream's channel count.
 * @param timestamp_buffer_elements The size of the timestamp buffer. If a timestamp buffer is
 * provided then this must correspond to the same number of samples as data_buffer_elements.
 * @param timeout The timeout for this operation, if any.
 *
 * When the timeout expires, the function may return before the entire buffer is filled.
 *
 * The default value of 0.0 will retrieve only data available for immediate pickup.
 * @param[out] ec Error code: can be either no error or #lsl_lost_error (if the stream source has
 * been lost).
 * @note If the timeout expires before a new sample was received the function returns 0.0; ec is
 * *not* set to #lsl_timeout_error (because this case is not considered an error condition).
 * @return data_elements_written Number of channel data elements written to the data buffer.
 */

extern LIBLSL_C_API unsigned long lsl_pull_chunk_buf(lsl_inlet in, char **data_buffer, uint32_t *lengths_buffer, double *timestamp_buffer, unsigned long data_buffer_elements, unsigned long timestamp_buffer_elements, double timeout, int32_t *ec);

/**
* Query whether samples are currently available for immediate pickup.
*
* Note that it is not a good idea to use samples_available() to determine whether
* a pull_*() call would block: to be sure, set the pull timeout to 0.0 or an acceptably
* low value. If the underlying implementation supports it, the value will be the number of
* samples available (otherwise it will be 1 or 0).
*/
extern LIBLSL_C_API uint32_t lsl_samples_available(lsl_inlet in);

/// Drop all queued not-yet pulled samples, return the nr of dropped samples
extern LIBLSL_C_API uint32_t lsl_inlet_flush(lsl_inlet in);

/**
* Query whether the clock was potentially reset since the last call to lsl_was_clock_reset().
*
* This is rarely-used function is only needed for applications that combine multiple time_correction
* values to estimate precise clock drift if they should tolerate cases where the source machine was
* hot-swapped or restarted.
*/
extern LIBLSL_C_API uint32_t lsl_was_clock_reset(lsl_inlet in);

/**
 * Override the half-time (forget factor) of the time-stamp smoothing.
 *
 * The default is 90 seconds unless a different value is set in the config file.
 *
 * Using a longer window will yield lower jitter in the time stamps, but longer windows will have
 * trouble tracking changes in the clock rate (usually due to  temperature changes); the default is
 * able to track changes up to 10  degrees C per minute sufficiently well.
 * @param in The lsl_inlet object to act on.
 * @param value The new value, in seconds. This is the time after which a past sample
 *			   will be weighted by 1/2 in the exponential smoothing window.
 * @return The error code: if nonzero, can be #lsl_argument_error if an unknown flag was passed in.
 */
extern LIBLSL_C_API int32_t lsl_smoothing_halftime(lsl_inlet in, float value);

/// @}
