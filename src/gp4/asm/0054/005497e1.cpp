#include <gp4/gp4.h>

// Copies the 6 bytes at 0x00831d0d over the 6 bytes at 0x00831cfd (`rep movsb`, direction flag
// clear). ecx/esi/edi are saved and restored; the flags are untouched.
// FUNCTION: GP4 0x005497e1
void __cdecl FUN_005497e1(gp4::Regs* /*r*/) {
    const uint8_t* src = &GP4_GLOBAL(uint8_t, 0x00831d0d);
    uint8_t* dst = &GP4_GLOBAL(uint8_t, 0x00831cfd);
    for (int i = 0; i < 6; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x005497e1, FUN_005497e1, "regs() -> (); globals=fuzz:i32+layout=src_00831d07")
