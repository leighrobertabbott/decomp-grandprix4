#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00418901
void __cdecl FUN_00418901(gp4::Regs* r) {
    // address of the 0xa8-byte entry selected by the byte at +0x202
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t left = static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 0x202)) * 0xa8u;
    const uint32_t right = reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00625abc));
    const uint32_t result = left + right;
    // the final add eax, imm sets the flags
    const uint32_t flags = (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418901, FUN_00418901, "regs(esi:ptr[0x20a]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
