#include <gp4/gp4.h>

// Static-object initialiser: resets the object at 0x00a64f38 (FUN_004ac380 clears +0x18)
// and registers its destructor stub 0x004ac370 with the CRT's atexit (0x005daa79).
// FUNCTION: GP4 0x004ac350
int __cdecl FUN_004ac350() {
    gp4::thiscall_<0x004ac380, int>((void*)0x00a64f38);
    return gp4::cdecl_<0x005daa79, int>(0x004ac370);
}
GP4_IMPL(0x004ac350, FUN_004ac350, "cdecl() -> i32")
