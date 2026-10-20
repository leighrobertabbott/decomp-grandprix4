#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` (32-bit).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `left - right` (8-bit, as cmp byte).
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Pit-stop timer driver for a car record (esi): byte +0x25c is the mode (0 = idle, 1 = start, 2 = timed,
// 3 = done), byte +0x25d the step handed to 0x00432acb, dword +0x258 the clock ([0x66d814]) time at which
// the next step is due. CF out is the carry of the last flag writer (0x00432acb's "not finished").
// FUNCTION: GP4 0x00432b7e
void __cdecl FUN_00432b7e(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t flags;
    uint8_t mode = GP4_FIELD(uint8_t, car, 0x25c);

    if (mode == 0) {
        GP4_FIELD(uint8_t, car, 0x25d) = 0xff;
        GP4_FIELD(uint8_t, car, 0x25c) = 1;
        mode = 1;
    }

    if (mode == 1) {
        gp4::call_regs(0x00432acb, c);
        flags = c.eflags & 0x8d5u;
        if ((flags & 1u) == 0u) {
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_FIELD(uint32_t, car, 0x258) = clock;
            GP4_FIELD(uint32_t, car, 0x258) += 0x7d0u;
            flags = addFlags32(clock, 0x7d0u);
            GP4_FIELD(uint8_t, car, 0x25d) = 0xff;
            GP4_FIELD(uint8_t, car, 0x25c) = 2;
        }
    } else if (mode == 2) {
        const uint32_t due = GP4_FIELD(uint32_t, car, 0x258);
        if (due == 0u) {
            flags = 0x44u;  // test of zero: PF | ZF
        } else {
            flags = addFlags32(0u - due, GP4_GLOBAL(uint32_t, 0x0066d814));
            if ((flags & 0x80u) == 0u) {
                gp4::call_regs(0x00432acb, c);
                flags = c.eflags & 0x8d5u;
                if ((flags & 1u) == 0u) {
                    GP4_FIELD(uint8_t, car, 0x25c) = 3;
                }
            }
        }
    } else {
        flags = subFlags8(mode, 2u);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432b7e, FUN_00432b7e, "regs(eax:u32, esi:ptr[0x268]:layout=pit_00432acb) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_00418c68+clock_0066d814")
