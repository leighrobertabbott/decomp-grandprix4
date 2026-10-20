#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042783c
// `pushal ; call 0x004275d1 ; mov [esi+0x142], ax ; popal ; ret`: stores the word computed by
// 0x004275d1 at +0x142 of the esi record. Every register is restored; the flags are those the
// callee's final 16-bit add left (the store and popal do not touch them).
void __cdecl FUN_0042783c(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::call_regs(0x004275d1, *r);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(saved.esi));
    *reinterpret_cast<uint16_t*>(record + 0x142) = static_cast<uint16_t>(r->eax);
    saved.eflags = (saved.eflags & ~0x8d5u) | (r->eflags & 0x8d5u);
    *r = saved;
}
GP4_IMPL(0x0042783c, FUN_0042783c, "regs(edi:ptr[0x1d0]:bytes, esi:ptr[0x41e]:layout=rec_004030c6) -> (cf, pf, af, zf, sf, of)")
