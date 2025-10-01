#include "velours.h"
#include "da.h"
#include "ht.h"
#include "xml/xml.h"
#include "utf8.h"
#include "file.h"
#include "memory.h"
#include "platform/window.h"

#include <windows.h>
#include <stdio.h>
#include <time.h>

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

const char* test_xml =  VL_STRINGIFY_VARIADIC(
<?xml version="1.0"?>
<customers>
   <customer id="55000">
      <name>Charter Group</name>
      <address>
         <street>100 Main</street>
         <city>Framingham</city>
         <state>MA</state>
         <zip>01701</zip>
      </address>
      <address>
         <street>720 Prospect</street>
         <city>Framingham</city> // intentional error
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
);

void da_test(void) {
    printf("initial memory usage: %zu\n", vl_get_memory_usage());
	int *test;
	VL_DA_NEW(test, int);
    printf("after da creation memory usage: %zu\n", vl_get_memory_usage());
	printf("%p\n", test);

	for (int i = 0; i < 10000; i++) {
		VL_DA_APPEND_CONST(test, int, i);
	}

    printf("peak memory usage: %zu\n", vl_get_memory_usage());

    VL_DA_FREE(test);

    printf("cleanup memory usage: %zu\n", vl_get_memory_usage());
}

void ht_test(void) {
    printf("initial memory usage: %zu\n", vl_get_memory_usage());
    VL_HT(int, int) ht;
    VL_HT_NEW(ht, int, int);
    printf("after ht creation memory usage: %zu\n", vl_get_memory_usage());

    srand((unsigned int) time(NULL));

    for (int i = 0; i < 1000; i++) {
        int key = i - 500;
        int val = i;
        VL_HT_PUT(ht, key, val);
    }

    printf("\n");
    vl_dump_all_allocations();
    printf("\n");

    VlHTEntry entry;
    VL_HT(int, int) iterator_pos = ht;
    int sum = 0;
    while (vl_ht_iterate(ht, &iterator_pos, &entry)) {
        sum += *((int*) entry.value);
    }
    printf("sum: %i\n", sum);

    VL_HT_RESET(ht);

    int redefine_test = 0, found = 0;
    VL_HT_PUT_CONST(ht, int, 69, int, 34);
    VL_HT_GET_CONST(ht, int, 69, redefine_test, found);
    if (found) printf("value of 69 is %i\n", redefine_test);

    VL_HT_PUT_CONST(ht, int, 69, int, 35);
    VL_HT_GET_CONST(ht, int, 69, redefine_test, found);
    if (found) printf("value of 69 is %i\n", redefine_test);

    VL_HT_PUT_CONST(ht, int, 69, int, 42);
    VL_HT_GET_CONST(ht, int, 69, redefine_test, found);
    if (found) printf("value of 69 is %i\n", redefine_test);

    VL_HT_DELETE_CONST(ht, int, 69, found);
    if (found) printf("deleted successfully!\n");
    else printf("failed to delete 69\n");

    VL_HT_GET_CONST(ht, int, 69, redefine_test, found);
    if (found) printf("value of 69 is %i\n", redefine_test);
    else printf("69 is not found\n");

    VL_HT_FREE(ht);

    printf("cleanup memory usage: %zu\n", vl_get_memory_usage());
}

void file_test(void) {
    VL_DA(char) content;

    if (vl_file_read("./30mb.xml", &content)) {
        printf("failed to read 30mb.xml\n");
        return;
    }
    printf("memory usage after reading a file: %zu\n", vl_get_memory_usage());

    VlXML xml;
    if (vl_xml_new(&xml, content, NULL)) {
        printf("failed to parse 30mb.xml\n");
        return;
    }
    VL_DA_FREE(content);
    printf("memory usage after parsing: %zu\n", vl_get_memory_usage());
    vl_dump_all_allocations();

}

void xml_test(void) {
    printf("initial memory usage: %zu\n", vl_get_memory_usage());

    while (1) {
        VlXML test;
        char error[512];
        VL_UNUSED(test);
        VlResult end_result = vl_xml_new(&test, test_xml, error);
        printf("memory usage after parsing: %zu\n", vl_get_memory_usage());
        if (end_result) {
            printf("end_result: %i;\nfailed to open test_xml!\n%s\n", end_result, error);
            return;
        }

        vl_xml_dump(&test, 0);

        vl_xml_free(&test);

        printf("cleanup memory usage: %zu\n", vl_get_memory_usage());
        vl_dump_all_allocations();
        break;
    }
}

void window_test(void) {
    VlWindow *win = vl_window_new("Hello!", 800, 600);
    vl_window_message_loop(win);
}

int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        printf("%s, ", argv[i]);
    }
    printf("\n");
	SetConsoleOutputCP(CP_UTF8);
    vl_memory_set_logging_level(VL_MEMORY_ONLY_ERRORS);

	// da_test();
    // ht_test();
    // file_test();
    // xml_test();
    window_test();

	return VL_SUCCESS;
}