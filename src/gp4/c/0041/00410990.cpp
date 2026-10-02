#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410990
void __cdecl FUN_00410990(gp4::Regs* r) {
    // pushal ... popal restores every register, but not the flags left by 0x410901
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    call.edx = GP4_GLOBAL(uint32_t, 0x007ad894);
    call.eax = (r->eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x007ad8a2);
    gp4::call_regs(0x00410901, call);
    GP4_GLOBAL(uint16_t, 0x007c2e96) = static_cast<uint16_t>(call.eax);
    saved.eflags = call.eflags;
    *r = saved;
}
GP4_IMPL(0x00410990, FUN_00410990, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
