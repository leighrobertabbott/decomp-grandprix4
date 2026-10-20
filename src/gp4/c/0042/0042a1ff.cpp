#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit "cmp left, right".
static inline uint32_t compareFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// esi is a car record of the 22-record array at 0x66fef0. When the mode word 0x60414e is zero and
// the sub-mode byte 0x60414c is nonzero the whole job is 0x0042a25b (its CF is kept: jae/clc and
// stc leave it as it is). Otherwise a car whose bit 6 of +0xfe is set is reset by 0x0042f9f8, and
// ebx is stored into the car at +0x114 when |mode| == 8 or at +0x118 when |mode| > 8 (signed 16-bit
// compare). dx keeps |mode|; the flags are those of "cmp dx, 8" with CF cleared by the clc.
// FUNCTION: GP4 0x0042a1ff
void __cdecl FUN_0042a1ff(gp4::Regs* r) {
    if (GP4_GLOBAL(uint16_t, 0x0060414e) == 0u && GP4_GLOBAL(uint8_t, 0x0060414c) != 0u) {
        gp4::call_regs(0x0042a25b, *r);
        return;
    }

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    if ((GP4_FIELD(uint8_t, car, 0xfe) & 0x40u) != 0u) {
        gp4::call_regs(0x0042f9f8, *r);
        car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    }

    int16_t mode = GP4_GLOBAL(int16_t, 0x0060414e);
    if (mode < 0) {
        mode = static_cast<int16_t>(-mode);
    }
    const uint16_t dx = static_cast<uint16_t>(mode);
    r->edx = (r->edx & 0xffff0000u) | dx;
    if (mode == 8) {
        GP4_FIELD(uint32_t, car, 0x114) = r->ebx;
    } else if (mode > 8) {
        GP4_FIELD(uint32_t, car, 0x118) = r->ebx;
    }
    r->eflags = (r->eflags & ~0x8d5u) | (compareFlags16(dx, 8u) & ~1u);
}
GP4_IMPL(0x0042a1ff, FUN_0042a1ff, "regs(esi:rec[0x66fef0,0x41c,0..21], ebx:u32, edx:u32) -> (edx, ebx, cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_00411017+cars_0066fef0")
