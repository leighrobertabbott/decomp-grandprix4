#include <gp4/gp4.h>
#include <gp4/flags.h>

// ((value >> 3) << 2) * 64 / divisor, the divisor being the dword 0x00875278 but at least 0x80 (signed). The
// 64-bit numerator is cdq + shld/shl, the division one idiv.
static inline int32_t ScaledQuotient(uint32_t value) {
    int32_t divisor = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00875278));
    if (divisor < 0x80) {
        divisor = 0x80;
    }
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(value) >> 3) << 2);
    int32_t remainder;
    return gp4::idiv64(static_cast<int64_t>(scaled) * 64, divisor, &remainder);
}

// Flags the comparison chain leaves for the pair (eax, ecx) of the pushal wrapper below.
static uint32_t RatioFlags(uint32_t eax, uint32_t ecx) {
    if (static_cast<int32_t>(ecx) < 0) {
        return gp4::flags::logic32(ecx);
    }
    if (static_cast<int32_t>(eax) < 0) {
        eax = 0u - eax;
    }
    if (static_cast<int32_t>(ecx) < 0) {
        ecx = 0u - ecx;
    }
    if (static_cast<int32_t>(eax) < static_cast<int32_t>(ecx)) {
        return gp4::flags::sub32(eax, ecx);
    }

    const uint32_t larger = (static_cast<int32_t>(ecx) >= static_cast<int32_t>(eax)) ? ecx : eax;
    const int32_t first = ScaledQuotient(larger);
    const int32_t limit = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x007f9af8));
    if (first < limit) {
        return gp4::flags::sub32(static_cast<uint32_t>(first), static_cast<uint32_t>(limit));
    }

    eax = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 2);
    if (static_cast<int32_t>(eax) < static_cast<int32_t>(ecx)) {
        return gp4::flags::sub32(eax, ecx);
    }
    const int32_t second = ScaledQuotient(eax);
    const int32_t limit2 = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x007f9af8));
    if (second > limit2) {
        return gp4::flags::sub32(static_cast<uint32_t>(second), static_cast<uint32_t>(limit2));
    }
    return gp4::flags::logic32(static_cast<uint32_t>(second));
}

// pushal/popal flag-only test: with |eax| >= ecx >= 0 the scaled ratio of the larger value to the divisor
// 0x00875278 (at least 0x80) must reach the limit [0x007f9af8], and so must the same ratio of |eax| >> 2 (which
// must still be >= ecx). No register or memory is written; the flags are those of the last compare, or of
// the final `or eax, eax` (the second quotient).
// FUNCTION: GP4 0x00551eee
void __cdecl FUN_00551eee(gp4::Regs* r) {
    r->eflags = gp4::flags::merge(r->eflags, RatioFlags(r->eax, r->ecx));
}
GP4_IMPL(0x00551eee, FUN_00551eee, "regs(eax:u32, ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
