#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00424553
void __cdecl FUN_00424553(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    // 0x5d8-byte entries of a table based at 0x006099c0; field +0x1c
    const uint32_t entry = 0x006099c0u + static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 0x6d)) * 0x5d8u;
    const uint32_t value = GP4_GLOBAL(uint32_t, entry + 0x1cu);
    const uint32_t result = value << 5;
    GP4_FIELD(uint32_t, record, 0x68) = result;

    // Flags of `shl eax, 5`: CF = last bit shifted out, OF = MSB(result) ^ CF, AF = 0.
    // eax is restored by the pop; only the flags escape.
    const uint32_t carry = (value >> 27) & 1u;
    const uint32_t flags = carry
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((result >> 31) ^ carry) & 1u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424553, FUN_00424553, "regs(esi:ptr[0x75]:bytes) -> (cf, pf, af, zf, sf, of)")
