#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
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

// `test edx, -1 ; jns ; neg edx`: the absolute value of a wrapped difference.
static inline uint32_t magnitude(uint32_t difference) {
    return static_cast<int32_t>(difference) < 0 ? 0u - difference : difference;
}

// One probe of the car response at the current scale [0x00604124]: 0x004054e6 returns it in eax.
static __declspec(noinline) uint32_t probe(const gp4::Regs* entry, uint32_t ebx, uint32_t ecx,
                                           uint32_t edx, uint32_t ebp) {
    gp4::Regs call = *entry;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = ebx;
    call.ecx = ecx;
    call.edx = edx;
    call.ebp = ebp;
    gp4::call_regs(0x004054e6, call);
    return call.eax;
}

// FUNCTION: GP4 0x00405552
// Binary search for the scale [0x00604124] (starting from the word [0x00604116], step half of it) at
// which the response of 0x004054e6 for the car in esi reaches the target in ecx: the scale is
// lowered while the response is below the target and raised once it overshoots, the step halving
// each time. It stops when the response is at or above the target at once, when the step drops
// below [0x00604130], when two responses differ by no more than [0x0060412c], or when the response
// is within [0x00604128] of the target. Returns the scale in eax and the last step in ebp; ebx, ecx,
// edx, edi and [0x006030d0] are restored, the flags are those of the comparison that ended the search.
void __cdecl FUN_00405552(gp4::Regs* r) {
    const uint32_t target = r->ecx;
    const uint32_t savedBias = GP4_GLOBAL(uint32_t, 0x006030d0);

    const uint32_t scale = GP4_GLOBAL(uint16_t, 0x00604116);
    GP4_GLOBAL(uint32_t, 0x00604124) = scale;
    uint32_t step = scale >> 1;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;

    uint32_t value = probe(r, ebx, target, edx, step);
    uint32_t flags = cmpFlags32(value, target);
    if (static_cast<int32_t>(value) < static_cast<int32_t>(target)) {
        ebx = value;
        edx = magnitude(value - target);
        flags = cmpFlags32(edx, GP4_GLOBAL(uint32_t, 0x00604128));
        if (static_cast<int32_t>(edx) > GP4_GLOBAL(int32_t, 0x00604128)) {
            bool lowering = true;
            for (;;) {
                if (lowering) {
                    GP4_GLOBAL(uint32_t, 0x00604124) -= step;
                } else {
                    GP4_GLOBAL(uint32_t, 0x00604124) += step;
                }
                step >>= 1;
                flags = cmpFlags32(step, GP4_GLOBAL(uint32_t, 0x00604130));
                if (static_cast<int32_t>(step) < GP4_GLOBAL(int32_t, 0x00604130)) break;

                value = probe(r, ebx, target, edx, step);
                edx = magnitude(value - ebx);
                ebx = value;
                flags = cmpFlags32(edx, GP4_GLOBAL(uint32_t, 0x0060412c));
                if (static_cast<int32_t>(edx) <= GP4_GLOBAL(int32_t, 0x0060412c)) break;

                edx = magnitude(value - target);
                flags = cmpFlags32(edx, GP4_GLOBAL(uint32_t, 0x00604128));
                if (static_cast<int32_t>(edx) <= GP4_GLOBAL(int32_t, 0x00604128)) break;

                if (lowering) {
                    lowering = static_cast<int32_t>(value) < static_cast<int32_t>(target);
                } else {
                    lowering = static_cast<int32_t>(value) <= static_cast<int32_t>(target);
                }
            }
        }
    }

    r->eax = GP4_GLOBAL(uint32_t, 0x00604124);
    GP4_GLOBAL(uint32_t, 0x006030d0) = savedBias;
    r->ebp = step;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405552, FUN_00405552, "regs(ecx:i32[0..0x4400], esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+bank_004279ab+state_00405489+engine_00413606+drive_00405552+search_00405552+seg_00405552")
