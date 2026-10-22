#include <gp4/gp4.h>

// Static-object initialiser: resets the object at 0x00a52030 (FUN_004ac380 clears +0x18)
// and registers its destructor stub 0x00493940 with the CRT's atexit (0x005daa79).
// FUNCTION: GP4 0x00493920
int __cdecl FUN_00493920() {
    gp4::thiscall_<0x004ac380, int>((void*)0x00a52030);
    return gp4::cdecl_<0x005daa79, int>(0x00493940);
}
GP4_IMPL(0x00493920, FUN_00493920, "cdecl() -> i32")
