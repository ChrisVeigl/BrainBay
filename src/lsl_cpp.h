#ifndef LSL_CPP_H
#define LSL_CPP_H

/**
 * @file lsl_cpp.h
 *
 * C++ API for the lab streaming layer.
 *
 * The lab streaming layer provides a set of functions to make instrument data accessible
 * in real time within a lab network. From there, streams can be picked up by recording programs,
 * viewing programs or custom experiment applications that access data streams in real time.
 *
 * The API covers two areas:
 * - The "push API" allows to create stream outlets and to push data (regular or irregular
 * measurement time series, event data, coded audio/video frames, etc.) into them.
 * - The "pull API" allows to create stream inlets and read time-synched experiment data from them
 *   (for recording, viewing or experiment control).
 *
 * To use this library you need to link to the shared library (lsl) that comes with
 * this header. Under Visual Studio the library is linked in automatically.
 */

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include "lsl_c.h"
}

namespace lsl {
/// Assert that no error happened; throw appropriate exception otherwise
int32_t check_error(int32_t ec);

/// Constant to indicate that a stream has variable sampling rate.
const double IRREGULAR_RATE = 0.0;

/**
 * Constant to indicate that a sample has the next successive time stamp.
 *
 * This is an optional optimization to transmit less data per sample.
 * The stamp is then deduced from the preceding one according to the stream's sampling rate
 * (in the case of an irregular rate, the same time stamp as before will is assumed).
 */
const double DEDUCED_TIMESTAMP = -1.0;

/**
 * A very large time duration (> 1 year) for timeout values.
 *
 * Note that significantly larger numbers can cause the timeout to be invalid on some operating
 * systems (e.g., 32-bit UNIX).
 */
const double FOREVER = 32000000.0;

/// Data format of a channel (each transmitted sample holds an array of channels).
enum channel_format_t {
	/** For up to 24-bit precision measurements in the appropriate physical unit (e.g., microvolts).
	Integers from -16777216 to 16777216 are represented accurately.*/
	cf_float32 = 1,
	/// For universal numeric data as long as permitted by network & disk budget.
	/// The largest representable integer is 53-bit.
	cf_double64 = 2,
	/// For variable-length ASCII strings or data blobs, such as video frames, complex event
	/// descriptions, etc.
	cf_string = 3,
	/// For high-rate digitized formats that require 32-bit precision.
	/// Depends critically on meta-data to represent meaningful units.
	/// Useful for application event codes or other coded data.
	cf_int32 = 4,
	/// For very high rate signals (40Khz+) or consumer-grade audio (for professional audio float is
	/// recommended).
	cf_int16 = 5,
	/// For binary signals or other coded data. Not recommended for encoding string data.
	cf_int8 = 6,
	/// For now only for future compatibility. Support for this type is not yet exposed in all
	/// languages. Also, some builds of liblsl will not be able to send or receive data of this
	/// type.
	cf_int64 = 7,
	/// Can not be transmitted.
	cf_undefined = 0
};

/// Post-processing options for stream inlets.
enum processing_options_t {
	/// No automatic post-processing; return the ground-truth time stamps for manual post-processing
	/// (this is the default behavior of the inlet).
	post_none = 0,
	/// Perform automatic clock synchronization; equivalent to manually adding the time_correction()
	/// value to the received time stamps.
	post_clocksync = 1,
	/// Remove jitter from time stamps. This will apply a smoothing algorithm to the received time
	/// stamps; the smoothing needs to see a minimum number of samples (30-120 seconds worst-case)
	/// until the remaining jitter is consistently below 1ms.
	post_dejitter = 2,
	/// Force the time-stamps to be monotonically ascending (only makes sense if timestamps are
	/// dejittered).
	post_monotonize = 4,
	/// Post-processing is thread-safe (same inlet can be read from by multiple threads); uses
	/// somewhat more CPU.
	post_threadsafe = 8,
	/// The combination of all possible post-processing options.
	post_ALL = 1 | 2 | 4 | 8
};

/**
 * Protocol version.
 *
 * The major version is `protocol_version() / 100`;
 * The minor version is `protocol_version() % 100`;
 * Clients with different minor versions are protocol-compatible with each other
 * while clients with different major versions will refuse to work together.
 */
inline int32_t protocol_version() { return lsl_protocol_version(); }

/// @copydoc ::lsl_library_version()
inline int32_t library_version() { return lsl_library_version(); }

/**
 * Get a string containing library information.
 *
 * The format of the string shouldn't be used for anything important except giving a a debugging
 * person a good idea which exact library version is used. */
inline const char *library_info() { return lsl_library_info(); }

/**
 * Obtain a local system time stamp in seconds.
 *
 * The resolution is better than a millisecond.
 * This reading can be used to assign time stamps to samples as they are being acquired.
 * If the "age" of a sample is known at a particular time (e.g., from USB transmission
 * delays), it can be used as an offset to local_clock() to obtain a better estimate of
 * when a sample was actually captured. See stream_outlet::push_sample() for a use case.
 */
inline double local_clock() { return lsl_local_clock(); }


/// @section Stream Declaration

class xml_element;

/**
 * The stream_info object stores the declaration of a data stream.
 *
 * Represents the following information:
 *  a) stream data format (number of channels, channel format)
 *  b) core information (stream name, content type, sampling rate)
 *  c) optional meta-data about the stream content (channel labels, measurement units, etc.)
 *
 * Whenever a program wants to provide a new stream on the lab network it will typically first
 * create a stream_info to describe its properties and then construct a stream_outlet with it to
 * create the stream on the network. Recipients who discover the outlet can query the stream_info;
 * it is also written to disk when recording the stream (playing a similar role as a file header).
 */
class stream_info {
public:
	/**
	 * Construct a new stream_info object.
	 *
	 * Core stream information is specified here. Any remaining meta-data can be added later.
	 * @param name Name of the stream. Describes the device (or product series) that this stream
	 * makes available (for use by programs, experimenters or data analysts). Cannot be empty.
	 * @param type Content type of the stream. Please see https://github.com/sccn/xdf/wiki/Meta-Data
	 * (or web search for: XDF meta-data) for pre-defined content-type names, but you can also make
	 * up your own. The content type is the preferred way to find streams (as opposed to searching
	 * by name).
	 * @param channel_count Number of channels per sample. This stays constant for the lifetime of
	 * the stream.
	 * @param nominal_srate The sampling rate (in Hz) as advertised by the data source, if regular
	 * (otherwise set to IRREGULAR_RATE).
	 * @param channel_format Format/type of each channel. If your channels have different formats,
	 * consider supplying multiple streams or use the largest type that can hold them all (such as
	 * cf_double64).
	 * @param source_id Unique identifier of the device or source of the data, if available (such as
	 * the serial number). This is critical for system robustness since it allows recipients to
	 * recover from failure even after the serving app, device or computer crashes (just by finding
	 * a stream with the same source id on the network again). Therefore, it is highly recommended
	 * to always try to provide whatever information can uniquely identify the data source itself.
	 */
	stream_info(const std::string &name, const std::string &type, int32_t channel_count = 1,
		double nominal_srate = IRREGULAR_RATE, channel_format_t channel_format = cf_float32,
		const std::string &source_id = std::string())
		: obj(lsl_create_streaminfo((name.c_str()), (type.c_str()), channel_count, nominal_srate,
			  (lsl_channel_format_t)channel_format, (source_id.c_str())), &lsl_destroy_streaminfo) {
		if (obj == nullptr) throw std::invalid_argument(lsl_last_error());
	}

	/// Default contructor.
	stream_info(): stream_info("untitled", "", 0, 0, cf_undefined, ""){}

	/// Copy constructor. Only increments the reference count! @see clone()
	stream_info(const stream_info &) noexcept = default;
	stream_info(lsl_streaminfo handle) : obj(handle, &lsl_destroy_streaminfo) {}

	/// Clones a streaminfo object.
	stream_info clone() { return stream_info(lsl_copy_streaminfo(obj.get())); }


	// ========================
	// === Core Information ===
	// ========================
	// (these fields are assigned at construction)

