#pragma once
#include "common.h"
#include "types.h"

/// @file streaminfo.h Stream info functions

/** @defgroup streaminfo The lsl_streaminfo object
 *
 * The #lsl_streaminfo object keeps a stream's meta data and connection settings.
 * @{
 */

/**
 * Construct a new streaminfo object.
 *
 * Core stream information is specified here. Any remaining meta-data can be added later.
 * @param name Name of the stream.<br>
 * Describes the device (or product series) that this stream makes available
 *             (for use by programs, experimenters or data analysts). Cannot be empty.
 * @param type Content type of the stream. Please see https://github.com/sccn/xdf/wiki/Meta-Data (or
 * web search for: XDF meta-data) for pre-defined content-type names, but you can also make up your
 * own. The content type is the preferred way to find streams (as opposed to searching by name).
 * @param channel_count Number of channels per sample.
 * This stays constant for the lifetime of the stream.
 * @param nominal_srate The sampling rate (in Hz) as advertised by the
 * datasource, if regular (otherwise set to #LSL_IRREGULAR_RATE).
 * @param channel_format Format/type of each channel.<br>
 * If your channels have different formats, consider supplying multiple streams
 * or use the largest type that can hold them all (such as #cft_double64).
 *
 * A good default is #cft_float32.
 * @param source_id Unique identifier of the source or device, if available (e.g. a serial number).
 * Allows recipients to recover from failure even after the serving app or device crashes.
 * May in some cases also be constructed from device settings.
 * @return A newly created streaminfo handle or NULL in the event that an error occurred.
 */
extern LIBLSL_C_API lsl_streaminfo lsl_create_streaminfo(const char *name, const char *type, int32_t channel_count, double nominal_srate, lsl_channel_format_t channel_format, const char *source_id);

/// Destroy a previously created streaminfo object.
extern LIBLSL_C_API void lsl_destroy_streaminfo(lsl_streaminfo info);

/// Copy an existing streaminfo object (rarely used).
extern LIBLSL_C_API lsl_streaminfo lsl_copy_streaminfo(lsl_streaminfo info);

/**
 * Name of the stream.
 *
 * This is a human-readable name.
 * For streams offered by device modules, it refers to the type of device or product series  that is
 * generating the data of the stream. If the source is an application, the name may be a more
 * generic or specific identifier. Multiple streams with the same name can coexist, though
 * potentially at the cost of ambiguity (for the recording app or experimenter).
 * @return An immutable library-owned pointer to the string value. @sa lsl_destroy_string()
 */
extern LIBLSL_C_API const char *lsl_get_name(lsl_streaminfo info);

/**
 * Content type of the stream.
 *
 * The content type is a short string such as "EEG", "Gaze" which describes the content carried by
 * the channel (if known). If a stream contains mixed content this value need not be assigned but
 * may instead be stored in the description of channel types. To be useful to applications and
 * automated processing systems using the recommended content types is preferred. Content types
 * usually follow those pre-defined in the [wiki](https://github.com/sccn/xdf/wiki/Meta-Data) (or
 * web search for: XDF meta-data).
 * @return An immutable library-owned pointer to the string value. @sa lsl_destroy_string()
 */
extern LIBLSL_C_API const char *lsl_get_type(lsl_streaminfo info);

/**
* Number of channels of the stream.
* A stream has at least one channels; the channel count stays constant for all samples.
*/
extern LIBLSL_C_API int32_t lsl_get_channel_count(lsl_streaminfo info);

/**
 * Sampling rate of the stream, according to the source (in Hz).
 *
 * If a stream is irregularly sampled, this should be set to #LSL_IRREGULAR_RATE.
 *
 * Note that no data will be lost even if this sampling rate is incorrect or if a device has
 * temporary hiccups, since all samples will be recorded anyway (except for those dropped by the
 * device itself). However, when the recording is imported into an application, a good importer may
 * correct such errors more accurately if the advertised sampling rate was close to the specs of the
 * device.
 */
extern LIBLSL_C_API double lsl_get_nominal_srate(lsl_streaminfo info);

/**
 * Channel format of the stream.
 * All channels in a stream have the same format.
 * However, a device might offer multiple time-synched streams  each with its own format.
 */
extern LIBLSL_C_API lsl_channel_format_t lsl_get_channel_format(lsl_streaminfo info);

