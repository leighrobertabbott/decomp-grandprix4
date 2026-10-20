#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042e2f1
// If word [esi+0x1bc] is not 0xffff and ebx == 0x0066fef0 + that word, the word becomes 0xffff.
// edx is preserved (push/pop); the flags are those of the last compare executed.
void __cdecl FUN_0042e2f1(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t slot = GP4_FIELD(uint16_t, record, 0x1bc);
    uint32_t flags;
    if (slot == 0xffffu) {
        // cmp dx, -1 with equal operands: PF|ZF
        flags = 0x44u;
    } else {
        const uint32_t expected = slot + 0x0066fef0u;
        flags = cmpFlags32(r->ebx, expected);
        if (r->ebx == expected) {
            GP4_FIELD(uint16_t, record, 0x1bc) = 0xffffu;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042e2f1, FUN_0042e2f1, "regs(ebx:rec[0x66fef0,0x41c,0..3], esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
