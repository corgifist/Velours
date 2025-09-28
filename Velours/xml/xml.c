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

#define VL_EXPECTED_CLOSING_TAG 2
#define VL_SKIP_TAG 4
#define VL_SPECIAL_TAG 8

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

static VlResult vl_xml_parser_parse_node(VlXMLNode *node, VlXMLParser *parser, VlXML *xml, char *error, size_t *error_offset, char *parse_child_nodes) {
	VL_UNUSED(xml);
	VL_UNUSED(node);
	// utf8_advance(&p, parser->pos);
	// printf("\n'%s'\n", parser->source);
	// printf("%zu\n", parser->pos);

	int len;
	uint32_t codepoint = utf8_decode(&parser->source, &len);
	*parse_child_nodes = 1;

#define APPEND_ERROR(...) \
	do { \
		if (error) { \
			if (*error_offset != 0) \
				*error_offset += sprintf(error + *error_offset, "\n"); \
			*error_offset += sprintf(error + *error_offset, __VA_ARGS__); \
		} \
	} while (0)

#define ADVANCE() \
	do { \
		if (parser->pos >= parser->len) { \
			APPEND_ERROR("EOF at line %zu", parser->line); \
			return VL_ERROR; \
		} \
		parser->pos++; \
		codepoint = utf8_decode(&parser->source, &len); \
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
			APPEND_ERROR("unexpected special symbol at line %zu", parser->line);
			return VL_ERROR;
		}
		// node is not complex, just collecting text until we encounter some special symbols
		node->is_complex = 0;
		while (!SPECIAL(codepoint)) {
			if (codepoint == '\n') {
				ADVANCE();
				continue;
			}

			const char* c = parser->source - len;
			if (len >= 1) VL_DA_APPEND_CONST(node->text, char, *c);
			if (len >= 2) VL_DA_APPEND_CONST(node->text, char, *(c + 1));
			if (len >= 3) VL_DA_APPEND_CONST(node->text, char, *(c + 2));
			if (len >= 4) VL_DA_APPEND_CONST(node->text, char, *(c + 3));

			ADVANCE();
		}
		parser->source -= len;
		VL_DA_APPEND_CONST(node->text, char, 0);
		*parse_child_nodes = 0;
		return VL_SUCCESS;
	}
	ADVANCE();
	parser->state = COLLECTING_NODE_NAME;

	char block_attributes = 0;
	char expecting_closing_tag = 0;
	char encountered_attributes = 0;
	char encountered_name = 0;
	char expect_full_expression = 0;
	char comment_level = 0;
	char special_level = 0;
	node->is_complex = 1;
	while (parser->pos < parser->len) {
		if (USELESS(codepoint) && comment_level != 0 && comment_level < 3) {
			APPEND_ERROR("invalid beginning of comment tag at line %zu", parser->line);
			return VL_ERROR;
		}
		if (comment_level >= 3) {
			// printf("%i, %.*s\n", comment_level, len, p - len);
			if (codepoint == '>') break;
			if (comment_level > 3 && USELESS(codepoint)) {
				APPEND_ERROR("invalid ending of comment tag at line %zu", parser->line);
				return VL_ERROR;
			}
			if (codepoint == '-') comment_level++;
			ADVANCE();
			continue;
		}

		if (USELESS(codepoint) && parser->state == COLLECTING_PROPERTY_NAME) {
			if (VL_DA_LENGTH(parser->property_name)) {
				VlXMLAttribute attribute;
				VL_DA_APPEND_CONST(parser->property_name, char, '\0');
				VL_DA_APPEND_CONST(parser->property_value, char, 't');
				VL_DA_APPEND_CONST(parser->property_value, char, 'r');
				VL_DA_APPEND_CONST(parser->property_value, char, 'u');
				VL_DA_APPEND_CONST(parser->property_value, char, 'e');
				VL_DA_APPEND_CONST(parser->property_value, char, '\0');
				attribute.name = parser->property_name;
				attribute.value = parser->property_value;
				VL_DA_APPEND(node->attributes, attribute);
				VL_DA_NEW(parser->property_name, char);
				VL_DA_NEW(parser->property_value, char);
			}
			ADVANCE();
			continue;
		}

		if (USELESS(codepoint) && parser->state != COLLECTING_PROPERTY_VALUE) {
			if (parser->state == COLLECTING_NODE_NAME && VL_DA_LENGTH(parser->node_name)) {
				parser->state = COLLECTING_PROPERTY_NAME;
				VL_DA_APPEND_CONST(parser->node_name, char, 0);
				node->name = parser->node_name;
				VL_DA_NEW(parser->node_name, char);
				// printf("node name: '%s'\n", node->name);
				encountered_name = 1;
			}
			ADVANCE();
			continue;
		}

		if (codepoint == '>') break;

		if (!encountered_name && !encountered_attributes && codepoint == '!') {
			comment_level = 1;
			ADVANCE();
			continue;
		}

		if (comment_level >= 1 && comment_level <= 3 && codepoint == '-') {
			comment_level++;
			// printf("incrementing comment_level\n");
			ADVANCE();
			continue;
		}

		if (codepoint == '/') {
			*parse_child_nodes = 0;
			if (!encountered_name) expecting_closing_tag = 1;
			else block_attributes = 1;
			ADVANCE();
			continue;
		}

		if (encountered_name && special_level == 1 && codepoint == '?') {
			block_attributes = 1;
			special_level = 2;
			ADVANCE();
			continue;
		}

		if (!encountered_name && codepoint == '?') {
			special_level = 1;
			*parse_child_nodes = 0;
			ADVANCE();
			continue;
		}

		if (codepoint == '"' && parser->state != COLLECTING_PROPERTY_VALUE) {
			APPEND_ERROR("unexpected \" at line %zu", parser->line);
			return VL_ERROR;
		}

		if (codepoint == '"' && parser->state == COLLECTING_PROPERTY_VALUE) {
			// printf("property value: \"%s\"\n", parser->property_value);
			if (!block_attributes) {
				VlXMLAttribute attribute;
				VL_DA_APPEND_CONST(parser->property_value, char, 0);
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
			expect_full_expression = 0;
			ADVANCE();
			continue;
		}

		if (codepoint == '=' && parser->state == COLLECTING_PROPERTY_NAME) {
			parser->state = COLLECTING_PROPERTY_VALUE;
			if (VL_DA_LENGTH(parser->property_name) <= 0) {
				APPEND_ERROR("invalid property name at line %zu", parser->line);
				return VL_ERROR;
			}
			VL_DA_APPEND_CONST(parser->property_name, char, 0);
			// printf("property name: \"%s\"\n", parser->property_name);
			ADVANCE();
			while (codepoint != '"') ADVANCE();
			ADVANCE();
			expect_full_expression = 1;
			continue;
		}

		VL_DA(char)* target_array = NULL;
		if (parser->state == COLLECTING_NODE_NAME)
			target_array = &parser->node_name;
		else if (parser->state == COLLECTING_PROPERTY_NAME)
			target_array = &parser->property_name;
		else if (parser->state == COLLECTING_PROPERTY_VALUE)
			target_array = &parser->property_value;

		const char* c = parser->source - len;
		if (len >= 1) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *c));
		if (len >= 2) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 1)));
		if (len >= 3) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 2)));
		if (len >= 4) VL_DA_INDIRECT(target_array, VL_DA_APPEND_CONST(INDIRECT, char, *(c + 3)));

		ADVANCE();
	}

	if (comment_level > 0 && comment_level < 5) {
		APPEND_ERROR("incomplete comment at line %zu", parser->line);
		return VL_ERROR;
	}

	if (special_level == 1) {
		APPEND_ERROR("incomplete processing instruction at line %zu", parser->line);
		return VL_ERROR;
	}

	if (comment_level >= 5) {
		return VL_SKIP_TAG;
	}

	if (!VL_DA_LENGTH(node->name)) {
		VL_DA_APPEND_CONST(parser->node_name, char, 0);
		node->name = parser->node_name;
		VL_DA_NEW(parser->node_name, char);
		// printf("node name: '%s'\n", node->name);
	}

	if (expect_full_expression) {
		APPEND_ERROR("invalid syntax at line %zu", parser->line);
		return VL_ERROR;
	}

