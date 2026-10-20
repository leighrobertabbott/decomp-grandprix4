#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` (dword).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// test v,-1 / jns / neg v
static inline int32_t absOf(int32_t v) {
    return v < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(v)) : v;
}

// Largest |component| of a signed triple. `last` is the (possibly negated) third component,
// which the original leaves in edx.
static inline int32_t maxAbs3(int32_t a, int32_t b, int32_t c, int32_t* last) {
    int32_t best = absOf(a);
    const int32_t second = absOf(b);
    if (best < second) {
        best = second;
    }
    const int32_t third = absOf(c);
    if (best < third) {
        best = third;
    }
    *last = third;
    return best;
}

// FUNCTION: GP4 0x00421b34
// Sets the byte at 0x0060827c to 1 when at least three entries are present (0x00608274) and the
// largest |component| of each of three signed triples (0x00607f34.., 0x00607f40.., 0x006081d4..)
// is below its limit (0x0060805c, 0x00608060, 0x00608064); otherwise 3.
// Leaves eax = the last tested maximum (or the count), edx = the last |component| and the flags
// of the last cmp.
void __cdecl FUN_00421b34(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0060827c) = 3;

    const int32_t count = GP4_GLOBAL(int32_t, 0x00608274);
    uint32_t eax = static_cast<uint32_t>(count);
    uint32_t edx = r->edx;
    uint32_t flags = subFlags(eax, 3u);

    if (count >= 3) {
        int32_t last = 0;
        int32_t best = maxAbs3(GP4_GLOBAL(int32_t, 0x00607f34), GP4_GLOBAL(int32_t, 0x00607f38),
                               GP4_GLOBAL(int32_t, 0x00607f3c), &last);
        eax = static_cast<uint32_t>(best);
        edx = static_cast<uint32_t>(last);
        flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x0060805c));
        if (best < GP4_GLOBAL(int32_t, 0x0060805c)) {
            best = maxAbs3(GP4_GLOBAL(int32_t, 0x00607f40), GP4_GLOBAL(int32_t, 0x00607f44),
                           GP4_GLOBAL(int32_t, 0x00607f48), &last);
            eax = static_cast<uint32_t>(best);
            edx = static_cast<uint32_t>(last);
            flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x00608060));
            if (best < GP4_GLOBAL(int32_t, 0x00608060)) {
                best = maxAbs3(GP4_GLOBAL(int32_t, 0x006081d4), GP4_GLOBAL(int32_t, 0x006081d8),
                               GP4_GLOBAL(int32_t, 0x006081dc), &last);
                eax = static_cast<uint32_t>(best);
                edx = static_cast<uint32_t>(last);
                flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x00608064));
                if (best < GP4_GLOBAL(int32_t, 0x00608064)) {
                    GP4_GLOBAL(uint8_t, 0x0060827c) = 1;
                }
            }
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00421b34, FUN_00421b34, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
