#include <gp4/gp4.h>

// neg eax: flags of 0 - value (CF = value != 0, OF = value == 0x80000000).
static inline uint32_t negationFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (value != 0u ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | ((value & 0xfu) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

static inline int32_t negate32(int32_t v) { return static_cast<int32_t>(0u - static_cast<uint32_t>(v)); }

// Raises |v| to at least 0x800 keeping its sign (the signed compares of the original).
static inline int32_t atLeast800(int32_t v) {
    if (v < 0) {
        int32_t magnitude = negate32(v);
        if (magnitude < 0x800) {
            magnitude = 0x800;
        }
        return negate32(magnitude);
    }
    return v < 0x800 ? 0x800 : v;
}

// FUNCTION: GP4 0x0043687a
// Direction vector (dx, dy) = (DAT_00629e34 - DAT_00629e0c, DAT_00629e38 - DAT_00629e10), its dominant
// component raised to a magnitude of at least 0x800 and then both halved until that component is
// below 0x1000. Stores dx -> DAT_00934df0 / DAT_00934de0, -dx -> DAT_00934de8, dy -> DAT_00934df4 /
// DAT_00934de4, -dy -> DAT_00934dec and |dx| -> DAT_00934df8. eax = -dy, flags of that last neg.
void __cdecl FUN_0043687a(gp4::Regs* r) {
    int32_t dx = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00629e34) - GP4_GLOBAL(uint32_t, 0x00629e0c));
    GP4_GLOBAL(int32_t, 0x00934df0) = dx;
    const int32_t absDx = dx < 0 ? negate32(dx) : dx;
    GP4_GLOBAL(int32_t, 0x00934df8) = absDx;

    int32_t dy = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00629e38) - GP4_GLOBAL(uint32_t, 0x00629e10));
    GP4_GLOBAL(int32_t, 0x00934df4) = dy;
    const int32_t absDy = dy < 0 ? negate32(dy) : dy;

    int32_t scale;
    if (absDy < GP4_GLOBAL(int32_t, 0x00934df8)) {
        scale = GP4_GLOBAL(int32_t, 0x00934df8);
        if (scale < 0x800) {
            scale = 0x800;
            dx = atLeast800(dx);
        }
    } else {
        scale = absDy;
        if (scale < 0x800) {
            scale = 0x800;
            dy = atLeast800(dy);
        }
    }

    while (scale >= 0x1000) {
        dx >>= 1;
        dy >>= 1;
        scale = static_cast<int32_t>(static_cast<uint32_t>(scale) >> 1);
    }

    GP4_GLOBAL(int32_t, 0x00934df0) = dx;
    GP4_GLOBAL(int32_t, 0x00934df4) = dy;
    GP4_GLOBAL(int32_t, 0x00934de0) = dx;
    GP4_GLOBAL(int32_t, 0x00934de8) = negate32(dx);
    GP4_GLOBAL(int32_t, 0x00934de4) = dy;
    GP4_GLOBAL(int32_t, 0x00934dec) = negate32(dy);

    r->eax = static_cast<uint32_t>(negate32(dy));
    r->eflags = (r->eflags & ~0x8d5u) | negationFlags32(static_cast<uint32_t>(dy));
}
GP4_IMPL(0x0043687a, FUN_0043687a, "regs() -> (eax,cf,pf,af,zf,sf,of); globals=fuzz:i32")