	/**
	 * Name of the stream.
	 *
	 * This is a human-readable name. For streams offered by device modules, it refers to the type
	 * of device or product series that is generating the data of the stream. If the source is an
	 * application, the name may be a more generic or specific identifier. Multiple streams with the
	 * same name can coexist, though potentially at the cost of ambiguity (for the recording app or
	 * experimenter).
	 */
	std::string name() const { return lsl_get_name(obj.get()); }

	/**
	 * Content type of the stream.
	 *
	 * The content type is a short string such as "EEG", "Gaze" which describes the content carried
	 * by the channel (if known). If a stream contains mixed content this value need not be assigned
	 * but may instead be stored in the description of channel types. To be useful to applications
	 * and automated processing systems using the recommended content types is preferred. Content
	 * types usually follow those pre-defined in https://github.com/sccn/xdf/wiki/Meta-Data (or web
	 * search for: XDF meta-data).
	 */
	std::string type() const { return lsl_get_type(obj.get()); }

	/**
	 * Number of channels of the stream.
	 *
	 * A stream has at least one channel; the channel count stays constant for all samples.
	 */
	int32_t channel_count() const { return lsl_get_channel_count(obj.get()); }

	/**
	 * Sampling rate of the stream, according to the source (in Hz).
	 *
	 * If a stream is irregularly sampled, this should be set to IRREGULAR_RATE.
	 *
	 * Note that no data will be lost even if this sampling rate is incorrect or if a device has
	 * temporary hiccups, since all samples will be recorded anyway (except for those dropped by the
	 * device itself). However, when the recording is imported into an application, a good importer
	 * may correct such errors more accurately if the advertised sampling rate was close to the
	 * specs of the device.
	 */
	double nominal_srate() const { return lsl_get_nominal_srate(obj.get()); }

	/**
	 * Channel format of the stream.
	 *
	 * All channels in a stream have the same format. However, a device might offer multiple
	 * time-synched streams each with its own format.
	 */
	channel_format_t channel_format() const {
		return static_cast<channel_format_t>(lsl_get_channel_format(obj.get()));
	}

	/**
	 * Unique identifier of the stream's source, if available.
	 *
	 * The unique source (or device) identifier is an optional piece of information that, if
	 * available, allows that endpoints (such as the recording program) can re-acquire a stream
	 * automatically once it is back online.
	 */
	std::string source_id() const { return lsl_get_source_id(obj.get()); }


	// ======================================
	// === Additional Hosting Information ===
	// ======================================
	// (these fields are implicitly assigned once bound to an outlet/inlet)

	/// Protocol version used to deliver the stream.
	int32_t version() const { return lsl_get_version(obj.get()); }

	/**
	 * Creation time stamp of the stream.
	 *
	 * This is the time stamp when the stream was first created
	 * (as determined via #lsl::local_clock() on the providing machine).
	 */
	double created_at() const { return lsl_get_created_at(obj.get()); }

	/**
	 * Unique ID of the stream outlet instance (once assigned).
	 *
	 * This is a unique identifier of the stream outlet, and is guaranteed to be different
	 * across multiple instantiations of the same outlet (e.g., after a re-start).
	 */
	std::string uid() const { return lsl_get_uid(obj.get()); }

	/**
	 * Session ID for the given stream.
	 *
	 * The session id is an optional human-assigned identifier of the recording session.
	 * While it is rarely used, it can be used to prevent concurrent recording activitites
	 * on the same sub-network (e.g., in multiple experiment areas) from seeing each other's streams
	 * (assigned via a configuration file by the experimenter, see Network Connectivity in the LSL
	 * wiki).
	 */
	std::string session_id() const { return lsl_get_session_id(obj.get()); }

	/// Hostname of the providing machine.
	std::string hostname() const { return lsl_get_hostname(obj.get()); }


	// ========================
	// === Data Description ===
	// ========================

	/**
	 * Extended description of the stream.
	 *
	 * It is highly recommended that at least the channel labels are described here.
	 * See code examples on the LSL wiki. Other information, such as amplifier settings,
	 * measurement units if deviating from defaults, setup information, subject information, etc.,
	 * can be specified here, as well. Meta-data recommendations follow the XDF file format project
	 * (github.com/sccn/xdf/wiki/Meta-Data or web search for: XDF meta-data).
	 *
	 * Important: if you use a stream content type for which meta-data recommendations exist, please
	 * try to lay out your meta-data in agreement with these recommendations for compatibility with
	 * other applications.
	 */
	xml_element desc();

	/// lsl_stream_info_matches_query
	bool matches_query(const char *query) const {
		return lsl_stream_info_matches_query(obj.get(), query) != 0;
	}


	// ===============================
	// === Miscellaneous Functions ===
	// ===============================

	/** Retrieve the entire streaminfo in XML format.
	 * This yields an XML document (in string form) whose top-level element is `<info>`. The info
	 * element contains one element for each field of the streaminfo class, including:
	 *
	 *   - the core elements `<name>`, `<type>`, `<channel_count`, `<nominal_srate>`,
	 *   `<channel_format>`, `<source_id>`
	 *   - the misc elements `<version>`, `<created_at>`, `<uid>`, `<session_id>`,
	 *   `<v4address>`, `<v4data_port>`, `<v4service_port>`, `<v6address>`, `<v6data_port>`,
	 *   `<v6service_port>`
	 *   - the extended description element `<desc>` with user-defined sub-elements.
	 */
	std::string as_xml() const {
		char *tmp = lsl_get_xml(obj.get());
		std::string result(tmp);
		lsl_destroy_string(tmp);
		return result;
	}

	/// Number of bytes occupied by a channel (0 for string-typed channels).
	int32_t channel_bytes() const { return lsl_get_channel_bytes(obj.get()); }

	/// Number of bytes occupied by a sample (0 for string-typed channels).
	int32_t sample_bytes() const { return lsl_get_sample_bytes(obj.get()); }

	/// Get the implementation handle.
	std::shared_ptr<lsl_streaminfo_struct_> handle() const { return obj; }

	/// Assignment operator.
	stream_info &operator=(const stream_info &rhs) {
		if (this != &rhs) obj = stream_info(rhs).handle();
		return *this;
	}

	stream_info(stream_info &&rhs) noexcept = default;

	stream_info &operator=(stream_info &&rhs) noexcept = default;

	/// Utility function to create a stream_info from an XML representation
	static stream_info from_xml(const std::string &xml) {
		return stream_info(lsl_streaminfo_from_xml(xml.c_str()));
	}

private:
	std::shared_ptr<lsl_streaminfo_struct_> obj;
};


// =======================
// ==== Stream Outlet ====
// =======================

/** A stream outlet.
 * Outlets are used to make streaming data (and the meta-data) available on the lab network.
 */
class stream_outlet {
public:
	/** Establish a new stream outlet. This makes the stream discoverable.
	 * @param info The stream information to use for creating this stream. Stays constant over the
	 * lifetime of the outlet.
	 * @param chunk_size Optionally the desired chunk granularity (in samples) for transmission. If
	 * unspecified, each push operation yields one chunk. Inlets can override this setting.
	 * @param max_buffered Optionally the maximum amount of data to buffer (in seconds if there is a
	 * nominal sampling rate, otherwise x100 in samples). The default is 6 minutes of data.
	 */
	stream_outlet(const stream_info &info, int32_t chunk_size = 0, int32_t max_buffered = 360,
		lsl_transport_options_t flags = transp_default)
		: channel_count(info.channel_count()), sample_rate(info.nominal_srate()),
		  obj(lsl_create_outlet_ex(info.handle().get(), chunk_size, max_buffered, flags),
			  &lsl_destroy_outlet) {}

	// ========================================
	// === Pushing a sample into the outlet ===
	// ========================================

	/** Push a C array of values as a sample into the outlet.
	 * Each entry in the array corresponds to one channel.
	 * The function handles type checking & conversion.
	 * @param data An array of values to push (one per channel).
	 * @param timestamp Optionally the capture time of the sample, in agreement with
	 * lsl::local_clock(); if omitted, the current time is used.
	 * @param pushthrough Whether to push the sample through to the receivers instead of
	 * buffering it with subsequent samples.
	 * Note that the chunk_size, if specified at outlet construction, takes precedence over the
	 * pushthrough flag.
	 */
	template <class T, int32_t N>
	void push_sample(const T data[N], double timestamp = 0.0, bool pushthrough = true) {
		check_numchan(N);
		push_sample(&data[0], timestamp, pushthrough);
	}

