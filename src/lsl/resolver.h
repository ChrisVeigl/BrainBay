#pragma once
#include "common.h"
#include "types.h"

/// @file resolver.h Stream resolution functions

/** @defgroup continuous_resolver The lsl_continuous_resolver
 * @ingroup resolve
 *
 * Streams can be resolved at a single timepoint once (@ref resolve) or continuously in the
 * background.
 * @{
 */

/**
 * Construct a new #lsl_continuous_resolver that resolves all streams on the network.
 *
 * This is analogous to the functionality offered by the free function lsl_resolve_streams().
 * @param forget_after When a stream is no longer visible on the network (e.g. because it was shut
 * down), this is the time in seconds after which it is no longer reported by the resolver.
 *
 * The recommended default value is 5.0.
 */
extern LIBLSL_C_API lsl_continuous_resolver lsl_create_continuous_resolver(double forget_after);

/**
 * Construct a new lsl_continuous_resolver that resolves all streams with a specific value for a given
 * property.
 *
 * This is analogous to the functionality provided by the free function lsl_resolve_byprop()
 * @param prop The #lsl_streaminfo property that should have a specific value (e.g., "name", "type",
 * "source_id", or "desc/manufaturer").
 * @param value The string value that the property should have (e.g., "EEG" as the type property).
 * @param forget_after When a stream is no longer visible on the network (e.g., because it was shut
 * down), this is the time in seconds after which it is no longer reported by the resolver.
 * The recommended default value is 5.0.
 */
extern LIBLSL_C_API lsl_continuous_resolver lsl_create_continuous_resolver_byprop(const char *prop, const char *value, double forget_after);

/**
 * Construct a new lsl_continuous_resolver that resolves all streams that match a given XPath 1.0
 * predicate.
 *
 * This is analogous to the functionality provided by the free function lsl_resolve_bypred()
 * @param pred The predicate string, e.g.
 * `"name='BioSemi'" or "type='EEG' and starts-with(name,'BioSemi') and count(info/desc/channel)=32"`
 * @param forget_after When a stream is no longer visible on the network (e.g., because it was shut
 * down), this is the time in seconds after which it is no longer reported by the resolver.
 * The recommended default value is 5.0.
 */
extern LIBLSL_C_API lsl_continuous_resolver lsl_create_continuous_resolver_bypred(const char *pred, double forget_after);

/**
 * Obtain the set of currently present streams on the network (i.e. resolve result).
 *
 * @param res A continuous resolver (previously created with one of the
 * lsl_create_continuous_resolver() functions).
 * @param buffer A user-allocated buffer to hold the current resolve results.<br>
 * @attention It is the user's responsibility to either destroy the resulting streaminfo objects or
 * to pass them back to the LSL during during creation of an inlet.
 * @attention The stream_infos returned by the resolver are only short versions that do not include
 * the lsl_get_desc() field (which can be arbitrarily big).
 *
 * To obtain the full stream information you need to call lsl_get_info() on the inlet after you have
 * created one.
 * @param buffer_elements The user-provided buffer length.
 * @return The number of results written into the buffer (never more than the provided # of slots)
 * or a negative number if an error has occurred (values corresponding to #lsl_error_code_t).
 */
extern LIBLSL_C_API int32_t lsl_resolver_results(lsl_continuous_resolver res, lsl_streaminfo *buffer, uint32_t buffer_elements);

/// Destructor for the continuous resolver.
extern LIBLSL_C_API void lsl_destroy_continuous_resolver(lsl_continuous_resolver res);

/// @}

/** @defgroup resolve Resolving streams on the network
 * @{*/

