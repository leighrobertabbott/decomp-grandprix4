#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// The angle-table lookup 0x0040119c takes and returns its value in ax.
static inline int16_t lookup16(const gp4::Regs* r, uint16_t angle) {
    gp4::Regs callee = *r;
    callee.eax = (callee.eax & 0xffff0000u) | angle;
    gp4::call_regs(0x0040119c, callee);
    return static_cast<int16_t>(callee.eax & 0xffffu);
}

// FUNCTION: GP4 0x0040aa7c
// pushal ... popal wrapper: every register is restored, the flags are those of the final
// `add [0x62ac70], eax`.
void __cdecl FUN_0040aa7c(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint16_t angle = *reinterpret_cast<const uint16_t*>(body + 0x18);

    int32_t scale = GP4_GLOBAL(int32_t, 0x0062ac68);
    uint16_t shifted;
    if (GP4_GLOBAL(int32_t, 0x0062ac64) < 0) {
        scale = static_cast<int32_t>(0u - static_cast<uint32_t>(scale));
        shifted = static_cast<uint16_t>(angle - 0x4000u);
    } else {
        shifted = static_cast<uint16_t>(angle + 0x4000u);
    }
    const uint16_t mirrored = static_cast<uint16_t>(0x4000u - shifted);

    const int16_t first = lookup16(r, mirrored);
    const int16_t second = lookup16(r, shifted);

    const int64_t p0 = static_cast<int64_t>(first) * static_cast<int64_t>(scale);
    const uint32_t step0 = static_cast<uint32_t>(static_cast<uint64_t>(p0) >> 14);
    GP4_GLOBAL(uint32_t, 0x0062ac6c) = GP4_GLOBAL(uint32_t, 0x0062ac6c) + step0;

    const int64_t p1 = static_cast<int64_t>(second) * static_cast<int64_t>(scale);
    const uint32_t step1 = static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062ac70);
    const uint32_t after = before + step1;
    GP4_GLOBAL(uint32_t, 0x0062ac70) = after;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(before, step1, after);
}
GP4_IMPL(0x0040aa7c, FUN_0040aa7c, "regs(esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