/**
 * Unique identifier of the stream's source, if available.
 *
 * The unique source (or device) identifier is an optional piece of information that, if available,
 * allows that endpoints (such as the recording program) can re-acquire a stream automatically once
 * it is back online.
 * @return An immutable library-owned pointer to the string value. @sa lsl_destroy_string()
 */
extern LIBLSL_C_API const char *lsl_get_source_id(lsl_streaminfo info);

/**
* Protocol version used to deliver the stream.
*/
extern LIBLSL_C_API int32_t lsl_get_version(lsl_streaminfo info);

/**
* Creation time stamp of the stream.
*
* This is the time stamp when the stream was first created
* (as determined via local_clock() on the providing machine).
*/
extern LIBLSL_C_API double lsl_get_created_at(lsl_streaminfo info);

/**
 * Unique ID of the stream outlet (once assigned).
 *
 * This is a unique identifier of the stream outlet, and is guaranteed to be different
 * across multiple instantiations of the same outlet (e.g., after a re-start).
 * @return An immutable library-owned pointer to the string value. @sa lsl_destroy_string()
 */
extern LIBLSL_C_API const char *lsl_get_uid(lsl_streaminfo info);

/**
 * Session ID for the given stream.
 *
 * The session id is an optional human-assigned identifier of the recording session.
 * While it is rarely used, it can be used to prevent concurrent recording activitites
 * on the same sub-network (e.g., in multiple experiment areas) from seeing each other's streams
 * (assigned via a configuration file by the experimenter, see Network Connectivity on the LSL
 * wiki).
 * @return An immutable library-owned pointer to the string value. @sa lsl_destroy_string()
 */
extern LIBLSL_C_API const char *lsl_get_session_id(lsl_streaminfo info);

/// Hostname of the providing machine (once bound to an outlet). Modification is not permitted.
extern LIBLSL_C_API const char *lsl_get_hostname(lsl_streaminfo info);

/**
* Extended description of the stream.
*
* It is highly recommended that at least the channel labels are described here.
* See code examples on the LSL wiki. Other information, such as amplifier settings,
* measurement units if deviating from defaults, setup information, subject information, etc.,
* can be specified here, as well. Meta-data recommendations follow the XDF file format project
* (github.com/sccn/xdf/wiki/Meta-Data or web search for: XDF meta-data).
*
* @attention if you use a stream content type for which meta-data recommendations exist, please
* try to lay out your meta-data in agreement with these recommendations for compatibility with other applications.
*/
extern LIBLSL_C_API lsl_xml_ptr lsl_get_desc(lsl_streaminfo info);

/**
 * Retrieve the entire streaminfo in XML format.
 *
 * This yields an XML document (in string form) whose top-level element is `<info>`. The info
 * element contains one element for each field of the streaminfo class, including:
 *
 *   - the core elements `<name>`, `<type>`, `<channel_count`, `<nominal_srate>`,
 *   `<channel_format>`, `<source_id>`
 *   - the misc elements `<version>`, `<created_at>`, `<uid>`, `<session_id>`,
 *   `<v4address>`, `<v4data_port>`, `<v4service_port>`, `<v6address>`, `<v6data_port>`,
 *   `<v6service_port>`
 *   - the extended description element `<desc>` with user-defined sub-elements.
 * @return A pointer to a copy of the XML text or NULL in the event that an error occurred.
 * @note It is the user's responsibility to deallocate this string when it is no longer needed.
 */
extern LIBLSL_C_API char *lsl_get_xml(lsl_streaminfo info);

/// Number of bytes occupied by a channel (0 for string-typed channels).
extern LIBLSL_C_API int32_t lsl_get_channel_bytes(lsl_streaminfo info);

/// Number of bytes occupied by a sample (0 for string-typed channels).
extern LIBLSL_C_API int32_t lsl_get_sample_bytes(lsl_streaminfo info);

/**
 * Tries to match the stream info XML element @p info against an
 * <a href="https://en.wikipedia.org/wiki/XPath#Syntax_and_semantics_(XPath_1.0)">XPath</a> query.
 *
 * Example query strings:
 * @code
 * channel_count>5 and type='EEG'
 * type='TestStream' or contains(name,'Brain')
 * name='ExampleStream'
 * @endcode
 */
extern LIBLSL_C_API int32_t lsl_stream_info_matches_query(lsl_streaminfo info, const char *query);

/// Create a streaminfo object from an XML representation
extern LIBLSL_C_API lsl_streaminfo lsl_streaminfo_from_xml(const char *xml);

/// @}
