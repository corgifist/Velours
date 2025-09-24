#ifndef VELOURS_XML_H
#define VELOURS_XML_H

#include "velours.h"
#include "da.h"

typedef struct {
	VL_DA(char) name;
	VL_DA(char) value;
} VlXMLAttribute;

struct VlXMLNode {
	const char *name;

	VL_DA(VlXMLAttribute) attributes;
	VL_DA(struct VlXMLNode) nodes;
};

typedef struct VlXMLNode VlXMLNode;

typedef struct {
	const char *source;

	const char *version;
	const char *encoding;

	VlXMLNode root;
} VlXML;

VL_API VlResult vl_xml_open(VlXML *xml, const char* source);

#endif