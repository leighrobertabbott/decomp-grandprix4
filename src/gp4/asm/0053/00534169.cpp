#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00534169
// Range tracker over 12 channels. Table 0x008281d4 holds one current value per channel and table
// 0x00828174 one (minimum, maximum) pair per channel. edx is consumed one bit per channel (lowest
// bit first): a set bit widens the channel's pair to include the current value; a clear bit
// replaces the current value with the midpoint of the pair ((min + max) >> 1, arithmetic).
// eax, ecx, esi and edi are preserved; edx comes back shifted right by 12. Flags are those of the
// final `dec ecx`, which keeps the carry of the preceding `add edi, 8`.
void __cdecl FUN_00534169(gp4::Regs* r) {
    int32_t* current = GP4_ARRAY(int32_t, 0x008281d4);
    int32_t* range = GP4_ARRAY(int32_t, 0x00828174);
    uint32_t bits = r->edx;
    uint32_t pairAddress = 0x00828174u;

    for (uint32_t remaining = 12; remaining != 0u; --remaining) {
        const int32_t value = *current;
        const uint32_t bit = bits & 1u;
        bits >>= 1;
        if (bit == 0u) {
            *current = static_cast<int32_t>(static_cast<uint32_t>(range[0]) + static_cast<uint32_t>(range[1])) >> 1;
        } else {
            if (value < range[0]) {
                range[0] = value;
            }
            if (value > range[1]) {
                range[1] = value;
            }
        }
        current += 1;
        range += 2;
        pairAddress += 8u;
    }

    r->edx = bits;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, gp4::flags::add32(pairAddress - 8u, 8u)));
}
GP4_IMPL(0x00534169, FUN_00534169, "regs(edx:u32) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
