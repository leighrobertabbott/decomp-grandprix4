#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u, kCarry = 1u };

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a 32-bit logic op / test (CF = OF = AF = 0).
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, 14: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline int32_t absDiff(uint32_t a, uint32_t b) {
    const uint32_t d = a - b;
    return static_cast<int32_t>((static_cast<int32_t>(d) < 0) ? 0u - d : d);
}

// Vertex arrays (16 slots; slots 12 and 13 hold the edge being clipped) and outcodes:
// bit 0 x >= [0x607a2c], bit 3 x <= [0x607a38], bit 1 y >= [0x607a30], bit 4 y <= [0x607a3c],
// bit 2 z >= [0x607a34], bit 5 z <= [0x607a40].
#define VX GP4_ARRAY(uint32_t, 0x00607ca0)
#define VY GP4_ARRAY(uint32_t, 0x00607ce0)
#define VZ GP4_ARRAY(uint32_t, 0x00607d20)
#define VCODE GP4_ARRAY(uint32_t, 0x00607d60)

// Moves vertex `out` along the edge towards `in` onto the plane `bound` of axis `p` (Q14 fraction,
// clamped to 0..0x4000), interpolating the other two axes; the clipped axis becomes bound + offset.
static inline void clipVertex(uint32_t* p, uint32_t* q1, uint32_t* q2, int out, int in, uint32_t bound, uint32_t offset) {
    int32_t span = static_cast<int32_t>(p[out] - p[in]);
    if (span == 0) {
        span = 1;
    }
    const int32_t reach = static_cast<int32_t>(bound - p[in]);
    int32_t rest = 0;
    int32_t t = gp4::idiv64(static_cast<int64_t>(reach) * 16384, span, &rest);
    if (t > 0x4000) {
        t = 0x4000;
    }
    if (t < 0) {
        t = 0;
    }
    q1[out] = mulShr14(q1[out] - q1[in], static_cast<uint32_t>(t)) + q1[in];
    q2[out] = mulShr14(q2[out] - q2[in], static_cast<uint32_t>(t)) + q2[in];
    p[out] = bound + offset;
}

// Clips one endpoint on axis 0 (x), 1 (y) or 2 (z): the first of out/in whose outcode has the
// axis' high bit (bound [hi], offset [0x607a10]) or low bit (bound [lo], offset [0x607a0c]).
// Returns false when neither endpoint is outside on that axis.
static inline bool clipAxis(int axis, int& out, int& in) {
    for (int pass = 0; pass < 2; ++pass) {
        const uint32_t code = VCODE[out];
        if (axis == 0) {
            if ((code & 1u) != 0u) {
                clipVertex(VX, VY, VZ, out, in, GP4_GLOBAL(uint32_t, 0x00607a2c), GP4_GLOBAL(uint32_t, 0x00607a10));
                return true;
            }
            if ((code & 8u) != 0u) {
                clipVertex(VX, VY, VZ, out, in, GP4_GLOBAL(uint32_t, 0x00607a38), GP4_GLOBAL(uint32_t, 0x00607a0c));
                return true;
            }
        } else if (axis == 1) {
            if ((code & 2u) != 0u) {
                clipVertex(VY, VX, VZ, out, in, GP4_GLOBAL(uint32_t, 0x00607a30), GP4_GLOBAL(uint32_t, 0x00607a10));
                return true;
            }
            if ((code & 0x10u) != 0u) {
                clipVertex(VY, VX, VZ, out, in, GP4_GLOBAL(uint32_t, 0x00607a3c), GP4_GLOBAL(uint32_t, 0x00607a0c));
                return true;
            }
        } else {
            if ((code & 4u) != 0u) {
                clipVertex(VZ, VX, VY, out, in, GP4_GLOBAL(uint32_t, 0x00607a34), GP4_GLOBAL(uint32_t, 0x00607a10));
                return true;
            }
            if ((code & 0x20u) != 0u) {
                clipVertex(VZ, VX, VY, out, in, GP4_GLOBAL(uint32_t, 0x00607a40), GP4_GLOBAL(uint32_t, 0x00607a0c));
                return true;
            }
        }
        const int t = out;
        out = in;
        in = t;
    }
    return false;
}

static inline uint32_t outcode(int v) {
    uint32_t code = 0;
    const int32_t x = static_cast<int32_t>(VX[v]);
    if (x >= GP4_GLOBAL(int32_t, 0x00607a2c)) {
        code |= 1u;
    } else if (x <= GP4_GLOBAL(int32_t, 0x00607a38)) {
        code |= 8u;
    }
    const int32_t y = static_cast<int32_t>(VY[v]);
    if (y >= GP4_GLOBAL(int32_t, 0x00607a30)) {
        code |= 2u;
    } else if (y <= GP4_GLOBAL(int32_t, 0x00607a3c)) {
        code |= 0x10u;
    }
    const int32_t z = static_cast<int32_t>(VZ[v]);
    if (z >= GP4_GLOBAL(int32_t, 0x00607a34)) {
        code |= 4u;
    } else if (z <= GP4_GLOBAL(int32_t, 0x00607a40)) {
        code |= 0x20u;
    }
    return code;
}

