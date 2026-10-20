#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 32-bit logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of `shl value, 8` in the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit 31 of (value << 7) ^ (value << 8).
static inline uint32_t shiftLeft8Flags(uint32_t value) {
    const uint32_t result = value << 8;
    return ((value >> 24) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul; shrd eax, edx, 14 (low dword).
static inline int32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// FUNCTION: GP4 0x0042e330
// Slip response of the pair of bodies esi / ebx for the steering value eax (clamped to +-0x1000 in
// edi): the word +0x9a difference (at least 1) and the +0xe2 distance (or the result of 0x0040dc8a when
// bit 1 of byte +0x9c is set on either) give a ratio r = min(0x500, ((dist * 16 - 0x1de) << 8) / diff);
// below the word 0x603b52 or a negative offset the result is the constant pair (eax, edx) =
// (0xff000000, 0) for edi >= 0 or (0, 0x1000000) for edi < 0. Otherwise a divisor
// max(0x100, r * r * word(+0x9a) >> 8) scales ([0x603dcc] << 9) * [0x6f2bf0], and the two outputs are
// edx = clamp((0x200 - edi) * q >> 14, 0, 0x10000) << 8 and
// eax = clamp((-0x200 - edi) * q >> 14, -0x10000, 0) << 8. ecx, edi, ebp are saved; the flags are those
// of the last `or edi, edi` / `shl eax, 8`.
void __cdecl FUN_0042e330(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    uint32_t steer = r->eax;
    if (static_cast<int32_t>(steer) < 0) {
        steer = 0u - steer;
        if (steer >= 0x1000u) {
            steer = 0x1000u;
        }
        steer = 0u - steer;
    } else if (steer >= 0x1000u) {
        steer = 0x1000u;
    }

    int32_t spread = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, first, 0x9a) - GP4_FIELD(uint16_t, second, 0x9a)));
    if (spread < 1) {
        spread = 1;
    }

    uint32_t distance;
    if ((GP4_FIELD(uint8_t, first, 0x9c) & 2u) != 0u || (GP4_FIELD(uint8_t, second, 0x9c) & 2u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        distance = call.eax & 0xffffu;
    } else {
        distance = (GP4_FIELD(uint16_t, second, 0xe2) - static_cast<uint32_t>(GP4_FIELD(uint16_t, first, 0xe2))) & 0xffffu;
    }

    int32_t offset = 0;
    bool small = static_cast<int16_t>(distance) < static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603b52));
    if (!small) {
        offset = (static_cast<int32_t>(static_cast<int16_t>(distance)) << 4) - 0x1de;
        small = offset < 0;
    }
    if (small) {
        uint32_t eax;
        uint32_t edx;
        if (static_cast<int32_t>(steer) < 0) {
            eax = 0u;
            edx = 0x1000000u;
        } else {
            eax = 0xff000000u;
            edx = 0u;
        }
        r->eax = eax;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(steer);
        return;
    }

    if (offset > 0x10000) {
        offset = 0x10000;
    }
    int32_t remainder;
    int32_t ratio = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(offset) << 8)), spread, &remainder);
    if (ratio > 0x500) {
        ratio = 0x500;
    }
    const int32_t square = ratio * ratio;
    const int64_t weighted = static_cast<int64_t>(square) * static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(int16_t, first, 0x9a)));
    int32_t divisor = static_cast<int32_t>(static_cast<uint64_t>(weighted) >> 8);
    if (divisor < 0x100) {
        divisor = 0x100;
    }

    const int32_t numerator = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00603dcc)) << 9);
    const int32_t gain = gp4::idiv64(static_cast<int64_t>(numerator) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bf0)), divisor, &remainder);

    int32_t upper = mulShift14(static_cast<int32_t>(0x200u - steer), gain);
    if (upper > 0x10000) {
        upper = 0x10000;
    }
    if (upper < 0) {
        upper = 0;
    }
    const uint32_t edx = static_cast<uint32_t>(upper) << 8;

    int32_t lower = mulShift14(static_cast<int32_t>(0xfffffe00u - steer), gain);
    if (lower > 0) {
        lower = 0;
    }
    if (lower < static_cast<int32_t>(0xffff0000u)) {
        lower = static_cast<int32_t>(0xffff0000u);
    }

    r->eax = static_cast<uint32_t>(lower) << 8;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | shiftLeft8Flags(static_cast<uint32_t>(lower));
}
GP4_IMPL(0x0042e330, FUN_0042e330, "regs(eax:i32, esi:ptr[0x225]:bytes, ebx:ptr[0x225]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=bounds_0042e330")
