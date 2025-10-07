#ifndef VELOURS_DOM_H
#define VELOURS_DOM_H

#include "velours.h"
#include "memory.h"

typedef struct {

} VlDOM;

VL_API VlDOM *vl_dom_new();

VL_API VlResult vl_dom_free(VlDOM *dom);

#endif // VELOURS_DOM_H