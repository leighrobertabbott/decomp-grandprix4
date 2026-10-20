#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Record addressed through esi (only the fields this routine reads or writes).
struct Unknown_0040304d_Esi {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[0x9a - 0x1a];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xa4 - 0x9c];
    int32_t UnknownA4;
    int32_t UnknownA8;
};

// Word lookup in the 16-bit table at 0x007c5c5c: the index is (v >> 2) & 0xfffe for the 16-bit
// magnitude v (a sign test / neg on the word).
static inline int32_t tableValue(uint32_t word) {
    word &= 0xffffu;
    if ((word & 0x8000u) != 0u) {
        word = (0u - word) & 0xffffu;
    }
    const uint32_t index = (word >> 2) & 0xfffeu;
    return static_cast<int32_t>(GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), index));
}

// FUNCTION: GP4 0x0040304d
// Stores table(0x4000 - word +0x18) * word +0x9a * 4 at +0xa4 and table(word +0x18) * word +0x9a * 4
// at +0xa8 (table = the signed words at 0x007c5c5c indexed by (|angle| >> 2) & 0xfffe), then ends
// with 0x403041. edx / eax / flags handed to the callee are those of the second product's
// shld/shl by 2.
void __cdecl FUN_0040304d(gp4::Regs* r) {
    Unknown_0040304d_Esi* rec = reinterpret_cast<Unknown_0040304d_Esi*>(static_cast<uintptr_t>(r->esi));

    const uint32_t angle = static_cast<uint16_t>(rec->Unknown18);

    const int64_t first = static_cast<int64_t>(tableValue(0x4000u - angle)) * static_cast<int64_t>(rec->Unknown9A);
    rec->UnknownA4 = static_cast<int32_t>(static_cast<uint32_t>(first) << 2);

    const int64_t second = static_cast<int64_t>(tableValue(angle)) * static_cast<int64_t>(rec->Unknown9A);
    const uint32_t low = static_cast<uint32_t>(second);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    const uint32_t shifted = low << 2;
    rec->UnknownA8 = static_cast<int32_t>(shifted);

    // shl eax, 2: CF = last bit shifted out; OF = bit31((low << 1) ^ result), AF = 0 (pinned oracle model)
    const uint32_t flags = ((low >> 30) & 1u)
        | parityFlag(shifted)
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((low << 1) ^ shifted) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = shifted;
    r->edx = (high << 2) | (low >> 30);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    gp4::call_regs(0x00403041, *r);
}
GP4_IMPL(0x0040304d, FUN_0040304d, "regs(esi:ptr[0x158]:bytes, edi:ptr[0x16a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
