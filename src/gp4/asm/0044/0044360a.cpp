#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Carry is set when the car record in esi has bit 1 of byte +0x7b set, a positive signed byte at +0x72 and a word at
// +0xee that is at least the signed dword [0x006039e0]. eax receives that word on the paths that read it. The other
// flags are those of the last compare on the path (stc / clc replace only CF).
// FUNCTION: GP4 0x0044360a
void __cdecl FUN_0044360a(gp4::Regs* r) {
    const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t flags;
    uint32_t carry = 0u;
    const uint32_t state = car[0x7b] & 2u;
    flags = byteParity(state) | (state == 0u ? 0x40u : 0u);          // test byte [esi + 0x7b], 2
    if (state != 0u) {
        const int8_t level = static_cast<int8_t>(car[0x72]);
        flags = cmpFlags8(static_cast<uint8_t>(level), 1u);          // cmp byte [esi + 0x72], 1
        if (level >= 1) {
            const uint32_t word = *reinterpret_cast<const uint16_t*>(car + 0xee);
            const int32_t limit = GP4_GLOBAL(int32_t, 0x006039e0);
            r->eax = word;
            flags = cmpFlags32(word, static_cast<uint32_t>(limit));  // cmp eax, [0x6039e0]
            if (static_cast<int32_t>(word) >= limit) {
                carry = 1u;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0044360a, FUN_0044360a, "regs(eax:u32, esi:ptr[0xf8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
