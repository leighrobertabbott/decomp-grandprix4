#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `left + right`.
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return parityFlag(result)
        | (result < left ? 0x01u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t absolute(uint32_t value) {
    return static_cast<int32_t>(value) < 0 ? 0u - value : value;
}

// Angle of the vector (edx, eax) in 16-bit angle units (0x10000 = full circle) from the arctangent
// table at 0x007c7c60 (word per 1/2048 of the ratio of the smaller to the larger magnitude, 0..0x2000).
// Returns eax = 0 when the larger magnitude is 0. ebx, ecx, edx, ebp, esi and edi are preserved.
// FUNCTION: GP4 0x00401315
void __cdecl FUN_00401315(gp4::Regs* r) {
    const uint32_t first = r->eax;
    const uint32_t second = r->edx;
    const uint32_t signs = first ^ second;
    const uint32_t a = absolute(first);
    const uint32_t b = absolute(second);
    const int16_t* table = GP4_ARRAY(int16_t, 0x007c7c60);

    uint16_t angle;
    if (static_cast<int32_t>(a) < static_cast<int32_t>(b)) {
        if (b == 0u) {
            r->eax = 0;
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags(b);
            return;
        }
        uint32_t remainder;
        const uint32_t index = gp4::div64(static_cast<uint64_t>(a) << 11, b, &remainder);
        angle = static_cast<uint16_t>(table[index]);
    } else {
        if (a == 0u) {
            r->eax = 0;
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags(a);
            return;
        }
        uint32_t remainder;
        const uint32_t index = gp4::div64(static_cast<uint64_t>(b) << 11, a, &remainder);
        angle = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(table[index]));
    }

    if (static_cast<int32_t>(signs) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    uint32_t flags = logicFlags(second);
    if (static_cast<int32_t>(second) < 0) {
        flags = addFlags16(angle, 0x8000u);
        angle = static_cast<uint16_t>(angle + 0x8000u);
    }

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(angle)));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401315, FUN_00401315, "regs(eax:i32[-0x7fffffff..0x7fffffff], edx:i32[-0x7fffffff..0x7fffffff]) -> (eax, cf, pf, af, zf, sf, of); cases=1000")