	/** Push a std vector of values as a sample into the outlet.
	 * Each entry in the vector corresponds to one channel. The function handles type checking &
	 * conversion.
	 * @param data A vector of values to push (one for each channel).
	 * @param timestamp Optionally the capture time of the sample, in agreement with local_clock();
	 * if omitted, the current time is used.
	 * @param pushthrough Whether to push the sample through to the receivers instead of buffering
	 * it with subsequent samples. Note that the chunk_size, if specified at outlet construction,
	 * takes precedence over the pushthrough flag.
	 */
	template<typename T>
	void push_sample(
		const std::vector<T> &data, double timestamp = 0.0, bool pushthrough = true) {
		check_numchan(data.size());
		push_sample(data.data(), timestamp, pushthrough);
	}

	/** Push a pointer to some values as a sample into the outlet.
	 * This is a lower-level function for cases where data is available in some buffer.
	 * Handles type checking & conversion.
	 * @param data A pointer to values to push. The number of values pointed to must not be less
	 * than the number of channels in the sample.
	 * @param timestamp Optionally the capture time of the sample, in agreement with local_clock();
	 * if omitted, the current time is used.
	 * @param pushthrough Whether to push the sample through to the receivers instead of buffering
	 * it with subsequent samples. Note that the chunk_size, if specified at outlet construction,
	 * takes precedence over the pushthrough flag.
	 */
	void push_sample(const float *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_ftp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const double *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_dtp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const int64_t *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_ltp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const int32_t *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_itp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const int16_t *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_stp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const char *data, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_ctp(obj.get(), (data), timestamp, pushthrough);
	}
	void push_sample(const std::string *data, double timestamp = 0.0, bool pushthrough = true) {
		std::vector<uint32_t> lengths(channel_count);
		std::vector<const char *> pointers(channel_count);
		for (int32_t k = 0; k < channel_count; k++) {
			pointers[k] = data[k].c_str();
			lengths[k] = (uint32_t)data[k].size();
		}
		lsl_push_sample_buftp(obj.get(), pointers.data(), lengths.data(), timestamp, pushthrough);
	}

	/** Push a packed C struct (of numeric data) as one sample into the outlet (search for
	 * [`#``pragma pack`](https://stackoverflow.com/a/3318475/73299) for information on packing
	 * structs appropriately).<br>
	 * Overall size checking but no type checking or conversion are done.<br>
	 * Can not be used forvariable-size / string-formatted data.
	 * @param sample The sample struct to push.
	 * @param timestamp Optionally the capture time of the sample, in agreement with
	 * local_clock(); if omitted, the current time is used.
	 * @param pushthrough Whether to push the sample through to the receivers instead of
	 * buffering it with subsequent samples. Note that the chunk_size, if specified at outlet
	 * construction, takes precedence over the pushthrough flag.
	 */
	template <class T>
	void push_numeric_struct(const T &sample, double timestamp = 0.0, bool pushthrough = true) {
		if (info().sample_bytes() != sizeof(T))
			throw std::runtime_error(
				"Provided object size does not match the stream's sample size.");
		push_numeric_raw((void *)&sample, timestamp, pushthrough);
	}

	/** Push a pointer to raw numeric data as one sample into the outlet.
	 * This is the lowest-level function; performs no checking whatsoever. Cannot be used for
	 * variable-size / string-formatted channels.
	 * @param sample A pointer to the raw sample data to push.
	 * @param timestamp Optionally the capture time of the sample, in agreement with local_clock();
	 * if omitted, the current time is used.
	 * @param pushthrough Whether to push the sample through to the receivers instead of buffering
	 * it with subsequent samples. Note that the chunk_size, if specified at outlet construction,
	 * takes precedence over the pushthrough flag.
	 */
	void push_numeric_raw(const void *sample, double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_sample_vtp(obj.get(), (sample), timestamp, pushthrough);
	}


	// ===================================================
	// === Pushing an chunk of samples into the outlet ===
	// ===================================================

