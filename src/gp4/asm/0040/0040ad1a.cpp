#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040ad1a
void __cdecl FUN_0040ad1a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    // eax = |eax >> 16| (pop eax restores the register at the end)
    int32_t magnitude = static_cast<int32_t>(r->eax) >> 16;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    const uint32_t left = static_cast<uint32_t>(magnitude);
    const int32_t limit = GP4_GLOBAL(int32_t, 0x0062697c);
    uint32_t flags;
    if (magnitude < limit) {
        // cmp eax, [0x62697c] ; jl: the flags stay those of the cmp
        const uint32_t right = static_cast<uint32_t>(limit);
        const uint32_t result = left - right;
        flags = (left < right ? 1u : 0u)
            | parityFlag(result)
            | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    } else {
        // or byte [edi+0x12], 0x20 leaves CF=OF=AF=0, ZF=0 (bit 5 is set)
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x12) | 0x20u);
        GP4_FIELD(uint8_t, record, 0x12) = value;
        flags = parityFlag(value) | ((value & 0x80u) != 0u ? 0x80u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ad1a, FUN_0040ad1a, "regs(eax:u32, edi:ptr[0x1c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
