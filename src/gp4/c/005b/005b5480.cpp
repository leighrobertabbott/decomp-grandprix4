#include <gp4/gp4.h>

// Static-object initialiser: runs the constructor thunk 0x005b5490, then registers the
// destructor 0x005b54b0 with the CRT's atexit (0x005daa79) and returns its result.
// FUNCTION: GP4 0x005b5480
int __cdecl FUN_005b5480() {
    gp4::cdecl_<0x005b5490, int>();
    return gp4::cdecl_<0x005daa79, int>(0x005b54b0);
}
GP4_IMPL(0x005b5480, FUN_005b5480, "cdecl() -> i32")
