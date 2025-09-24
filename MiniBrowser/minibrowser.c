#include "velours.h"
#include "da.h"
#include "xml/xml.h"
#include "utf8.h"
#include <windows.h>

#include <stdio.h>

const char *test_xml = "    " VL_STRINGIFY_VARIADIC(
<abc property1="Hello, Мир! Пока, мир" property2="4.512 + три целых четырнадцать сотых"></abc>
) "    ";

void da_test(void) {
	int *test;
	VL_DA_ALLOC(test, int);
	printf("%p\n", test);

	for (int i = 0; i < 10000; i++) {
		VL_DA_APPEND_CONST(test, int, i);
		VL_DA_DUMP_HEADER(test);
	}
}

int main(void) {
	SetConsoleOutputCP(CP_UTF8);
	// da_test();

	VlXML test;
	VL_UNUSED(test);
	if (vl_xml_new(&test, test_xml)) {
		printf("failed to open test_xml!");
		return VL_ERROR;
	}

	return VL_SUCCESS;
}