#undef ADVANCE
#undef USELESS

	if (expecting_closing_tag) return VL_EXPECTED_CLOSING_TAG;

	return special_level >= 2 ? VL_SPECIAL_TAG : VL_SUCCESS;
}

static VlResult vl_xml_parser_parse_node_recursively(VlXMLNode *node, VlXMLParser *parser, VlXML *xml, char *error, size_t *error_offset) {
	char parse_child_nodes;
	VL_DA(VlXMLNode) stack;
	VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR_AND_CAPACITY(stack, sizeof(VlXMLNode), VL_MALLOC, 24);

	VlXMLNode intermediate;
	vl_xml_node_new(&intermediate);

#define APPEND_ERROR(...) \
	do { \
		if (error) { \
			if (*error_offset != 0) \
				*error_offset += sprintf(error + *error_offset, "\n"); \
			*error_offset += sprintf(error + *error_offset, __VA_ARGS__); \
		} \
	} while (0)

#define PUSH_NODE(NODE) \
	VL_DA_APPEND(stack, NODE)

#define TOP \
	(stack[VL_DA_HEADER(stack)->count - 1])

#define UNDER_TOP \
	(stack[VL_DA_HEADER(stack)->count - 2])

#define POP() \
	VL_DA_DELETE(stack, VL_DA_HEADER(stack)->count - 1)

#define FREE_STACK() \
	do { \
		VL_DA_FOREACH(stack, __index) { \
			vl_xml_node_free(&stack[__index]); \
		} \
		VL_DA_FREE(stack); \
	} while (0)

	while (parser->pos < parser->len) {
		VlResult parse_result = vl_xml_parser_parse_node(&intermediate, parser, xml, error, error_offset, &parse_child_nodes);

		if (parse_result == VL_ERROR) {
			vl_xml_node_free(&intermediate);
			FREE_STACK();
			return VL_ERROR;
		}

		if (parse_result == VL_SKIP_TAG) {
			continue;
		}

		if (parse_result == VL_SPECIAL_TAG) {
			if (VL_DA_HEADER(stack)->count != 0) {
				APPEND_ERROR("processing instruction at the wrong place!\n");
				vl_xml_node_free(&intermediate);
				return VL_ERROR;
			}
			VL_DA_APPEND(xml->pi, intermediate);
			vl_xml_node_new(&intermediate);
			continue;
		}

		if (parse_result == VL_EXPECTED_CLOSING_TAG) {
			if (strcmp(TOP.name, intermediate.name) != 0) {
				APPEND_ERROR("mismatched tags <%s> and </%s> at line %zu\n", TOP.name, intermediate.name, parser->line);
				vl_xml_node_free(&intermediate);
				return VL_ERROR;
			}
			vl_xml_node_free(&intermediate);
			if (VL_DA_HEADER(stack)->count > 1) {
				VL_DA_APPEND(UNDER_TOP.children, TOP);
				POP();
			} else {
				break;
			}
			vl_xml_node_new(&intermediate);
			continue;
		}

		if (!parse_child_nodes || !intermediate.is_complex) {
			VL_DA_APPEND(TOP.children, intermediate);
		} else {
			PUSH_NODE(intermediate);
		}
		vl_xml_node_new(&intermediate);
	}

	*node = TOP;

	VL_DA_FREE(stack);
	vl_xml_node_free(&intermediate);

#undef APPEND_ERROR
#undef PUSH_NODE
#undef TOP
#undef UNDER_TOP
#undef POP
#undef FREE_STACK

	return VL_SUCCESS;
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

	node->is_complex = 1;

	return !(node->attributes && node->children && node->text);
}

