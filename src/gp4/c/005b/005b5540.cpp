#include <gp4/gp4.h>

// Static-object initialiser: runs the constructor thunk 0x005b5550, then registers the
// destructor 0x005b5570 with the CRT's atexit (0x005daa79) and returns its result.
// FUNCTION: GP4 0x005b5540
int __cdecl FUN_005b5540() {
    gp4::cdecl_<0x005b5550, int>();
    return gp4::cdecl_<0x005daa79, int>(0x005b5570);
}
GP4_IMPL(0x005b5540, FUN_005b5540, "cdecl() -> i32")
