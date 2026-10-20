#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `test` / `or` / `and` with result `result`: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of `test word ptr [m], 0xffff`.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

struct Unknown_0042aa47_Segment {
    uint8_t Unknown00[0x12];
    uint8_t Flags12;
};

struct Unknown_0042aa47_Track {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
};

struct Unknown_0042aa47_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x66 - 0x20];
    int16_t Unknown66;
    uint8_t Unknown68[0x71 - 0x68];
    uint8_t Flags71;
    uint8_t Unknown72[0x8d - 0x72];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Unknown90;
    uint8_t Unknown92[0x94 - 0x92];
    int16_t Unknown94;
    uint8_t Unknown96[0xca - 0x96];
    uint8_t FlagsCA;
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xdc - 0xcc];
    int16_t UnknownDC;
    uint16_t UnknownDE;
    uint8_t UnknownE0[0xe5 - 0xe0];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xea - 0xe6];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x174 - 0xff];
    uint16_t Unknown174;
    uint8_t Unknown176[0x1a4 - 0x176];
    uint32_t Unknown1A4;
    uint8_t Unknown1A8[0x230 - 0x1a8];
    uint8_t Flags230;
    uint8_t Unknown231[0x26b - 0x231];
    uint8_t Flags26B;
    uint8_t Unknown26C[0x272 - 0x26c];
    uint8_t Unknown272;
    uint8_t Flags273;
};

// 0x0042abd2: 0x0042a9cf decides (CF set); then edi = the car's segment, whose byte +0x12 bit 2
// selects the word [0x00603c2c] (loaded through ax) or 8 for +0x174, and the car is marked.
static inline void startCheck(gp4::Regs* r, Unknown_0042aa47_Car* car) {
    gp4::call_regs(0x0042a9cf, *r);
    if ((r->eflags & gp4::CF) == 0u) {
        return;
    }
    r->edi = car->Segment1C;
    const Unknown_0042aa47_Segment* segment =
        reinterpret_cast<const Unknown_0042aa47_Segment*>(static_cast<uintptr_t>(r->edi));
    if ((segment->Flags12 & 4u) != 0u) {
        const uint16_t value = GP4_GLOBAL(uint16_t, 0x00603c2c);
        r->eax = (r->eax & 0xffff0000u) | value;
        car->Unknown174 = value;
    } else {
        car->Unknown174 = 8;
    }
    car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 1u);
    car->FlagsCB = static_cast<uint8_t>(car->FlagsCB | 0x80u);
    setFlags(r, logicFlags8(car->FlagsCB));
}

// 0x0042abc7: only an idle word +0xea lets the start check run.
static inline void idleCheck(gp4::Regs* r, Unknown_0042aa47_Car* car) {
    const uint32_t idle = car->UnknownEA;
    if (idle != 0u) {
        setFlags(r, logicFlags16(idle));
        return;
    }
    startCheck(r, car);
}

