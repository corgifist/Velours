#include "velours.h"
#include "da.h"
#include "xml/xml.h"
#include "utf8.h"
#include <windows.h>

#include <stdio.h>

const char* test_xml = VL_STRINGIFY_VARIADIC(
<?xml version="1.0"?>
<Контент></Контент>
);

void da_test(void) {
	int *test;
	VL_DA_ALLOC(test, int);
	printf("%p\n", test);

	VL_DA_APPEND_CONST(test, int, 34);
	VL_DA_APPEND_CONST(test, int, 35);
	VL_DA_APPEND_CONST(test, int, 69);
	VL_DA_APPEND_CONST(test, int, 84);
	VL_DA_APPEND_CONST(test, int, 67);
	VL_DA_APPEND_CONST(test, int, 666);
	VL_DA_APPEND_CONST(test, int, 782);

	VL_DA_FOREACH(test, i) {
		printf("%zu: %i\n", i, test[i]);
	}

	VL_DA_DUMP_HEADER(test);

	printf("=====================\n");

	while (VL_DA_LENGTH(test) != 0) {
		VL_DA_DELETE(test, 0);
		VL_DA_DUMP_HEADER(test);
	}
	   
	/* while (VL_DA_HEADER(test)->count != 0) {
		VL_DA_DELETE(test, 0);
		VL_DA_DUMP_HEADER(test);
	} */
}

int main(void) {
	SetConsoleOutputCP(CP_UTF8);
	// da_test();

	VlXML test;
	if (vl_xml_open(&test, test_xml) != VL_SUCCESS) {
		printf("failed to open test_xml!");
		return VL_ERROR;
	}

	const char* emoji = "🤤";
	printf("original codepoint: %s\n", emoji);
	int len;
	uint32_t codepoint = utf8_decode(&emoji, &len);
	utf8_dump_codepoint(codepoint);
	printf("\n");
	char rawEmoji[4];
	len = utf8_encode(codepoint, (unsigned char*) rawEmoji);
	printf("reencoded codepoint: %.*s\n", len, rawEmoji);

	return VL_SUCCESS;
}