#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the NUL-terminated string at esi one character at a time, at most ecx characters: each byte goes to the
// text output 0x0054d3a1 (al = the character), then 0x0054cc1e reloads ebx = text x and eax = text y from
// 0x00833cc5 / 0x00833cc9 and ebx advances by 0xf. When ebx reaches the width limit [0x00826e36] (unsigned) the
// loop stops after printing the overflow marker, text id 0x24e, through 0x0054d1a2 (esi saved and restored).
// A NUL terminator, or ecx running out, also ends it. Registers and flags thread through the callees; the flags
// are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x0052f1c5
void __cdecl FUN_0052f1c5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t cursor = r->esi;
    uint32_t remaining = r->ecx;
    for (;;) {
        const uint32_t ch = GP4_FIELD(uint8_t, cursor, 0);
        cursor = cursor + 1u;
        c.eax = (c.eax & 0xffffff00u) | ch;
        c.esi = cursor;
        c.ecx = remaining;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(ch));
        if (ch == 0u) {
            break;
        }

        gp4::call_regs(0x0054d3a1, c);
        gp4::call_regs(0x0054cc1e, c);

        const uint32_t advanced = c.ebx + 0xfu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.ebx, 0xfu));
        c.ebx = advanced;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00826e36);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(advanced, limit));
        if (advanced >= limit) {
            c.esi = 0x0000024eu;
            gp4::call_regs(0x0054d1a2, c);
            c.esi = cursor;
            break;
        }

        remaining = remaining - 1u;
        c.ecx = remaining;
        if (remaining == 0u) {
            break;
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0052f1c5, FUN_0052f1c5, "regs(eax:u32, ecx:u32[1..0x40], esi:ptr[0x40]:u8{0,1,2,0x20,0x41,0x7f,0x80,0xff}) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730")
