#include <gp4/gp4.h>

struct Unknown_0042e8e6 {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
    uint8_t Unknown6A[0x9c - 0x6a];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    int16_t UnknownE2;
    uint8_t UnknownE4;
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xea - 0xe6];
    int16_t UnknownEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic operation (test / "cmp x, 0"): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag(result)
        | ((result & (signBit * 2u - 1u)) == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u);
}

// Flags of "cmp/sub left, right" for a 16-bit or 32-bit operand.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t signBit) {
    const uint32_t mask = signBit * 2u - 1u;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & signBit) != 0u ? 0x800u : 0u);
}

// Returns true when the pair passes (clc), false when it fails (stc); `flags` receives the
// arithmetic flags of the last flag-writing instruction (CF adjusted for stc / clc).
static bool run(gp4::Regs& regs, uint32_t& flags) {
    Unknown_0042e8e6* self = reinterpret_cast<Unknown_0042e8e6*>(regs.esi);
    Unknown_0042e8e6* other = GP4_GLOBAL(Unknown_0042e8e6*, 0x006043e0);
    regs.ebx = (uint32_t)(uintptr_t)other;

    flags = subFlags((uint32_t)(uintptr_t)other, regs.esi, 0x80000000u);
    if (other == self) return false;

    flags = logicFlags(other->FlagsE5 & 4u, 0x80u);
    if ((other->FlagsE5 & 4u) != 0u) return false;

    const uint32_t left = (uint16_t)other->Unknown68;
    const uint32_t right = (uint16_t)self->Unknown68;
    const uint32_t diff = (left - right) & 0xffffu;
    regs.edx = (regs.edx & 0xffff0000u) | diff;
    flags = subFlags(left, right, 0x8000u);
    if ((diff & 0x8000u) != 0u) return false;

    const uint32_t limit = (uint16_t)GP4_GLOBAL(int16_t, 0x00603afc);
    flags = subFlags(diff, limit, 0x8000u);
    if ((int16_t)diff >= (int16_t)limit) return false;

    flags = logicFlags(self->Flags9C & 2u, 0x80u);
    bool useCallee = (self->Flags9C & 2u) != 0u;
    if (!useCallee) {
        flags = logicFlags(other->Flags9C & 2u, 0x80u);
        useCallee = (other->Flags9C & 2u) != 0u;
    }
    if (useCallee) {
        regs.eflags = (regs.eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x0040dc8a, regs);
    } else {
        const uint32_t a = (uint16_t)other->UnknownE2;
        const uint32_t b = (uint16_t)self->UnknownE2;
        regs.eax = (regs.eax & 0xffff0000u) | ((a - b) & 0xffffu);
    }

    const uint32_t value = regs.eax & 0xffffu;
    const uint32_t minimum = (uint16_t)GP4_GLOBAL(int16_t, 0x00603b28);
    flags = subFlags(value, minimum, 0x8000u);
    if ((int16_t)value < (int16_t)minimum) return false;

    flags = logicFlags((uint16_t)self->UnknownEA, 0x8000u);
    if (self->UnknownEA != 0) return false;

    flags = logicFlags((uint16_t)other->UnknownEA, 0x8000u);
    if (other->UnknownEA != 0) return false;

    flags = logicFlags(self->FlagsFE & 0x40u, 0x80u);
    if ((self->FlagsFE & 0x40u) != 0u) return false;

    flags = logicFlags(other->FlagsFE & 0x40u, 0x80u);
    if ((other->FlagsFE & 0x40u) != 0u) return false;

    return true;
}

// CF clear (clc) when the record at [0x006043e0] is a different car that is flagged neither
// by bit 2 of +0xe5 nor by bit 6 of +0xfe (in either record), lies ahead of the record in esi
// by a +0x68 word difference in [0, [0x00603afc]), has +0xea clear in both, and has a +0xe2
// separation (via 0x0040dc8a when bit 1 of +0x9c is set in either) of at least [0x00603b28]
// FUNCTION: GP4 0x0042e8e6
void __cdecl FUN_0042e8e6(gp4::Regs* r) {
    gp4::Regs regs = *r;
    uint32_t flags;
    const bool pass = run(regs, flags);

    flags = pass ? (flags & ~1u) : (flags | 1u);
    r->eax = regs.eax;
    r->ebx = regs.ebx;
    r->edx = regs.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0042e8e6, FUN_0042e8e6, "regs(eax:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=pair_0042e8e6")