	/** Push a chunk of samples (batched into an STL vector) into the outlet.
	 * @param samples A vector of samples in some supported format (each sample can be a data
	 * pointer, data array, or std vector of data).
	 * @param timestamp Optionally the capture time of the most recent sample, in agreement with
	 * local_clock(); if omitted, the current time is used. The time stamps of other samples are
	 * automatically derived according to the sampling rate of the stream.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template <class T>
	void push_chunk(
		const std::vector<T> &samples, double timestamp = 0.0, bool pushthrough = true) {
		if (!samples.empty()) {
			if (timestamp == 0.0) timestamp = local_clock();
			if (sample_rate != IRREGULAR_RATE)
				timestamp = timestamp - (samples.size() - 1) / sample_rate;
			push_sample(samples[0], timestamp, pushthrough && samples.size() == 1);
			for (std::size_t k = 1; k < samples.size(); k++)
				push_sample(samples[k], DEDUCED_TIMESTAMP, pushthrough && k == samples.size() - 1);
		}
	}

	/** Push a chunk of samples (batched into an STL vector) into the outlet.
	 * Allows to specify a separate time stamp for each sample (for irregular-rate streams).
	 * @param samples A vector of samples in some supported format (each sample can be a data
	 * pointer, data array, or std vector of data).
	 * @param timestamps A vector of capture times for each sample, in agreement with local_clock().
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template <class T>
	void push_chunk(const std::vector<T> &samples, const std::vector<double> &timestamps,
		bool pushthrough = true) {
		for (unsigned k = 0; k < samples.size() - 1; k++)
			push_sample(samples[k], timestamps[k], false);
		if (!samples.empty()) push_sample(samples.back(), timestamps.back(), pushthrough);
	}

	/** Push a chunk of numeric data as C-style structs (batched into an STL vector) into the
	 * outlet. This performs some size checking but no type checking. Can not be used for
	 * variable-size / string-formatted data.
	 * @param samples A vector of samples, as C structs.
	 * @param timestamp Optionally the capture time of the sample, in agreement with local_clock();
	 * if omitted, the current time is used.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template <class T>
	void push_chunk_numeric_structs(
		const std::vector<T> &samples, double timestamp = 0.0, bool pushthrough = true) {
		if (!samples.empty()) {
			if (timestamp == 0.0) timestamp = local_clock();
			if (sample_rate != IRREGULAR_RATE)
				timestamp = timestamp - (samples.size() - 1) / sample_rate;
			push_numeric_struct(samples[0], timestamp, pushthrough && samples.size() == 1);
			for (std::size_t k = 1; k < samples.size(); k++)
				push_numeric_struct(
					samples[k], DEDUCED_TIMESTAMP, pushthrough && k == samples.size() - 1);
		}
	}

	/** Push a chunk of numeric data from C-style structs (batched into an STL vector), into the
	 * outlet. This performs some size checking but no type checking. Can not be used for
	 * variable-size / string-formatted data.
	 * @param samples A vector of samples, as C structs.
	 * @param timestamps A vector of capture times for each sample, in agreement with local_clock().
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template <class T>
	void push_chunk_numeric_structs(const std::vector<T> &samples,
		const std::vector<double> &timestamps, bool pushthrough = true) {
		for (unsigned k = 0; k < samples.size() - 1; k++)
			push_numeric_struct(samples[k], timestamps[k], false);
		if (!samples.empty()) push_numeric_struct(samples.back(), timestamps.back(), pushthrough);
	}

	/** Push a chunk of multiplexed data into the outlet.
	 * @name Push functions
	 * @param buffer A buffer of channel values holding the data for zero or more successive samples
	 * to send.
	 * @param timestamp Optionally the capture time of the most recent sample, in agreement with
	 * local_clock(); if omitted, the current time is used. The time stamps of other samples are
	 * automatically derived according to the sampling rate of the stream.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template<typename T>
	void push_chunk_multiplexed(
		const std::vector<T> &buffer, double timestamp = 0.0, bool pushthrough = true) {
		if (!buffer.empty())
			push_chunk_multiplexed(
				buffer.data(), static_cast<unsigned long>(buffer.size()), timestamp, pushthrough);
	}

	/** Push a chunk of multiplexed data into the outlet. One timestamp per sample is provided.
	 * Allows to specify a separate time stamp for each sample (for irregular-rate streams).
	 * @param buffer A buffer of channel values holding the data for zero or more successive samples
	 * to send.
	 * @param timestamps A buffer of timestamp values holding time stamps for each sample in the
	 * data buffer.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	template<typename T>
	void push_chunk_multiplexed(const std::vector<T> &buffer,
		const std::vector<double> &timestamps, bool pushthrough = true) {
		if (!buffer.empty() && !timestamps.empty())
			push_chunk_multiplexed(
				buffer.data(), static_cast<unsigned long>(buffer.size()), timestamps.data(), pushthrough);
	}

	/** Push a chunk of multiplexed samples into the outlet. Single timestamp provided.
	 * @warning The provided buffer size is measured in channel values (e.g., floats), not samples.
	 * @param buffer A buffer of channel values holding the data for zero or more successive samples
	 * to send.
	 * @param buffer_elements The number of channel values (of type T) in the buffer. Must be a
	 * multiple of the channel count.
	 * @param timestamp Optionally the capture time of the most recent sample, in agreement with
	 * local_clock(); if omitted, the current time is used. The time stamps of other samples are
	 * automatically derived based on the sampling rate of the stream.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the stream_outlet() constructur parameter @p chunk_size,
	 * if specified at outlet construction, takes precedence over the pushthrough flag.
	 */
	void push_chunk_multiplexed(const float *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_ftp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const double *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_dtp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const int64_t *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_ltp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const int32_t *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_itp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const int16_t *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_stp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const char *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		lsl_push_chunk_ctp(obj.get(), buffer, static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
	}
	void push_chunk_multiplexed(const std::string *buffer, std::size_t buffer_elements,
		double timestamp = 0.0, bool pushthrough = true) {
		if (buffer_elements) {
			std::vector<uint32_t> lengths(buffer_elements);
			std::vector<const char *> pointers(buffer_elements);
			for (std::size_t k = 0; k < buffer_elements; k++) {
				pointers[k] = buffer[k].c_str();
				lengths[k] = (uint32_t)buffer[k].size();
			}
			lsl_push_chunk_buftp(obj.get(), pointers.data(), lengths.data(),
				static_cast<unsigned long>(buffer_elements), timestamp, pushthrough);
		}
	}

	/** Push a chunk of multiplexed samples into the outlet. One timestamp per sample is provided.
	 * @warning Note that the provided buffer size is measured in channel values (e.g., floats)
	 * rather than in samples.
	 * @param data_buffer A buffer of channel values holding the data for zero or more successive
	 * samples to send.
	 * @param timestamp_buffer A buffer of timestamp values holding time stamps for each sample in
	 * the data buffer.
	 * @param data_buffer_elements The number of data values (of type T) in the data buffer. Must be
	 * a multiple of the channel count.
	 * @param pushthrough Whether to push the chunk through to the receivers instead of buffering it
	 * with subsequent samples. Note that the chunk_size, if specified at outlet construction, takes
	 * precedence over the pushthrough flag.
	 */
	void push_chunk_multiplexed(const float *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_ftnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}
	void push_chunk_multiplexed(const double *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_dtnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}
	void push_chunk_multiplexed(const int64_t *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_ltnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}
	void push_chunk_multiplexed(const int32_t *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_itnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}
	void push_chunk_multiplexed(const int16_t *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_stnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}
	void push_chunk_multiplexed(const char *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		lsl_push_chunk_ctnp(obj.get(), data_buffer, static_cast<unsigned long>(data_buffer_elements),
			(timestamp_buffer), pushthrough);
	}

	void push_chunk_multiplexed(const std::string *data_buffer, const double *timestamp_buffer,
		std::size_t data_buffer_elements, bool pushthrough = true) {
		if (data_buffer_elements) {
			std::vector<uint32_t> lengths(data_buffer_elements);
			std::vector<const char *> pointers(data_buffer_elements);
			for (std::size_t k = 0; k < data_buffer_elements; k++) {
				pointers[k] = data_buffer[k].c_str();
				lengths[k] = (uint32_t)data_buffer[k].size();
			}
			lsl_push_chunk_buftnp(obj.get(), pointers.data(), lengths.data(),
				static_cast<unsigned long>(data_buffer_elements), timestamp_buffer, pushthrough);
		}
	}


	// ===============================
	// === Miscellaneous Functions ===
	// ===============================

	/** Check whether consumers are currently registered.
	 * While it does not hurt, there is technically no reason to push samples if there is no
	 * consumer.
	 */
	bool have_consumers() { return lsl_have_consumers(obj.get()) != 0; }

	/** Wait until some consumer shows up (without wasting resources).
	 * @return True if the wait was successful, false if the timeout expired.
	 */
	bool wait_for_consumers(double timeout) { return lsl_wait_for_consumers(obj.get(), timeout) != 0; }

	/** Retrieve the stream info provided by this outlet.
	 * This is what was used to create the stream (and also has the Additional Network Information
	 * fields assigned).
	 */
	stream_info info() const { return stream_info(lsl_get_info(obj.get())); }

	/// Return a shared pointer to pass to C-API functions that aren't wrapped yet
	///
	/// Example: @code lsl_push_chunk_buft(outlet.handle().get(), data, …); @endcode
	std::shared_ptr<lsl_outlet_struct_> handle() { return obj; }

	/** Destructor.
	 * The stream will no longer be discoverable after destruction and all paired inlets will stop
	 * delivering data.
	 */
	~stream_outlet() = default;

	/// stream_outlet move constructor
	stream_outlet(stream_outlet &&res) noexcept  = default;

	stream_outlet &operator=(stream_outlet &&rhs) noexcept = default;


private:
	// The outlet is a non-copyable object.
	stream_outlet(const stream_outlet &rhs);
	stream_outlet &operator=(const stream_outlet &rhs);

	/// Check whether a given data length matches the number of channels; throw if not
	void check_numchan(std::size_t N) const {
		if (N != static_cast<std::size_t>(channel_count))
			throw std::runtime_error("Provided element count (" + std::to_string(N) +
									 ") does not match the stream's channel count (" +
									 std::to_string(channel_count) + '.');
	}

	int32_t channel_count;
	double sample_rate;
	std::shared_ptr<lsl_outlet_struct_> obj;
};


// ===========================
// ==== Resolve Functions ====
// ===========================

/** Resolve all streams on the network.
 * This function returns all currently available streams from any outlet on the network.
 * The network is usually the subnet specified at the local router, but may also include
 * a multicast group of machines (given that the network supports it), or list of hostnames.
 * These details may optionally be customized by the experimenter in a configuration file
 * (see Network Connectivity in the LSL wiki).
 * This is the default mechanism used by the browsing programs and the recording program.
 * @param wait_time The waiting time for the operation, in seconds, to search for streams.
 * If this is too short (<0.5s) only a subset (or none) of the outlets that are present on the
 * network may be returned.
 * @return A vector of stream info objects (excluding their desc field), any of which can
 *         subsequently be used to open an inlet. The full info can be retrieve from the inlet.
 */
inline std::vector<stream_info> resolve_streams(double wait_time = 1.0) {
	lsl_streaminfo buffer[1024];
	int nres = check_error(lsl_resolve_all(buffer, sizeof(buffer) / sizeof(lsl_streaminfo), wait_time));
	return std::vector<stream_info>(&buffer[0], &buffer[nres]);
}

/** Resolve all streams with a specific value for a given property.
 * If the goal is to resolve a specific stream, this method is preferred over resolving all streams
 * and then selecting the desired one.
 * @param prop The stream_info property that should have a specific value (e.g., "name", "type",
 * "source_id", or "desc/manufaturer").
 * @param value The string value that the property should have (e.g., "EEG" as the type property).
 * @param minimum Return at least this number of streams.
 * @param timeout Optionally a timeout of the operation, in seconds (default: no timeout).
 *                 If the timeout expires, less than the desired number of streams (possibly none)
 * will be returned.
 * @return A vector of matching stream info objects (excluding their meta-data), any of
 *         which can subsequently be used to open an inlet.
 */
inline std::vector<stream_info> resolve_stream(const std::string &prop, const std::string &value,
	int32_t minimum = 1, double timeout = FOREVER) {
	lsl_streaminfo buffer[1024];
	int nres = check_error(
		lsl_resolve_byprop(buffer, sizeof(buffer) / sizeof(lsl_streaminfo), prop.c_str(), value.c_str(), minimum, timeout));
	return std::vector<stream_info>(&buffer[0], &buffer[nres]);
}

/** Resolve all streams that match a given predicate.
 *
 * Advanced query that allows to impose more conditions on the retrieved streams; the given
 * string is an [XPath 1.0](http://en.wikipedia.org/w/index.php?title=XPath_1.0) predicate for
 * the `<info>` node (omitting the surrounding []'s)
 * @param pred The predicate string, e.g. `name='BioSemi'` or
 * `type='EEG' and starts-with(name,'BioSemi') and count(info/desc/channel)=32`
 * @param minimum Return at least this number of streams.
 * @param timeout Optionally a timeout of the operation, in seconds (default: no timeout).
 *                 If the timeout expires, less than the desired number of streams (possibly
 * none) will be returned.
 * @return A vector of matching stream info objects (excluding their meta-data), any of
 *         which can subsequently be used to open an inlet.
 */
inline std::vector<stream_info> resolve_stream(
	const std::string &pred, int32_t minimum = 1, double timeout = FOREVER) {
	lsl_streaminfo buffer[1024];
	int nres =
		check_error(lsl_resolve_bypred(buffer, sizeof(buffer) / sizeof(lsl_streaminfo), pred.c_str(), minimum, timeout));
	return std::vector<stream_info>(&buffer[0], &buffer[nres]);
}


// ======================
// ==== Stream Inlet ====
// ======================

/** A stream inlet.
 * Inlets are used to receive streaming data (and meta-data) from the lab network.
 */
class stream_inlet {
public:
	/**
	 * Construct a new stream inlet from a resolved stream info.
	 * @param info A resolved stream info object (as coming from one of the resolver functions).
	 * Note: The stream_inlet may also be constructed with a fully-specified stream_info, if the
	 * desired channel format and count is already known up-front, but this is strongly discouraged
	 * and should only ever be done if there is no time to resolve the stream up-front (e.g., due
	 * to limitations in the client program).
	 * @param max_buflen Optionally the maximum amount of data to buffer (in seconds if there is a
	 * nominal sampling rate, otherwise x100 in samples). Recording applications want to use a
	 * fairly large buffer size here, while real-time applications would only buffer as much as
	 * they need to perform their next calculation.
	 * @param max_chunklen Optionally the maximum size, in samples, at which chunks are transmitted
	 * (the default corresponds to the chunk sizes used by the sender).
	 * Recording applications can use a generous size here (leaving it to the network how to pack
	 * things), while real-time applications may want a finer (perhaps 1-sample) granularity.
	 * If left unspecified (=0), the sender determines the chunk granularity.
	 * @param recover Try to silently recover lost streams that are recoverable (=those that that
	 * have a source_id set).
	 * In all other cases (recover is false or the stream is not recoverable) functions may throw a
	 * lsl::lost_error if the stream's source is lost (e.g., due to an app or computer crash).
	 */
	stream_inlet(const stream_info &info, int32_t max_buflen = 360, int32_t max_chunklen = 0,
		bool recover = true, lsl_transport_options_t flags = transp_default)
		: channel_count(info.channel_count()),
		  obj(lsl_create_inlet_ex(info.handle().get(), max_buflen, max_chunklen, recover, flags),
			  &lsl_destroy_inlet) {}

