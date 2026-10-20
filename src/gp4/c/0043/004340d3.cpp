#include <gp4/gp4.h>

// FUNCTION: GP4 0x004340d3
void __cdecl FUN_004340d3(gp4::Regs* r) {
    uint32_t* table = GP4_ARRAY(uint32_t, 0x0062fc18);
    for (int i = 0; i < 2; ++i) {
        table[2 * i] = 0;
        table[2 * i + 1] = 0;
    }
    // edi ends one past the cleared 16 bytes (0x62fc28) and the loop consumed ecx;
    // the last add edi, 8 (0x62fc20 + 8) leaves only PF set.
    r->edi = 0x0062fc28u;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x4u;
}
GP4_IMPL(0x004340d3, FUN_004340d3, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of)")