static inline void finish(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}

// FUNCTION: GP4 0x0043981a
// pushal routine, ecx = edge index (the caller tries edges 0..11 and stops on CF). The edge joins
// vertices [0x607ec8 + 4*ecx] and [0x607ecc + 4*ecx]. CF=1 when either endpoint's outcode is 0
// (inside); CF=0 when both share an outside bit (trivially rejected). Otherwise the endpoints are
// copied to slots 12/13 and clipped Cohen-Sutherland style (axis order by the larger |extent|),
// recomputing the clipped endpoint's outcode, for at most 6 rounds ([0x607da0] counts them,
// [0x607da4] keeps the maximum); CF=0 after the sixth. GPRs are restored; the other flags are
// those of the last or/and/test/cmp before the clc/stc.
void __cdecl FUN_0043981a(gp4::Regs* r) {
    const uint32_t* const edges = GP4_ARRAY(uint32_t, 0x00607ec8);
    const uint32_t first = edges[r->ecx];
    const uint32_t second = edges[r->ecx + 1u];

    const uint32_t codeFirst = VCODE[first];
    if (codeFirst == 0u) {
        finish(r, logicFlags32(0u) | kCarry);
        return;
    }
    const uint32_t codeSecond = VCODE[second];
    if (codeSecond == 0u) {
        finish(r, logicFlags32(0u) | kCarry);
        return;
    }
    if ((codeFirst & codeSecond) != 0u) {
        finish(r, logicFlags32(codeFirst & codeSecond));
        return;
    }

    VX[12] = VX[first];
    VY[12] = VY[first];
    VZ[12] = VZ[first];
    VCODE[12] = VCODE[first];
    VX[13] = VX[second];
    VY[13] = VY[second];
    VZ[13] = VZ[second];
    VCODE[13] = VCODE[second];
    GP4_GLOBAL(uint32_t, 0x00607da0) = 0;

    for (;;) {
        int out = 12;
        int in = 13;
        const uint32_t either = VCODE[12] | VCODE[13];
        const int32_t dx = absDiff(VX[12], VX[13]);
        const int32_t dy = absDiff(VY[12], VY[13]);
        const int32_t dz = absDiff(VZ[12], VZ[13]);

        // Axis preference: largest extent first, then the larger of the other two.
        int order[3];
        if (dx < dy) {
            if (dy >= dz) {
                order[0] = 1;
                order[1] = (dx >= dz) ? 0 : 2;
                order[2] = (dx >= dz) ? 2 : 0;
            } else {
                order[0] = 2;
                order[1] = 1;
                order[2] = 0;
            }
        } else if (dx < dz) {
            order[0] = 2;
            order[1] = 0;
            order[2] = 1;
        } else {
            order[0] = 0;
            order[1] = (dy >= dz) ? 1 : 2;
            order[2] = (dy >= dz) ? 2 : 1;
        }

        int axis = -1;
        for (int k = 0; k < 3; ++k) {
            const uint32_t bits = (order[k] == 0) ? 9u : (order[k] == 1) ? 0x12u : 0x24u;
            if ((either & bits) != 0u) {
                axis = order[k];
                break;
            }
        }
        if (axis < 0 || !clipAxis(axis, out, in)) {
            finish(r, logicFlags32(0u));
            return;
        }

        const uint32_t code = outcode(out);
        VCODE[out] = code;
        if (code == 0u) {
            finish(r, logicFlags32(0u) | kCarry);
            return;
        }
        const uint32_t other = VCODE[in];
        if (other == 0u) {
            finish(r, logicFlags32(0u) | kCarry);
            return;
        }
        if ((code & other) != 0u) {
            finish(r, logicFlags32(code & other));
            return;
        }

        const uint32_t rounds = GP4_GLOBAL(uint32_t, 0x00607da0) + 1u;
        GP4_GLOBAL(uint32_t, 0x00607da0) = rounds;
        if (GP4_GLOBAL(int32_t, 0x00607da4) < static_cast<int32_t>(rounds)) {
            GP4_GLOBAL(uint32_t, 0x00607da4) = rounds;
        }
        if (static_cast<int32_t>(rounds) >= 6) {
            finish(r, cmpFlags32(rounds, 6u));
            return;
        }
    }
}
GP4_IMPL(0x0043981a, FUN_0043981a, "regs(ecx:u32[0..11]) -> (cf, pf, af, zf, sf, of); globals=layout=clip_0043981a")
