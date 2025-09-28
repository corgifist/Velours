/*
	file.h - different file utilities
	provides ways to read, write, get size of files
*/
#ifndef VELOURS_FILE_H
#define VELOURS_FILE_H

#include "velours.h"
#include "da.h"

VL_API VlResult vl_file_read(const char *path, VL_DA(char) *output);

#endif // VELOURS_FILE_H