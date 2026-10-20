#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the byte in al as two characters through 0x0051f3bd (al = character): first the high nibble
// as a digit '0'..'?' (add 0x30, so values above 9 give ':'..'?'), a blank (0x20) when the high nibble
// is 0, then the low nibble the same way without the blank. eax is restored by the `pop eax` before the
// second character is built; the other registers and the flags are what the last callee leaves.
// FUNCTION: GP4 0x00510e7c
void __cdecl FUN_00510e7c(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t byte = value & 0xffu;
    const uint32_t high = byte >> 4;

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::szp(high, 8) | (((byte >> 3) & 1u) ? gp4::flags::CF : 0u));
    uint32_t ch = 0x20u;
    if (high != 0u) {
        ch = (high + 0x30u) & 0xffu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(high, 0x30u));
    }
    c.eax = (value & 0xffffff00u) | ch;
    gp4::call_regs(0x0051f3bd, c);

    const uint32_t low = value & 0xfu;
    c.eax = (value & 0xffffff00u) | ((low + 0x30u) & 0xffu);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(low, 0x30u));
    gp4::call_regs(0x0051f3bd, c);
    *r = c;
}
GP4_IMPL(0x00510e7c, FUN_00510e7c, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
