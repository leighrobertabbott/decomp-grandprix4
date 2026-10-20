#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a word logic result (test / xor): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u);
}

// Arithmetic flags of the 16-bit `left - right` (sub / cmp; neg is 0 - value).
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Replaces the low word of a register, keeping the upper half.
static inline uint32_t withLow16(uint32_t full, uint32_t low) {
    return (full & 0xffff0000u) | (low & 0xffffu);
}

static inline void finish(gp4::Regs* r, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebp, uint32_t flags) {
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}

// FUNCTION: GP4 0x0042b740
// Car record in esi, signed steering-like word in cx. After the pit test 0x0042c0c4 (CF set: return),
// bails out unless the sign of cx agrees with the words [0x603c92] and (when bit 3 of +0x162 is set)
// the byte [0x868582]. Then:
//   t  = (word +0xe - word +0x142) * [0x603c64] >> 14       (signed word, imul/shrd 14)
//   bp = word +0xb0, reduced by min(|t|, [0x603c68]) when t has cx's sign (xor non-negative)
//   returns when |cx| < bp, or when |[0x604402]| - 0x100 > word +0xb0 and |t| < [0x603b2e]
//   cx = -min(|cx|, bp) for a negative cx (else min(cx, bp)); if (cx - [0x604402]) ^ cx is negative
//   and word +0x174 < 0x40, it is set to 0x40.
// The 16-bit register halves are kept, the flags are those of the last test/cmp/xor/sub executed.
void __cdecl FUN_0042b740(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    gp4::Regs pit = *r;
    pit.st_in = 0;
    pit.st_out = 0;
    gp4::call_regs(0x0042c0c4, pit);
    uint32_t flags = pit.eflags & 0x8d5u;
    if ((flags & 1u) != 0u) {
        finish(r, eax, ecx, edx, ebp, flags);
        return;
    }

    const uint32_t laneWord = GP4_GLOBAL(uint16_t, 0x00603c92);
    flags = logicFlags16(laneWord);
    if (laneWord != 0u) {
        const uint32_t mixed = (laneWord ^ ecx) & 0xffffu;
        edx = withLow16(edx, mixed);
        flags = logicFlags16(mixed);
        if ((mixed & 0x8000u) == 0u) {
            finish(r, eax, ecx, edx, ebp, flags);
            return;
        }
    }

    const uint32_t modeBit = GP4_FIELD(uint8_t, car, 0x162) & 8u;
    flags = logicFlags8(modeBit);
    if (modeBit != 0u) {
        const uint32_t sign = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582))));
        const uint32_t mixed = (sign ^ ecx) & 0xffffu;
        edx = withLow16(sign, mixed);
        flags = logicFlags16(mixed);
        if ((mixed & 0x8000u) == 0u) {
            finish(r, eax, ecx, edx, ebp, flags);
            return;
        }
    }

    ebp = withLow16(ebp, GP4_FIELD(uint16_t, car, 0xb0));
    const uint32_t difference = (GP4_FIELD(uint16_t, car, 0xe) - GP4_FIELD(uint16_t, car, 0x142)) & 0xffffu;
    const int64_t product = static_cast<int64_t>(static_cast<int16_t>(difference))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603c64));
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    edx = withLow16(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32), eax);
    const uint32_t sameSign = (eax ^ ecx) & 0xffffu;
    edx = withLow16(edx, sameSign);
    flags = logicFlags16(sameSign);
    if ((sameSign & 0x8000u) == 0u) {
        flags = logicFlags16(eax);
        if ((eax & 0x8000u) != 0u) {
            flags = subFlags16(0u, eax);
            eax = withLow16(eax, 0u - eax);
        }
        const uint32_t cap = GP4_GLOBAL(uint16_t, 0x00603c68);
        flags = subFlags16(eax, cap);
        if (static_cast<int16_t>(eax) > static_cast<int16_t>(cap)) {
            eax = withLow16(eax, GP4_GLOBAL(uint16_t, 0x00603c68));
        }
        flags = subFlags16(ebp, eax);
        ebp = withLow16(ebp, ebp - eax);
    }

    uint32_t magnitude = ecx & 0xffffu;
    edx = withLow16(edx, magnitude);
    flags = logicFlags16(magnitude);
    if ((magnitude & 0x8000u) != 0u) {
        flags = subFlags16(0u, magnitude);
        magnitude = (0u - magnitude) & 0xffffu;
        edx = withLow16(edx, magnitude);
    }
    flags = subFlags16(magnitude, ebp);
    if (static_cast<int16_t>(magnitude) < static_cast<int16_t>(ebp)) {
        finish(r, eax, ecx, edx, ebp, flags);
        return;
    }

    uint32_t level = GP4_GLOBAL(uint16_t, 0x00604402);
    edx = withLow16(edx, level);
    flags = logicFlags16(level);
    if ((level & 0x8000u) != 0u) {
        flags = subFlags16(0u, level);
        level = (0u - level) & 0xffffu;
    }
    flags = subFlags16(level, 0x100u);
    level = (level - 0x100u) & 0xffffu;
    edx = withLow16(edx, level);
    const uint32_t reach = GP4_FIELD(uint16_t, car, 0xb0);
    flags = subFlags16(level, reach);
    if (static_cast<int16_t>(level) > static_cast<int16_t>(reach)) {
        uint32_t cut = eax & 0xffffu;
        edx = withLow16(edx, cut);
        flags = logicFlags16(cut);
        if ((cut & 0x8000u) != 0u) {
            flags = subFlags16(0u, cut);
            cut = (0u - cut) & 0xffffu;
            edx = withLow16(edx, cut);
        }
        const uint32_t floorValue = GP4_GLOBAL(uint16_t, 0x00603b2e);
        flags = subFlags16(cut, floorValue);
        if (static_cast<int16_t>(cut) < static_cast<int16_t>(floorValue)) {
            finish(r, eax, ecx, edx, ebp, flags);
            return;
        }
    }

    uint32_t steer = ecx & 0xffffu;
    const uint32_t bp = ebp & 0xffffu;
    flags = logicFlags16(steer);
    if ((steer & 0x8000u) != 0u) {
        flags = subFlags16(0u, steer);
        steer = (0u - steer) & 0xffffu;
        flags = subFlags16(steer, bp);
        if (!(steer < bp)) {
            steer = bp;
        }
        flags = subFlags16(0u, steer);
        steer = (0u - steer) & 0xffffu;
    } else {
        flags = subFlags16(steer, bp);
        if (!(steer < bp)) {
            steer = bp;
        }
    }
    ecx = withLow16(ecx, steer);

    uint32_t delta = steer;
    flags = subFlags16(delta, GP4_GLOBAL(uint16_t, 0x00604402));
    delta = (delta - GP4_GLOBAL(uint16_t, 0x00604402)) & 0xffffu;
    delta ^= steer;
    flags = logicFlags16(delta);
    edx = withLow16(edx, delta);
    if ((delta & 0x8000u) == 0u) {
        finish(r, eax, ecx, edx, ebp, flags);
        return;
    }
    const uint32_t current = GP4_FIELD(uint16_t, car, 0x174);
    flags = subFlags16(current, 0x40u);
    if (static_cast<int16_t>(current) < 0x40) {
        GP4_FIELD(uint16_t, car, 0x174) = 0x40;
    }
    finish(r, eax, ecx, edx, ebp, flags);
}
GP4_IMPL(0x0042b740, FUN_0042b740, "regs(esi:rec[0x66fef0,0x41c,0..21], eax:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+rel_0042c0c4+state_0042b740")
