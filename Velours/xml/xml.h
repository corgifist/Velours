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

VL_API VlResult vl_xml_new(VlXML *xml, const char *source);

#endif