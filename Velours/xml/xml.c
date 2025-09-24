#include "xml.h"
#include "utf8.h"

#define COLLECTING_NOTHING 0
#define COLLECTING_NODE_NAME 1
#define COLLECTING_PROPERTY_NAME 2
#define COLLECTING_PROPERTY_VALUE 3

typedef struct {
	size_t pos, line, len;

	VL_DA(char) node_name;
	VL_DA(char) property_name;
	VL_DA(char) property_value;
	char state;
} VlXMLParser;

typedef struct {
	uint32_t codepoint;
	int len;
} VlXMLParserSymbol;

static VlResult vl_xml_parser_open(VlXMLParser* parser, VlXML* xml) {
	parser->pos = 0;
	parser->line = 1;
	parser->len = utf8_strlen(xml->source);
	parser->node_name = NULL;
	parser->property_name = NULL;
	parser->state = COLLECTING_NOTHING;

	VL_DA_ALLOC(parser->node_name, char);
	VL_DA_ALLOC(parser->property_name, char);
	VL_DA_ALLOC(parser->property_value, char);

	return VL_SUCCESS;
}

static VlResult vl_xml_parser_parse_node(VlXMLNode *node, VlXMLParser *parser, VlXML *xml) {
	const char *p = xml->source;
	utf8_advance(&p, parser->pos);

	int len;
	uint32_t codepoint = utf8_decode(&p, &len);

#define ADVANCE() \
	do { \
		parser->pos++; codepoint = utf8_decode(&p, &len); \
	} while (0) \

#define USELESS(C) ((C == ' ') || (C == '\n'))

	while (parser->pos < parser->len) {
		if (USELESS(codepoint)) {
			ADVANCE();
			continue;
		}
		if (codepoint == '<') {
			ADVANCE();
			if (codepoint == '/') break;
			parser->state = COLLECTING_NODE_NAME;

			while (codepoint != '>' && parser->pos < parser->len) {
				const char* c = p - len;
				if (USELESS(codepoint) && parser->state != COLLECTING_PROPERTY_VALUE) {
					if (parser->state == COLLECTING_NODE_NAME && VL_DA_LENGTH(parser->node_name) > 0) {
						parser->state = COLLECTING_PROPERTY_NAME;
						VL_DA_APPEND_CONST(parser->node_name, char, 0);
						printf("node name: \"%s\"\n", parser->node_name);
						VL_DA_RESET(parser->node_name);
					}
					ADVANCE();
					continue;
				}

				if (codepoint == '"' && parser->state == COLLECTING_PROPERTY_VALUE) {
					VL_DA_APPEND_CONST(parser->property_value, char, 0);
					printf("property value: \"%s\"\n", parser->property_value);
					VL_DA_RESET(parser->property_value);
					parser->state = COLLECTING_PROPERTY_NAME;
					ADVANCE();
					continue;
				}

				if (codepoint == '=' && parser->state == COLLECTING_PROPERTY_NAME) {
					parser->state = COLLECTING_PROPERTY_VALUE;
					VL_DA_APPEND_CONST(parser->property_name, char, 0);
					printf("property name: \"%s\"\n", parser->property_name);
					VL_DA_RESET(parser->property_name);
					ADVANCE();
					if (codepoint != '"') {
						printf("expected '\"' after '='\n");
						return VL_ERROR;
					}
					ADVANCE();
					parser->state = COLLECTING_PROPERTY_VALUE;
					continue;
				}

				VL_DA(char) *target_array = NULL;
				if (parser->state == COLLECTING_NODE_NAME)
					target_array = &parser->node_name;
				else if (parser->state == COLLECTING_PROPERTY_NAME)
					target_array = &parser->property_name;
				else if (parser->state == COLLECTING_PROPERTY_VALUE)
					target_array = &parser->property_value;

				if (len >= 1) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *c));
				if (len >= 2) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 1)));
				if (len >= 3) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 2)));
				if (len >= 4) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 3)));

				ADVANCE();
			}
			parser->state = COLLECTING_NOTHING;
		}

		ADVANCE();
	}

#undef ADVANCE

	return VL_SUCCESS;
}

VL_API VlResult vl_xml_open(VlXML *xml, const char *source) {
	xml->source = source;
	xml->version = NULL;
	xml->encoding = NULL;

	VlXMLParser parser;
	vl_xml_parser_open(&parser, xml);

	if (vl_xml_parser_parse_node(&xml->root, &parser, xml)) {
		return VL_ERROR;
	}

	if (!xml->version)
		xml->version = "1.0";
	if (!xml->encoding)
		xml->encoding = "UTF-8";

	return VL_SUCCESS;
}