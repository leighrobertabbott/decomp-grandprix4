#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` / `sub left, right` (dword).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of `add left, right` (dword).
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// Advances `rec` to the next 0x160-byte record of the ring (or stays on the last one when the
// ring does not wrap) and returns the flags of the last flag-writing instruction executed.
static inline uint32_t nextRecord(uint32_t* rec) {
    uint32_t edi = *rec;
    uint32_t flags = addFlags(edi, 0x160u);
    edi += 0x160u;
    if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0) {
        const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ef8);
        flags = subFlags(edi, end);
        if (edi >= end) {
            flags = subFlags(edi, 0x160u);
            edi -= 0x160u;
        }
    } else {
        const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ecc);
        flags = subFlags(edi, wrap);
        if (edi >= wrap) {
            const uint32_t mid = GP4_GLOBAL(uint32_t, 0x00891500);
            flags = subFlags(edi, mid);
            if (edi < mid) {
                const uint32_t back = GP4_GLOBAL(uint32_t, 0x00866ec4);
                flags = subFlags(edi, back);
                edi -= back;
            } else {
                const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ef8);
                flags = subFlags(edi, end);
                if (edi >= end) {
                    edi = GP4_GLOBAL(uint32_t, 0x00866ef8);
                    flags = subFlags(edi, 0x160u);
                    edi -= 0x160u;
                }
            }
        }
    }
    *rec = edi;
    return flags;
}

// FUNCTION: GP4 0x0043ea29
// Walks (count - 2) records of the 0x160-byte record ring starting one record after the pointer
// at 0x00626990 and stores in 0x00602d88 the record whose word +0x162 minus word -0x15e (the
// words at +2 of the following and the preceding record) is smallest as a signed 16-bit
// difference; with fewer than three entries it stores the start pointer unchanged.
// A pushal/popal routine: only the flags of the last flag writer survive.
void __cdecl FUN_0043ea29(gp4::Regs* r) {
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00626990);
    const int32_t count = GP4_GLOBAL(int32_t, 0x0062ac48);
    uint32_t flags = subFlags(static_cast<uint32_t>(count), 3u);

    if (count >= 3) {
        uint32_t remaining = static_cast<uint32_t>(count) - 2u;
        flags = nextRecord(&edi);
        uint32_t best = edi;
        int32_t bestDiff = 0x7fffffff;
        do {
            const int16_t next = GP4_FIELD(int16_t, edi, 0x162);
            const int16_t prev = GP4_FIELD(int16_t, edi, -0x15e);
            const int32_t diff = static_cast<int16_t>(static_cast<uint16_t>(next - prev));
            if (diff < bestDiff) {
                bestDiff = diff;
                best = edi;
            }
            flags = nextRecord(&edi);
            --remaining;
        } while (remaining != 0u);
        edi = best;
    }

    GP4_GLOBAL(uint32_t, 0x00602d88) = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ea29, FUN_0043ea29, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
