#include <gp4/gp4.h>
#include <gp4/flags.h>

// Opens the message box record esi: remembers it in [0x0083e6a2], loads its corner words (ebx = x0 at +4, eax = y0
// at +6, ebp = x1 at +8, ecx = y1 at +0xa, zero-extended) and queues the box in the sorted list 0x00549db9.
// The empty routine 0x00531b3c runs, then 0x00531b2c (empty) inside pushal/popal with the corners moved 0xf
// inwards (the adjustment is discarded by popal). The corners are then moved 0x10 inwards and the box is drawn
// by 0x0054fee0, whose registers and flags are this routine's result.
// FUNCTION: GP4 0x0054fe9b
void __cdecl FUN_0054fe9b(gp4::Regs* r) {
    const uint32_t box = r->esi;
    GP4_GLOBAL(uint32_t, 0x0083e6a2) = box;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint16_t, box, 4);
    c.eax = GP4_FIELD(uint16_t, box, 6);
    c.ebp = GP4_FIELD(uint16_t, box, 8);
    c.ecx = GP4_FIELD(uint16_t, box, 0x0a);
    gp4::call_regs(0x00549db9, c);
    gp4::call_regs(0x00531b3c, c);

    gp4::Regs inner = c;
    inner.ebx = c.ebx + 0xfu;
    inner.eax = c.eax + 0xfu;
    inner.ebp = c.ebp - 0xfu;
    inner.ecx = c.ecx - 0xfu;
    inner.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 0xfu));
    gp4::call_regs(0x00531b2c, inner);
    c.eflags = inner.eflags;

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 0x10u));
    c.ebx = c.ebx + 0x10u;
    c.eax = c.eax + 0x10u;
    c.ebp = c.ebp - 0x10u;
    c.ecx = c.ecx - 0x10u;
    gp4::call_regs(0x0054fee0, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054fe9b, FUN_0054fe9b, "regs(esi:ptr[0x10]:layout=tboxrec_0054fee0, edx:u32) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+script_0054d798+log_004fd730+textout_0054d211+tbox2_0054fee0")
