/*
	xml.h - XML parsing utilities
	PLEASE NOTICE: 
	this parser was created to be compatible with HTML and XML at the same time
	but HTML and XML are not interchangeable standards, which means that 
	some functionality of HTML and XML may not be supported by this parser or
    some documents may be parsed incorrectly	

	for instance:
	    * CDATA tags are not supported by this parser (error will be thrown)
		* custom entity tables are not supported (error will be thrown)
		* nodes like this
			  <paragraph>
				  What a <inline>beautiful</inline> world!
			  </paragraph>

		  would be parsed like this:
		       paragraph:
			      text node: 'What a'
				  inline:
				      text node: 'beautiful'
				  text node: 'world!'

		  instead of this (the way standard XML parser does):
		       paragraph:
			       inline:
				       text node: 'beautiful'
			       text node: 'What a \n world!'

		  the reason is again parser should be compatible with HTML and XML at the same time
		  
*/

#ifndef VELOURS_XML_H
#define VELOURS_XML_H

#include "velours.h"
#include "da.h"
#include "file.h"

typedef struct {
	// name and value ARE null-terminated utf-8 strings!
	VL_DA(u8) name;
	VL_DA(u8) value;
} VlXMLAttribute;

struct VlXMLNode {
	VL_DA(u8) name;

	// is_complex is set to 0 if the node is just a text
	// otherwise, is_complex is 1
	// examples:
	//     <node property="..." /> (is_complex = 1)
	//     <node> (is_complex = 1)
	//         Hello, World! (is_complex = 0)
	//     </node>
	char is_complex;

	VL_DA(u8) text;

	VL_DA(VlXMLAttribute) attributes;
	VL_DA(struct VlXMLNode) children;
};

typedef struct VlXMLNode VlXMLNode;

typedef struct {
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
//     char error[512];
//	   VlXML *xml = vl_xml_new("<abc>...</abc>", error);
//     if (!xml) {
//         printf("failed to parse xml: %s\n", error);
//     }
VL_API VlXML *vl_xml_new(const u8 *source, u8 *error);
VL_API VlXML *vl_xml_new_from_file(VlFile *file, u8 *error);

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