// esi = car record, edi = a track-segment record (ring at 0x008943a0) whose byte +0x92 bits 0/1
// select the interpolator 0x004038a0 / 0x004038ec for word +0xdc (unless 0x0042ac09 computes it).
// FUNCTION: GP4 0x0042aa47
void __cdecl FUN_0042aa47(gp4::Regs* r) {
    Unknown_0042aa47_Car* car = reinterpret_cast<Unknown_0042aa47_Car*>(static_cast<uintptr_t>(r->esi));

    const uint32_t done = car->FlagsCB & 0x80u;
    if (done != 0u) {
        setFlags(r, logicFlags8(done));
        return;
    }
    if ((car->Flags71 & 0x80u) == 0u) {
        idleCheck(r, car);
        return;
    }
    car->Unknown174 = 8;

    if ((car->Flags8D & 2u) != 0u) {
        // (the original also tests byte +0x9c bit 4, but both outcomes continue here)
        car->FlagsFE = static_cast<uint8_t>(car->FlagsFE & 0x7fu);
        car->Unknown1A4 = 0x4000;
        car->Unknown272 = 1;
        car->Flags273 = static_cast<uint8_t>(car->Flags273 | 0x80u);
        car->FlagsCB = static_cast<uint8_t>(car->FlagsCB | 0x80u);
        car->FlagsCA = static_cast<uint8_t>(car->FlagsCA & 0xfeu);
        const uint32_t flag26b = car->Flags26B & 0x80u;
        if (flag26b == 0u) {
            setFlags(r, logicFlags8(flag26b));
            return;
        }
        if ((car->FlagsE5 & 1u) != 0u && (car->Flags230 & 0x80u) != 0u) {
            int32_t magnitude = car->Unknown90;
            if (magnitude < 0) {
                magnitude = -magnitude;
            }
            r->eax = static_cast<uint32_t>(magnitude);
            if (magnitude < 0x180) {
                setFlags(r, subFlags32(static_cast<uint32_t>(magnitude), 0x180u));
                return;
            }
        }
        car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 4u);
        setFlags(r, logicFlags8(car->FlagsCA));
        return;
    }

    r->eax = car->Segment1C;
    if (r->eax >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        car->UnknownDC = 0;
        car->UnknownDE = 0;
    } else {
        const Unknown_0042aa47_Track* track =
            reinterpret_cast<const Unknown_0042aa47_Track*>(static_cast<uintptr_t>(r->edi));
        if ((track->Flags92 & 1u) != 0u) {
            car->FlagsCB = static_cast<uint8_t>(car->FlagsCB | 0x10u);
            car->Flags273 = static_cast<uint8_t>(car->Flags273 | 8u);
            bool interpolate = true;
            if ((car->FlagsE5 & 1u) != 0u) {
                gp4::call_regs(0x0042ac09, *r);
                interpolate = (r->eflags & gp4::CF) != 0u;
            }
            if (interpolate) {
                const uint32_t savedEdi = r->edi;
                r->edi = car->Segment1C;
                r->eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown66));
                gp4::call_regs(0x004038a0, *r);
                r->edi = savedEdi;
                car->UnknownDC = static_cast<int16_t>(r->eax);
            }
            gp4::call_regs(0x0042a963, *r);
        } else {
            if ((track->Flags92 & 2u) == 0u) {
                idleCheck(r, car);
                return;
            }
            if ((car->Flags230 & 0x80u) == 0u) {
                startCheck(r, car);
                return;
            }
            car->FlagsCB = static_cast<uint8_t>(car->FlagsCB | 0x10u);
            car->Flags273 = static_cast<uint8_t>(car->Flags273 | 4u);
            bool interpolate = true;
            if ((car->FlagsE5 & 1u) != 0u) {
                gp4::call_regs(0x0042ac09, *r);
                interpolate = (r->eflags & gp4::CF) != 0u;
            }
            if (interpolate) {
                const uint32_t savedEdi = r->edi;
                r->edi = car->Segment1C;
                r->eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown66));
                gp4::call_regs(0x004038ec, *r);
                car->UnknownDC = static_cast<int16_t>(r->eax);
                r->edi = savedEdi;
            }
        }
    }

    car->FlagsCB = static_cast<uint8_t>(car->FlagsCB | 0x80u);
    car->FlagsCA = static_cast<uint8_t>(car->FlagsCA & 0xfeu);
    car->Flags273 = static_cast<uint8_t>(car->Flags273 | 0x80u);
    car->Flags273 = static_cast<uint8_t>(car->Flags273 | 0x20u);
    r->ecx = static_cast<uint32_t>(static_cast<int32_t>(car->UnknownDC));
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown94));
    gp4::call_regs(0x004037a9, *r);
}
GP4_IMPL(0x0042aa47, FUN_0042aa47, "regs(eax:u32, edi:rec[0x8943a0,0x160,0..25], esi:ptr[0x420]:layout=rec_0043569a+rec_0042a9cf+pos_00405cf8+lane_0042df3a+car_0043c677) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+state_00403800+div_00403bb6+words_008943a0+geom_008943a0+ring_00405a12+links_00405a12+flags_0042aa47+clock_0066d814")
