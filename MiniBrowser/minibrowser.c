#include "velours.h"
#include "da.h"
#include "ht.h"
#include "xml/xml.h"
#include "utf.h"
#include "file.h"
#include "memory.h"
#include "platform/window.h"
#include "platform/graphics.h"
#include "platform/main.h"

#include <windows.h>
#include <stdio.h>
#include <time.h>

/*
const char* test_xml =  VL_STRINGIFY_VARIADIC(
<?xml version="1.0"?>
<customers>
   <customer id="&quot;sAome guy&apos;s id&quot;">
        <name person>
               &quot;sSome guy&apos;s name &amp; co.&quot;
        </name>
    </customer>
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
*/

const char *test_xml = VL_STRINGIFY_VARIADIC(
<test>Hello, World!</test>
);

void da_test(void) {
    printf("initial memory usage: %zu\n", vl_memory_get_usage());
	int *test;
	VL_DA_NEW(test, int);
    printf("after da creation memory usage: %zu\n", vl_memory_get_usage());
	printf("%p\n", test);

	for (int i = 0; i < 10000; i++) {
		VL_DA_APPEND_CONST(test, int, i);
	}

    printf("peak memory usage: %zu\n", vl_memory_get_usage());
    vl_memory_dump();
    VL_DA_FREE(test);
    printf("cleanup memory usage: %zu\n", vl_memory_get_usage());
}

void ht_test(void) {
    printf("initial memory usage: %zu\n", vl_memory_get_usage());
    VL_HT(int, int) ht;
    VL_HT_NEW(ht, int, int);
    printf("after ht creation memory usage: %zu\n", vl_memory_get_usage());

    srand((unsigned int)time(NULL));

    for (int i = 0; i < 1000; i++) {
        int key = i - 500;
        int val = i;
        VL_HT_PUT(ht, key, val);
    }

    printf("intermediate (usage: %zu):\n", vl_memory_get_usage());
    vl_memory_dump();
    printf("\n");

    VlHTEntry entry;
    VL_HT(int, int) iterator_pos = ht;
    int sum = 0;
    while (vl_ht_iterate(ht, &iterator_pos, &entry)) {
        sum += *((int*)entry.value);
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

    printf("cleanup memory usage: %zu\n", vl_memory_get_usage());
}

void file_test(void) {
    VlFile* file = vl_file_new("30mb.xml", "r");

    VlXML* xml = vl_xml_new_from_file(file, NULL);
    if (!xml) {
        printf("failed to parse 30mb.xml\n");
        return;
    }
    printf("memory usage after parsing: %zu\n", vl_memory_get_usage());
    vl_file_free(file);
    vl_xml_free(xml);
    vl_memory_dump();
}

void xml_test(void) {
    printf("initial memory usage: %zu\n", vl_memory_get_usage());

    char error[512];
    VlXML* test = vl_xml_new(test_xml, error);
    printf("memory usage after parsing: %zu\n", vl_memory_get_usage());
    if (!test) {
        printf("end_result: %p;\nfailed to open test_xml!\n%s\n", test, error);
        return;
    }

    vl_xml_dump(test, 0);

    vl_xml_free(test);

    printf("cleanup memory usage: %zu\n", vl_memory_get_usage());
    vl_memory_dump();
}

static VlWindow s_window;
static VlGraphics s_graphics;

void resize(VlWindow window) {
    vl_graphics_resize(s_graphics, window->cw, window->ch);
}

void paint(VlWindow window) {
    VL_UNUSED(window);
    vl_graphics_presentation_begin(s_graphics);
    vl_graphics_begin(s_graphics);

    vl_graphics_clear(s_graphics, VL_RGBA(1, 0, 1, 1));
    
    vl_graphics_end(s_graphics);
    vl_graphics_presentation_end(s_graphics);
}

void window_test(void) {
    s_window = vl_window_new("Velours Software Renderer Backend", 0, 0, 640, 480);
    if (!s_window) {
        printf("failed to create window\n");
        return;
    }

    if (vl_graphics_initialize(VL_GRAPHICS_BACKEND_SOFTWARE)) {
        printf("failed to initialize graphics!\n");
        return;
    }

    printf("x, y, w, h: %i, %i, %i, %i\n", s_window->x, s_window->y, s_window->w, s_window->h);
    printf("cx, cy, cw, ch: %i, %i, %i, %i\n", s_window->cx, s_window->cy, s_window->cw, s_window->ch);
    s_graphics = vl_graphics_new(s_window);
    if (!s_graphics) {
        printf("failed to create graphics!\n");
        return;
    }

    vl_window_set_paint_function(s_window, paint);
    vl_window_set_resize_function(s_window, resize);
    vl_window_set_visible(s_window, 1);
    vl_window_message_loop(s_window);

    vl_graphics_free(s_graphics);
    vl_window_free(s_window);

    if (vl_graphics_terminate()) {
        printf("failed to terminate graphics!\n");
        return;
    }
}

void utf_test(void) {
    VlFile *file = vl_file_new("utf8.txt", "rb");
    printf("bom: %i\n", file->bom);
    u32 cp;
    while ((cp = vl_file_read_codepoint(file))) {
        u8 enc[4];
        int len = utf8_encode(cp, enc);
        printf("%.*s", len, enc);
    }
    printf("\n");
    vl_file_free(file);
}

void stream_test(void) {
    VlFile* f = vl_file_new("ascii.txt", "r");

    char error[512];
    VlXML *test = vl_xml_new_from_file(f, error);
    printf("memory usage after parsing: %zu\n", vl_memory_get_usage());
    if (test) {
        printf("end_result: %p;\nfailed to open test_xml!\n%s\n", test, error);
        return;
    }

    vl_xml_dump(test, 0);

    vl_xml_free(test);
    vl_file_free(f);

    printf("cleanup memory usage: %zu\n", vl_memory_get_usage());
    vl_memory_dump();
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
    // stream_test();
    // utf_test();
    window_test();
    vl_memory_dump();
	return VL_SUCCESS;
}