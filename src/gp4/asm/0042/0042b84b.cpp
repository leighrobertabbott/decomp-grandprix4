#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right` for an operand whose sign bit is `signBit`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right, uint32_t signBit) {
    const uint32_t mask = signBit | (signBit - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & signBit) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042b84b
// If the word at [esi+0xea] is -8 and ebx equals the dword at [esi+0x114], the word becomes 8.
// dx is loaded with the original word; the flags are those of the last compare executed.
void __cdecl FUN_0042b84b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint16_t word = GP4_FIELD(uint16_t, record, 0xea);
    uint32_t flags = cmpFlags(word, 0xfff8u, 0x8000u);
    if (word == 0xfff8u) {
        const uint32_t other = GP4_FIELD(uint32_t, record, 0x114);
        flags = cmpFlags(r->ebx, other, 0x80000000u);
        if (r->ebx == other) {
            GP4_FIELD(uint16_t, record, 0xea) = 8u;
        }
    }
    r->edx = (r->edx & 0xffff0000u) | word;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b84b, FUN_0042b84b, "regs(edx:u32, ebx:u32[0..3], esi:ptr[0x11c]:layout=rec_0042b84b) -> (edx, cf, pf, af, zf, sf, of)")
