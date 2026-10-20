#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: clears the four source pointers 0x6f2c0c/10/14/18, then for each of the
// four control codes 0x7ab54a/0x7ab550/0x7ab553/0x7ab556 looks up its record through 0x0051da29
// (bl = code, bh = 0; the third uses bh = 0xff when its code equals the second's). On a hit the
// record address goes to the matching source pointer and, unless byte 0x7ab0f1 is 0xff, the
// looked-up dword pair (eax, edx) is latched into 0x7ab134/0x7ab13c, 0x7ab140/44, 0x7ab148/4c,
// 0x7ab150/54 (the first also latches the bh = 0xff value at 0x7ab138). All GPRs are restored;
// the flags are those of the last lookup or of the last `cmp byte [0x7ab0f1], 0xff`.
// FUNCTION: GP4 0x0051dbf1
void __cdecl FUN_0051dbf1(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t flags;

    c.eax = 0;
    GP4_GLOBAL(uint32_t, 0x006f2c0c) = 0;
    GP4_GLOBAL(uint32_t, 0x006f2c10) = 0;
    GP4_GLOBAL(uint32_t, 0x006f2c14) = 0;
    GP4_GLOBAL(uint32_t, 0x006f2c18) = 0;

    // first code
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint8_t, 0x007ab54a);
    gp4::call_regs(0x0051da29, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint32_t, 0x006f2c0c) = c.ecx;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0xffu) {
            GP4_GLOBAL(uint32_t, 0x007ab134) = c.eax;
            GP4_GLOBAL(uint32_t, 0x007ab13c) = c.edx;
            c.ebx = (c.ebx & 0xffff00ffu) | 0xff00u;
            gp4::call_regs(0x0051da29, c);
            GP4_GLOBAL(uint32_t, 0x007ab138) = c.eax;
        }
    }

    // second code
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint8_t, 0x007ab550);
    gp4::call_regs(0x0051da29, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint32_t, 0x006f2c10) = c.ecx;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0xffu) {
            GP4_GLOBAL(uint32_t, 0x007ab140) = c.eax;
            GP4_GLOBAL(uint32_t, 0x007ab144) = c.edx;
        }
    }

    // third code (bh = 0xff when it repeats the second)
    {
        const uint8_t code = GP4_GLOBAL(uint8_t, 0x007ab553);
        const uint32_t high = code == GP4_GLOBAL(uint8_t, 0x007ab550) ? 0xff00u : 0u;
        c.ebx = (c.ebx & 0xffff0000u) | high | code;
    }
    gp4::call_regs(0x0051da29, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint32_t, 0x006f2c14) = c.ecx;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0xffu) {
            GP4_GLOBAL(uint32_t, 0x007ab148) = c.eax;
            GP4_GLOBAL(uint32_t, 0x007ab14c) = c.edx;
        }
    }

    // fourth code
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint8_t, 0x007ab556);
    gp4::call_regs(0x0051da29, c);
    flags = c.eflags;
    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint32_t, 0x006f2c18) = c.ecx;
        const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab0f1);
        flags = gp4::flags::sub8(mode, 0xffu);
        if (mode != 0xffu) {
            GP4_GLOBAL(uint32_t, 0x007ab150) = c.eax;
            GP4_GLOBAL(uint32_t, 0x007ab154) = c.edx;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051dbf1, FUN_0051dbf1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctl_007ab548+save_00510309")
