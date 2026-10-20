#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame-cell closer: eax = frame number. When the byte [0x0081cd90] is zero the routine just sets CF.
// Otherwise it stores eax in [0x0081cd58], validates the frame's handle cell with 0x00527eae (-1 means the
// cell is still live -> CF clear and return), then stc (0x00527d03). The `jb` that follows is therefore
// always taken, so the reload through 0x00527dee never runs; the flags are saved around the file-close
// helper 0x0053646a (pushfd/popfd).
// FUNCTION: GP4 0x005280ac
void __cdecl FUN_005280ac(gp4::Regs* r) {
    const uint32_t frame = r->eax;
    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x0081cd90);
    if (enabled == 0u) {
        const uint32_t f = gp4::flags::sub8(enabled, 0u);
        r->eflags = gp4::flags::merge(r->eflags, f) | 1u;
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0081cd58) = frame;
    gp4::Regs c = *r;
    gp4::call_regs(0x00527eae, c);
    const uint32_t cmpf = gp4::flags::sub32(c.eax, 0xffffffffu);
    if (c.eax == 0xffffffffu) {
        c.eflags = gp4::flags::merge(c.eflags, cmpf) & ~1u;
        *r = c;
        return;
    }

    c.eflags = gp4::flags::merge(c.eflags, cmpf);
    gp4::call_regs(0x00527d03, c);
    if ((c.eflags & 1u) == 0u) {
        c.eax = GP4_GLOBAL(uint32_t, 0x0081cd58);
        gp4::call_regs(0x00527dee, c);
    }
    const uint32_t saved = c.eflags;
    gp4::Regs closer = c;
    gp4::call_regs(0x0053646a, closer);
    c.eflags = saved;
    *r = c;
}
GP4_IMPL(0x005280ac, FUN_005280ac, "regs(eax:u32[0..807]) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae")
