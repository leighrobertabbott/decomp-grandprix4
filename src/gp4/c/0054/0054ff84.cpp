#include <gp4/gp4.h>
#include <gp4/flags.h>

// One step of a shift-register generator over the five bytes 0x0088f5e7..0x0088f5eb (a byte b,
// then words w1 at +1 and w2 at +3): the bytes shift up by one position (w1 = (w1 << 8) | b,
// w2 = (w2 << 8) | w1 >> 8) and the new byte b is the low byte of (w1 >> 4) ^ (w2 >> 1). ax
// receives that xor (the high half of eax, ebx and ecx are preserved); the flags are those of the
// 16-bit xor.
// FUNCTION: GP4 0x0054ff84
void __cdecl FUN_0054ff84(gp4::Regs* r) {
    const uint16_t w1 = GP4_GLOBAL(uint16_t, 0x0088f5e8);
    const uint16_t w2 = GP4_GLOBAL(uint16_t, 0x0088f5ea);
    const uint8_t b = GP4_GLOBAL(uint8_t, 0x0088f5e7);
    const uint16_t mixed = static_cast<uint16_t>((w1 >> 4) ^ (w2 >> 1));

    GP4_GLOBAL(uint16_t, 0x0088f5e8) = static_cast<uint16_t>(((w1 & 0xffu) << 8) | b);
    GP4_GLOBAL(uint16_t, 0x0088f5ea) = static_cast<uint16_t>(((w2 & 0xffu) << 8) | (w1 >> 8));
    GP4_GLOBAL(uint8_t, 0x0088f5e7) = static_cast<uint8_t>(mixed);

    r->eax = (r->eax & 0xffff0000u) | mixed;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(mixed));
}
GP4_IMPL(0x0054ff84, FUN_0054ff84, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
