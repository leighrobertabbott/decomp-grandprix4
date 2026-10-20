#include <gp4/gp4.h>

// FUNCTION: GP4 0x0052dda1
// Key-combination formatter. eax = key code with modifier bits (0x8000 = Control, 0x4000 = Shift)
// and edi = output cursor. For each modifier bit set it writes that modifier's key name through
// 0x0052dde1 (virtual key 0x11 / 0x10) followed by '+', then writes the name of the remaining key
// code. edi comes back advanced; eax is restored (pushed and popped); the flags are those the
// final 0x0052dde1 call leaves behind.
void __cdecl FUN_0052dda1(gp4::Regs* r) {
    const uint32_t original = r->eax;
    uint32_t code = original;
    gp4::Regs c = *r;

    if ((code & 0x8000u) != 0u) {
        code &= 0xffff7fffu;
        c.eax = 0x11u;
        gp4::call_regs(0x0052dde1, c);
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = 0x2bu;
        c.edi += 1u;
    }

    if ((code & 0x4000u) != 0u) {
        code &= 0xffffbfffu;
        c.eax = 0x10u;
        gp4::call_regs(0x0052dde1, c);
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = 0x2bu;
        c.edi += 1u;
    }

    c.eax = code;
    gp4::call_regs(0x0052dde1, c);

    *r = c;
    r->eax = original;
}
GP4_IMPL(0x0052dda1, FUN_0052dda1, "regs(eax:u32, edi:ptr[0x100]) -> (edi, cf, pf, af, zf, sf, of)")
