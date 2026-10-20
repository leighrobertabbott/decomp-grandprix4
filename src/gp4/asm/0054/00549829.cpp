#include <gp4/gp4.h>

// Resets the 6-byte record at 0x00831cfd to 0xff (`rep stosb`, direction flag clear) and clears
// the dword at 0x00831d03 that follows it. eax/ecx/edi are pushed and restored; the flags are
// untouched.
// FUNCTION: GP4 0x00549829
void __cdecl FUN_00549829(gp4::Regs* /*r*/) {
    uint8_t* slots = &GP4_GLOBAL(uint8_t, 0x00831cfd);
    for (int i = 0; i < 6; ++i) {
        slots[i] = 0xffu;
    }
    GP4_GLOBAL(uint32_t, 0x00831d03) = 0u;
}
GP4_IMPL(0x00549829, FUN_00549829, "regs() -> (); globals=fuzz:i32")
