#include <gp4/gp4.h>
#include <gp4/x87.h>

// Negation with two's-complement wrap (neg eax leaves 0x80000000 alone).
static inline int32_t negateWrap(int32_t value) {
    return static_cast<int32_t>(0u - static_cast<uint32_t>(value));
}

static inline int32_t absWrap(int32_t value) {
    return value < 0 ? negateWrap(value) : value;
}

// FUNCTION: GP4 0x00422ac2
// Gated by the flag dword +0x4c of the block at [0x00609744] (at least one of 0x14280 set, none of the
// bits outside 0xfffe1c3f... i.e. only bits of the mask ~0xfffe1c3f), and by the magnitudes of the
// positions 0x00607f34 / 0x00607f3c / 0x00607f38 staying below the limits 0x006080b0 / 0x006080b4.
// Then it refreshes the rounded vector (0x00421887), rotates its first two components by the angle
// 0x006082dc, scales by cos(angle 0x006082d4), stores the rounded result in 0x006f2bf4, clamps it to
// +-0x2000000 and derives the two scaled terms 0x006080c4 and 0x006080bc (never below 0x4000).
void __cdecl FUN_00422ac2() {
    uint8_t* const block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609744)));
    const uint32_t flags = GP4_FIELD(uint32_t, block, 0x4c);
    if ((flags & 0x14280u) == 0u) {
        return;
    }
    if ((flags & 0xfffe1c3fu) != 0u) {
        return;
    }

    int32_t magnitude = absWrap(GP4_GLOBAL(int32_t, 0x00607f34));
    const int32_t other = absWrap(GP4_GLOBAL(int32_t, 0x00607f3c));
    if (magnitude < other) {
        magnitude = other;
    }
    if (magnitude >= GP4_GLOBAL(int32_t, 0x006080b0)) {
        return;
    }
    if (absWrap(GP4_GLOBAL(int32_t, 0x00607f38)) >= GP4_GLOBAL(int32_t, 0x006080b4)) {
        return;
    }

    gp4::Regs call = {};
    gp4::call_regs(0x00421887, call);

    using namespace gp4::x87;
    const Ext scale = ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    fsincos(ext_i32(GP4_GLOBAL(int32_t, 0x006082dc)) * scale, &sine, &cosine);
    const Ext cosTerm = cosine * ext_i32(GP4_GLOBAL(int32_t, 0x00607fd4));
    const Ext sinTerm = sine * ext_i32(GP4_GLOBAL(int32_t, 0x00607fd8));
    const Ext sum = cosTerm + sinTerm;
    const Ext tilt = fcos(ext_i32(GP4_GLOBAL(int32_t, 0x006082d4)) * scale);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(sum * tilt);

    const uint32_t rounded = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006f2bf4));
    uint32_t negated = 0u - rounded;
    if (static_cast<int32_t>(negated) < 0) {
        uint32_t limited = 0u - negated;
        if (limited >= 0x2000000u) {
            limited = 0x2000000u;
        }
        negated = 0u - limited;
    } else if (negated >= 0x2000000u) {
        negated = 0x2000000u;
    }

    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(negated)) * GP4_GLOBAL(int32_t, 0x006080c0);
    GP4_GLOBAL(uint32_t, 0x006080c4) = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);

    const int32_t positive = absWrap(static_cast<int32_t>(negated));
    const int64_t second = static_cast<int64_t>(positive) * GP4_GLOBAL(int32_t, 0x006080b8);
    int32_t result = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14));
    if (result < 0x4000) {
        result = 0x4000;
    }
    GP4_GLOBAL(int32_t, 0x006080bc) = result;
}
GP4_IMPL(0x00422ac2, FUN_00422ac2, "cdecl() -> void; globals=layout=view_00609860+state_00422ac2")