VL_API VlResult vl_xml_node_free(VlXMLNode *node) {
	if (node->children)
		VL_DA_FOREACH(node->children, i) {
			vl_xml_node_free(&node->children[i]);
		}

	if (node->attributes)
		VL_DA_FOREACH(node->attributes, i) {
			vl_xml_attribute_free(&node->attributes[i]);
		}

	VL_DA_FREE(node->name);

	VL_DA_FREE(node->attributes);
	VL_DA_FREE(node->children);
	VL_DA_FREE(node->text);

	return VL_SUCCESS;
}

VL_API void vl_xml_node_dump(VlXMLNode *node, int indent) {
	if (indent > 0) for (int i = 0; i < indent; i++) printf("    ");
	if (node->is_complex) printf("%s:\n", node->name);
	else printf("text: '%s'\n", node->text);

	VL_DA_FOREACH(node->attributes, i) {
		VlXMLAttribute* attr = &node->attributes[i];
		if (indent > 0) for (int j = 0; j < indent + 1; j++) printf("    ");
		printf("%s = \"%s\"\n", attr->name, attr->value);
	}
}

VL_API void vl_xml_node_dump_recursive(VlXMLNode *node, int indent) {
	vl_xml_node_dump(node, indent);

	VL_DA_FOREACH(node->children, i) {
		vl_xml_node_dump_recursive(&node->children[i], indent + 1);
	}
}

VL_API void vl_xml_dump(VlXML *xml, int indent) {
	vl_xml_node_dump_recursive(&xml->root, indent);
}

VL_API VlResult vl_xml_new(VlXML *xml, const char *source, char *error) {
	xml->version = NULL;
	xml->encoding = NULL;

	VL_DA_NEW(xml->pi, VlXMLNode);

	VlXMLParser parser;
	vl_xml_parser_new(&parser, source);

	size_t error_offset = 0;
	if (vl_xml_parser_parse_node_recursively(&xml->root, &parser, xml, error, &error_offset)) {
		vl_xml_parser_free(&parser);
		return VL_ERROR;
	}

	vl_xml_parser_free(&parser);
	if (!xml->version)
		xml->version = "1.0";
	if (!xml->encoding)
		xml->encoding = "UTF-8";

	return VL_SUCCESS;
}

VL_API VlResult vl_xml_free(VlXML *xml) {
	VL_DA_FOREACH(xml->pi, i) {
		vl_xml_node_free(&xml->pi[i]);
	}
	VL_DA_FREE(xml->pi);

	vl_xml_node_free(&xml->root);

	return VL_SUCCESS;
}