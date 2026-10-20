#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left - right` for a 16-bit or 32-bit operation (`sign` is the top bit, `mask` the width).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

// pushal ... popal wrapper (only the flags of the last flag writer, CF forced, and memory escape).
// esi / ebx = two objects: dwords +0 / +4 a position, dword +0x1c the current track-segment record,
// word +0x90 a lateral value. Each position is rotated into its segment's frame by 0x0042b3fd (CF = 0
// when that fails). Stores the two results in [0x604402..] / [0x603d9c..], the difference in [0x604404],
// and returns CF = 1 when both succeeded and the longitudinal gap is not below -0x160.
// FUNCTION: GP4 0x0042b4b3
void __cdecl FUN_0042b4b3(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t segment1 = GP4_FIELD(uint32_t, first, 0x1c);
    c.edi = segment1;
    GP4_GLOBAL(uint16_t, 0x006077c7) = 0;
    if (segment1 < GP4_GLOBAL(uint32_t, 0x00891500)) {
        const uint16_t word = GP4_FIELD(uint16_t, reinterpret_cast<void*>(static_cast<uintptr_t>(segment1)), 0x16);
        GP4_GLOBAL(uint16_t, 0x006077c7) = word;
        c.eax = (c.eax & 0xffff0000u) | word;
    }
    gp4::call_regs(0x0042b3fd, c);
    if ((c.eflags & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u);  // popal; clc
        return;
    }
    GP4_GLOBAL(uint16_t, 0x00604402) = (uint16_t)c.eax;
    GP4_GLOBAL(uint16_t, 0x00604408) = (uint16_t)c.edx;
    GP4_GLOBAL(uint32_t, 0x00604410) = c.ecx;

    // xchg ebx, esi: the second call works on the other object.
    const uint32_t swap = c.ebx;
    c.ebx = c.esi;
    c.esi = swap;
    c.edi = GP4_FIELD(uint32_t, second, 0x1c);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0042b3fd, c);
    if ((c.eflags & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u);  // popal; clc
        return;
    }
    GP4_GLOBAL(uint16_t, 0x00603d9c) = (uint16_t)c.eax;
    GP4_GLOBAL(uint16_t, 0x0060440a) = (uint16_t)c.edx;
    GP4_GLOBAL(uint32_t, 0x00604414) = c.ecx;

    // xchg ebx, esi back
    const uint32_t back = c.ebx;
    c.ebx = c.esi;
    c.esi = back;

    const uint16_t gap = (uint16_t)((uint16_t)(0u - (uint16_t)c.eax) + GP4_GLOBAL(uint16_t, 0x00604402));
    GP4_GLOBAL(uint16_t, 0x00604404) = gap;
    const uint32_t depth = GP4_GLOBAL(uint32_t, 0x00604414) - GP4_GLOBAL(uint32_t, 0x00604410);
    if ((int32_t)depth < (int32_t)0xfffffea0) {
        r->eflags = (r->eflags & ~0x8d5u) | (subFlags(depth, 0xfffffea0u, 0xffffffffu, 0x80000000u) & 0x8d4u);
        return;
    }

    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0042b451, c);

    uint32_t flags;
    const uint32_t current = GP4_FIELD(uint32_t, first, 0x1c);
    const uint32_t pit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (current < pit) {
        const uint16_t lateral = GP4_FIELD(uint16_t, first, 0x90);
        GP4_GLOBAL(uint16_t, 0x00604402) = lateral;
        const uint16_t sub = GP4_GLOBAL(uint16_t, 0x00604404);
        GP4_GLOBAL(uint16_t, 0x00603d9c) = (uint16_t)(lateral - sub);
        flags = subFlags(lateral, sub, 0xffffu, 0x8000u);
    } else {
        flags = subFlags(current, pit, 0xffffffffu, 0x80000000u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | 1u;  // popal; stc
}
GP4_IMPL(0x0042b4b3, FUN_0042b4b3, "regs(esi:ptr[0x18e]:layout=car_00426fa5+pos_00405cf8, ebx:ptr[0x18e]:layout=car_00426fa5+pos_00405cf8) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12+links_0042b3c8")
