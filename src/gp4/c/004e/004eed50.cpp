#include <gp4/gp4.h>

// Static-initialiser: sets bit 0 of the guard byte 0x00b47ad8 and registers the destructor stub
// 0x0046c270 with the CRT's atexit (0x005daa79).
// FUNCTION: GP4 0x004eed50
int __cdecl FUN_004eed50() {
    uint8_t guard = GP4_GLOBAL(uint8_t, 0x00b47ad8);
    if ((1 & guard) == 0) {
        GP4_GLOBAL(uint8_t, 0x00b47ad8) = guard | 1;
    }
    return gp4::cdecl_<0x005daa79, int>(0x0046c270);
}
GP4_IMPL(0x004eed50, FUN_004eed50,
         "cdecl() -> i32; gfields={0x00b47ad8:u8{=0,1}, 0x00b47e70:ptr[0x100]:bytes, 0x00b47e74:ptr[0x100]:bytes}")
