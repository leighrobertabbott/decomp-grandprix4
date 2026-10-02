#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408742
void __cdecl FUN_00408742(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    body[0x7a] |= 0x80;
    body[0x7a] |= 0x20;
    GP4_GLOBAL(uint8_t, 0x0062f8f0) = 0xff;
    body[0xe5] |= 1;
    GP4_FIELD(uint16_t, body, 0xea) = 0x28;
    body[0xce] |= 0x10;
    body[0xcc] |= 0x10;
    body[0xfe] &= 0xfb;

    // pushal ... popal: every GPR is restored, only the flags left by the callees survive.
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x004086f9, callee);
    gp4::call_regs(0x0043b9ae, callee);

    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x00408742, FUN_00408742, "regs(esi:ptr[0x106]:bytes) -> (cf, pf, af, zf, sf, of)")
