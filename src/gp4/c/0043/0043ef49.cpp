#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a byte TEST / OR: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// One 0x160-byte track-segment record forward along the ring: at the lap end [0x866ecc] a
// record below the pit start [0x891500] wraps back by [0x866ec4]; from the pit start on it
// stops at the last record before the pit end [0x866ef8].
static inline uint32_t stepForward(uint32_t record) {
    record += 0x160u;
    if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
            record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return record;
}

// Runs 0x43edd3 and 0x43ee18 on the record and compares their results (negated when the sign
// selector 0x627b65 is set) with the thresholds 0x627aa0..0x627ab0.
static inline bool gapExceeds(const gp4::Regs* r, uint32_t record) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.edi = record;
    gp4::call_regs(0x0043edd3, s);
    uint32_t first = s.eax;
    gp4::call_regs(0x0043ee18, s);
    uint32_t level = s.eax;
    uint32_t mean = s.edx;
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
        first = 0u - first;
        level = 0u - level;
        mean = 0u - mean;
    }
    if (static_cast<int32_t>(first) < GP4_GLOBAL(int32_t, 0x00627ab0)) {
        return static_cast<int32_t>(level) >= GP4_GLOBAL(int32_t, 0x00627aa8)
            || static_cast<int32_t>(mean) >= GP4_GLOBAL(int32_t, 0x00627aac);
    }
    return static_cast<int32_t>(level) >= GP4_GLOBAL(int32_t, 0x00627aa0)
        || static_cast<int32_t>(mean) >= GP4_GLOBAL(int32_t, 0x00627aa4);
}

static inline void markRecord(uint32_t record) {
    uint8_t* seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
    seg[0x73] = static_cast<uint8_t>(seg[0x73] | 1u);
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
        seg[0x73] = static_cast<uint8_t>(seg[0x73] | 2u);
    }
    GP4_GLOBAL(uint32_t, 0x00627b2c) += 1u;
}

// Walks the segment records [0x627b28]..[0x627b24] (at least three) tracking the running
// maximum of the signed word +0x16 (negated with the selector 0x627b65) against the edge word
// +0x62 (+0x60) - 0xed (less [0x627b08] when bit 15 of 0x86855c is set), remembers the first and
// last peaks that drop away by 0x80 or more (0x627b14/0x627b1c, 0x627b18/0x627b20), marks the
// peaks whose 0x43edd3/0x43ee18 values pass the thresholds in byte +0x73 and, when any was
// marked, flags the first and last records of the range (+0x73 bits 0x10 / 0x20).
// pushal/popal keep every register; the flags are those of the last compare/test/or.
// FUNCTION: GP4 0x0043ef49
void __cdecl FUN_0043ef49(gp4::Regs* r) {
    const uint32_t firstIndex = GP4_GLOBAL(uint32_t, 0x00627b28);
    uint32_t count = GP4_GLOBAL(uint32_t, 0x00627b24) - firstIndex + 1u;
    uint32_t flags = cmpFlags32(count, 3u);

    if (static_cast<int32_t>(count) >= 3) {
        uint32_t record = firstIndex * 0x160u + 0x008943a0u;
        GP4_GLOBAL(uint32_t, 0x00627b2c) = 0u;
        GP4_GLOBAL(uint8_t, 0x00627b66) = 0u;
        int32_t best = static_cast<int32_t>(0x90000000u);
        GP4_GLOBAL(uint8_t, 0x00627b67) = 0u;
        do {
            const uint8_t* seg = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record));
            int32_t value = *reinterpret_cast<const int16_t*>(seg + 0x16);
            uint32_t edge;
            if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
                value = -value;
                edge = *reinterpret_cast<const uint16_t*>(seg + 0x60);
            } else {
                edge = *reinterpret_cast<const uint16_t*>(seg + 0x62);
            }
            edge = edge + 0x50u - 0xbdu - 0x80u;
            if ((GP4_GLOBAL(uint16_t, 0x0086855c) & 0x8000u) != 0u) {
                edge -= GP4_GLOBAL(uint32_t, 0x00627b08);
            }
            if (value >= best) {
                best = value;
                GP4_GLOBAL(uint32_t, 0x00627b0c) = count;
                GP4_GLOBAL(uint32_t, 0x00627b10) = record;
            } else if (GP4_GLOBAL(uint8_t, 0x00627b67) != 0u
                       && static_cast<int32_t>(edge - static_cast<uint32_t>(value)) >= 0x80) {
                if (GP4_GLOBAL(uint8_t, 0x00627b66) == 0u) {
                    GP4_GLOBAL(uint32_t, 0x00627b14) = GP4_GLOBAL(uint32_t, 0x00627b10);
                    GP4_GLOBAL(uint32_t, 0x00627b1c) = GP4_GLOBAL(uint32_t, 0x00627b0c);
                    GP4_GLOBAL(uint8_t, 0x00627b66) = 0xffu;
                }
                GP4_GLOBAL(uint32_t, 0x00627b18) = GP4_GLOBAL(uint32_t, 0x00627b10);
                GP4_GLOBAL(uint32_t, 0x00627b20) = GP4_GLOBAL(uint32_t, 0x00627b0c);
                best = static_cast<int32_t>(0x90000000u);
                GP4_GLOBAL(uint8_t, 0x00627b67) = 0u;
            }
            if (value >= static_cast<int32_t>(edge)) {
                GP4_GLOBAL(uint8_t, 0x00627b67) = 0xffu;
            }
            record = stepForward(record);
        } while (--count != 0u);

        if (GP4_GLOBAL(uint8_t, 0x00627b66) != 0u) {
            const uint32_t peak = GP4_GLOBAL(uint32_t, 0x00627b14);
            if (gapExceeds(r, peak)) {
                markRecord(peak);
            }
            const uint32_t last = GP4_GLOBAL(uint32_t, 0x00627b18);
            if (peak != last) {
                if (gapExceeds(r, last)) {
                    markRecord(last);
                    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00627b14)));
                    if ((other[0x73] & 1u) != 0u) {
                        other[0x73] = static_cast<uint8_t>(other[0x73] | 4u);
                        other[0x73] = static_cast<uint8_t>(other[0x73] | 8u);
                        uint8_t* seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(last));
                        seg[0x73] = static_cast<uint8_t>(seg[0x73] | 4u);
                    }
                }
            }
        }

        const uint32_t marked = GP4_GLOBAL(uint32_t, 0x00627b2c);
        flags = cmpFlags32(marked, 0u);
        if (marked != 0u) {
            uint8_t* start = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(
                GP4_GLOBAL(uint32_t, 0x00627b28) * 0x160u + 0x008943a0u));
            uint32_t t = start[0x73] & 0x10u;
            flags = logicFlags8(t);
            if (t == 0u) {
                start[0x73] = static_cast<uint8_t>(start[0x73] | 0x10u);
                uint8_t* end = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(
                    GP4_GLOBAL(uint32_t, 0x00627b24) * 0x160u + 0x008943a0u));
                t = end[0x73] & 0x20u;
                flags = logicFlags8(t);
                if (t == 0u) {
                    end[0x73] = static_cast<uint8_t>(end[0x73] | 0x20u);
                    flags = logicFlags8(end[0x73]);
                }
            }
        }
    }

    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x0043ef49, FUN_0043ef49, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043edd3+ring_008943a0+words_008943a0+idx_0043ef49")
