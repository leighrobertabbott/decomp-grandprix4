#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x00545cf7 that prints the numbers in the opposite order: dl first (script dl + 0x6e)
// and dh (script dh + 0x21), then ecx with width 4 padded with blanks (0x54642c, bh = 0x40,
// bl = 0x20, cl = 4). When the mode byte 0x0082974e is 0xff the three numbers dl, dh, ecx are printed
// bare, the first two followed by the separator character [0x00829750]. Registers and flags come
// back as the last callee leaves them (edi is the output cursor).
// FUNCTION: GP4 0x00545c84
void __cdecl FUN_00545c84(gp4::Regs* r) {
    const uint32_t saved_ecx = r->ecx;
    const uint32_t saved_edx = r->edx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if (GP4_GLOBAL(uint8_t, 0x0082974e) == 0xffu) {
        c.eax = c.edx & 0xffu;
        c.ebx &= 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, 0x00829750);
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi += 1u;

        c.eax = (c.edx >> 8) & 0xffu;
        c.ebx &= 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, 0x00829750);
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi += 1u;

        c.eax = c.ecx;
        c.ebx &= 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
    } else {
        c.eax = c.edx & 0xffu;
        c.ebx &= 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        c.esi = c.eax + 0x6eu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.eax, 0x6eu));
        gp4::call_regs(0x0054d798, c);
        GP4_GLOBAL(uint8_t, c.edi) = 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi += 1u;

        c.edx = saved_edx;
        c.esi = ((c.edx >> 8) & 0xffu) + 0x21u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32((c.edx >> 8) & 0xffu, 0x21u));
        gp4::call_regs(0x0054d798, c);
        GP4_GLOBAL(uint8_t, c.edi) = 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi += 1u;

        c.ecx = saved_ecx;
        c.eax = c.ecx;
        c.ebx = (c.ebx & 0xffff0000u) | 0x4020u;
        c.ecx = (c.ecx & 0xffffff00u) | 4u;
        gp4::call_regs(0x0054642c, c);
    }
    *r = c;
}
GP4_IMPL(0x00545c84, FUN_00545c84, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, edi:ptr[0x200]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_00545c84")
