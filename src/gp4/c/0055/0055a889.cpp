#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mirror of the ring pass 0x0055a71c: a descending counter. For ([0x008705e8] + 0x100 + 1) track-segment
// records of 0x160 bytes, starting at the lap start [0x00891504] and stepping forward with the ring wrap
// (lap end [0x00866ecc], pit start [0x00891500], pit end [0x00866ef8], wrap distance [0x00866ec4]), a
// record whose byte +0x16 is 0x3c or more (and not below the running mark) raises the mark to that byte
// and remembers the record's byte +0x18; every record whose flag +0x1a bit 0 is clear and whose byte
// +0x20 is not above the mark gets +0x20 = mark and the low six bits of its word +0xc replaced by the
// remembered byte. The mark then counts down (floor 0). Records with +0x16 in 1..0x3b are skipped.
// pushal/popal restore every GPR; the flags are those the final call to 0x0055a71c leaves.
// FUNCTION: GP4 0x0055a889
void __cdecl FUN_0055a889(gp4::Regs* r) {
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00891504);
    uint16_t dx = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705e8) + 0x100u);
    uint16_t cx = 0;
    uint16_t bp = 0;
    uint8_t al;
    uint8_t bl;

    do {
        al = GP4_FIELD(uint8_t, edi, 0x16);
        bl = GP4_FIELD(uint8_t, edi, 0x18);
        if (al == 0u || al >= 0x3cu) {
            if (al != 0u && al >= static_cast<uint8_t>(cx)) {
                cx = static_cast<uint16_t>((cx & 0xff00u) | al);
                bp = bl;
            }
            if ((GP4_FIELD(uint8_t, edi, 0x1a) & 1u) == 0u && static_cast<uint8_t>(cx) >= GP4_FIELD(uint8_t, edi, 0x20)) {
                GP4_FIELD(uint8_t, edi, 0x20) = static_cast<uint8_t>(cx);
                GP4_FIELD(uint16_t, edi, 0x0c) = static_cast<uint16_t>((GP4_FIELD(uint16_t, edi, 0x0c) & 0xffc0u) | bp);
            }
            cx = static_cast<uint16_t>(cx - 1u);
            if (static_cast<int16_t>(cx) < 0) {
                cx = 0;
            }
        }
        edi += 0x160u;
        if (edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        dx = static_cast<uint16_t>(dx - 1u);
    } while (static_cast<int16_t>(dx) >= 0);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = (r->eax & 0xffffff00u) | al;
    c.ebx = (r->ebx & 0xffff0000u) | bl;
    c.ecx = (r->ecx & 0xffff0000u) | cx;
    c.edx = (r->edx & 0xffff0000u) | dx;
    c.ebp = (r->ebp & 0xffff0000u) | bp;
    c.edi = edi;
    gp4::call_regs(0x0055a71c, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0055a889, FUN_0055a889, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+walk_0055a71c")
