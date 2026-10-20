#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005280e2
// Loads `ecx` consecutive frames starting at frame `eax`: stores them in 0x81cd58 / 0x81cd5c, then
// calls 0x00527d03 (`stc; ret`) and leaves at once when CF is set (`jb`), else loops
// { 0x00527dee(frame); frame++; count-- } until the count is zero. As 0x00527d03 always sets CF,
// the loop (0x5280f4..0x52810a) is never reached. Then the file is closed by 0x0053646a between
// `pushfd` and `popfd`: the registers and flags are those before that call (CF set, the other
// arithmetic flags as on entry).
void __cdecl FUN_005280e2(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0081cd58) = r->eax;
    GP4_GLOBAL(uint32_t, 0x0081cd5c) = r->ecx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00527d03, c);

    if ((c.eflags & gp4::flags::CF) == 0u) {
        uint32_t arith;
        do {
            c.eax = GP4_GLOBAL(uint32_t, 0x0081cd58);
            gp4::call_regs(0x00527dee, c);
            const uint32_t frame = GP4_GLOBAL(uint32_t, 0x0081cd58);
            GP4_GLOBAL(uint32_t, 0x0081cd58) = frame + 1u;
            const uint32_t afterInc = gp4::flags::inc32(frame, c.eflags);
            const uint32_t left = GP4_GLOBAL(uint32_t, 0x0081cd5c);
            GP4_GLOBAL(uint32_t, 0x0081cd5c) = left - 1u;
            arith = gp4::flags::dec32(left, afterInc);
            c.eflags = gp4::flags::merge(c.eflags, arith);
        } while ((arith & gp4::flags::ZF) == 0u);
    }

    // pushfd; call 0x0053646a; popfd - the callee keeps every GPR and the popfd restores the flags.
    gp4::Regs closing = c;
    gp4::call_regs(0x0053646a, closing);

    *r = c;
}
GP4_IMPL(0x005280e2, FUN_005280e2, "regs(eax:u32, ecx:u32) -> (cf, pf, af, zf, sf, of)")
