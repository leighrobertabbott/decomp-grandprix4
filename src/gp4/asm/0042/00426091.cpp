#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "neg value".
static inline uint32_t negFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Flags of "test value, value" (CF = OF = AF = 0).
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Caches, for the pair of bodies (esi, ebx), the sum of FUN_00426031 of both "+0x98" words (kept in
// DAT_00603e90 for the first) and DAT_00603e8c = |0x10000000 / |average||. Nothing happens (only the
// flags of "cmp ebx, DAT_00603e88") when the pair is already cached in DAT_00603e84/DAT_00603e88.
// Otherwise leaves eax = DAT_00603e8c, edx = the remainder, ebp = |average| (or 1 for zero) and the
// flags of the final "test eax, eax" / "neg eax".
// FUNCTION: GP4 0x00426091
void __cdecl FUN_00426091(gp4::Regs* r) {
    const uint32_t first = r->esi;
    const uint32_t second = r->ebx;

    if (first == GP4_GLOBAL(uint32_t, 0x00603e84) && second == GP4_GLOBAL(uint32_t, 0x00603e88)) {
        // cmp ebx, [0x603e88] with equal operands: ZF and PF.
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00603e84) = first;
    GP4_GLOBAL(uint32_t, 0x00603e88) = second;

    gp4::Regs callee = *r;
    callee.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(first) + 0x98);
    gp4::call_regs(0x00426031, callee);
    GP4_GLOBAL(uint32_t, 0x00603e90) = callee.eax;

    callee = *r;
    callee.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(second) + 0x98);
    gp4::call_regs(0x00426031, callee);

    const int32_t average = static_cast<int32_t>(callee.eax + GP4_GLOBAL(uint32_t, 0x00603e90)) >> 1;
    int32_t divisor = average;
    if (divisor < 0) {
        divisor = static_cast<int32_t>(0u - static_cast<uint32_t>(divisor));
    }
    if (divisor == 0) {
        divisor = 1;
    }

    int32_t remainder;
    int32_t quotient = gp4::idiv64(0x10000000, divisor, &remainder);
    uint32_t flags;
    if (quotient < 0) {
        flags = negFlags32(static_cast<uint32_t>(quotient));
        quotient = static_cast<int32_t>(0u - static_cast<uint32_t>(quotient));
    } else {
        flags = testFlags32(static_cast<uint32_t>(quotient));
    }
    GP4_GLOBAL(int32_t, 0x00603e8c) = quotient;

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(divisor);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426091, FUN_00426091, "regs(esi:ptr[0xe8]:layout=rec_00426091, ebx:ptr[0xa0]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00426091")