	/// Return a shared pointer to pass to C-API functions that aren't wrapped yet
	///
	/// Example: @code lsl_pull_sample_buf(inlet.handle().get(), buf, …); @endcode
	std::shared_ptr<lsl_inlet_struct_> handle() { return obj; }

	/// Move constructor for stream_inlet
	stream_inlet(stream_inlet &&rhs) noexcept = default;
	stream_inlet &operator=(stream_inlet &&rhs) noexcept= default;


	/** Retrieve the complete information of the given stream, including the extended description.
	 * Can be invoked at any time of the stream's lifetime.
	 * @param timeout Timeout of the operation (default: no timeout).
	 * @throws timeout_error (if the timeout expires), or lost_error (if the stream source has been
	 * lost).
	 */
	stream_info info(double timeout = FOREVER) {
		int32_t ec = 0;
		lsl_streaminfo res = lsl_get_fullinfo(obj.get(), timeout, &ec);
		check_error(ec);
		return stream_info(res);
	}

	/** Subscribe to the data stream.
	 * All samples pushed in at the other end from this moment onwards will be queued and
	 * eventually be delivered in response to pull_sample() or pull_chunk() calls.
	 * Pulling a sample without some preceding open_stream() is permitted (the stream will then be
	 * opened implicitly).
	 * @param timeout Optional timeout of the operation (default: no timeout).
	 * @throws timeout_error (if the timeout expires), or lost_error (if the stream source has been
	 * lost).
	 */
	void open_stream(double timeout = FOREVER) {
		int32_t ec = 0;
		lsl_open_stream(obj.get(), timeout, &ec);
		check_error(ec);
	}

	/** Drop the current data stream.
	 *
	 * All samples that are still buffered or in flight will be dropped and transmission
	 * and buffering of data for this inlet will be stopped. If an application stops being
	 * interested in data from a source (temporarily or not) but keeps the outlet alive,
	 * it should call close_stream() to not waste unnecessary system and network
	 * resources.
	 */
	void close_stream() { lsl_close_stream(obj.get()); }

	/** Retrieve an estimated time correction offset for the given stream.
	 *
	 * The first call to this function takes several milliseconds until a reliable first estimate
	 * is obtained. Subsequent calls are instantaneous (and rely on periodic background updates).
	 * On a well-behaved network, the precision of these estimates should be below 1 ms
	 * (empirically it is within +/-0.2 ms).
	 *
	 * To get a measure of whether the network is well-behaved, use the extended version
	 * time_correction(double*,double*,double) and check uncertainty (i.e. the round-trip-time).
	 *
	 * 0.2 ms is typical of wired networks. 2 ms is typical of wireless networks.
	 * The number can be much higher on poor networks.
	 *
	 * @param timeout Timeout to acquire the first time-correction estimate (default: no timeout).
	 * @return The time correction estimate. This is the number that needs to be added to a time
	 * stamp that was remotely generated via lsl_local_clock() to map it into the local clock
	 * domain of this machine.
	 * @throws #lsl::timeout_error (if the timeout expires), or #lsl::lost_error (if the stream
	 * source has been lost).
	 */

	double time_correction(double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_time_correction(obj.get(), timeout, &ec);
		check_error(ec);
		return res;
	}
	/** @copydoc time_correction(double)
	 * @param remote_time The current time of the remote computer that was used to generate this
	 * time_correction. If desired, the client can fit time_correction vs remote_time to improve
	 * the real-time time_correction further.
	 * @param uncertainty The maximum uncertainty of the given time correction.
	 */
	double time_correction(double *remote_time, double *uncertainty, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_time_correction_ex(obj.get(), remote_time, uncertainty, timeout, &ec);
		check_error(ec);
		return res;
	}

	/** Set post-processing flags to use.
	 *
	 * By default, the inlet performs NO post-processing and returns the ground-truth time
	 * stamps, which can then be manually synchronized using .time_correction(), and then
	 * smoothed/dejittered if desired.<br>
	 * This function allows automating these two and possibly more operations.<br>
	 * @warning When you enable this, you will no longer receive or be able to recover the original
	 * time stamps.
	 * @param flags An integer that is the result of bitwise OR'ing one or more options from
	 * processing_options_t together (e.g., `post_clocksync|post_dejitter`); the default is to
	 * enable all options.
	 */
	void set_postprocessing(uint32_t flags = post_ALL) {
		check_error(lsl_set_postprocessing(obj.get(), flags));
	}

