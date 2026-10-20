#include <gp4/gp4.h>
#include <gp4/flags.h>

// Prints the 32-bit value in eax as 32 binary digits ('0'/'1', most significant bit first) through the character
// writer 0x0051f3bd (al = character; edi is the output cursor of its buffer mode and starts at 0, the counter of
// the original), advancing the glyph column byte 0x88fd96 after every 8 digits. The column byte is restored at
// the end; pushal/popal keep every register. The flags are those of the last flag writer of the final pass:
// `and eax, 7` or, when the digit count is a multiple of 8, the `inc byte [0x88fd96]` after it (CF stays clear).
// FUNCTION: GP4 0x0051f424
void __cdecl FUN_0051f424(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t column = GP4_GLOBAL(uint8_t, 0x0088fd96);

    gp4::Regs c = *r;
    c.ebx = value;
    c.ecx = 0x20u;
    c.edi = 0u;
    uint32_t flags;
    do {
        uint32_t digit = 0x30u;
        if ((c.ebx & 0x80000000u) != 0u) {
            digit = 0x31u;
        }
        c.ebx <<= 1;
        c.eax = (c.eax & 0xffffff00u) | digit;
        gp4::call_regs(0x0051f3bd, c);
        c.edi += 1u;
        c.eax = c.edi & 7u;
        flags = gp4::flags::logic32(c.eax);
        if (c.eax == 0u) {
            const uint32_t before = GP4_GLOBAL(uint8_t, 0x0088fd96);
            GP4_GLOBAL(uint8_t, 0x0088fd96) = (uint8_t)(before + 1u);
            flags = (gp4::flags::add8(before, 1u) & ~gp4::CF) | (flags & gp4::CF);
        }
        c.ecx -= 1u;
    } while (c.ecx != 0u);
    GP4_GLOBAL(uint8_t, 0x0088fd96) = (uint8_t)column;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f424, FUN_0051f424, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329+mode_0051f424")
