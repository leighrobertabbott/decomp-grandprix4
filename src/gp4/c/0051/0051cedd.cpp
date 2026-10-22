#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that runs the legacy per-level set-up passes in order: 0x00551b3d, then the 0x0051c6c3,
// 0x0051c690, 0x0051c770, 0x0051c8c0, 0x0051c8e9, 0x0051c746 and 0x0051ca84 passes, the 0x0051c91d pass only
// while the byte at 0x007ad869 is zero, and finally 0x0051ca38, 0x0051ccd6, 0x0051cac6, 0x0051cb69,
// 0x0051cc50, 0x0051cc96 and 0x0051cb4c. One register block is threaded through every call (the callees
// leave their registers live for the next one); popal restores every register, so only the flags of the last
// callee survive.
// FUNCTION: GP4 0x0051cedd
void __cdecl FUN_0051cedd(gp4::Regs* r) {
    gp4::Regs c = *r;

    gp4::call_regs(0x00551b3d, c);
    gp4::call_regs(0x0051c6c3, c);
    gp4::call_regs(0x0051c690, c);
    gp4::call_regs(0x0051c770, c);
    gp4::call_regs(0x0051c8c0, c);
    gp4::call_regs(0x0051c8e9, c);
    gp4::call_regs(0x0051c746, c);
    gp4::call_regs(0x0051ca84, c);

    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x007ad869);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(gate, 0u));
    if (gate == 0u) {
        gp4::call_regs(0x0051c91d, c);
    }

    gp4::call_regs(0x0051ca38, c);
    gp4::call_regs(0x0051ccd6, c);
    gp4::call_regs(0x0051cac6, c);
    gp4::call_regs(0x0051cb69, c);
    gp4::call_regs(0x0051cc50, c);
    gp4::call_regs(0x0051cc96, c);
    gp4::call_regs(0x0051cb4c, c);

    r->eflags = c.eflags;
}
GP4_IMPL(0x0051cedd, FUN_0051cedd, "regs(cf:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0051cac6+dw_00532d48+slots_0051cb69+tex_0051c6c3+ent_0051c91d+tex_0051c8e9+tex_0051c746+tex_0051c690+args_0051c865+slots_0051c894+slots_0051cc50+slots_0051cedd; maxinsn=16000000; wall=3600")
