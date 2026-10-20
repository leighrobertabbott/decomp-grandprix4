#include <gp4/gp4.h>

// Picks the rotated point of the body at edi: when bit 7 of (al ^ byte 0x00868582) is set it takes
// 0x40ce7b then (ecx = its eax) 0x40cf40, otherwise 0x40ceb0 then (ecx = its eax) 0x40cf52.
// eax / edx / flags are the second call's results; ecx = the first call's eax.
// FUNCTION: GP4 0x0040d17c
void __cdecl FUN_0040d17c(gp4::Regs* r) {
    const uint32_t selector = (r->eax ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0xffu;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = (r->eax & ~0xffu) | selector;
    if ((selector & 0x80u) != 0u) {
        gp4::call_regs(0x0040ce7b, callee);
        callee.ecx = callee.eax;
        gp4::call_regs(0x0040cf40, callee);
    } else {
        gp4::call_regs(0x0040ceb0, callee);
        callee.ecx = callee.eax;
        gp4::call_regs(0x0040cf52, callee);
    }

    r->eax = callee.eax;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040d17c, FUN_0040d17c, "regs(eax:u32, edi:ptr[0x158]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")
