#include "velours.h"
#include "da.h"
#include "xml/xml.h"
#include "utf8.h"
#include <windows.h>

#include "memory.h"

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
<hello />
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

int main(void) {
	SetConsoleOutputCP(CP_UTF8);
	// da_test();

    printf("initial memory usage: %zu\n", vl_get_memory_usage());

    while (1) {
        VlXML test;
        char error[512];
        VL_UNUSED(test);
        VlResult end_result = vl_xml_new(&test, test_xml, error);
        printf("memory usage after parsing: %zu\n", vl_get_memory_usage());
        if (end_result) {
            printf("end_result: %i;\nfailed to open test_xml!\n%s\n", end_result, error);
            return VL_ERROR;
        }

        vl_xml_dump(&test, 0);

        vl_xml_free(&test);

        printf("cleanup memory usage: %zu\n", vl_get_memory_usage());
        break;
    }

	return VL_SUCCESS;
}