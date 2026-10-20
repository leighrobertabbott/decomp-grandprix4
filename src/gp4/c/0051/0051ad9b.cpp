#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051ad9b
// Returns the address of the "english" string at 0x008339b4.
const char* __cdecl FUN_0051ad9b() {
    return GP4_ARRAY(const char, 0x008339b4);
}
GP4_IMPL(0x0051ad9b, FUN_0051ad9b, "cdecl() -> u32")
