#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00433ff8
// Tests bit (record byte +0x7c & 0x3f) - 1 (clamped to 21) of the bit mask dword
// slot `eax` of the table at DAT_0062ff64. CF = 0 when clear. When set, CF = 1 and
// for slot 1 / 2 eax becomes byte +0 / +1 of entry `bit` of the table at DAT_0062ff68.
// edx is left holding the clamped bit index; ebx and ecx are preserved.
void __cdecl FUN_00433ff8(gp4::Regs* r) {
    const uint32_t slot = r->eax;
    uint8_t* body = (uint8_t*)r->esi;

    int32_t bit = (int32_t)(GP4_FIELD(uint8_t, body, 0x7c) & 0x3fu) - 1;
    const uint32_t cmpFlags = subtractionFlags32((uint32_t)bit, 0x15u);   // cmp edx, 0x15
    if (bit > 0x15) {
        bit = 0x15;
    }
    r->edx = (uint32_t)bit;

    // bt dword [ecx], edx: a register bit offset addresses dword (bit >> 5) relative
    // to the operand, so bit = -1 reaches the dword before it.
    const uint32_t* mask = (const uint32_t*)(uintptr_t)(slot * 4u + GP4_GLOBAL(uint32_t, 0x0062ff64));
    const uint32_t tested = (mask[bit >> 5] >> (bit & 31)) & 1u;

    if (tested == 0u) {
        // clc after bt: bt leaves the other flags of the cmp
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags & ~1u);
        return;
    }

    const uint8_t* entry = (const uint8_t*)(uintptr_t)((uint32_t)bit * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68));
    uint32_t flags;
    if (slot == 1u) {
        r->eax = entry[0];
        flags = subtractionFlags32(slot, 1u);     // cmp ebx, 1
    } else {
        flags = subtractionFlags32(slot, 2u);     // cmp ebx, 2
        if (slot == 2u) {
            r->eax = entry[1];
        }
    }
    // stc after the last cmp
    r->eflags = (r->eflags & ~0x8d5u) | (flags | 1u);
}
GP4_IMPL(0x00433ff8, FUN_00433ff8, "regs(eax:u32[0..8], esi:ptr[0x84]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00433ff8")