	// =======================================
	// === Pulling a sample from the inlet ===
	// =======================================

	/** Pull a sample from the inlet and read it into an array of values.
	 * Handles type checking & conversion.
	 * @param sample An array to hold the resulting values.
	 * @param timeout The timeout for this operation, if any.  Use 0.0 to make the function
	 * non-blocking.
	 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
	 * available. To remap this time stamp to the local clock, add the value returned by
	 * .time_correction() to it.
	 * @throws lost_error (if the stream source has been lost).
	 */
	template <class T, int N> double pull_sample(T sample[N], double timeout = FOREVER) {
		return pull_sample(&sample[0], N, timeout);
	}

	/** Pull a sample from the inlet and read it into a std vector of values.
	 * Handles type checking & conversion and allocates the necessary memory in the vector if
	 * necessary.
	 * @param sample An STL vector to hold the resulting values.
	 * @param timeout The timeout for this operation, if any.  Use 0.0 to make the function
	 * non-blocking.
	 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
	 * available. To remap this time stamp to the local clock, add the value returned by
	 * .time_correction() to it.
	 * @throws lost_error (if the stream source has been lost).
	 */
	double pull_sample(std::vector<float> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<double> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<int64_t> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<int32_t> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<int16_t> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<char> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}
	double pull_sample(std::vector<std::string> &sample, double timeout = FOREVER) {
		sample.resize(channel_count);
		return pull_sample(&sample[0], (int32_t)sample.size(), timeout);
	}

