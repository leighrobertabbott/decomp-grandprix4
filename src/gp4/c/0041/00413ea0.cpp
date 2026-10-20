#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of a 16-bit "cmp left, right"
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// flags of a 32-bit "cmp left, right"
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00413ea0
// Walks the 0xffff-terminated u16 list at 0x0066fe3c (starting at byte index
// [esi+0x7d]) of record offsets into the 0x41c-stride array at 0x0066fef0.
// CF set = a stop condition was hit, CF clear = a qualifying record was found.
// pushal/popal wrapper: only the flags escape.
void __cdecl FUN_00413ea0(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t ebp = r->ebp;

    gp4::Regs callee = *r;
    gp4::call_regs(0x00414548, callee);
    uint32_t flags = callee.eflags & 0x8d5u;

    if ((flags & 1u) == 0u) {
        uint32_t index = self[0x7d];
        for (;;) {
            const uint16_t word = GP4_FIELD(uint16_t, GP4_ARRAY(uint8_t, 0x0066fe3e), index);
            if (word == 0xffffu) {
                flags = cmpFlags16(word, 0xffffu) | 1u;
                break;
            }
            uint8_t* const rec = GP4_ARRAY(uint8_t, 0x0066fef0) + static_cast<int32_t>(static_cast<int16_t>(word));
            if (self == rec) {
                flags = cmpFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(self)),
                                   static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec))) | 1u;
                break;
            }
            if ((rec[0x7a] & 0x80u) == 0u && (rec[0x7a] & 0x40u) == 0u
                && (rec[0x231] & 0x40u) == 0u && (rec[0x203] & 0x02u) == 0u) {
                const uint16_t diff = static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x68) - GP4_FIELD(uint16_t, rec, 0x68));
                const uint16_t limit = GP4_GLOBAL(uint16_t, 0x00603b0c);
                if (static_cast<int16_t>(diff) >= static_cast<int16_t>(limit)) {
                    flags = cmpFlags16(diff, limit) | 1u;
                    break;
                }
                const uint32_t other = GP4_FIELD(uint16_t, rec, 0x9a);
                uint32_t smaller = GP4_FIELD(uint16_t, self, 0xd8);
                if (smaller > other) smaller = other;
                const uint32_t mean = (other + smaller) >> 1;
                // imul ebp; shrd eax, edx, 10 - only bits 10..25 of the product reach ax
                const uint16_t bound = static_cast<uint16_t>((mean * ebp) >> 10);
                if (static_cast<int16_t>(diff) < static_cast<int16_t>(bound)) {
                    flags = cmpFlags16(diff, bound) & ~1u;
                    break;
                }
            }
            index += 2;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413ea0, FUN_00413ea0, "regs(esi:rec[0x66fef0,0x41c,0..21], ebp:u32) -> (cf, pf, af, zf, sf, of); globals=layout=list_0066fe3c; cases=200")
