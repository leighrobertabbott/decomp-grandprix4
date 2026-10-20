#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scales the three bytes at esi (an RGB triple, 8-bit channels) to 16-bit channels (value << 6) and stores
// them at 0x00933a14, 0x00933a16 and 0x00933a18. eax is pushed and restored. The flags are those of the last
// `shl ax, 6` on a zero-extended byte: CF = OF = AF = 0 and SF/ZF/PF of the 16-bit result.
// FUNCTION: GP4 0x0051b0d4
void __cdecl FUN_0051b0d4(gp4::Regs* r) {
    const uint32_t esi = r->esi;

    GP4_GLOBAL(uint16_t, 0x00933a14) = static_cast<uint16_t>(GP4_FIELD(uint8_t, esi, 0) << 6);
    GP4_GLOBAL(uint16_t, 0x00933a16) = static_cast<uint16_t>(GP4_FIELD(uint8_t, esi, 1) << 6);
    const uint16_t blue = static_cast<uint16_t>(GP4_FIELD(uint8_t, esi, 2) << 6);
    GP4_GLOBAL(uint16_t, 0x00933a18) = blue;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(blue));
}
GP4_IMPL(0x0051b0d4, FUN_0051b0d4, "regs(esi:ptr[0x10]:u8{0,0,1,2,0x40,0x80,0xff,3,7,0x20}) -> (cf, pf, af, zf, sf, of)")
