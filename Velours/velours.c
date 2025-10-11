#include "velours.h"
#include "memory.h"

VL_API u8 *vl_format(const u8 *format, ...) {
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);
    int size = vsnprintf(NULL, 0, format, args1);
    // printf("%i size\n", size);
    u8* mem = VL_MALLOC(sizeof(u8) * size + 1);
    vsnprintf(mem, size + 1, format, args2);
    va_end(args1);
    va_end(args2);
    return mem;
}