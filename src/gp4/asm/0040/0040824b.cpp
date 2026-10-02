#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040824b
void __cdecl FUN_0040824b(gp4::Regs* r) {
    // |[esi+0x98]| >> 8 (a 24.8 fixed-point magnitude)
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t value = GP4_FIELD(uint32_t, record, 0x98);
    if (static_cast<int32_t>(value) < 0) value = 0u - value;
    const uint32_t result = value >> 8;
    // shr eax, 8 sets the flags (AF=0, OF model: bit31(prev ^ result))
    const uint32_t previous = value >> 7;
    const uint32_t flags = (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) << 11);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040824b, FUN_0040824b, "regs(esi:ptr[0xa0]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
