#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Car record fields this routine reads (tests, words) and the flag byte it updates.
struct Unknown_0042f7fb_Car {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Word90;
    uint8_t Unknown92[0x94 - 0x92];
    int16_t Word94;
    uint8_t Unknown96[0x9c - 0x96];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xce - 0x9d];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xe5 - 0xcf];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xea - 0xe6];
    uint16_t StateEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x26b - 0xff];
    uint8_t Flags26B;
};

// Sets or clears bit 4 of the car's +0xce flag byte: clear (the car's lateral value is within the
// threshold [0x603bfe] of the wanted one) only for a car with bit 2 and without bit 0 of +0xe5 whose
// target offset (taken from the track side, the lane state +0xea, or the lane offset of 0x0042b88b /
// the scaled state) differs from the lateral word +0x90 by less than the threshold. The flags are those
// of the final or / and on the byte; eax / edx hold the intermediate target (ax = |target - +0x90| ).
// FUNCTION: GP4 0x0042f7fb
void __cdecl FUN_0042f7fb(gp4::Regs* r) {
    Unknown_0042f7fb_Car* const car = reinterpret_cast<Unknown_0042f7fb_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    bool near = false;
    if ((car->FlagsE5 & 4u) != 0u && (car->FlagsE5 & 1u) == 0u) {
        bool compute = true;
        if ((car->Flags8D & 2u) != 0u) {
            if ((car->Flags9C & 0x10u) != 0u || (car->Flags26B & 0x80u) != 0u) {
                compute = false;
            } else {
                const uint16_t state = car->StateEA;
                c.eax = (c.eax & 0xffff0000u) | state;
                if (state == 0x18u) {
                    c.eax &= 0xffff0000u;
                } else if (state == 0x20u) {
                    c.eax = GP4_GLOBAL(uint32_t, 0x00627874);
                    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
                        c.eax = GP4_GLOBAL(uint32_t, 0x00627870);
                    }
                    if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
                        c.eax = (c.eax & 0xffff0000u) | static_cast<uint16_t>(0u - c.eax);
                    }
                } else {
                    compute = false;
                }
            }
        } else if ((car->FlagsFE & 0x40u) != 0u) {
            gp4::call_regs(0x0042b88b, c);
        } else {
            const int64_t product = static_cast<int64_t>(static_cast<int16_t>(car->StateEA)) *
                                    static_cast<int64_t>(GP4_GLOBAL(uint16_t, 0x00603ae0));
            c.edx = static_cast<uint32_t>(product >> 32);
            const uint32_t low = static_cast<uint32_t>(product);
            c.eax = (low >> 3) | (c.edx << 29);
            c.eax = (c.eax & 0xffff0000u) | static_cast<uint16_t>(c.eax + static_cast<uint16_t>(car->Word94));
        }

        if (compute) {
            uint16_t ax = static_cast<uint16_t>(static_cast<uint16_t>(c.eax) - static_cast<uint16_t>(car->Word90));
            if ((ax & 0x8000u) != 0u) ax = static_cast<uint16_t>(0u - ax);
            c.eax = (c.eax & 0xffff0000u) | ax;
            near = static_cast<int16_t>(ax) < GP4_GLOBAL(int16_t, 0x00603bfe);
        }
    }

    if (near) {
        car->FlagsCE = static_cast<uint8_t>(car->FlagsCE & 0xefu);
    } else {
        car->FlagsCE = static_cast<uint8_t>(car->FlagsCE | 0x10u);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(car->FlagsCE);
}
GP4_IMPL(0x0042f7fb, FUN_0042f7fb, "regs(eax:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+pair_0042e8e6+cars_0066fef0+links_00405a12+div_00403bb6+ringw_004291f4+carf_004036ca+carea_0042f6ee")
