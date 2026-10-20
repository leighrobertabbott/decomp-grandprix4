#include <gp4/gp4.h>

// Copies the 0x24 bytes at 0x00822300 back to 0x007ab56c (`rep movsb`, direction flag clear). Leaves
// esi/edi just past both blocks and ecx = 0; the flags are not touched.
// FUNCTION: GP4 0x0052b930
void __cdecl FUN_0052b930(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x00822300);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007ab56c);
    for (uint32_t i = 0; i != 0x24u; ++i) {
        dst[i] = src[i];
    }
    r->esi = 0x00822300u + 0x24u;
    r->edi = 0x007ab56cu + 0x24u;
    r->ecx = 0;
}
GP4_IMPL(0x0052b930, FUN_0052b930, "regs() -> (esi, edi, ecx); globals=fuzz:i32")
