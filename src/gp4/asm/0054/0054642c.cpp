#include <gp4/gp4.h>
#include <gp4/flags.h>

// Decimal formatter: writes the number eax at the cursor edi (advanced past the text) and a terminating zero
// (not counted). bh holds the options: 0x80 = signed (a negative eax prints as '-' and its magnitude), 0x40 =
// pad to the width cl with the character bl; 0x20 = a sign character is due (set internally for negatives, or by
// the caller to print '+'). With zero padding the sign goes first. A number that does not fit the width prints
// '?' for each digit. eax, ebx, ecx, edx, esi, ebp are restored; the flags are those of the last `or ecx, ecx`
// (zero) of the digit loop, or of the final `dec ecx` of the '?' loop (CF kept from the width compare).
// FUNCTION: GP4 0x0054642c
void __cdecl FUN_0054642c(gp4::Regs* r) {
    uint32_t value = r->eax;
    uint32_t options = (r->ebx >> 8) & 0xffu;
    const uint8_t padChar = static_cast<uint8_t>(r->ebx);
    const uint32_t width = r->ecx & 0xffu;
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t sign = 0x2bu;

    if ((options & 0x80u) != 0u && static_cast<int32_t>(value) < 0) {
        sign = 0x2du;
        value = 0u - value;
        options |= 0x20u;
    }

    uint8_t digits[10];
    uint32_t count = 0u;
    do {
        digits[count] = static_cast<uint8_t>(value % 10u);
        count += 1u;
        value = value / 10u;
    } while (value != 0u);

    bool overflow = false;
    uint32_t carry = 0u;
    if ((options & 0x40u) != 0u) {
        int32_t room = static_cast<int32_t>(width - count);
        carry = width < count ? 1u : 0u;
        if (room < 0) {
            overflow = true;
        } else {
            if ((options & 0x20u) != 0u) {
                room -= 1;
                if (room < 0) {
                    overflow = true;
                } else if (padChar == 0x30u) {
                    *out++ = sign;
                    options &= 0xdfu;
                }
            }
            if (!overflow) {
                while (room != 0) {
                    *out++ = padChar;
                    room -= 1;
                }
            }
        }
    }

    uint32_t flags;
    if (overflow) {
        for (uint32_t left = count; left != 0u; --left) {
            *out++ = 0x3fu;
        }
        flags = gp4::flags::dec32(1u, carry);
    } else {
        if ((options & 0x20u) != 0u) {
            *out++ = sign;
        }
        for (uint32_t left = count; left != 0u; --left) {
            *out++ = static_cast<uint8_t>(digits[left - 1u] + 0x30u);
        }
        flags = gp4::flags::logic32(0u);
    }
    *out = 0u;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054642c, FUN_0054642c, "regs(eax:u32{0,1,5,9,10,99,100,12345,0xfffffffb,0xffffffff,0xffff0000,0x7fffffff,0x80000000,1000000}, ebx:u32{0x0030,0x0020,0x4030,0x4020,0x8030,0x8020,0xc030,0xc020,0xe030,0xe020,0x2030,0x6030,0xc000,0x8000}, ecx:u32[0..0x20], edi:ptr[0x40]) -> (edi, cf, pf, af, zf, sf, of)")
