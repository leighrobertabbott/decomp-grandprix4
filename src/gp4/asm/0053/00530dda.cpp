#include <gp4/gp4.h>

// Copies the 0x38 bytes at 0x00827425 to 0x007ad7f8 (`rep movsb`, forward). ecx/esi/edi are saved
// and restored; no flag is written.
// FUNCTION: GP4 0x00530dda
void __cdecl FUN_00530dda(gp4::Regs* r) {
    (void)r;
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x00827425);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007ad7f8);

    for (uint32_t i = 0; i != 0x38u; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x00530dda, FUN_00530dda, "regs() -> (); globals=layout=src_00530dda")
