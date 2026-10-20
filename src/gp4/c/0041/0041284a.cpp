#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (test / cmp against zero): CF = OF = AF = 0.
static inline uint32_t logic8Flags(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// (a * b) >> count as `imul; shrd eax, edx, count` leaves it in eax.
static inline uint32_t mulShrd(int32_t a, int32_t b, unsigned count) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> count);
}

// pushal/popal restore every register and popal does not restore the flags: whichever test, compare
// or add ran last decides them.
static inline void finish(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x0041284a
// Lets the car record 0x00638de8 take up grip transferred from the working record 0x00639204
// (both with a mode byte +0x294 that must be zero). Depending on the sign of 0x006036dc, bit 7 of
// byte +0x51 or bit 0 of byte +0xa8 of the second record's segment ([+0x1c]) decide whether the
// transfer runs (the other side also flags 0x00601a74's record, byte +0x20e = 0x80, and tests the
// first record's segment); 0x004127b3 gives the ratio 0x006038ec, bit 3 of the segments' +0x51 /
// +0x71 must agree, and dword +0x1a0 of the first record grows by
// ((0x0060384c * 0x006038d8 >> 16) * ratio >> 14) clamped to [0, 0x4000 - +0x1a0].
void __cdecl FUN_0041284a(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006038ecu) = 0u;

    uint8_t* const first = GP4_ARRAY(uint8_t, 0x00638de8);
    const uint8_t* const firstSegment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, first, 0x1c)));
    if (GP4_FIELD(uint8_t, first, 0x294) != 0u) {
        finish(r, logic8Flags(GP4_FIELD(uint8_t, first, 0x294)));
        return;
    }

    uint8_t* const second = GP4_ARRAY(uint8_t, 0x00639204);
    const uint8_t* const secondSegment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, second, 0x1c)));
    if (GP4_FIELD(uint8_t, second, 0x294) != 0u) {
        finish(r, logic8Flags(GP4_FIELD(uint8_t, second, 0x294)));
        return;
    }

    uint8_t* const marked = GP4_GLOBAL(uint8_t*, 0x00601a74);
    uint32_t test;
    if (GP4_GLOBAL(int32_t, 0x006036dc) < 0) {
        if ((secondSegment[0xa8] & 1u) != 0u) {
            GP4_FIELD(uint8_t, marked, 0x20e) = 0x80u;
            test = firstSegment[0x51] & 0x80u;
        } else {
            test = secondSegment[0x51] & 0x80u;
        }
        if (test == 0u) {
            finish(r, logic8Flags(test));
            return;
        }
    } else {
        if ((secondSegment[0x51] & 0x80u) != 0u) {
            GP4_FIELD(uint8_t, marked, 0x20e) = 0x80u;
            test = firstSegment[0xa8] & 1u;
        } else {
            test = secondSegment[0xa8] & 1u;
        }
        if (test == 0u) {
            finish(r, logic8Flags(test));
            return;
        }
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x00638de8u;
    c.ebx = 0x00639204u;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(firstSegment));
    c.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(secondSegment));
    gp4::call_regs(0x004127b3, c);
    GP4_GLOBAL(uint32_t, 0x006038ecu) = c.eax;

    if ((secondSegment[0x51] & 8u) != 0u) {
        test = second[0x71] & 8u;
        if (test == 0u) {
            finish(r, logic8Flags(test));
            return;
        }
    } else {
        test = second[0x71] & 8u;
        if (test != 0u) {
            finish(r, logic8Flags(test));
            return;
        }
    }

    const uint32_t rate = mulShrd(GP4_GLOBAL(int32_t, 0x0060384c), GP4_GLOBAL(int32_t, 0x006038d8), 16);
    const int32_t amount = static_cast<int32_t>(mulShrd(static_cast<int32_t>(rate), GP4_GLOBAL(int32_t, 0x006038ec), 14));
    int32_t take = 0x4000 - GP4_FIELD(int32_t, first, 0x1a0);
    if (take > amount) {
        take = amount;
    }
    if (take < 0) {
        take = 0;
    }
    const uint32_t before = GP4_FIELD(uint32_t, first, 0x1a0);
    GP4_FIELD(uint32_t, first, 0x1a0) = before + static_cast<uint32_t>(take);
    finish(r, addFlags(before, static_cast<uint32_t>(take)));
}
GP4_IMPL(0x0041284a, FUN_0041284a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+state_004127b3+work_00639204+fixed_0041284a")
