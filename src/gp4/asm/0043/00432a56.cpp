#include <gp4/gp4.h>

// FUNCTION: GP4 0x00432a56
void __cdecl FUN_00432a56(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);

    const uint32_t elapsed = now - GP4_FIELD(uint32_t, body, 0x274);
    GP4_FIELD(uint32_t, body, 0x274) = elapsed;
    GP4_FIELD(uint32_t, body, 0x1fc) = now + 0x7d0u;

    // final flags come from test byte [esi+0x7b], 2 (CF=OF=AF=SF=0)
    const uint32_t masked = GP4_FIELD(uint8_t, body, 0x7b) & 2u;
    if (masked == 0u) {
        GP4_FIELD(uint8_t, body, 0x237) = 1;
    }

    r->eax = elapsed;
    r->eflags = (r->eflags & ~0x8d5u) | (masked == 0u ? 0x44u : 0u);
}
GP4_IMPL(0x00432a56, FUN_00432a56, "regs(esi:ptr[0x284]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
