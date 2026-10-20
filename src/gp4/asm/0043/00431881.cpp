#include <gp4/gp4.h>

struct Unknown_00431881_Segment {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
};

struct Unknown_00431881 {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[2];
    Unknown_00431881_Segment* Unknown1C;
    uint8_t Unknown20[0x70];
    int16_t Unknown90;
    uint8_t Unknown92[2];
    int16_t Unknown94;
    uint8_t Unknown96[0xcb - 0x96];
    uint8_t FlagsCB;
    uint8_t FlagsCC;
    uint8_t UnknownCD[0x170 - 0xcd];
    int16_t Unknown170;
    uint8_t Unknown172[2];
    int16_t Unknown174;
    uint8_t Unknown176[0x1a0 - 0x176];
    int32_t Unknown1A0;
    uint8_t Unknown1A4[0x203 - 0x1a4];
    uint8_t Flags203;
    uint8_t Unknown204[0x230 - 0x204];
    uint8_t Flags230;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic operation (test / or) or of "cmp x, 0": CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag(result)
        | ((result & (signBit * 2u - 1u)) == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u);
}

// Flags of "sub/cmp word left, right".
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Runs the routine on the working register block; every early exit leaves the flags of the last
// flag-writing instruction in `flags`.
static void run(gp4::Regs& regs, uint32_t& flags) {
    Unknown_00431881* car = reinterpret_cast<Unknown_00431881*>(regs.esi);

    const int32_t state = car->Unknown1A0;
    flags = logicFlags((uint32_t)state, 0x80000000u);
    if (state < 0) return;

    flags = logicFlags(car->Flags203 & 1u, 0x80u);
    if ((car->Flags203 & 1u) == 0u) return;

    Unknown_00431881_Segment* segment = car->Unknown1C;
    regs.edi = (uint32_t)(uintptr_t)segment;
    flags = logicFlags(segment->Flags51 & 0x20u, 0x80u);
    if ((segment->Flags51 & 0x20u) == 0u) return;

    const uint32_t left = (uint16_t)car->Unknown90;
    const uint32_t right = (uint16_t)car->Unknown94;
    const uint32_t diff = (left - right) & 0xffffu;
    regs.eax = (regs.eax & 0xffff0000u) | diff;
    flags = subFlags16(left, right);

    if ((diff & 0x8000u) != 0u) {
        flags = logicFlags(segment->Flags51 & 4u, 0x80u);
        if ((segment->Flags51 & 4u) == 0u) return;

        regs.eflags = (regs.eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x004317ae, regs);
        flags = regs.eflags & 0x8d5u;
        if ((flags & 1u) == 0u) return;

        regs.eax = 0xffffe000u;
        gp4::call_regs(0x00431838, regs);
        flags = regs.eflags & 0x8d5u;
        if ((flags & 1u) == 0u) return;
    } else {
        flags = logicFlags(segment->Flags51 & 2u, 0x80u);
        if ((segment->Flags51 & 2u) == 0u) return;

        regs.eflags = (regs.eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x004317f3, regs);
        flags = regs.eflags & 0x8d5u;
        if ((flags & 1u) == 0u) return;

        regs.eax = 0x2000u;
        gp4::call_regs(0x00431838, regs);
        flags = regs.eflags & 0x8d5u;
        if ((flags & 1u) == 0u) return;
    }

    car->FlagsCC |= 2u;
    car->Flags230 |= 0x10u;
    car->FlagsCB |= 0x80u;
    flags = logicFlags(car->FlagsCB, 0x80u);
    regs.eax = (regs.eax & 0xffff0000u) | (uint16_t)car->Unknown18;
    car->Unknown170 = car->Unknown18;
    car->Unknown174 = 0xff;
}

// When the segment record flagged by the car (+0x1c) allows it, tests the nearby records ahead
// (0x004317f3) or behind (0x004317ae) and then the lateral limits (0x00431838); on success it sets
// flag bits in the car and copies the word at +0x18 to +0x170, writing 0xff to +0x174.
// FUNCTION: GP4 0x00431881
void __cdecl FUN_00431881(gp4::Regs* r) {
    gp4::Regs regs = *r;
    uint32_t flags;
    run(regs, flags);

    r->eax = regs.eax;
    r->ecx = regs.ecx;
    r->edx = regs.edx;
    r->edi = regs.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00431881, FUN_00431881, "regs(eax:u32, ecx:u32, edx:u32, edi:u32, esi:u32[0x66fef0..0x66fef0]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_004317ae+state_00431838+car0_00431881")
