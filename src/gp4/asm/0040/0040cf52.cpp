#include <gp4/gp4.h>

struct Unknown_0040cf52 {
    uint8_t Unknown00[0x62];
    uint16_t Unknown62;
};

// Calls the rotation helper 0x40cee7 with the length (word at edi+0x62 + 0x50) << 8 in eax.
// eax / edx / flags are the helper's results.
// FUNCTION: GP4 0x0040cf52
void __cdecl FUN_0040cf52(gp4::Regs* r) {
    const Unknown_0040cf52* body = reinterpret_cast<const Unknown_0040cf52*>(static_cast<uintptr_t>(r->edi));
    uint32_t length = body->Unknown62;
    length += 0x50u;
    length <<= 8;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = length;
    gp4::call_regs(0x0040cee7, callee);

    r->eax = callee.eax;
    r->edx = callee.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040cf52, FUN_0040cf52, "regs(edi:ptr[0x100]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")
