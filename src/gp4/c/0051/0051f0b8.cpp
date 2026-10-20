#include <gp4/gp4.h>

// "mov ax, 0x4c; call 0x00531f9c; jb -> clc; else stc; ret": runs dispatcher command 0x4c (handler
// 0x532d85: CF = 1 unless the link-session byte [0x7aafd3] is 0xff) and returns the inverted carry
// (CF = 1 when the handler cleared CF). The upper half of eax is untouched; eax and the other flags
// are the dispatcher's.
// FUNCTION: GP4 0x0051f0b8
void __cdecl FUN_0051f0b8(gp4::Regs* r) {
    r->eax = (r->eax & 0xffff0000u) | 0x4cu;
    gp4::call_regs(0x00531f9c, *r);
    const uint32_t carry = (r->eflags & 1u) != 0u ? 0u : 1u;
    r->eflags = (r->eflags & ~1u) | carry;
}
GP4_IMPL(0x0051f0b8, FUN_0051f0b8, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=recv_00445423")
