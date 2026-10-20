#include <gp4/gp4.h>

// Copies the 0x24 bytes at 0x007ab548 to 0x00822300 (`rep movsb`, direction flag clear). Leaves
// esi/edi just past both blocks and ecx = 0; the flags are not touched.
// FUNCTION: GP4 0x0052b91e
void __cdecl FUN_0052b91e(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x007ab548);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00822300);
    for (uint32_t i = 0; i != 0x24u; ++i) {
        dst[i] = src[i];
    }
    r->esi = 0x007ab548u + 0x24u;
    r->edi = 0x00822300u + 0x24u;
    r->ecx = 0;
}
GP4_IMPL(0x0052b91e, FUN_0052b91e, "regs() -> (esi, edi, ecx); globals=fuzz:i32")
