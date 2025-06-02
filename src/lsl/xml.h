#pragma once
#include "common.h"
#include "types.h"

/// @file inlet.h XML functions

/** @defgroup xml_ptr The lsl_xml_ptr object
 * @{
 */

// XML Tree Navigation

/** Get the first child of the element. */
extern LIBLSL_C_API lsl_xml_ptr lsl_first_child(lsl_xml_ptr e);

/** Get the last child of the element. */
extern LIBLSL_C_API lsl_xml_ptr lsl_last_child(lsl_xml_ptr e);

/** Get the next sibling in the children list of the parent node. */
extern LIBLSL_C_API lsl_xml_ptr lsl_next_sibling(lsl_xml_ptr e);

/** Get the previous sibling in the children list of the parent node. */
extern LIBLSL_C_API lsl_xml_ptr lsl_previous_sibling(lsl_xml_ptr e);

/** Get the parent node. */
extern LIBLSL_C_API lsl_xml_ptr lsl_parent(lsl_xml_ptr e);


// XML Tree Navigation by Name

/** Get a child with a specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_child(lsl_xml_ptr e, const char *name);

/** Get the next sibling with the specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_next_sibling_n(lsl_xml_ptr e, const char *name);

/** Get the previous sibling with the specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_previous_sibling_n(lsl_xml_ptr e, const char *name);


// Content Queries

/** Whether this node is empty. */
extern LIBLSL_C_API int32_t lsl_empty(lsl_xml_ptr e);

/** Whether this is a text body (instead of an XML element). True both for plain char data and CData. */
extern LIBLSL_C_API int32_t  lsl_is_text(lsl_xml_ptr e);

/** Name of the element. */
extern LIBLSL_C_API const char *lsl_name(lsl_xml_ptr e);

/** Value of the element. */
extern LIBLSL_C_API const char *lsl_value(lsl_xml_ptr e);

/** Get child value (value of the first child that is text). */
extern LIBLSL_C_API const char *lsl_child_value(lsl_xml_ptr e);

/** Get child value of a child with a specified name. */
extern LIBLSL_C_API const char *lsl_child_value_n(lsl_xml_ptr e, const char *name);


// Data Modification

/// Append a child node with a given name, which has a (nameless) plain-text child with the given text value.
extern LIBLSL_C_API lsl_xml_ptr lsl_append_child_value(lsl_xml_ptr e, const char *name, const char *value);

/// Prepend a child node with a given name, which has a (nameless) plain-text child with the given text value.
extern LIBLSL_C_API lsl_xml_ptr lsl_prepend_child_value(lsl_xml_ptr e, const char *name, const char *value);

/// Set the text value of the (nameless) plain-text child of a named child node.
extern LIBLSL_C_API int32_t lsl_set_child_value(lsl_xml_ptr e, const char *name, const char *value);

/**
* Set the element's name.
* @return 0 if the node is empty (or if out of memory).
*/
extern LIBLSL_C_API int32_t lsl_set_name(lsl_xml_ptr e, const char *rhs);

/**
* Set the element's value.
* @return 0 if the node is empty (or if out of memory).
*/
extern LIBLSL_C_API int32_t lsl_set_value(lsl_xml_ptr e, const char *rhs);

/** Append a child element with the specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_append_child(lsl_xml_ptr e, const char *name);

/** Prepend a child element with the specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_prepend_child(lsl_xml_ptr e, const char *name);

/** Append a copy of the specified element as a child. */
extern LIBLSL_C_API lsl_xml_ptr lsl_append_copy(lsl_xml_ptr e, lsl_xml_ptr e2);

/** Prepend a child element with the specified name. */
extern LIBLSL_C_API lsl_xml_ptr lsl_prepend_copy(lsl_xml_ptr e, lsl_xml_ptr e2);

/** Remove a child element with the specified name. */
extern LIBLSL_C_API void lsl_remove_child_n(lsl_xml_ptr e, const char *name);

/** Remove a specified child element. */
extern LIBLSL_C_API void lsl_remove_child(lsl_xml_ptr e, lsl_xml_ptr e2);

/// @}
