#include <gp4/gp4.h>

// Number of 0x160-byte records between the pointer stored at 0x00891500 and the one at
// 0x00866ed0, counting both ends: (0x00866ed0 - 0x00891500) / 0x160 + 1 (unsigned division, edx = 0).
// FUNCTION: GP4 0x0050bd7b
int32_t __cdecl FUN_0050bd7b() {
    uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ed0);
    uint32_t begin = GP4_GLOBAL(uint32_t, 0x00891500);
    return (int32_t)((end - begin) / 0x160u + 1u);
}
GP4_IMPL(0x0050bd7b, FUN_0050bd7b, "cdecl() -> i32; globals=fuzz:i32")