/**
 * Resolve all streams on the network.
 *
 * This function returns all currently available streams from any outlet on the network.
 * The network is usually the subnet specified at the local router, but may also include a multicast
 * group of machines (given that the network supports it), or a list of hostnames.<br>
 * These details may optionally be customized by the experimenter in a configuration file
 * (see page Network Connectivity in the LSL wiki).
 * This is the default mechanism used by the browsing programs and the recording program.
 * @param[out] buffer A user-allocated buffer to hold the resolve results.
 * @attention It is the user's responsibility to either destroy the resulting streaminfo objects or
 * to pass them back to the LSL during during creation of an inlet.
 *
 * @attention The stream_info's returned by the resolver are only short versions that do not include
 * the lsl_get_desc() field (which can be arbitrarily big).
 * To obtain the full stream information you need to call lsl_get_info() on the inlet after you have
 * created one.
 * @param buffer_elements The user-provided buffer length.
 * @param wait_time The waiting time for the operation, in seconds, to search for streams.
 * The recommended wait time is 1 second (or 2 for a busy and large recording operation).
 * @warning If this is too short (<0.5s) only a subset (or none) of the outlets that are present on
 * the network may be returned.
 * @return The number of results written into the buffer (never more than the provided # of slots)
 *         or a negative number if an error has occurred (values corresponding to lsl_error_code_t).
 */
extern LIBLSL_C_API int32_t lsl_resolve_all(lsl_streaminfo *buffer, uint32_t buffer_elements, double wait_time);

/**
 * Resolve all streams with a given value for a property.
 *
 * If the goal is to resolve a specific stream, this method is preferred over resolving all streams
 * and then selecting the desired one.
 * @param[out] buffer A user-allocated buffer to hold the resolve results.
 * @attention It is the user's responsibility to either destroy the resulting streaminfo objects or
 * to pass them back to the LSL during during creation of an inlet.
 *
 * @attention The stream_info's returned by the resolver are only short versions that do not include
 * the lsl_get_desc() field (which can be arbitrarily big). To obtain the full stream information
 * you need to call lsl_get_info() on the inlet after you have created one.
 * @param buffer_elements The user-provided buffer length.
 * @param prop The streaminfo property that should have a specific value (`"name"`, `"type"`,
 * `"source_id"`, or, e.g., `"desc/manufaturer"` if present).
 * @param value The string value that the property should have (e.g., "EEG" as the type).
 * @param minimum Return at least this number of streams.
 * @param timeout Optionally a timeout of the operation, in seconds (default: no timeout).
 * If the timeout expires, less than the desired number of streams (possibly none) will be returned.
 * @return The number of results written into the buffer (never more than the provided # of slots)
 * or a negative number if an error has occurred (values corresponding to #lsl_error_code_t).
 */
extern LIBLSL_C_API int32_t lsl_resolve_byprop(lsl_streaminfo *buffer, uint32_t buffer_elements, const char *prop, const char *value, int32_t minimum, double timeout);

/**
 * Resolve all streams that match a given predicate.
 *
 * Advanced query that allows to impose more conditions on the retrieved streams;
 * the given string is an [XPath 1.0 predicate](http://en.wikipedia.org/w/index.php?title=XPath_1.0)
 * for the `<info>` node (omitting the surrounding []'s)
 * @param[out] buffer A user-allocated buffer to hold the resolve results.
 * @attention It is the user's responsibility to either destroy the resulting streaminfo objects or
 * to pass them back to the LSL during during creation of an inlet.
 *
 * @attention The stream_info's returned by the resolver are only short versions that do not include
 * the lsl_get_desc() field (which can be arbitrarily big). To obtain the full stream information
 * you need to call lsl_get_info() on the inlet after you have created one.
 * @param buffer_elements The user-provided buffer length.
 * @param pred The predicate string, e.g.
 * `name='BioSemi'` or `type='EEG' and starts-with(name,'BioSemi') and count(info/desc/channel)=32`
 * @param minimum Return at least this number of streams.
 * @param timeout Optionally a timeout of the operation, in seconds (default: no timeout).
 *                If the timeout expires, less than the desired number of streams (possibly none)
 * will be returned.
 * @return The number of results written into the buffer (never more than the provided # of slots)
 *         or a negative number if an error has occurred (values corresponding to lsl_error_code_t).
 */
extern LIBLSL_C_API int32_t lsl_resolve_bypred(lsl_streaminfo *buffer, uint32_t buffer_elements, const char *pred, int32_t minimum, double timeout);

/// @}
