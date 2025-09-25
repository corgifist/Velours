#include "velours.h"
#include "da.h"
#include "xml/xml.h"
#include "utf8.h"
#include <windows.h>

#include <stdio.h>

const char *test_xml = "    " VL_STRINGIFY_VARIADIC(
<customers>
   <customer id="55000">
	Unexpected string 1
      <name>Charter Group</name>
      <address>
	Unexpected string      2
         <street>100 Main</street>
         <city>Framingham</city>
         <state>MA</state>
         <zip>01701</zip>
      </address>
      <address>
         <street>720 Prospect</street>
         <city>Framingham</city>
         <state>MA</state>
         <zip>01701</zip>
      </address>
      <address>
         <street>120 Ridge</street>
         <state>MA</state>
         <zip>01760</zip>
      </address>
   </customer>
</customers>
) "    ";

void da_test(void) {
	int *test;
	VL_DA_NEW(test, int);
	printf("%p\n", test);

	for (int i = 0; i < 10000; i++) {
		VL_DA_APPEND_CONST(test, int, i);
		VL_DA_DUMP_HEADER(test);
	}
}

void dump_xml_node(VlXMLNode *node, int indent) {
	for (int i = 0; i < indent; i++) printf("    ");
	if (node->is_complex) printf("%s\n", node->name);
	else printf("text: %s\n", node->text);

	VL_DA_FOREACH(node->attributes, i) {
		VlXMLAttribute* attr = &node->attributes[i];
		for (int j = 0; j < indent; j++) printf("    ");
		printf("    %s = \"%s\"\n", attr->name, attr->value);
	}

	VL_DA_FOREACH(node->children, i) {
		dump_xml_node(&node->children[i], indent + 1);
	}
}

int main(void) {
	SetConsoleOutputCP(CP_UTF8);
	// da_test();

	VlXML test;
	char error[512];
	VL_UNUSED(test);
	VlResult end_result = vl_xml_new(&test, test_xml, error);
	if (end_result) {
		printf("end_result: %i;\nfailed to open test_xml!\n%s\n", end_result, error);
		return VL_ERROR;
	}

	dump_xml_node(&test.root, 0);

	return VL_SUCCESS;
}