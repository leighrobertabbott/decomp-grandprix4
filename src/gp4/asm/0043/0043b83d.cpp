#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (result < lhs ? 1u : 0u) | parityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Searches the 22 records (stride 0x41c) rooted at 0x0066fef0 for one whose byte at +0x7c matches
// al in its low six bits. Found: esi = that record, ah = the matching key, carry clear (flags of
// the equal "cmp al, ah"). Not found: esi = one past the last record, ah = the last record's key,
// carry set and the other flags those of the final "add esi, 0x41c". ecx is preserved.
// FUNCTION: GP4 0x0043b83d
void __cdecl FUN_0043b83d(gp4::Regs* r) {
    const uint32_t wanted = r->eax & 0x3fu;
    const uint8_t* const records = GP4_ARRAY(uint8_t, 0x0066fef0);

    uint32_t offset = 0u;
    uint32_t key = 0u;
    uint32_t flags = 0u;
    for (uint32_t remaining = 22u; remaining != 0u; --remaining) {
        key = records[offset + 0x7cu] & 0x3fu;
        if (wanted == key) {
            r->esi = 0x0066fef0u + offset;
            r->eax = (r->eax & 0xffff0000u) | (key << 8) | wanted;
            // cmp al, ah (equal) then clc
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        const uint32_t previous = 0x0066fef0u + offset;
        offset += 0x41cu;
        flags = AddFlags(previous, 0x41cu, 0x0066fef0u + offset);
    }
    r->esi = 0x0066fef0u + offset;
    r->eax = (r->eax & 0xffff0000u) | (key << 8) | wanted;
    // add esi, 0x41c leaves its flags, then stc
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | 1u;
}
GP4_IMPL(0x0043b83d, FUN_0043b83d, "regs(eax:u32) -> (eax, esi, cf, pf, af, zf, sf, of)")
