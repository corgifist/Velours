#include "xml.h"
#include "utf8.h"

typedef struct {
	size_t pos, line, len;
} VlXMLParser;

static VlResult vl_xml_parser_open(VlXMLParser *parser, VlXML *xml) {
	parser->pos = 0;
	parser->line = 1;
	parser->len = utf8_strlen(xml->source);

	return VL_SUCCESS;
}

static VlResult vl_xml_parser_parse_node(VlXMLNode *node, VlXMLParser *parser, VlXML *xml) {
	VL_UNUSED(parser);
	VL_UNUSED(node);
	const char* c = xml->source;
	uint32_t cp = 0;
	int len = 0;
	while ((cp = utf8_decode(&c, &len))) {
		printf("%.*s", len, c - len);
	}

	return VL_SUCCESS;
}

VL_API VlResult vl_xml_open(VlXML *xml, const char *source) {
	xml->source = source;

	VlXMLParser parser;
	vl_xml_parser_open(&parser, xml);
	vl_xml_parser_parse_node(&xml->root, &parser, xml);

	return VL_SUCCESS;
}