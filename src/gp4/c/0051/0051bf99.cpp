#include <gp4/gp4.h>

// Index of the first of the 11 per-driver flag bytes at 0x007c2a48 that has bit 7 set and bit 6 clear
// (11 when there is none), halved (sar 1). Callers only use eax.
// FUNCTION: GP4 0x0051bf99
int __cdecl FUN_0051bf99() {
    const uint8_t* driverFlags = GP4_ARRAY(uint8_t, 0x007c2a48);
    int index = 0;
    do {
        if ((driverFlags[index] & 0x40u) == 0u && (driverFlags[index] & 0x80u) != 0u) {
            break;
        }
        ++index;
    } while (index != 0xb);
    return index >> 1;
}
GP4_IMPL(0x0051bf99, FUN_0051bf99, "cdecl() -> i32; globals=layout=drv22_0052d402")
