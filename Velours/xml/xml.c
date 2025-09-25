#include "xml.h"
#include "utf8.h"

#define COLLECTING_NOTHING 0
#define COLLECTING_NODE_NAME 1
#define COLLECTING_PROPERTY_NAME 2
#define COLLECTING_PROPERTY_VALUE 3

typedef struct {
	const char *source;
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

#define VL_EXPECTED_CLOSING_TAG 2

static VlResult vl_xml_parser_new(VlXMLParser *parser, const char *source) {
	parser->pos = 0;
	parser->line = 1;
	parser->source = source;
	parser->len = utf8_strlen(parser->source);
	parser->node_name = NULL;
	parser->property_name = NULL;
	parser->state = COLLECTING_NOTHING;

	VL_DA_NEW(parser->node_name, char);
	VL_DA_NEW(parser->property_name, char);
	VL_DA_NEW(parser->property_value, char);

	return VL_SUCCESS;
}

static VlResult vl_xml_parser_free(VlXMLParser *parser) {
	VL_DA_FREE(parser->node_name);
	VL_DA_FREE(parser->property_name);
	VL_DA_FREE(parser->property_value);

	return VL_SUCCESS;
}

static VlResult vl_xml_parser_parse_node(VlXMLNode *node, VlXMLParser *parser, VlXML *xml, char *error) {
	const char *p = parser->source;
	utf8_advance(&p, parser->pos);
	// printf("\n'%s'\n", p);

	int len;
	uint32_t codepoint = utf8_decode(&p, &len);

#define ADVANCE() \
	do { \
		if (++parser->pos >= parser->len) { \
			return VL_ERROR; \
		} \
		codepoint = utf8_decode(&p, &len); \
	} while (0) \

#define USELESS(C) ((C == ' ') || (C == '\n') || (C == '\t'))
#define SPECIAL(C) ((C == '<') || (C == '>'))

	// skip all useless symbols so we can proceed to parsing
	while (parser->pos < parser->len) {
		if (USELESS(codepoint)) {
			ADVANCE();
			continue;
		}
		break;
	}

	// collecting node name, properties etc.
	if (codepoint != '<') {
		if (SPECIAL(codepoint)) {
			if (error) sprintf(error, "unexpected special symbol at line %zu", parser->line);
			return VL_ERROR;
		}
		// node is not complex, just collecting text until we encounter some special symbols
		node->is_complex = 0;
		while (!SPECIAL(codepoint)) {
			if (codepoint == '\n') {
				ADVANCE();
				continue;
			}

			const char* c = p - len;
			if (len >= 1) VL_DA_INDIRECT(&node->text, VL_DA_APPEND_CONST(INDIRECT, char, *c));
			if (len >= 2) VL_DA_INDIRECT(&node->text, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 1)));
			if (len >= 3) VL_DA_INDIRECT(&node->text, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 2)));
			if (len >= 4) VL_DA_INDIRECT(&node->text, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 3)));
			ADVANCE();
		}
		VL_DA_APPEND_CONST(node->text, char, 0);
		return VL_SUCCESS;
	}
	ADVANCE();
	parser->state = COLLECTING_NODE_NAME;

	char parse_child_nodes = 1;
	char block_attributes = 0;
	char expecting_closing_tag = 0;
	char encountered_attributes = 0;
	char encountered_name = 0;
	node->is_complex = 1;
	while (parser->pos < parser->len) {
		if (USELESS(codepoint) && parser->state != COLLECTING_PROPERTY_VALUE) {
			if (parser->state == COLLECTING_NODE_NAME && VL_DA_LENGTH(parser->node_name) > 0) {
				parser->state = COLLECTING_PROPERTY_NAME;
				VL_DA_APPEND_CONST(parser->node_name, char, 0);
				node->name = parser->node_name;
				// printf("node name: '%s'\n", node->name);
				encountered_name = 1;
			}
			ADVANCE();
			continue;
		}

		if (codepoint == '>') break;

		if (codepoint == '/') {
			parse_child_nodes = 0;
			if (!encountered_name) expecting_closing_tag = 1;
			else block_attributes = 1;
			ADVANCE();
			continue;
		}

		if (codepoint == '"' && parser->state == COLLECTING_PROPERTY_VALUE) {
			VL_DA_APPEND_CONST(parser->property_value, char, 0);
			// printf("property value: \"%s\"\n", parser->property_value);
			if (!block_attributes) {
				VlXMLAttribute attribute;
				attribute.name = parser->property_name;
				attribute.value = parser->property_value;
				VL_DA_APPEND(node->attributes, attribute);
				VL_DA_NEW(parser->property_name, char);
				VL_DA_NEW(parser->property_value, char);
			} else {
				VL_DA_RESET(parser->property_name);
				VL_DA_RESET(parser->property_value);
			}
			parser->state = COLLECTING_PROPERTY_NAME;
			encountered_attributes = 1;
			ADVANCE();
			continue;
		}

		if (codepoint == '=' && parser->state == COLLECTING_PROPERTY_NAME) {
			parser->state = COLLECTING_PROPERTY_VALUE;
			VL_DA_APPEND_CONST(parser->property_name, char, 0);
			// printf("property name: \"%s\"\n", parser->property_name);
			ADVANCE();
			while (codepoint != '"') ADVANCE();
			ADVANCE();
			parser->state = COLLECTING_PROPERTY_VALUE;
			continue;
		}

		VL_DA(char)* target_array = NULL;
		if (parser->state == COLLECTING_NODE_NAME)
			target_array = &parser->node_name;
		else if (parser->state == COLLECTING_PROPERTY_NAME)
			target_array = &parser->property_name;
		else if (parser->state == COLLECTING_PROPERTY_VALUE)
			target_array = &parser->property_value;

		const char* c = p - len;
		if (len >= 1) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *c));
		if (len >= 2) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 1)));
		if (len >= 3) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 2)));
		if (len >= 4) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 3)));

		ADVANCE();
	}
	// skip '>'
	ADVANCE();

	if (VL_DA_LENGTH(node->name) == 0) {
		parser->state = COLLECTING_PROPERTY_NAME;
		VL_DA_APPEND_CONST(parser->node_name, char, 0);
		node->name = parser->node_name;
		// printf("node name: '%s'\n", node->name);
	}

	// collecting children nodes
	while (parser->pos < parser->len && parse_child_nodes) {
		VlXMLParser child_parser;
		vl_xml_parser_new(&child_parser, parser->source);
		child_parser.pos = parser->pos;
		child_parser.line = parser->line;
		child_parser.len = parser->len;

		VlXMLNode child;
		vl_xml_node_new(&child);

		char child_error[512];
		VlResult parse_result = vl_xml_parser_parse_node(&child, &child_parser, xml, child_error);
		parser->pos = child_parser.pos;
		parser->line = parser->line;
		p = parser->source;
		if (parse_result == VL_ERROR) {
			vl_xml_parser_free(&child_parser);
			vl_xml_node_free(&child);
			// printf("%s\n", child_error);

			break;
		}

		if (parse_result == VL_EXPECTED_CLOSING_TAG &&
			(utf8_strlen(node->name) != utf8_strlen(child.name) || strcmp(node->name, child.name) != 0)) {
			sprintf(error, "mismatching tags <%s> and </%s>", node->name, child.name);
			vl_xml_parser_free(&child_parser);
			vl_xml_node_free(&child);
			return VL_ERROR;
		}

		if (parse_result == VL_EXPECTED_CLOSING_TAG) {
			vl_xml_parser_free(&child_parser);
			return VL_SUCCESS;
		}

		VL_DA_APPEND(node->children, child);
	}

