#include <gp4/gp4.h>

// FUNCTION: GP4 0x004340f0
void __cdecl FUN_004340f0(gp4::Regs* r) {
    uint32_t* table = GP4_ARRAY(uint32_t, 0x0063b94c);
    for (int i = 0; i < 2; ++i) {
        table[2 * i] = 0;
        table[2 * i + 1] = 0;
    }
    // edi ends one past the cleared 16 bytes (0x63b95c) and the loop consumed ecx;
    // the last add edi, 8 (0x63b954 + 8) leaves only PF set.
    r->edi = 0x0063b95cu;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x4u;
}
GP4_IMPL(0x004340f0, FUN_004340f0, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of)")
