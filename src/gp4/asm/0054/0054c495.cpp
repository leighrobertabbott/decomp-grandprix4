#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parses an unsigned decimal number at esi: eax = the value (32-bit wraparound), esi advanced past the digits.
// Carry clear when at least one digit was read; carry set (eax = 0, esi unchanged) when the first byte is not
// an ASCII digit. The other flags are those of the last comparison against '0' or '9'. ebx is preserved.
// FUNCTION: GP4 0x0054c495
void __cdecl FUN_0054c495(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t eax = 0u;
    uint32_t arith;
    uint32_t carry;

    const uint32_t first = GP4_FIELD(uint8_t, esi, 0);
    if (first < 0x30u || first > 0x39u) {
        arith = (first < 0x30u) ? gp4::flags::sub8(first, 0x30u) : gp4::flags::sub8(first, 0x39u);
        carry = gp4::flags::CF;
    } else {
        for (;;) {
            const uint32_t ch = GP4_FIELD(uint8_t, esi, 0);
            if (ch < 0x30u) {
                arith = gp4::flags::sub8(ch, 0x30u);
                break;
            }
            if (ch > 0x39u) {
                arith = gp4::flags::sub8(ch, 0x39u);
                break;
            }
            eax = eax * 10u + (ch - 0x30u);
            esi++;
        }
        carry = 0u;
    }

    r->eax = eax;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0054c495, FUN_0054c495, "regs(esi:ptr[0x40]:u8{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x39,0x30,0x35,0x20,0x2f,0x3a,0x00,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of)")
