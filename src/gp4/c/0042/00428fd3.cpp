#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Flags of `shl value, 16` (oracle model: CF = last bit shifted out, AF = 0,
// OF = bit 31 of (value << 15) ^ result).
static __declspec(noinline) uint32_t shl16Flags(uint32_t value) {
    const uint32_t result = value << 16;
    const uint32_t half = value << 15;
    uint32_t flags = (value >> 16) & 1u;
    flags |= parityFlag8(result);
    if (result == 0u) {
        flags |= 0x40u;
    }
    if ((result & 0x80000000u) != 0u) {
        flags |= 0x80u;
    }
    if (((half ^ result) & 0x80000000u) != 0u) {
        flags |= 0x800u;
    }
    return flags;
}

// Fields of the car record (esi) this routine reads / writes.
struct Unknown_00428fd3_Car {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[0x3c - 0x1a];
    uint32_t Unknown3C;
    uint8_t Unknown40[0x71 - 0x40];
    uint8_t Flags71;
    uint8_t Unknown72[0x9d - 0x72];
    uint8_t Flags9D;
    uint8_t Unknown9E[0xb4 - 0x9e];
    uint16_t UnknownB4;
    uint8_t UnknownB6[0xca - 0xb6];
    uint8_t FlagsCA;
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xe5 - 0xcc];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xfe - 0xe6];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x170 - 0xff];
    int16_t Unknown170;
    uint8_t Unknown172[0x174 - 0x172];
    uint16_t Unknown174;
    uint8_t Unknown176[0x180 - 0x176];
    uint16_t Unknown180;
    uint8_t Unknown182[0x1a0 - 0x182];
    uint32_t Unknown1A0;
    uint32_t Unknown1A4;
    uint8_t Unknown1A8[0x1ce - 0x1a8];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x1f8 - 0x1cf];
    uint32_t Unknown1F8;
    uint8_t Unknown1FC[0x230 - 0x1fc];
    uint8_t Flags230;
    uint8_t Unknown231[0x272 - 0x231];
    uint8_t Unknown272;
    uint8_t Flags273;
    uint8_t Unknown274[0x294 - 0x274];
    uint8_t Unknown294;
};

// FUNCTION: GP4 0x00428fd3
// Steering / lateral-offset hand-over for the car at esi (edi = its track segment). After
// 0x00428bea, bit 3 of +0xfe is cleared. With bit 7 of +0xcb clear (or +0x273 bit 7 clear without
// +0x230 bit 4 / +0xca bit 0) the "free" state is set up (+0x1a4 = +0x1a0, +0x272 = +0x294, +0xfe
// bit 7 follows +0x71 bit 3, +0x174 = 8) and then the common tail: +0xca |= 4, 0x004298e4. With
// +0x230 bit 4 the word +0xb4 becomes the signed difference +0x170 - +0x18 clamped to +-[0x6013ac];
// with +0xca bit 0 the group 0x00428a53 / 0x00428efb runs first; with +0xe5 and +0xcb bit 4 both
// set +0x1ce gets bit 5 and +0xb4 is cleared and the routine ends. The tail finally copies +0xb4
// to +0x180 and (<< 16) to +0x3c and clears +0x1f8 unless bit 0 of +0xe5 is set.
void __cdecl FUN_00428fd3(gp4::Regs* r) {
    Unknown_00428fd3_Car* car = reinterpret_cast<Unknown_00428fd3_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00428bea, c);

    if ((car->FlagsFE & 8u) != 0u) {
        car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0xf7u);
    }

    enum { StepFree, StepCommon, StepTail, StepEnd };
    int step;
    uint32_t flags = 0;

    if ((car->FlagsCB & 0x80u) == 0u) {
        step = StepFree;
    } else if ((car->Flags230 & 0x10u) != 0u) {
        // signed difference clamped to +-[0x6013ac]
        int32_t value = static_cast<int16_t>(static_cast<uint16_t>(car->Unknown170 - car->Unknown18));
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
        if (value < 0) {
            uint32_t magnitude = 0u - static_cast<uint32_t>(value);
            if (magnitude >= limit) {
                magnitude = limit;
            }
            c.eax = 0u - magnitude;
        } else {
            uint32_t magnitude = static_cast<uint32_t>(value);
            if (magnitude >= limit) {
                magnitude = limit;
            }
            c.eax = magnitude;
        }
        car->UnknownB4 = static_cast<uint16_t>(c.eax);
        car->Flags1CE = static_cast<uint8_t>(car->Flags1CE | 0x20u);
        step = StepTail;
    } else if ((car->FlagsCA & 1u) != 0u) {
        gp4::call_regs(0x00428a53, c);
        gp4::call_regs(0x00428efb, c);
        step = StepCommon;
    } else if ((car->Flags273 & 0x80u) == 0u) {
        step = StepFree;
    } else if ((car->FlagsE5 & 1u) == 0u || (car->FlagsCB & 0x10u) == 0u) {
        step = StepCommon;
    } else {
        car->Flags1CE = static_cast<uint8_t>(car->Flags1CE | 0x20u);
        flags = logicFlags8(car->Flags1CE);
        car->UnknownB4 = 0;
        step = StepEnd;
    }

    if (step == StepFree) {
        car->Flags9D = static_cast<uint8_t>(car->Flags9D | 0x40u);
        c.eax = car->Unknown1A0;
        car->Unknown1A4 = c.eax;
        c.eax = (c.eax & 0xffffff00u) | car->Unknown294;
        car->Unknown272 = static_cast<uint8_t>(c.eax);
        car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0x7fu);
        if ((car->Flags71 & 8u) != 0u) {
            car->FlagsFE = static_cast<uint8_t>(car->FlagsFE | 0x80u);
        }
        car->Flags9D = static_cast<uint8_t>(car->Flags9D | 0x40u);
        car->Unknown174 = 8;
        step = StepCommon;
    }
    if (step == StepCommon) {
        car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 0x04u);
        gp4::call_regs(0x004298e4, c);
        step = StepTail;
    }
    if (step == StepTail) {
        flags = logicFlags8(car->FlagsE5 & 1u);
        if ((car->FlagsE5 & 1u) == 0u) {
            c.eax = (c.eax & 0xffff0000u) | car->UnknownB4;
            car->Unknown180 = static_cast<uint16_t>(c.eax);
            flags = shl16Flags(c.eax);
            c.eax = c.eax << 16;
            car->Unknown3C = c.eax;
            car->Unknown1F8 = 0;
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00428fd3, FUN_00428fd3, "regs(esi:ptr[0x420]:layout=car_004129fa+car_00431ffb+car_0040362e+car_00426fa5+rec_004295fe+car_004035c3+car_004035f7+car_0042b17e+car_00428bea, edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); cases=500; globals=fuzz:i32+layout=state_00412af5+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+state_004059ef+geom_008943a0+rec_00433c4c+state_0042882e+state_00435945+state_004295fe+ref_004298e4+tune_00428dc1+rng_0066d8c0+state_0043c4d9+gain_00428bea+seg_00428bea")
