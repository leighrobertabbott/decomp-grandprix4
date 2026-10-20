#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051ef77
// Returns the address of the data block at 0x009e14dc.
uint8_t* __cdecl FUN_0051ef77() {
    return GP4_ARRAY(uint8_t, 0x009e14dc);
}
GP4_IMPL(0x0051ef77, FUN_0051ef77, "cdecl() -> u32")
