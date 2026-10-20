#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x004265a9
// Builds the five terms [0x00627aec..0x00627af8] from the segment record ebx (and edi) and the car
// esi through 0x0042640e / 0x00426261 / 0x00426147 (skipped when bit 1 of +0x8d is set) /
// 0x004263b6, moves ebx along the segment ring with 0x004264c4, then returns in ecx a distance
// limit: the difference of two words of the moved-to segment (+0xa4/+0x204 or +0xa6/+0x206 by
// bit 0 of +0x8d) scaled by the remainder 0x004264c4 left in edx, plus the first word, is
// clamped to >= 0x10 and refined by three divisions of dword esi+0xbc (after 0x004266ae updates
// [0x006247c4]); the result is raised to [0x006247c4] when byte [0x00625a54] is set and capped
// at 0x7000. eax = the last quotient sum, edx = its remainder, ebp = the first clamped distance;
// the flags are those of the final "cmp ecx, 0x7000".
void __cdecl FUN_004265a9(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.eax = s.ebx;
    gp4::call_regs(0x0042640e, s);
    GP4_GLOBAL(uint32_t, 0x00627aec) = s.eax;
    gp4::call_regs(0x00426261, s);
    GP4_GLOBAL(uint32_t, 0x00627af0) = s.eax;
    GP4_GLOBAL(uint32_t, 0x00627af4) = 0;
    if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0u) {
        gp4::call_regs(0x00426147, s);
        GP4_GLOBAL(uint32_t, 0x00627af4) = s.eax;
    }
    gp4::call_regs(0x004263b6, s);
    GP4_GLOBAL(uint32_t, 0x00627af8) = s.eax;

    gp4::call_regs(0x004264c4, s);
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.ebx));
    uint32_t base;
    uint32_t span;
    if ((GP4_FIELD(uint8_t, car, 0x8d) & 1u) != 0u) {
        base = GP4_FIELD(uint16_t, segment, 0xa4);
        span = GP4_FIELD(uint16_t, segment, 0x204);
    } else {
        base = GP4_FIELD(uint16_t, segment, 0xa6);
        span = GP4_FIELD(uint16_t, segment, 0x206);
    }
    span -= base;

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(span))
        * static_cast<int64_t>(static_cast<int32_t>(s.edx));
    s.ecx = base + static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    if (static_cast<int32_t>(s.ecx) < 0x10) {
        s.ecx = 0x10;
    }
    s.ebp = s.ecx;

    gp4::Regs limit = s;
    gp4::call_regs(0x004266ae, limit);

    const int32_t load = GP4_FIELD(int32_t, car, 0xbc);
    int32_t remainder;
    int32_t quotient;

    quotient = gp4::idiv64(static_cast<int64_t>(load), static_cast<int32_t>(s.ecx), &remainder);
    s.eax = (static_cast<uint32_t>(quotient) + s.ebp + s.ebp);
    s.edx = static_cast<uint32_t>(remainder);
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(s.eax) >> 1);
    s.ecx = s.eax;
    if (static_cast<int32_t>(s.ecx) < 0x10) {
        s.ecx = 0x10;
    }

    quotient = gp4::idiv64(static_cast<int64_t>(load), static_cast<int32_t>(s.ecx), &remainder);
    s.eax = (static_cast<uint32_t>(quotient) + s.ebp + s.ebp);
    s.edx = static_cast<uint32_t>(remainder);
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(s.eax) >> 1);
    s.ecx = s.eax;
    if (static_cast<int32_t>(s.ecx) < 0x10) {
        s.ecx = 0x10;
    }

    quotient = gp4::idiv64(static_cast<int64_t>(load), static_cast<int32_t>(s.ecx), &remainder);
    s.eax = static_cast<uint32_t>(quotient) + s.ebp;
    s.edx = static_cast<uint32_t>(remainder);
    s.ecx = s.eax;
    if (static_cast<int32_t>(s.ecx) < 0x10) {
        s.ecx = 0x10;
    }

    if (GP4_GLOBAL(uint8_t, 0x00625a54) != 0u) {
        if (static_cast<int32_t>(s.ecx) < GP4_GLOBAL(int32_t, 0x006247c4)) {
            s.ecx = GP4_GLOBAL(uint32_t, 0x006247c4);
        }
    }

    const uint32_t flags = subFlags32(s.ecx, 0x7000u);
    if (static_cast<int32_t>(s.ecx) > 0x7000) {
        s.ecx = 0x7000;
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004265a9, FUN_004265a9, "regs(edx:u32, ebx:rec[0x8943a0,0x160,0..25], esi:ptr[0x273]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0040c0ae")
