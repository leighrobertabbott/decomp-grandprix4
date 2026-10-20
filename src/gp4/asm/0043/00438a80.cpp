#include <gp4/gp4.h>

// Expands a 23-bit mask: eax = mask | OR of dword[0x0062a100 + i*4] for every bit i (0..22) set in
// the original mask. All other registers are saved and restored; the last flag writer is
// cmp ecx, 0x17 with equal operands (ZF and PF set).
// FUNCTION: GP4 0x00438a80
void __cdecl FUN_00438a80(gp4::Regs* r) {
    const uint32_t mask = r->eax;
    uint32_t result = mask;
    for (uint32_t i = 0; i != 0x17u; ++i) {
        if (((1u << i) & mask) != 0u) {
            result |= GP4_ARRAY(uint32_t, 0x0062a100)[i];
        }
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00438a80, FUN_00438a80, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
