#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// eax = (0x0043ce1d() >> 2) + (word table 0x0062eb20, 4-byte stride, indexed by (byte [ebx] - 1)
// & 0x3f, sign-extended, >> 1 logical). All other registers are pushed and popped; the flags are
// those of the final add.
// FUNCTION: GP4 0x004086bc
void __cdecl FUN_004086bc(gp4::Regs* r) {
    gp4::Regs inner = *r;
    gp4::call_regs(0x0043ce1d, inner);

    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(inner.ebx));
    const uint32_t index = (static_cast<uint32_t>(record[0]) - 1u) & 0x3fu;
    const int32_t entry = *reinterpret_cast<const int16_t*>(GP4_ARRAY(uint8_t, 0x0062eb20) + index * 4u);

    const uint32_t scaled = inner.eax >> 2;
    const uint32_t part = static_cast<uint32_t>(entry) >> 1;
    r->eax = scaled + part;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaled, part);
}
GP4_IMPL(0x004086bc, FUN_004086bc, "regs(eax:u32, ebx:ptr[8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c+state_0042595c+state_0066d8c0")
