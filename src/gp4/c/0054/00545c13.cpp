#include <gp4/gp4.h>
#include <gp4/flags.h>

// Date text for edx = (dh = month, dl = day) and ecx = year, written at the output cursor edi. When the
// byte [0x82974e] is not 0xff: text id dh + 0x21, a space, the day number dl (0x0054642c, no padding), text id
// dl + 0x6e, a space and the year padded to 4 characters with spaces. When it is 0xff: the numeric form
// dh, the separator byte [0x829750], dl, the separator, ecx. Registers and flags are those the last
// formatter call leaves.
// FUNCTION: GP4 0x00545c13
void __cdecl FUN_00545c13(gp4::Regs* r) {
    const uint32_t entry_ecx = r->ecx;
    const uint32_t entry_edx = r->edx;
    gp4::Regs c = *r;
    const uint8_t style = GP4_GLOBAL(uint8_t, 0x0082974e);
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(style, 0xffu));
    if (style != 0xffu) {
        const uint32_t month = (entry_edx >> 8) & 0xffu;
        c.esi = month + 0x21u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(month, 0x21u));
        gp4::call_regs(0x0054d798, c);
        GP4_FIELD(uint8_t, c.edi, 0) = 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi = c.edi + 1u;
        c.edx = entry_edx;
        c.eax = entry_edx & 0xffu;
        c.ebx = c.ebx & 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        c.esi = c.eax + 0x6eu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.eax, 0x6eu));
        gp4::call_regs(0x0054d798, c);
        GP4_FIELD(uint8_t, c.edi, 0) = 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi = c.edi + 1u;
        c.ecx = entry_ecx;
        c.eax = c.ecx;
        c.ebx = (c.ebx & 0xffff0000u) | 0x4020u;
        c.ecx = (c.ecx & 0xffffff00u) | 4u;
        gp4::call_regs(0x0054642c, c);
    } else {
        c.eax = (c.edx >> 8) & 0xffu;
        c.ebx = c.ebx & 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
        GP4_FIELD(uint8_t, c.edi, 0) = static_cast<uint8_t>(c.eax);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi = c.edi + 1u;
        c.eax = c.edx & 0xffu;
        c.ebx = c.ebx & 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
        GP4_FIELD(uint8_t, c.edi, 0) = static_cast<uint8_t>(c.eax);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
        c.edi = c.edi + 1u;
        c.eax = c.ecx;
        c.ebx = c.ebx & 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
    }
    *r = c;
}
GP4_IMPL(0x00545c13, FUN_00545c13, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32{=0x0001,0x0102,0x0305,0x050f,0x0b1f,0x0c0a,0x0014,0x011f,0x0210,0x0900,0x0c00}, ebp:u32, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846")
