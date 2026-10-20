#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `neg a` on a 32-bit operand.
static inline uint32_t negateFlags32(uint32_t a) {
    const uint32_t result = 0u - a;
    uint32_t flags = parityFlag8(result);
    if (a != 0u) flags |= 0x01u;
    if ((a & 0x0fu) != 0u) flags |= 0x10u;
    if (result == 0u) flags |= 0x40u;
    if ((result & 0x80000000u) != 0u) flags |= 0x80u;
    if (a == 0x80000000u) flags |= 0x800u;
    return flags;
}

// 0x145f30 / max(|value|, 0xa0) for a value whose magnitude is below 0x8000 (0 when it is not).
static inline bool scaledReciprocal(int32_t value, int32_t* out) {
    if (value == 0) {
        return false;
    }
    uint32_t magnitude = value < 0 ? 0u - static_cast<uint32_t>(value) : static_cast<uint32_t>(value);
    if (magnitude >= 0x8000u) {
        return false;
    }
    int32_t divisor = static_cast<int32_t>(magnitude);
    if (divisor < 0xa0) {
        divisor = 0xa0;
    }
    *out = 0x145f30 / divisor;
    return true;
}

// FUNCTION: GP4 0x0043df22
// Decodes the next key-frame record at esi (0x0040aadb); CF = 1 from the decoder (empty record)
// returns its registers with CF cleared. Otherwise it derives the reciprocal-style values
// 0x00626964 (from the start value 0x0062ac64 >> 3) and 0x00626968 (from start + step * count,
// >> 3), the per-step slope 0x0062696c = (0x00626968 - 0x00626964) / max-count (a zero count counts
// as 1; edx = remainder) and the signed copies 0x0062695c / 0x00626960 (negated while the start
// value is negative). Returns eax = slope, ecx = 0x00626964, CF = 1; the other flags are those of
// the last test (start value zero or positive) or neg (negative start value).
void __cdecl FUN_0043df22(gp4::Regs* r) {
    gp4::Regs decoded = *r;
    decoded.st_in = 0;
    decoded.st_out = 0;
    gp4::call_regs(0x0040aadb, decoded);

    r->eax = decoded.eax;
    r->edx = decoded.edx;
    r->esi = decoded.esi;
    r->ebp = decoded.ebp;

    if ((decoded.eflags & 1u) != 0u) {
        // jb -> clc
        r->eflags = (r->eflags & ~0x8d5u) | (decoded.eflags & 0x8d4u);
        return;
    }

    GP4_GLOBAL(int32_t, 0x00626964) = 0;
    GP4_GLOBAL(int32_t, 0x00626968) = 0;

    const int32_t start = GP4_GLOBAL(int32_t, 0x0062ac64);
    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062ac68)) * GP4_GLOBAL(int32_t, 0x0062ac48);
    const int32_t end = static_cast<int32_t>(static_cast<uint32_t>(product) + static_cast<uint32_t>(start));

    int32_t reciprocal;
    if (scaledReciprocal(end >> 3, &reciprocal)) {
        GP4_GLOBAL(int32_t, 0x00626968) = reciprocal;
    }
    if (scaledReciprocal(GP4_GLOBAL(int32_t, 0x0062ac64) >> 3, &reciprocal)) {
        GP4_GLOBAL(int32_t, 0x00626964) = reciprocal;
    }

    const int32_t high = GP4_GLOBAL(int32_t, 0x00626968);
    const int32_t low = GP4_GLOBAL(int32_t, 0x00626964);
    int32_t count = GP4_GLOBAL(int32_t, 0x0062ac48);
    if (count == 0) {
        count = 1;
    }
    const int32_t difference = static_cast<int32_t>(static_cast<uint32_t>(high) - static_cast<uint32_t>(low));
    const int32_t slope = difference / count;
    const int32_t remainder = difference % count;
    GP4_GLOBAL(int32_t, 0x0062696c) = slope;

    GP4_GLOBAL(int32_t, 0x0062695c) = low;
    GP4_GLOBAL(int32_t, 0x00626960) = high;

    uint32_t flags;
    if (GP4_GLOBAL(int32_t, 0x0062ac64) == 0) {
        flags = 0x44u;
    } else if (GP4_GLOBAL(int32_t, 0x0062ac64) > 0) {
        flags = parityFlag8(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062ac64)));
    } else {
        GP4_GLOBAL(uint32_t, 0x0062695c) = 0u - static_cast<uint32_t>(low);
        GP4_GLOBAL(uint32_t, 0x00626960) = 0u - static_cast<uint32_t>(high);
        flags = negateFlags32(static_cast<uint32_t>(high));
    }

    r->eax = static_cast<uint32_t>(slope);
    r->ecx = static_cast<uint32_t>(low);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | 1u;  // stc
}
GP4_IMPL(0x0043df22, FUN_0043df22, "regs(esi:ptr[0x40]:layout=stream_0040aadb, ebp:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of)")
