#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042ad15_Esi {
    uint8_t Unknown00[0xac];
    int32_t UnknownAC;
    uint8_t UnknownB0[0x284 - 0xb0];
    int32_t Limit284;
};

// 0x4800 minus the Q14 ratio |(UnknownAC * DAT_00601b50 >> 14) + addend| / Limit284 (the numerator is
// clamped to the limit, a zero limit divides by one); flags = those of the final "add eax, 0x4800".
static inline int32_t invertedRatio(const Unknown_0042ad15_Esi* self, int32_t addend, uint32_t* flags) {
    const int32_t limit = self->Limit284;

    const int64_t product = static_cast<int64_t>(self->UnknownAC) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601b50));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t scaled = (low >> 14) | (high << 18);

    int32_t value = static_cast<int32_t>(scaled + static_cast<uint32_t>(addend));
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    if (value > limit) {
        value = limit;
    }
    const int32_t divisor = (limit == 0) ? 1 : limit;

    // cdq; shld edx, eax, 14; shl eax, 14; idiv divisor
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(value) << 14, divisor, &remainder);

    const uint32_t negated = 0u - static_cast<uint32_t>(quotient);
    *flags = additionFlags32(negated, 0x4800u);
    return static_cast<int32_t>(negated + 0x4800u);
}

// FUNCTION: GP4 0x0042ad15
// Two independent normalised ratios from the same record in esi: edx receives the one for the incoming
// edx, eax the one for the incoming eax. ecx and ebp are preserved; the flags are those of the second
// "add eax, 0x4800".
void __cdecl FUN_0042ad15(gp4::Regs* r) {
    const Unknown_0042ad15_Esi* self = reinterpret_cast<const Unknown_0042ad15_Esi*>(static_cast<uintptr_t>(r->esi));
    const int32_t firstAddend = static_cast<int32_t>(r->edx);
    const int32_t secondAddend = static_cast<int32_t>(r->eax);

    uint32_t flags;
    const int32_t first = invertedRatio(self, firstAddend, &flags);
    const int32_t second = invertedRatio(self, secondAddend, &flags);

    r->eax = static_cast<uint32_t>(second);
    r->edx = static_cast<uint32_t>(first);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042ad15, FUN_0042ad15, "regs(eax:i32[-8..8], edx:i32[-8..8], esi:ptr[0x28c]:i32[-2..6]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
