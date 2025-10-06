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
    VlFile file;
    vl_file_new(&file, "30mb.xml", "r");

    VlXML xml;
    if (vl_xml_new_from_file(&xml, &file, NULL)) {
        printf("failed to parse 30mb.xml\n");
        return;
    }
    printf("memory usage after parsing: %zu\n", vl_get_memory_usage());
    vl_file_free(&file);
    vl_xml_free(&xml);
    vl_dump_all_allocations();
}

void xml_test(void) {
    printf("initial memory usage: %zu\n", vl_get_memory_usage());

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
}

static VlGraphics *graphics = NULL;
static VlGraphicsBrush yellow_brush;

void paint(VlWindow window) {
    VL_UNUSED(window);
    vl_graphics_begin(graphics);
    vl_graphics_clear(graphics, VL_RGBA(0.1, 0.1, 0.1, 1));
    vl_graphics_fill_rectangle(graphics, &yellow_brush, VL_RECT(100, 100, 400, 400));
    if (vl_graphics_end(graphics) == VL_GRAPHICS_SHOULD_TERMINATE) {
        vl_graphics_brush_free(&yellow_brush);
        vl_graphics_free(graphics);
        graphics = NULL;
    }
}

void resize(VlWindow window, int w, int h) {
    VL_UNUSED(window);
    vl_graphics_resize(graphics, w, h);
}

void window_test(void) {
    vl_graphics_initialize();
    printf("testing custom console!\n");
    VlWindow *win = vl_window_new("Velours windows are working!", 640, 480, 200, 200);
    graphics = vl_graphics_new(win);
    vl_graphics_brush_new_solid(graphics, &yellow_brush, VL_RGBA(1, 1, 0, 1));
    vl_window_set_paint_function(win, paint);
    vl_window_set_resize_function(win, resize);
    vl_window_set_visible(win, 1);
    vl_window_message_loop(win);
    vl_graphics_brush_free(&yellow_brush);
    vl_graphics_free(graphics);
    vl_window_free(win);

    vl_graphics_terminate();
}

void utf_test(void) {
    VlFile file;
    vl_file_new(&file, "utf8.txt", "rb");
    printf("bom: %i\n", file.bom);
    u32 cp;
    while ((cp = vl_file_read_codepoint(&file))) {
        u8 enc[4];
        int len = utf8_encode(cp, enc);
        printf("%.*s", len, enc);
    }
    printf("\n");
    vl_file_free(&file);
}

void stream_test(void) {
    VlFile f;
    vl_file_new(&f, "ascii.txt", "r");

    VlXML test;
    char error[512];
    VL_UNUSED(test);
    VlResult end_result = vl_xml_new_from_file(&test, &f, error);
    printf("memory usage after parsing: %zu\n", vl_get_memory_usage());
    if (end_result) {
        printf("end_result: %i;\nfailed to open test_xml!\n%s\n", end_result, error);
        return;
    }

    vl_xml_dump(&test, 0);

    vl_xml_free(&test);

    printf("cleanup memory usage: %zu\n", vl_get_memory_usage());
    vl_dump_all_allocations();
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
    file_test();
    // xml_test();
    // stream_test();
    // utf_test();
    // window_test();

	return VL_SUCCESS;
}