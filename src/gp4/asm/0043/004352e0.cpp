#include <gp4/gp4.h>

static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Copies the state of bit 0x20 of the source byte +0x1b into bit 2 of the target byte +0x8d and
// derives bit 0x10 of the target byte +0x9c: set only when the source bit is set and the low two
// bits of source byte +0x48 are both clear (that case also leaves al = those two bits, zero).
// The flags are those of the last or/and on the target byte +0x9c.
// FUNCTION: GP4 0x004352e0
void __cdecl FUN_004352e0(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t result;
    if ((GP4_FIELD(uint8_t, source, 0x1b) & 0x20u) != 0u) {
        GP4_FIELD(uint8_t, target, 0x8d) = static_cast<uint8_t>(GP4_FIELD(uint8_t, target, 0x8d) | 2u);
        const uint32_t low = GP4_FIELD(uint8_t, source, 0x48) & 3u;
        r->eax = (r->eax & ~0xffu) | low;
        if (low == 0u) {
            result = GP4_FIELD(uint8_t, target, 0x9c) | 0x10u;
            GP4_FIELD(uint8_t, target, 0x9c) = static_cast<uint8_t>(result);
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(result);
            return;
        }
    } else {
        GP4_FIELD(uint8_t, target, 0x8d) = static_cast<uint8_t>(GP4_FIELD(uint8_t, target, 0x8d) & 0xfdu);
    }
    result = GP4_FIELD(uint8_t, target, 0x9c) & 0xefu;
    GP4_FIELD(uint8_t, target, 0x9c) = static_cast<uint8_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(result);
}
GP4_IMPL(0x004352e0, FUN_004352e0, "regs(eax:u32, edi:ptr[0x50]:bytes, esi:ptr[0xa4]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
