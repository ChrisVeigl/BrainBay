#ifndef LSL_TYPES
#define LSL_TYPES

/**
 * @class lsl_streaminfo
 * Handle to a stream info object.
 *
 * Stores the declaration of a data stream.
 * Represents the following information:
 *
 *  - stream data format (number of channels, channel format)
 *  - core information (stream name, content type, sampling rate)
 *  - optional meta-data about the stream content (channel labels, measurement units, etc.)
 *
 * Whenever a program wants to provide a new stream on the lab network it will typically first
 * create an lsl_streaminfo to describe its properties and then construct an #lsl_outlet with it to
 * create the stream on the network. Other parties who discover/resolve the outlet on the network
 * can query the stream info; it is also written to disk when recording the stream (playing a
 * similar role as a file header).
 */
typedef struct lsl_streaminfo_struct_ *lsl_streaminfo;

/**
 * @class lsl_outlet
 * A stream outlet handle.
 * Outlets are used to make streaming data (and the meta-data) available on the lab network.
 */
typedef struct lsl_outlet_struct_ *lsl_outlet;

/**
 * @class lsl_inlet
 * A stream inlet handle.
 * Inlets are used to receive streaming data (and meta-data) from the lab network.
 */
typedef struct lsl_inlet_struct_ *lsl_inlet;

/**
 * @class lsl_xml_ptr
 * A lightweight XML element tree handle; models the description of a streaminfo object.
 * XML elements behave like advanced pointers into memory that is owned by some respective
 * streaminfo.
 * Has a name and can have multiple named children or have text content as value;
 * attributes are omitted.
 * @note The interface is modeled after a subset of pugixml's node type and is compatible with it.
 * Type-casts between pugi::xml_node_struct* and #lsl_xml_ptr are permitted (in both directions)
 * since the types are binary compatible.
 * @sa [pugixml documentation](https://pugixml.org/docs/manual.html#access).
 */
typedef struct lsl_xml_ptr_struct_ *lsl_xml_ptr;

/**
 * @class lsl_continuous_resolver
 *
 * Handle to a convenience object that resolves streams continuously in the background throughout
 * its lifetime and which can be queried at any time for the set of streams that are currently
 * visible on the network.
 */
typedef struct lsl_continuous_resolver_ *lsl_continuous_resolver;

#endif // LSL_TYPES
