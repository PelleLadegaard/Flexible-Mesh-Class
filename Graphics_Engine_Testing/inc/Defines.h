#pragma once

// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

typedef unsigned int uint;