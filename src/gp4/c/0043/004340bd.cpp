#include <gp4/gp4.h>

// FUNCTION: GP4 0x004340bd
void __cdecl FUN_004340bd(gp4::Regs* r) {
    uint32_t* table = GP4_ARRAY(uint32_t, 0x0062ff6c);
    for (int i = 0; i < 16; ++i) {
        table[i] = 0;
    }
    // edi ends one past the table (0x62ffac) and ecx is consumed by the loop;
    // the last add edi, 4 (0x62ffa8 + 4) leaves only PF set.
    r->edi = 0x0062ffacu;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x4u;
}
GP4_IMPL(0x004340bd, FUN_004340bd, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of)")
