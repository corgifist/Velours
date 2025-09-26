#include "velours.h"
#include "da.h"
#include "xml/xml.h"
#include "utf8.h"
#include <windows.h>

#include <stdio.h>

/* const char* test_xml = "    " VL_STRINGIFY_VARIADIC(
<?xml version="1.0" encoding="UTF-8"?>
<?processing instruction="example"?>
<!-- Top‑level comment -->
<catalog xmlns:bk="http://example.com/book"
         xmlns:auth="http://example.com/author"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://example.com/book book.xsd">

  <!-- Entity declaration (internal DTD) -->
  <!DOCTYPE catalog [
    <!ENTITY copy "©">
    <!ENTITY euro "€">
  ]>

  <!-- Mixed content with text and child elements -->
  <description>
    This catalog contains a collection of books and authors.
    It includes <em>italic</em> and <strong>bold</strong> markup.
  </description>

  <!-- Simple element with attributes -->
  <bk:book id="b001" genre="fiction" price="19.95" available="true">
    <bk:title>The Great Adventure &amp; Beyond</bk:title>
    <bk:author ref="a001"/>
    <bk:published>2023-07-15</bk:published>

    <!-- CDATA section (preserves special characters) -->
    <bk:summary><![CDATA[
      A thrilling tale of exploration. Characters say "Hello, world!" 
      and encounter <unknown> tags that must be ignored by the parser.
    ]]></bk:summary>

    <!-- Nested element with namespace prefix -->
    <auth:details>
      <auth:name>Jane Doe</auth:name>
      <auth:birthdate>1975-04-22</auth:birthdate>
      <auth:bio>Author of several best‑selling novels &copy; 2024.</auth:bio>
    </auth:details>

    <!-- Empty element (self‑closing) -->
    <bk:rating/>
  </bk:book>

  <!-- Another book with different attribute types -->
  <bk:book id="b002" genre="non-fiction" price="0" available="false">
    <bk:title>Understanding XML</bk:title>
    <bk:author ref="a002"/>
    <bk:published>2020-01-01</bk:published>
    <bk:summary>Learn XML basics, DTDs, schemas, and best practices.</bk:summary>
    <bk:pages>350</bk:pages>
    <bk:price currency="EUR">&euro;0.00</bk:price>
  </bk:book>

  <!-- Author definitions referenced above -->
  <auth:author id="a001">
    <auth:name>John Smith</auth:name>
    <auth:email>john.smith@example.com</auth:email>
    <auth:affiliation>Example University</auth:affiliation>
  </auth:author>

  <auth:author id="a002">
    <auth:name>Emily Johnson</auth:name>
    <auth:email>emily.j@example.org</auth:email>
    <auth:affiliation>Tech Press</auth:affiliation>
  </auth:author>

  <!-- Comment inside the root -->
  <!-- End of catalog -->
</catalog>
) "    "; */

const char* test_xml = VL_STRINGIFY_VARIADIC(
<!-- Testing comments -->
<ordinary>
<omg value="omg"/>
<!-- Testing comments part two -->
<omg value="omg2"/ >
</ordinary>
);

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
	else printf("text: '%s'\n", node->text);

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