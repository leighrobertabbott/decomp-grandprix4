#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `cmp a, b` on 8-bit operands.
static inline uint32_t compareFlags8(uint32_t a, uint32_t b) {
    a &= 0xffu;
    b &= 0xffu;
    const uint32_t diff = (a - b) & 0xffu;
    uint32_t flags = parityFlag8(diff);
    if (a < b) flags |= 0x01u;
    if (((a ^ b ^ diff) & 0x10u) != 0u) flags |= 0x10u;
    if (diff == 0u) flags |= 0x40u;
    if ((diff & 0x80u) != 0u) flags |= 0x80u;
    if ((((a ^ b) & (a ^ diff)) & 0x80u) != 0u) flags |= 0x800u;
    return flags;
}

// Flags of `cmp a, b` on 32-bit operands.
static inline uint32_t compareFlags32(uint32_t a, uint32_t b) {
    const uint32_t diff = a - b;
    uint32_t flags = parityFlag8(diff);
    if (a < b) flags |= 0x01u;
    if (((a ^ b ^ diff) & 0x10u) != 0u) flags |= 0x10u;
    if (diff == 0u) flags |= 0x40u;
    if ((diff & 0x80000000u) != 0u) flags |= 0x80u;
    if ((((a ^ b) & (a ^ diff)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// Flags of `neg a` on a 32-bit operand.
static inline uint32_t negateFlags32(uint32_t a) {
    const uint32_t result = 0u - a;
    uint32_t flags = parityFlag8(result);
    if (a != 0u) flags |= 0x01u;
    if ((a & 0x0fu) != 0u) flags |= 0x10u;
    if (result == 0u) flags |= 0x40u;
    if ((result & 0x80000000u) != 0u) flags |= 0x80u;
    if (a == 0x80000000u) flags |= 0x800u;
    return flags;
}

// FUNCTION: GP4 0x0043c784
// pushal ... popal ... clc/stc: every register comes back; the result is the carry flag (set =
// "the two cars are close") on top of the flags of the last flag writer.
// Only while byte [0x007c2ea9] is 0x80 it asks 0x0043b974 (esi = this car) for the teammate record
// (ebx, found and free: CF = 1). That record must have +0x71 bit 7 set, a segment pointer +0x1c at or
// above the pit start [0x00891500] and +0x230 bit 7 clear. Then the position sums (0x0043c75b) of
// the teammate and of this car must differ by less than 20000 (signed magnitude, 0x4e20).
void __cdecl FUN_0043c784(gp4::Regs* r) {
    uint32_t flags;
    bool close = false;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode != 0x80u) {
        flags = compareFlags8(mode, 0x80u);
    } else {
        gp4::Regs found = *r;
        found.st_in = 0;
        found.st_out = 0;
        gp4::call_regs(0x0043b974, found);

        uint8_t* mate = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(found.ebx));
        if ((found.eflags & 1u) == 0u) {
            flags = found.eflags & 0x8d5u;
        } else if ((GP4_FIELD(uint8_t, mate, 0x71) & 0x80u) == 0u) {
            flags = 0x44u;  // test with a zero result
        } else if (GP4_FIELD(uint32_t, mate, 0x1c) < GP4_GLOBAL(uint32_t, 0x00891500)) {
            flags = compareFlags32(GP4_FIELD(uint32_t, mate, 0x1c), GP4_GLOBAL(uint32_t, 0x00891500));
        } else if ((GP4_FIELD(uint8_t, mate, 0x230) & 0x80u) != 0u) {
            flags = 0x80u;  // test with the result 0x80: SF set, PF clear
        } else {
            gp4::Regs first = *r;
            first.st_in = 0;
            first.st_out = 0;
            first.eax = found.ebx;
            gp4::call_regs(0x0043c75b, first);
            const uint32_t mateSum = first.eax;

            gp4::Regs second = *r;
            second.st_in = 0;
            second.st_out = 0;
            second.eax = r->esi;
            gp4::call_regs(0x0043c75b, second);

            uint32_t distance = second.eax - mateSum;
            if (static_cast<int32_t>(distance) < 0) {
                distance = 0u - distance;
            }
            flags = compareFlags32(distance, 0x4e20u);
            close = static_cast<int32_t>(distance) < 0x4e20;
        }
    }

    flags = (flags & ~1u) | (close ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0043c784, FUN_0043c784, "regs(esi:ptr[0x238]:layout=car_0043c677) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_00411017+lap_0043b482+state_0041104f+state_00432330+state_0043c677+segw_0043c608")
