#include <gp4/gp4.h>
#include <gp4/flags.h>

// Speed readout (hand-written routine), esi = car record: the high word of the dword +0x244 (clamped to
// 0x96) is shown as three characters when it differs from the cached value [0x88fdb8]; equal values return
// at once with the flags of that compare. Otherwise the cache and the cursor state are reset
// ([0x88fe14] = 5, bytes 0x88fd94..0x88fd97 = 0), the value is converted to BCD by 0x0051fd0c and its
// digits are sent through the character writer 0x0051f3bd (al = character, edi = output cursor): the
// thousands digit (a blank when it is '0', also stored to [0x86610c]), the hundreds digit ([0x866110]) and
// the tens digit ([0x866114]). Finally [0x88fe14] = -1, bit 7 of car byte +0x89 is cleared and
// [0x954169] = 0xff. Registers and flags come back from the last callee; the flags of the early return
// are those of the cache compare, the flags of the full path those of the `and byte [esi + 0x89], 0x7f`.
// FUNCTION: GP4 0x00515322
void __cdecl FUN_00515322(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0040826a, c);
    uint32_t value = c.eax >> 16;
    if (value > 0x96u) {
        value = 0x96u;
    }
    const uint32_t cached = GP4_GLOBAL(uint32_t, 0x0088fdb8);
    c.eax = value;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(value, cached));
    if (value == cached) {
        *r = c;
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0088fdb8) = value;
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 5u;
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0u;
    gp4::call_regs(0x0051fd0c, c);
    const uint32_t bcd = c.eax;

    uint32_t digit = ((((bcd & 0xffffu) >> 8) & 0xffu) + 0x30u) & 0xffu;
    if (digit == 0x30u) {
        digit = 0x20u;
    }
    GP4_GLOBAL(uint32_t, 0x0086610c) = digit;
    c.eax = (bcd & 0xffff0000u) | digit;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(digit));
    gp4::call_regs(0x0051f3bd, c);

    const uint32_t shifted = (bcd & 0xffff0000u) | ((bcd & 0xffffu) >> 4);
    GP4_GLOBAL(uint32_t, 0x00866110) = (shifted & 0xfu) + 0x30u;
    c.eax = (shifted & 0xffffff00u) | ((shifted & 0xfu) + 0x30u);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(shifted & 0xfu, 0x30u));
    gp4::call_regs(0x0051f3bd, c);

    GP4_GLOBAL(uint32_t, 0x00866114) = (bcd & 0xfu) + 0x30u;
    c.eax = (bcd & 0xffffff00u) | ((bcd & 0xfu) + 0x30u);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(bcd & 0xfu, 0x30u));
    gp4::call_regs(0x0051f3bd, c);

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.esi, 0x89) & 0x7fu);
    GP4_FIELD(uint8_t, c.esi, 0x89) = cleared;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    *r = c;
}
GP4_IMPL(0x00515322, FUN_00515322, "regs(esi:rec[0x66fef0,0x41c,0..21], edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+readout_00515187+glyph_0051d329")
