#include <gp4/gp4.h>

static inline uint32_t Parity(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ~parity & 1u;
}

// flags of a 32-bit add lhs + rhs = result (CF, PF, AF, ZF, SF, OF)
static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | (Parity(result) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// flags of an 8-bit test: CF = OF = AF = 0
static inline uint32_t TestFlags8(uint32_t result) {
    return (Parity(result) << 2) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// FUNCTION: GP4 0x0043b974
void __cdecl FUN_0043b974(gp4::Regs* r) {
    const uint8_t* self = reinterpret_cast<const uint8_t*>(r->esi);
    const uint32_t key = (((self[0x7cu] - 1u) & 0x3fu) ^ 1u) + 1u;

    const uint32_t base = 0x0066fef0u;
    uint32_t entry = base;
    uint32_t flags = 0u;
    for (uint32_t remaining = 22u; remaining != 0u; --remaining) {
        const uint32_t offset = entry - base;
        if (key == (GP4_ARRAY(uint8_t, 0x0066fef0)[offset + 0x7cu] & 0x3fu)) {
            const uint32_t hit = GP4_ARRAY(uint8_t, 0x0066fef0)[offset + 0x7au] & 0x80u;
            flags = TestFlags8(hit);
            if (hit == 0u) {
                flags |= 1u;   // stc: record found and free
            }
            r->ebx = entry;
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
        const uint32_t previous = entry;
        entry += 0x41cu;
        flags = AddFlags(previous, 0x41cu, entry);
    }
    r->ebx = entry;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u);   // clc
}
GP4_IMPL(0x0043b974, FUN_0043b974, "regs(esi:ptr[0x84]:bytes) -> (ebx,cf,pf,af,zf,sf,of); globals=layout=search_0066d82c")
