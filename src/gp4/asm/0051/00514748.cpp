#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline bool isTerminator(uint32_t ch) {
    return ch == 0x2cu || ch == 0x0du || ch == 0x0au || ch == 0x20u || ch == 0x09u;
}

// Parses a decimal integer at esi: an optional '-' or '+' (followed by blanks skipped through
// 0x005146e8), then every byte up to a comma, CR, LF, space or tab is taken as a digit
// (acc = acc*10 + (byte - '0'), no digit check). esi is left on the terminator, eax is the value
// (negated for '-'), edx and ebp are restored. The flags are those of `or ebp, ebp` (sign +) or
// of the final `neg eax` (sign -).
// FUNCTION: GP4 0x00514748
void __cdecl FUN_00514748(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    bool negative = false;

    uint32_t ch = *p++;
    if (ch == 0x2du || ch == 0x2bu) {
        negative = (ch == 0x2du);
        gp4::Regs c = {};
        c.eax = ch;
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
        gp4::call_regs(0x005146e8, c);
        ch = c.eax & 0xffu;
        p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
    }

    uint32_t acc = 0;
    for (;;) {
        acc = acc * 10u + (ch - 0x30u);
        ch = *p++;
        if (isTerminator(ch)) {
            break;
        }
    }
    --p;

    uint32_t value = acc;
    uint32_t flags;
    if (negative) {
        flags = gp4::flags::neg32(acc);
        value = 0u - acc;
    } else {
        flags = gp4::flags::logic32(1u);
    }

    r->eax = value;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00514748, FUN_00514748, "regs(esi:ptr[0x40]:u8{=0x30,0x31,0x35,0x39,0x41,0x7a,0x80,0xff,0x2d,0x2b,0x2c,0x0d,0x0a,0x20,0x09}) -> (eax, esi, cf, pf, af, zf, sf, of)")
