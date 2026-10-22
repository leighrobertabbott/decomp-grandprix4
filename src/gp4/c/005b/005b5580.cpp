#include <gp4/gp4.h>

// Static-object initialiser: runs the constructor thunk 0x005b5590, then registers the
// destructor 0x005b55b0 with the CRT's atexit (0x005daa79) and returns its result.
// FUNCTION: GP4 0x005b5580
int __cdecl FUN_005b5580() {
    gp4::cdecl_<0x005b5590, int>();
    return gp4::cdecl_<0x005daa79, int>(0x005b55b0);
}
GP4_IMPL(0x005b5580, FUN_005b5580, "cdecl() -> i32")
