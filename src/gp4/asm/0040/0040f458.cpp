#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

static inline void SetArithFlags(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Pending-stage bits of the state byte at +0x82, in the order they are served.
static inline uint8_t StageBit(int i) {
    if (i == 0) return 0x40;
    if (i == 1) return 0x20;
    if (i == 2) return 0x08;
    if (i == 3) return 0x04;
    return 0x80;
}

static inline uint32_t StageDelay(int i) {
    if (i == 0) return 86400000u;
    if (i == 1) return 10000u;
    return 6000u;
}

// Index of the first pending stage at or after `from`, or 5 when none.
static inline int FirstStage(uint8_t state, int from) {
    for (int i = from; i < 5; ++i) {
        if (state & StageBit(i)) return i;
    }
    return 5;
}

// FUNCTION: GP4 0x0040f458
void __cdecl FUN_0040f458(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t* timer = reinterpret_cast<uint32_t*>(rec + 0x4c);
    const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
    uint8_t state = rec[0x82];

    if (state & 0x01) {
        // Start request: mark running and arm the first pending stage.
        state = static_cast<uint8_t>((state & 0xfe) | 0x02);
        const int first = FirstStage(state, 0);
        if (first < 5) {
            *timer = now + StageDelay(first);
        } else {
            state &= 0x7d;
            *timer = now;
        }
    } else {
        if ((state & 0x02) == 0 || *timer == 0) {
            // test ... -> zero result
            SetArithFlags(r, 0x44u);
            return;
        }
        // neg eax; add eax, now
        const uint32_t a = 0u - *timer;
        const uint32_t res = a + now;
        if (res & 0x80000000u) {
            const uint32_t flags = (res < a ? 0x1u : 0u)
                | ParityFlag(res)
                | ((a ^ now ^ res) & 0x10u)
                | 0x80u
                | ((((a ^ res) & (now ^ res)) & 0x80000000u) != 0u ? 0x800u : 0u);
            SetArithFlags(r, flags);
            return;
        }
        // Timer expired: finish the current stage and arm the next pending one.
        const int cur = FirstStage(state, 0);
        int next = 5;
        if (cur < 5) {
            state = static_cast<uint8_t>(state & ~StageBit(cur));
            next = FirstStage(state, cur + 1);
        }
        if (next < 5) {
            *timer = now + StageDelay(next);
        } else {
            state &= 0x7d;
        }
    }
    rec[0x82] = state;

    uint32_t flags;
    if (rec[0x83] & 0x20) {
        flags = 0u;   // test byte, 0x20 -> nonzero, odd parity
    } else {
        const uint8_t v = static_cast<uint8_t>(rec[0x88] | 0x80);
        rec[0x88] = v;
        flags = 0x80u | ParityFlag(v);
    }
    SetArithFlags(r, flags);
}
GP4_IMPL(0x0040f458, FUN_0040f458, "regs(esi:ptr[0x90]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
