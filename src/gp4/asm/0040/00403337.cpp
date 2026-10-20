#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// `imul ; shrd eax, edx, 14` : low dword of (a * b) >> 14.
static inline uint32_t mulQ14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00403337
// Interpolates (weight eax, Q14) between two lookups of 0x004032cb on the 0x160-byte segment
// record in edi (word +0xaa / +0x132 and the next record's +0xaa / +0x132 = +0x20a / +0x292,
// mode edx == 3 selects the 0x0040ce7b / +0xaa family, any other mode the 0x0040ceb0 / +0x132
// family; ecx is the position passed to 0x004032cb). It also stores the two record values
// (this record and the next one) in [0x006278a4]/[0x006278a8] and the two fractions left by
// 0x004032cb in [0x0062784c]/[0x00627850]. Returns eax = first interpolation, ecx = second
// (record values blended by [0x00627830], plus a second blend by [0x00627834] when the last
// lookup was slot +8, i.e. edx == 5), edx = the last 0x004032cb's edx. The flags are those of
// "cmp edx, 5" or of the final "add ecx, eax".
void __cdecl FUN_00403337(gp4::Regs* r) {
    const uint32_t weight = r->eax;
    const uint32_t mode = r->edx;
    const uint32_t record = r->edi;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    uint32_t index0;
    uint32_t index1;
    if (mode == 3u) {
        call.edi = record;
        gp4::call_regs(0x0040ce7b, call);
        GP4_GLOBAL(uint32_t, 0x006278a4) = call.eax;
        call.edi = record + 0x160u;
        gp4::call_regs(0x0040ce7b, call);
        GP4_GLOBAL(uint32_t, 0x006278a8) = call.eax;
        index0 = GP4_FIELD(uint16_t, record, 0xaa);
        index1 = GP4_FIELD(uint16_t, record, 0x20a);
    } else {
        call.edi = record;
        gp4::call_regs(0x0040ceb0, call);
        GP4_GLOBAL(uint32_t, 0x006278a4) = call.eax;
        call.edi = record + 0x160u;
        gp4::call_regs(0x0040ceb0, call);
        GP4_GLOBAL(uint32_t, 0x006278a8) = call.eax;
        index0 = GP4_FIELD(uint16_t, record, 0x132);
        index1 = GP4_FIELD(uint16_t, record, 0x292);
    }

    gp4::Regs lookup = *r;
    lookup.st_in = 0;
    lookup.st_out = 0;
    lookup.eax = index0;
    gp4::call_regs(0x004032cb, lookup);
    const uint32_t lowValue = lookup.eax;
    GP4_GLOBAL(uint32_t, 0x0062784c) = GP4_GLOBAL(uint32_t, 0x00627848);

    lookup = *r;
    lookup.st_in = 0;
    lookup.st_out = 0;
    lookup.eax = index1;
    gp4::call_regs(0x004032cb, lookup);
    GP4_GLOBAL(uint32_t, 0x00627850) = GP4_GLOBAL(uint32_t, 0x00627848);
    const uint32_t lookupEdx = lookup.edx;

    const uint32_t first = mulQ14(lookup.eax - lowValue, weight) + lowValue;
    const uint32_t fractionLow = GP4_GLOBAL(uint32_t, 0x0062784c);
    const uint32_t fraction = mulQ14(GP4_GLOBAL(uint32_t, 0x00627850) - fractionLow, weight) + fractionLow;
    const uint32_t valueLow = GP4_GLOBAL(uint32_t, 0x006278a4);
    uint32_t second = mulQ14(GP4_GLOBAL(uint32_t, 0x006278a8) - valueLow, GP4_GLOBAL(uint32_t, 0x00627830)) + valueLow;

    uint32_t flags;
    if (lookupEdx == 5u) {
        const uint32_t delta = mulQ14(GP4_GLOBAL(uint32_t, 0x00627834) - second, fraction);
        flags = addFlags32(second, delta);
        second += delta;
    } else {
        flags = subFlags32(lookupEdx, 5u);
    }

    r->eax = first;
    r->ecx = second;
    r->edx = lookupEdx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403337, FUN_00403337, "regs(eax:u32, ecx:i32[-64..64], edx:u32[2..4], edi:ptr[0x2b8]:i16[0..63]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=multiply_0040ce7b")
