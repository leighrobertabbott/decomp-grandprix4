#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// |sine| style lookup in the Q14 word table at 0x007c5c5c for a 16-bit angle (index (a >> 2) & 0xfffe).
static inline int32_t tableMagnitude(uint32_t angle) {
    const uint32_t offset = ((angle & 0xffffu) >> 2) & 0xfffeu;
    int32_t value = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    return value;
}

// One record's term: with d = (word at *[rec+0x1c]) - (word at +0x0e), |table[|0x4000 - d|]| >> 5
// plus |table[|d|]| >> 6.
static inline int32_t recordTerm(uint8_t* rec) {
    const uint16_t* word = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, rec, 0x1c)));
    const uint32_t difference = static_cast<uint16_t>(*word - GP4_FIELD(uint16_t, rec, 0x0e));

    uint32_t complement = (0x4000u - difference) & 0xffffu;
    if ((complement & 0x8000u) != 0u) {
        complement = (0u - complement) & 0xffffu;
    }
    uint32_t magnitude = difference;
    if ((magnitude & 0x8000u) != 0u) {
        magnitude = (0u - magnitude) & 0xffffu;
    }

    const int32_t cosine = tableMagnitude(complement);
    const int32_t sine = tableMagnitude(magnitude);
    return (cosine >> 5) + (sine >> 6);
}

// FUNCTION: GP4 0x00431165
// Sum over the two records in esi and ebx of |T[|0x4000 - d|]| >> 5 plus |T[|d|]| >> 6, where T is the
// Q14 word table at 0x007c5c5c and d = (word at *[rec+0x1c]) - (word at rec+0x0e). eax = the sum
// (esi's term added to ebx's); the flags are those of the final "add eax, edx"; edx is preserved.
void __cdecl FUN_00431165(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    const uint32_t firstTerm = static_cast<uint32_t>(recordTerm(first));
    const uint32_t secondTerm = static_cast<uint32_t>(recordTerm(second));

    r->eax = secondTerm + firstTerm;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(secondTerm, firstTerm);
}
GP4_IMPL(0x00431165, FUN_00431165, "regs(ebx:ptr[0xed]:layout=rec_00431838, esi:ptr[0xed]:layout=rec_00431838) -> (eax, cf, pf, af, zf, sf, of)")
