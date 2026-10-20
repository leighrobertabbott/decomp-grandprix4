#include <gp4/gp4.h>

static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00409b38
// Finds the record (22 records of 0x41c bytes from 0x0066fef0) whose byte +0x7c equals the
// partner of the body's byte +0x7c: the pair (table[2*(b73-1)], table[2*(b73-1)+1]) at 0x007c2a48
// gives the other member when the body's id is the first one, else the first one is the target.
// CF = 0 and ebx = the record when found; CF = 1 (flags of the final `add esi, 0x41c`) when not.
// eax, ecx, edx and esi are preserved.
void __cdecl FUN_00409b38(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;

    const uint32_t pairOffset = ((uint32_t)GP4_FIELD(uint8_t, body, 0x73) - 1u) << 1;
    const uint8_t first = GP4_ARRAY(uint8_t, 0x007c2a48)[pairOffset];
    uint8_t wanted;
    if (GP4_FIELD(uint8_t, body, 0x7c) == first) {
        wanted = GP4_ARRAY(uint8_t, 0x007c2a49)[pairOffset];
    } else {
        wanted = first;
    }

    uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (int remaining = 0x16; remaining != 0; --remaining) {
        if (GP4_FIELD(uint8_t, record, 0x7c) == wanted) {
            r->ebx = (uint32_t)(uintptr_t)record;
            // clc after the equal `cmp dl, [esi+0x7c]`: ZF = PF = 1
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        record += 0x41c;
    }
    // stc after the flags of the last `add esi, 0x41c`
    const uint32_t end = (uint32_t)(uintptr_t)record;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(end - 0x41cu, 0x41cu) | 1u;
}
GP4_IMPL(0x00409b38, FUN_00409b38, "regs(esi:ptr[0x84]:bytes) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
