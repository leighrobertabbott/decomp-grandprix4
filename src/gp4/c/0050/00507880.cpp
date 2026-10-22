#include <gp4/gp4.h>

// Static-object initialiser: runs FUN_0050a0a0 on the object at 0x00b1b378 and registers its
// destructor stub 0x005078a0 with the CRT's atexit (0x005daa79).
// FUNCTION: GP4 0x00507880
int __cdecl FUN_00507880() {
    gp4::thiscall_<0x0050a0a0, int>((void*)0x00b1b378);
    return gp4::cdecl_<0x005daa79, int>(0x005078a0);
}
GP4_IMPL(0x00507880, FUN_00507880, "cdecl() -> i32")