#undef ADVANCE
#undef USELESS

	return expecting_closing_tag ? VL_EXPECTED_CLOSING_TAG : VL_SUCCESS;
}

VL_API VlResult vl_xml_attribute_new(VlXMLAttribute *attribute) {
	VL_DA_NEW(attribute->name, char);
	if (!attribute->name) return VL_ERROR;

	VL_DA_NEW(attribute->value, char);
	if (!attribute->value) {
		VL_DA_FREE(attribute->name);
		return VL_ERROR;
	}

	return VL_SUCCESS;
}

VL_API VlResult vl_xml_attribute_free(VlXMLAttribute *attribute) {
	VL_DA_FREE(attribute->name);
	VL_DA_FREE(attribute->value);

	return attribute->name && attribute->value;
}

VL_API VlResult vl_xml_node_new(VlXMLNode *node) {
	node->name = NULL;

	VL_DA_NEW(node->attributes, VlXMLAttribute);
	VL_DA_NEW(node->children, struct VlXMLNode);
	VL_DA_NEW(node->text, char);

	return !(node->attributes && node->children);
}

VL_API VlResult vl_xml_node_free(VlXMLNode *node) {
	node->name = NULL;

	VL_DA_FREE(node->attributes);
	VL_DA_FREE(node->children);
	VL_DA_FREE(node->text);

	return VL_SUCCESS;
}

VL_API VlResult vl_xml_new(VlXML *xml, const char *source, char *error) {
	xml->version = NULL;
	xml->encoding = NULL;

	vl_xml_node_new(&xml->root);

	VlXMLParser parser;
	vl_xml_parser_new(&parser, source);

	if (vl_xml_parser_parse_node(&xml->root, &parser, xml, error)) {
		return VL_ERROR;
	}

	if (!xml->version)
		xml->version = "1.0";
	if (!xml->encoding)
		xml->encoding = "UTF-8";

	return VL_SUCCESS;
}