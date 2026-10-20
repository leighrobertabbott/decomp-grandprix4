#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the 0x00512b11 state: clears the bytes [0x0088fdd7] and [0x006975e8], copies the four
// bytes 0x0066d8c8 -> 0x007c2e9c (0x005204a9), runs command 4 of the dispatcher 0x00531f9c (ax = 4,
// the other bits of eax kept) and clears the dwords of 0x00512b11. [0x007a90fe] is then 0xff when
// bit 7 of [0x007c2ead] is clear, else 0. eax is the dispatcher's (the handler's) leftover, the
// flags are those of `test byte [0x007c2ead], 0x80`; every other register is preserved.
// FUNCTION: GP4 0x00512bee
void __cdecl FUN_00512bee(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0088fdd7) = 0u;
    GP4_GLOBAL(uint8_t, 0x006975e8) = 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005204a9, c);
    c.eax = (c.eax & 0xffff0000u) | 4u;
    gp4::call_regs(0x00531f9c, c);
    gp4::cdecl_<0x00512b11, void>();

    GP4_GLOBAL(uint8_t, 0x007a90fe) = 0u;
    const uint32_t masked = GP4_GLOBAL(uint8_t, 0x007c2ead) & 0x80u;
    if (masked == 0u) {
        GP4_GLOBAL(uint8_t, 0x007a90fe) = 0xffu;
    }
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(masked));
}
GP4_IMPL(0x00512bee, FUN_00512bee, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
