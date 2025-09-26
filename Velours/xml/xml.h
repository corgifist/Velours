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
	
	// pi means processing instructions
	// processing instruction in xml is an element that looks like:
	// <?something ... ?>
	VL_DA(VlXMLNode) pi;
} VlXML;

VL_API VlResult vl_xml_attribute_new(VlXMLAttribute *attribute);
VL_API VlResult vl_xml_attribute_free(VlXMLAttribute *attribute);

VL_API VlResult vl_xml_node_new(VlXMLNode *node);
VL_API VlResult vl_xml_node_free(VlXMLNode* node);

// void vl_xml_node_dump(VlXMLNode *node)
// dumps given node to stdout (not recursive!!!)
// 
// arguments:
//     node - xml node to dump
//     indent - indentation level of the output
//              zero means no additional indentation (default and preferred option),
//              negative means no indentation at all,
//              positive means additional indentation
// 
// usage:
//     VlXML xml;
//     vl_xml_new(&xml, "<abc>...</abc>", NULL);
//     vl_xml_node_dump(&xml->root, 0);
VL_API void vl_xml_node_dump(VlXMLNode *node, int indent);

// void vl_xml_node_dump_recursive(VlXMLNode *node)
// dumps given node to stdout in a recursive manner
// under the hood this function utilizes vl_xml_node_dump
// 
// arguments: same as vl_xml_node_dump 
// usage: same as vl_xml_node_dump
VL_API void vl_xml_node_dump_recursive(VlXMLNode *node, int indent);

// VlResult vl_xml_new(VlXML *xml, const char *source, char *error)
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

// VlResult vl_xml_free(VlXML *xml)
// frees given xml object
VL_API VlResult vl_xml_free(VlXML *xml);

// void vl_xml_dump(VlXML *xml);
// this function is an alias to vl_xml_node_dump_recursive(&xml->root);
// dumps given xml's object root node (recursively)
//
// arguments: same as vl_xml_node_dump_recursive
// usage:
//     VlXML xml;
//     vl_xml_new(&xml, "<abc>...</abc>", NULL);
//     vl_xml_dump(&xml, 0);
VL_API void vl_xml_dump(VlXML *xml, int indent);
#endif