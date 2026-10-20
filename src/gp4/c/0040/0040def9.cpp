#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040def9
// `pushal ; edi = [0x866ed0] ; call 0x40dde6 ; [0x62d2d4] = eax ; edi = [0x866ed4] ; call 0x40dde6 ;
// [0x62d2d8] = eax ; popal ; ret`: runs the nearest-record search (0x0040dde6) for the two records
// whose addresses sit in 0x866ed0 / 0x866ed4 and stores each answer. Every register comes back
// (pushal/popal); popal leaves the flags of the second search (the last flag writer).
void __cdecl FUN_0040def9(gp4::Regs* r) {
    const uint32_t first = GP4_GLOBAL(uint32_t, 0x00866ed0);
    const uint32_t second = GP4_GLOBAL(uint32_t, 0x00866ed4);

    gp4::Regs call = *r;
    call.edi = first;
    gp4::call_regs(0x0040dde6, call);
    GP4_GLOBAL(uint32_t, 0x0062d2d4) = call.eax;

    call.edi = second;
    gp4::call_regs(0x0040dde6, call);
    GP4_GLOBAL(uint32_t, 0x0062d2d8) = call.eax;

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040def9, FUN_0040def9, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040dde6+ends_0040def9")
