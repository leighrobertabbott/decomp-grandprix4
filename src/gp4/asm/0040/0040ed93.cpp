#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "sub left, right" on a dword
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// eax = table 0x7c3208[(byte [esi+0x7c] & 0x3f) - 1] when bit 4 of byte [esi+0x8d] is set,
// else DAT_0066d814 - DAT_0066d838 (0 when DAT_0066d838 is 0). The flags are those of the last
// flag writer on the path: the `sub eax, 1` slot computation, the `or eax, eax` zero test or
// the final `sub eax, [0x66d838]`.
// FUNCTION: GP4 0x0040ed93
void __cdecl FUN_0040ed93(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t value;
    uint32_t flags;

    if ((GP4_FIELD(uint8_t, record, 0x8d) & 4u) != 0u) {
        const uint32_t kind = GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu;
        flags = subFlags32(kind, 1u);                                   // sub eax, 1
        value = GP4_ARRAY(uint32_t, 0x007c3208)[static_cast<int32_t>(kind - 1u)];
    } else {
        value = GP4_GLOBAL(uint32_t, 0x0066d838);
        flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);   // or eax, eax
        if (value != 0u) {
            value = GP4_GLOBAL(uint32_t, 0x0066d814);
            flags = subFlags32(value, GP4_GLOBAL(uint32_t, 0x0066d838));
            value -= GP4_GLOBAL(uint32_t, 0x0066d838);
        }
    }
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ed93, FUN_0040ed93, "regs(esi:ptr[0x95]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
