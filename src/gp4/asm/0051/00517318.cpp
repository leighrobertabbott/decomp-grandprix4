#include <gp4/gp4.h>
#include <gp4/flags.h>

// Common tail applied to each digit character: 0x2d ('-') becomes 0x15, and characters '0'..'9'
// (0x30..0x39) are shifted down by 0x1a. `arith` receives the flags of the last compare/sub the
// original executes on this path.
static inline uint8_t MapDigitChar(uint8_t c, uint32_t* arith) {
    uint32_t f = gp4::flags::sub8(c, 0x2du);       // cmp al, 0x2d
    if (c == 0x2du) {
        c = 0x15;
    }
    f = gp4::flags::sub8(c, 0x30u);                // cmp al, 0x30
    if (c >= 0x30u) {
        f = gp4::flags::sub8(c, 0x39u);            // cmp al, 0x39
        if (c <= 0x39u) {
            f = gp4::flags::sub8(c, 0x1au);        // sub al, 0x1a
            c = (uint8_t)(c - 0x1au);
        }
    }
    *arith = f;
    return c;
}

// FUNCTION: GP4 0x00517318
// Writes the two characters of the byte in al to [edi], edi += 2 (direction flag clear): the high
// nibble first (a leading 0 nibble is a space 0x20, otherwise nibble + 0x30), then the low nibble
// (+0x30); each character then goes through the tail above, so digits map to 0x16..0x1f.
// al's upper bits are kept; al ends as the second character. Flags are those of the last
// compare/sub of the second character.
void __cdecl FUN_00517318(gp4::Regs* r) {
    const uint32_t value = r->eax;
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t arith = 0;

    uint8_t high = (uint8_t)((value & 0xffu) >> 4);
    if (high != 0) {
        high = (uint8_t)(high + 0x30u);
    } else {
        high = 0x20;
    }
    out[0] = MapDigitChar(high, &arith);

    const uint8_t low = MapDigitChar((uint8_t)((value & 0xfu) + 0x30u), &arith);
    out[1] = low;

    r->eax = (value & 0xffffff00u) | low;
    r->edi += 2;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00517318, FUN_00517318, "regs(eax:u32, edi:ptr[0x10]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of)")
