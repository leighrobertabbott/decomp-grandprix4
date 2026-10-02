#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00432095
void __cdecl FUN_00432095(gp4::Regs* r) {
    void* body = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = r->eax;
    const uint8_t bit = GP4_FIELD(uint8_t, body, 0x7c) & 1u;
    uint32_t flags;
    if (bit != 0u) {
        // test byte [esi+0x7c], 1 leaves its flags (CF=OF=AF=0, ZF=0, SF=0)
        flags = parityFlag(bit);
    } else {
        // add eax, 0x160 overwrites the flags
        const uint32_t result = value + 0x160u;
        flags = (result < value ? 1u : 0u)
            | parityFlag(result)
            | (((value ^ 0x160u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | ((~(value ^ 0x160u) & (value ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
        r->eax = result;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432095, FUN_00432095, "regs(eax:u32, esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
