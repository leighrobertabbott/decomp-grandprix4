#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040cceb
void __cdecl FUN_0040cceb(gp4::Regs* r) {
    // fill the byte at +0x65 of every 0x160-byte record in [edi, ebp) with bl
    uint32_t record = r->edi;
    const uint32_t end = r->ebp;
    const uint8_t fill = static_cast<uint8_t>(r->ebx);
    while (record < end) {
        GP4_FIELD(uint8_t, record, 0x65) = fill;
        record += 0x160u;
    }
    // last executed cmp record, end (record >= end), then clc
    const uint32_t result = record - end;
    const uint32_t flags = parityFlag(result)
        | (((record ^ end ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((record ^ end) & (record ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->edi = record;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040cceb, FUN_0040cceb, "regs(edi:ptr[0x600]:bytes, ebp:rel[edi,0x160,-1..3], ebx:u32) -> (edi, cf, pf, af, zf, sf, of)")
