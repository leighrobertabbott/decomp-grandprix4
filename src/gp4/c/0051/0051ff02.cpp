#include <gp4/gp4.h>

// Copies twelve dwords of the 8-byte-stride table at 0x007ab02c: the entry at 0x007ab034 + k * 0x10 goes to
// 0x007ab02c + k * 0x10 (k = 0..11), i.e. each 16-byte pair's second dword moves into its first. No
// register or flag is touched (push/pop of memory operands).
// FUNCTION: GP4 0x0051ff02
void __cdecl FUN_0051ff02() {
    GP4_GLOBAL(uint32_t, 0x007ab02c) = GP4_GLOBAL(uint32_t, 0x007ab034);
    GP4_GLOBAL(uint32_t, 0x007ab03c) = GP4_GLOBAL(uint32_t, 0x007ab044);
    GP4_GLOBAL(uint32_t, 0x007ab04c) = GP4_GLOBAL(uint32_t, 0x007ab054);
    GP4_GLOBAL(uint32_t, 0x007ab05c) = GP4_GLOBAL(uint32_t, 0x007ab064);
    GP4_GLOBAL(uint32_t, 0x007ab06c) = GP4_GLOBAL(uint32_t, 0x007ab074);
    GP4_GLOBAL(uint32_t, 0x007ab07c) = GP4_GLOBAL(uint32_t, 0x007ab084);
    GP4_GLOBAL(uint32_t, 0x007ab08c) = GP4_GLOBAL(uint32_t, 0x007ab094);
    GP4_GLOBAL(uint32_t, 0x007ab09c) = GP4_GLOBAL(uint32_t, 0x007ab0a4);
    GP4_GLOBAL(uint32_t, 0x007ab0ac) = GP4_GLOBAL(uint32_t, 0x007ab0b4);
    GP4_GLOBAL(uint32_t, 0x007ab0bc) = GP4_GLOBAL(uint32_t, 0x007ab0c4);
    GP4_GLOBAL(uint32_t, 0x007ab0cc) = GP4_GLOBAL(uint32_t, 0x007ab0d4);
    GP4_GLOBAL(uint32_t, 0x007ab0dc) = GP4_GLOBAL(uint32_t, 0x007ab0e4);
}
GP4_IMPL(0x0051ff02, FUN_0051ff02, "cdecl() -> void; globals=fuzz:i32")
