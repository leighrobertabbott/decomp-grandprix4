#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// One record back along the ring of 0x160-byte track segments: clamped at the pit start
// [0x891500] from there on, wrapped forward by [0x866ec4] below the lap start [0x891504] before it.
static inline uint32_t stepBack(uint32_t record) {
    if (record >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        record -= 0x160u;
        if (record < GP4_GLOBAL(uint32_t, 0x00891500)) record = GP4_GLOBAL(uint32_t, 0x00891500);
    } else {
        record -= 0x160u;
        if (record < GP4_GLOBAL(uint32_t, 0x00891504)) record += GP4_GLOBAL(uint32_t, 0x00866ec4);
    }
    return record;
}

// One record forward: at the lap end [0x866ecc] a record below the pit start wraps back by
// [0x866ec4]; from the pit start on it stops at the last record before the pit end [0x866ef8].
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

static inline int32_t levelOf(uint32_t record) {
    return static_cast<int32_t>(GP4_FIELD(int16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record)), 0x14));
}

// FUNCTION: GP4 0x0043ee18
// edi = track-segment record. Sums the signed words +0x14 of the record, the three before it and
// the two after it along the ring: edx = that sum / 6, eax = (record + previous) >> 1. ebx is
// preserved; the flags are those of the final "sar eax, 1".
void __cdecl FUN_0043ee18(gp4::Regs* r) {
    const uint32_t start = r->edi;
    int32_t sum = levelOf(start);

    const uint32_t back1 = stepBack(start);
    sum += levelOf(back1);
    const int32_t pair = sum;

    const uint32_t back2 = stepBack(back1);
    sum += levelOf(back2);
    const uint32_t back3 = stepBack(back2);
    sum += levelOf(back3);

    const uint32_t ahead1 = stepForward(start);
    sum += levelOf(ahead1);
    const uint32_t ahead2 = stepForward(ahead1);
    sum += levelOf(ahead2);

    const uint32_t mean = static_cast<uint32_t>(pair >> 1);
    r->edx = static_cast<uint32_t>(sum / 6);
    r->eax = mean;
    // sar by 1: CF = bit shifted out, OF = AF = 0, PF/ZF/SF from the result.
    r->eflags = (r->eflags & ~0x8d5u)
        | (static_cast<uint32_t>(pair) & 1u)
        | parityFlag(mean)
        | (mean == 0u ? 0x40u : 0u)
        | ((mean >> 31) << 7);
}
GP4_IMPL(0x0043ee18, FUN_0043ee18, "regs(edi:rec[0x8943a0,0x160,0..29]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