	/** Pull a sample from the inlet and read it into a pointer to values.
	 * Handles type checking & conversion.
	 * @param buffer A pointer to hold the resulting values.
	 * @param buffer_elements The number of samples allocated in the buffer. Note: it is the
	 * responsibility of the user to allocate enough memory.
	 * @param timeout The timeout for this operation, if any.  Use 0.0 to make the function
	 * non-blocking.
	 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
	 * available. To remap this time stamp to the local clock, add the value returned by
	 * .time_correction() to it.
	 * @throws lost_error (if the stream source has been lost).
	 */
	double pull_sample(float *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_f(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(double *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_d(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(int64_t *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_l(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(int32_t *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_i(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(int16_t *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_s(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(char *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_c(obj.get(), buffer, buffer_elements, timeout, &ec);
		check_error(ec);
		return res;
	}
	double pull_sample(std::string *buffer, int32_t buffer_elements, double timeout = FOREVER) {
		int32_t ec = 0;
		if (buffer_elements) {
			std::vector<char *> result_strings(buffer_elements);
			std::vector<uint32_t> result_lengths(buffer_elements);
			double res = lsl_pull_sample_buf(
				obj.get(), result_strings.data(), result_lengths.data(), buffer_elements, timeout, &ec);
			check_error(ec);
			for (int32_t k = 0; k < buffer_elements; k++) {
				buffer[k].assign(result_strings[k], result_lengths[k]);
				lsl_destroy_string(result_strings[k]);
			}
			return res;
		} else
			throw std::runtime_error(
				"Provided element count does not match the stream's channel count.");
	}

	/**
	 * Pull a sample from the inlet and read it into a custom C-style struct.
	 *
	 * Overall size checking but no type checking or conversion are done.
	 * Do not use for variable-size/string-formatted streams.
	 * @param sample The raw sample object to hold the data (packed C-style struct).
	 * Search for [`#``pragma pack`](https://stackoverflow.com/a/3318475/73299) for information
	 * on how to pack structs correctly.
	 * @param timeout The timeout for this operation, if any. Use 0.0 to make the function
	 * non-blocking.
	 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
	 * available. To remap this time stamp to the local clock, add the value returned by
	 * .time_correction() to it.
	 * @throws lost_error (if the stream source has been lost).
	 */
	template <class T> double pull_numeric_struct(T &sample, double timeout = FOREVER) {
		return pull_numeric_raw((void *)&sample, sizeof(T), timeout);
	}

	/**
	 * Pull a sample from the inlet and read it into a pointer to raw data.
	 *
	 * No type checking or conversions are done (not recommended!).<br>
	 * Do not use for variable-size/string-formatted streams.
	 * @param sample A pointer to hold the resulting raw sample data.
	 * @param buffer_bytes The number of bytes allocated in the buffer.<br>
	 * Note: it is the responsibility of the user to allocate enough memory.
	 * @param timeout The timeout for this operation, if any. Use 0.0 to make the function
	 * non-blocking.
	 * @return The capture time of the sample on the remote machine, or 0.0 if no new sample was
	 * available. To remap this time stamp to the local clock, add the value returned by
	 * .time_correction() to it.
	 * @throws lost_error (if the stream source has been lost).
	 */
	double pull_numeric_raw(void *sample, int32_t buffer_bytes, double timeout = FOREVER) {
		int32_t ec = 0;
		double res = lsl_pull_sample_v(obj.get(), sample, buffer_bytes, timeout, &ec);
		check_error(ec);
		return res;
	}


	// =================================================
	// === Pulling a chunk of samples from the inlet ===
	// =================================================

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This is the most complete version, returning both the data and a timestamp for each sample.
	 * @param chunk A vector of vectors to hold the samples.
	 * @param timestamps A vector to hold the time stamps.
	 * @return True if some data was obtained.
	 * @throws lost_error (if the stream source has been lost).
	 */
	template <class T>
	bool pull_chunk(std::vector<std::vector<T>> &chunk, std::vector<double> &timestamps) {
		std::vector<T> sample;
		chunk.clear();
		timestamps.clear();
		while (double ts = pull_sample(sample, 0.0)) {
			chunk.push_back(sample);
			timestamps.push_back(ts);
		}
		return !chunk.empty();
	}

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This version returns only the most recent sample's time stamp.
	 * @param chunk A vector of vectors to hold the samples.
	 * @return The time when the most recent sample was captured
	 *         on the remote machine, or 0.0 if no new sample was available.
	 * @throws lost_error (if the stream source has been lost)
	 */
	template <class T> double pull_chunk(std::vector<std::vector<T>> &chunk) {
		double timestamp = 0.0;
		std::vector<T> sample;
		chunk.clear();
		while (double ts = pull_sample(sample, 0.0)) {
			chunk.push_back(sample);
			timestamp = ts;
		}
		return timestamp;
	}

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This function does not return time stamps for the samples. Invoked as: mychunk =
	 * pull_chunk<float>();
	 * @return A vector of vectors containing the obtained samples; may be empty.
	 * @throws lost_error (if the stream source has been lost)
	 */
	template <class T> std::vector<std::vector<T>> pull_chunk() {
		std::vector<std::vector<T>> result;
		std::vector<T> sample;
		while (pull_sample(sample, 0.0)) result.push_back(sample);
		return result;
	}

	/**
	 * Pull a chunk of data from the inlet into a pre-allocated buffer.
	 *
	 * This is a high-performance function that performs no memory allocations
	 * (useful for very high data rates or on low-powered devices).
	 * @warning The provided buffer size is measured in channel values (e.g., floats), not samples.
	 * @param data_buffer A pointer to a buffer of data values where the results shall be stored.
	 * @param timestamp_buffer A pointer to a buffer of timestamp values where time stamps shall be
	 * stored. If this is NULL, no time stamps will be returned.
	 * @param data_buffer_elements The size of the data buffer, in channel data elements (of type
	 * T). Must be a multiple of the stream's channel count.
	 * @param timestamp_buffer_elements The size of the timestamp buffer. If a timestamp buffer is
	 * provided then this must correspond to the same number of samples as data_buffer_elements.
	 * @param timeout The timeout for this operation, if any. When the timeout expires, the function
	 * may return before the entire buffer is filled. The default value of 0.0 will retrieve only
	 * data available for immediate pickup.
	 * @return data_elements_written Number of channel data elements written to the data buffer.
	 * @throws lost_error (if the stream source has been lost).
	 */
	std::size_t pull_chunk_multiplexed(float *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_f(obj.get(), data_buffer, timestamp_buffer,
			(unsigned long)data_buffer_elements, (unsigned long)timestamp_buffer_elements, timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(double *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_d(obj.get(), data_buffer, timestamp_buffer,
			(unsigned long)data_buffer_elements, (unsigned long)timestamp_buffer_elements, timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(int64_t *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_l(obj.get(), data_buffer, timestamp_buffer,
			(unsigned long)data_buffer_elements, (unsigned long)timestamp_buffer_elements, timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(int32_t *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_i(obj.get(), data_buffer, timestamp_buffer,
			(unsigned long)data_buffer_elements, (unsigned long)timestamp_buffer_elements, timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(int16_t *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_s(obj.get(), data_buffer, timestamp_buffer,
			(unsigned long)data_buffer_elements, (unsigned long)timestamp_buffer_elements, timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(char *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		std::size_t res = lsl_pull_chunk_c(obj.get(), data_buffer, timestamp_buffer,
			static_cast<unsigned long>(data_buffer_elements), static_cast<unsigned long>(timestamp_buffer_elements), timeout,
			&ec);
		check_error(ec);
		return res;
	}
	std::size_t pull_chunk_multiplexed(std::string *data_buffer, double *timestamp_buffer,
		std::size_t data_buffer_elements, std::size_t timestamp_buffer_elements,
		double timeout = 0.0) {
		int32_t ec = 0;
		if (data_buffer_elements) {
			std::vector<char *> result_strings(data_buffer_elements);
			std::vector<uint32_t> result_lengths(data_buffer_elements);
			std::size_t num = lsl_pull_chunk_buf(obj.get(), result_strings.data(), result_lengths.data(),
				timestamp_buffer, static_cast<unsigned long>(data_buffer_elements),
				static_cast<unsigned long>(timestamp_buffer_elements), timeout, &ec);
			check_error(ec);
			for (std::size_t k = 0; k < num; k++) {
				data_buffer[k].assign(result_strings[k], result_lengths[k]);
				lsl_destroy_string(result_strings[k]);
			}
			return num;
		};
		return 0;
	}

	/**
	 * Pull a multiplexed chunk of samples and optionally the sample timestamps from the inlet.
	 *
	 * @param chunk A vector to hold the multiplexed (Sample 1 Channel 1,
	 * S1C2, S2C1, S2C2, S3C1, S3C2, ...) samples
	 * @param timestamps A vector to hold the timestamps or nullptr
	 * @param timeout Time to wait for the first sample. The default value of 0.0 will not wait
	 * for data to arrive, pulling only samples already received.
	 * @param append (True:) Append data or (false:) clear them first
	 * @return True if some data was obtained.
	 * @throws lost_error (if the stream source has been lost).
	 */
	template <typename T>
	bool pull_chunk_multiplexed(std::vector<T> &chunk, std::vector<double> *timestamps = nullptr,
		double timeout = 0.0, bool append = false) {
		if (!append) {
			chunk.clear();
			if (timestamps) timestamps->clear();
		}
		std::vector<T> sample;
		double ts;
		if ((ts = pull_sample(sample, timeout)) == 0.0) return false;
		chunk.insert(chunk.end(), sample.begin(), sample.end());
		if (timestamps) timestamps->push_back(ts);
		const auto target = samples_available();
		chunk.reserve(chunk.size() + target * this->channel_count);
		if (timestamps) timestamps->reserve(timestamps->size() + target);
		while ((ts = pull_sample(sample, 0.0)) != 0.0) {
#if LSL_CPP11
			chunk.insert(chunk.end(), std::make_move_iterator(sample.begin()),
				std::make_move_iterator(sample.end()));
#else
			chunk.insert(chunk.end(), sample.begin(), sample.end());
#endif
			if (timestamps) timestamps->push_back(ts);
		}
		return true;
	}

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This is the most complete version, returning both the data and a timestamp for each sample.
	 * @param chunk A vector of C-style structs to hold the samples.
	 * @param timestamps A vector to hold the time stamps.
	 * @return True if some data was obtained.
	 * @throws lost_error (if the stream source has been lost)
	 */
	template <class T>
	bool pull_chunk_numeric_structs(std::vector<T> &chunk, std::vector<double> &timestamps) {
		T sample;
		chunk.clear();
		timestamps.clear();
		while (double ts = pull_numeric_struct(sample, 0.0)) {
			chunk.push_back(sample);
			timestamps.push_back(ts);
		}
		return !chunk.empty();
	}

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This version returns only the most recent sample's time stamp.
	 * @param chunk A vector of C-style structs to hold the samples.
	 * @return The time when the most recent sample was captured
	 *         on the remote machine, or 0.0 if no new sample was available.
	 * @throws lost_error (if the stream source has been lost)
	 */
	template <class T> double pull_chunk_numeric_structs(std::vector<T> &chunk) {
		double timestamp = 0.0;
		T sample;
		chunk.clear();
		while (double ts = pull_numeric_struct(sample, 0.0)) {
			chunk.push_back(sample);
			timestamp = ts;
		}
		return timestamp;
	}

	/**
	 * Pull a chunk of samples from the inlet.
	 *
	 * This function does not return time stamps. Invoked as: mychunk = pull_chunk<mystruct>();
	 * @return A vector of C-style structs containing the obtained samples; may be empty.
	 * @throws lost_error (if the stream source has been lost)
	 */
	template <class T> std::vector<T> pull_chunk_numeric_structs() {
		std::vector<T> result;
		T sample;
		while (pull_numeric_struct(sample, 0.0)) result.push_back(sample);
		return result;
	}

	/**
	 * Query whether samples are currently available for immediate pickup.
	 *
	 * Note that it is not a good idea to use samples_available() to determine whether
	 * a pull_*() call would block: to be sure, set the pull timeout to 0.0 or an acceptably
	 * low value. If the underlying implementation supports it, the value will be the number of
	 * samples available (otherwise it will be 1 or 0).
	 */
	std::size_t samples_available() { return lsl_samples_available(obj.get()); }

	/// Drop all queued not-yet pulled samples, return the nr of dropped samples
	uint32_t flush() noexcept { return lsl_inlet_flush(obj.get()); }

	/**
	 * Query whether the clock was potentially reset since the last call to was_clock_reset().
	 *
	 * This is a rarely-used function that is only useful to applications that combine multiple
	 * time_correction values to estimate precise clock drift; it allows to tolerate cases where the
	 * source machine was hot-swapped or restarted in between two measurements.
	 */
	bool was_clock_reset() { return lsl_was_clock_reset(obj.get()) != 0; }

	/**
	 * Override the half-time (forget factor) of the time-stamp smoothing.
	 *
	 * The default is 90 seconds unless a different value is set in the config file.
	 * Using a longer window will yield lower jitter in the time stamps, but longer
	 * windows will have trouble tracking changes in the clock rate (usually due to
	 * temperature changes); the default is able to track changes up to 10
	 * degrees C per minute sufficiently well.
	 */
	void smoothing_halftime(float value) { check_error(lsl_smoothing_halftime(obj.get(), value)); }

	int get_channel_count() const { return channel_count; }

private:
	// The inlet is a non-copyable object.
	stream_inlet(const stream_inlet &rhs);
	stream_inlet &operator=(const stream_inlet &rhs);

	int32_t channel_count;
	std::shared_ptr<lsl_inlet_struct_> obj;
};


// =====================
// ==== XML Element ====
// =====================

/**
 * A lightweight XML element tree; models the .desc() field of stream_info.
 *
 * Has a name and can have multiple named children or have text content as value; attributes are
 * omitted. Insider note: The interface is modeled after a subset of pugixml's node type and is
 * compatible with it. See also
 * http://pugixml.googlecode.com/svn/tags/latest/docs/manual/access.html for additional
 * documentation.
 */
class xml_element {
public:
	/// Constructor.
	xml_element(lsl_xml_ptr obj = 0) : obj(obj) {}


	// === Tree Navigation ===

	/// Get the first child of the element.
	xml_element first_child() const { return lsl_first_child(obj); }

	/// Get the last child of the element.
	xml_element last_child() const { return lsl_last_child(obj); }

	/// Get the next sibling in the children list of the parent node.
	xml_element next_sibling() const { return lsl_next_sibling(obj); }

	/// Get the previous sibling in the children list of the parent node.
	xml_element previous_sibling() const { return lsl_previous_sibling(obj); }

	/// Get the parent node.
	xml_element parent() const { return lsl_parent(obj); }


	// === Tree Navigation by Name ===

	/// Get a child with a specified name.
	xml_element child(const std::string &name) const { return lsl_child(obj, (name.c_str())); }

	/// Get the next sibling with the specified name.
	xml_element next_sibling(const std::string &name) const {
		return lsl_next_sibling_n(obj, (name.c_str()));
	}

	/// Get the previous sibling with the specified name.
	xml_element previous_sibling(const std::string &name) const {
		return lsl_previous_sibling_n(obj, (name.c_str()));
	}


	// === Content Queries ===

	/// Whether this node is empty.
	bool empty() const { return lsl_empty(obj) != 0; }

	/// Is this a text body (instead of an XML element)? True both for plain char data and CData.
	bool is_text() const { return lsl_is_text(obj) != 0; }

	/// Name of the element.
	const char *name() const { return lsl_name(obj); }

	/// Value of the element.
	const char *value() const { return lsl_value(obj); }

	/// Get child value (value of the first child that is text).
	const char *child_value() const { return lsl_child_value(obj); }

	/// Get child value of a child with a specified name.
	const char *child_value(const std::string &name) const {
		return lsl_child_value_n(obj, (name.c_str()));
	}


	// === Modification ===

	/// Append a child node with a given name, which has a (nameless) plain-text child with the
	/// given text value.
	xml_element append_child_value(const std::string &name, const std::string &value) {
		return lsl_append_child_value(obj, (name.c_str()), (value.c_str()));
	}

	/// Prepend a child node with a given name, which has a (nameless) plain-text child with the
	/// given text value.
	xml_element prepend_child_value(const std::string &name, const std::string &value) {
		return lsl_prepend_child_value(obj, (name.c_str()), (value.c_str()));
	}

	/// Set the text value of the (nameless) plain-text child of a named child node.
	bool set_child_value(const std::string &name, const std::string &value) {
		return lsl_set_child_value(obj, (name.c_str()), (value.c_str())) != 0;
	}

	/**
	 * Set the element's name.
	 * @return False if the node is empty (or if out of memory).
	 */
	bool set_name(const std::string &rhs) { return lsl_set_name(obj, rhs.c_str()) != 0; }

	/**
	 * Set the element's value.
	 * @return False if the node is empty (or if out of memory).
	 */
	bool set_value(const std::string &rhs) { return lsl_set_value(obj, rhs.c_str()) != 0; }

	/// Append a child element with the specified name.
	xml_element append_child(const std::string &name) {
		return lsl_append_child(obj, name.c_str());
	}

	/// Prepend a child element with the specified name.
	xml_element prepend_child(const std::string &name) {
		return lsl_prepend_child(obj, (name.c_str()));
	}

	/// Append a copy of the specified element as a child.
	xml_element append_copy(const xml_element &e) { return lsl_append_copy(obj, e.obj); }

	/// Prepend a child element with the specified name.
	xml_element prepend_copy(const xml_element &e) { return lsl_prepend_copy(obj, e.obj); }

	/// Remove a child element with the specified name.
	void remove_child(const std::string &name) { lsl_remove_child_n(obj, (name.c_str())); }

	/// Remove a specified child element.
	void remove_child(const xml_element &e) { lsl_remove_child(obj, e.obj); }

private:
	lsl_xml_ptr obj;
};

inline xml_element stream_info::desc() { return lsl_get_desc(obj.get()); }


// =============================
// ==== Continuous Resolver ====
// =============================

/**
 * A convenience class that resolves streams continuously in the background throughout
 * its lifetime and which can be queried at any time for the set of streams that are currently
 * visible on the network.
 */
class continuous_resolver {
public:
	/**
	 * Construct a new continuous_resolver that resolves all streams on the network.
	 *
	 * This is analogous to the functionality offered by the free function resolve_streams().
	 * @param forget_after When a stream is no longer visible on the network (e.g., because it was
	 * shut down), this is the time in seconds after which it is no longer reported by the resolver.
	 */
	continuous_resolver(double forget_after = 5.0)
		: obj(lsl_create_continuous_resolver(forget_after), &lsl_destroy_continuous_resolver) {}

	/**
	 * Construct a new continuous_resolver that resolves all streams with a specific value for a
	 * given property.
	 *
	 * This is analogous to the functionality provided by the free function resolve_stream(prop,value).
	 * @param prop The stream_info property that should have a specific value (e.g., "name", "type",
	 * "source_id", or "desc/manufaturer").
	 * @param value The string value that the property should have (e.g., "EEG" as the type
	 * property).
	 * @param forget_after When a stream is no longer visible on the network (e.g., because it was
	 * shut down), this is the time in seconds after which it is no longer reported by the resolver.
	 */
	continuous_resolver(
		const std::string &prop, const std::string &value, double forget_after = 5.0)
		: obj(lsl_create_continuous_resolver_byprop((prop.c_str()), (value.c_str()), forget_after),
			  &lsl_destroy_continuous_resolver) {}

	/**
	 * Construct a new continuous_resolver that resolves all streams that match a given XPath 1.0
	 * predicate.
	 *
	 * This is analogous to the functionality provided by the free function resolve_stream(pred).
	 * @param pred The predicate string, e.g.
	 * `name='BioSemi'` or
	 * `type='EEG' and starts-with(name,'BioSemi') and count(info/desc/channel)=32`
	 * @param forget_after When a stream is no longer visible on the network (e.g., because it was
	 * shut down), this is the time in seconds after which it is no longer reported by the resolver.
	 */
	continuous_resolver(const std::string &pred, double forget_after = 5.0)
		: obj(lsl_create_continuous_resolver_bypred((pred.c_str()), forget_after), &lsl_destroy_continuous_resolver) {}

	/**
	 * Obtain the set of currently present streams on the network (i.e. resolve result).
	 * @return A vector of matching stream info objects (excluding their meta-data), any of
	 *         which can subsequently be used to open an inlet.
	 */
	std::vector<stream_info> results() {
		lsl_streaminfo buffer[1024];
		return std::vector<stream_info>(
			buffer, buffer + check_error(lsl_resolver_results(obj.get(), buffer, sizeof(buffer))));
	}

	/// Move constructor for stream_inlet
	continuous_resolver(continuous_resolver &&rhs) noexcept = default;
	continuous_resolver &operator=(continuous_resolver &&rhs) noexcept = default;

private:
	std::unique_ptr<lsl_continuous_resolver_, void(*)(lsl_continuous_resolver_*)> obj;
};


// ===============================
// ==== Exception Definitions ====
// ===============================

/// Exception class that indicates that a stream inlet's source has been irrecoverably lost.
class lost_error : public std::runtime_error {
public:
	explicit lost_error(const std::string &msg) : std::runtime_error(msg) {}
};


/// Exception class that indicates that an operation failed due to a timeout.
class timeout_error : public std::runtime_error {
public:
	explicit timeout_error(const std::string &msg) : std::runtime_error(msg) {}
};

/// Check error codes returned from the C interface and translate into appropriate exceptions.
inline int32_t check_error(int32_t ec) {
	if (ec < 0) {
		switch (ec) {
		case lsl_timeout_error: throw timeout_error("The operation has timed out.");
		case lsl_lost_error:
			throw lost_error(
				"The stream has been lost; to continue reading, you need to re-resolve it.");
		case lsl_argument_error:
			throw std::invalid_argument("An argument was incorrectly specified.");
		case lsl_internal_error: throw std::runtime_error("An internal error has occurred.");
		default: throw std::runtime_error("An unknown error has occurred.");
		}
	}
	return ec;
}

} // namespace lsl

#endif // LSL_CPP_H
