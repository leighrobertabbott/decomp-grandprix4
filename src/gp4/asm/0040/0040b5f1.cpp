#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic operation (test/or/and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the byte compare `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// When the low byte of the dword at +0x240 is >= 0x30 and bit 0 of byte +0x230 is clear, sets that
// bit and asks 0x00409b38 for the partner record (CF = 0, ebx = record); the partner's byte +0x230
// gets bit 0 as well. eax = the dword at +0x240; ebx is saved and restored. Flags are those of the
// last flag writer on the path taken.
// FUNCTION: GP4 0x0040b5f1
void __cdecl FUN_0040b5f1(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = GP4_FIELD(uint32_t, body, 0x240);
    r->eax = value;

    const uint32_t low = value & 0xffu;
    uint32_t flags;
    if (low < 0x30u) {
        flags = cmpFlags8(low, 0x30u);
    } else {
        const uint32_t bit = GP4_FIELD(uint8_t, body, 0x230) & 1u;
        if (bit != 0u) {
            flags = logicFlags8(bit);
        } else {
            GP4_FIELD(uint8_t, body, 0x230) = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x230) | 1u);
            gp4::Regs callee = *r;
            gp4::call_regs(0x00409b38, callee);
            if ((callee.eflags & 1u) == 0u) {
                uint8_t* partner = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(callee.ebx));
                const uint8_t updated = static_cast<uint8_t>(GP4_FIELD(uint8_t, partner, 0x230) | 1u);
                GP4_FIELD(uint8_t, partner, 0x230) = updated;
                flags = logicFlags8(updated);
            } else {
                flags = callee.eflags & 0x8d5u;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b5f1, FUN_0040b5f1, "regs(esi:ptr[0x248]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=search_0066d82c")
