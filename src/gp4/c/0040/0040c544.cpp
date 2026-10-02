#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c544
int __cdecl FUN_0040c544() {
    int value = static_cast<int>(GP4_GLOBAL(uint32_t, 0x0063b838) >> 8);
    if (value > 0xff) {
        value = 0xff;
    }
    return value;
}
GP4_IMPL(0x0040c544, FUN_0040c544, "cdecl() -> i32; globals=fuzz:i32")
