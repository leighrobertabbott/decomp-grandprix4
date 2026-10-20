#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parses an unsigned decimal number at esi (any number of ASCII digits, 32-bit wraparound): eax = the value,
// esi advanced past the digits. The flags are those of `cmp ecx, 1` with ecx = the digit count, so ZF is
// set for exactly one digit. ebx, ecx, edx and ebp are saved and restored.
// FUNCTION: GP4 0x0052b379
void __cdecl FUN_0052b379(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t eax = 0u;
    uint32_t digits = 0u;

    for (;;) {
        const int32_t digit = static_cast<int32_t>(GP4_FIELD(uint8_t, esi, 0)) - 0x30;
        if (digit < 0 || digit > 9) {
            break;
        }
        eax = eax * 10u + static_cast<uint32_t>(digit);
        ++esi;
        ++digits;
    }

    r->eax = eax;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(digits, 1u));
}
GP4_IMPL(0x0052b379, FUN_0052b379, "regs(esi:ptr[0x40]:u8{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x39,0x30,0x35,0x20,0x2f,0x3a,0x00,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of)")
