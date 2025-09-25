#ifndef VELOURS_XML_H
#define VELOURS_XML_H

#include "velours.h"
#include "da.h"

typedef struct {
	// name and value ARE null-terminated utf-8 strings!
	VL_DA(char) name;
	VL_DA(char) value;
} VlXMLAttribute;

struct VlXMLNode {
	const char *name;

	// is_complex is set to 0 if the node is just a text
	// otherwise, is_complex is 1
	// examples:
	//     <node property="..." /> (is_complex = 1)
	//     <node> (is_complex = 1)
	//         Hello, World! (is_complex = 0)
	//     </node>
	char is_complex;

	VL_DA(char) text;

	VL_DA(VlXMLAttribute) attributes;
	VL_DA(struct VlXMLNode) children;
};

typedef struct VlXMLNode VlXMLNode;

typedef struct {
	const char *version;
	const char *encoding;

	VlXMLNode root;
} VlXML;

VL_API VlResult vl_xml_attribute_new(VlXMLAttribute *attribute);
VL_API VlResult vl_xml_attribute_free(VlXMLAttribute *attribute);

VL_API VlResult vl_xml_node_new(VlXMLNode *node);
VL_API VlResult vl_xml_node_free(VlXMLNode *node);

// vl_xml_new(VlXML *xml, const char *source, char *error)
// initialize and parse given string as XML
// 
// arguments:
//     xml - xml object to store data into
//     source - XML string to parse
//     error - some memory to store the error message if parsing has failed
// 
// returns:
//     VL_SUCCESS if parsing succedded, otherwise VL_ERROR
// 
// usage:
//     VlXML xml;
//     char error[512];
//     if (vl_xml_new(&xml, "<abc>...</abc>", error)) {
//         printf("failed to parse xml: %s\n", error);
//     }
VL_API VlResult vl_xml_new(VlXML *xml, const char *source, char *error);

#endif