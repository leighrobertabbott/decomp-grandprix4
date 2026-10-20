#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fields of the car records at 0x0066fef0 (22 records, stride 0x41c) this routine reads / writes.
struct Unknown_00430aef_Car {
    uint8_t Unknown000[0x9c];
    uint8_t Flags9C;          // +0x9c  bit 1 selects 0x0040dc8a for the gap
    uint8_t Unknown09D[0xce - 0x9d];
    uint8_t FlagsCE;          // +0xce  bit 7
    uint8_t Unknown0CF[0xe2 - 0xcf];
    int16_t UnknownE2;        // +0xe2  along-track position word
    uint8_t Unknown0E4[0xea - 0xe4];
    int16_t UnknownEA;        // +0xea  lane / state word
    uint8_t Unknown0EC[0xfe - 0xec];
    uint8_t FlagsFE;          // +0xfe  bits 6 and 4
    uint8_t Unknown0FF[0x114 - 0xff];
    uint32_t Unknown114;      // +0x114 other car (0 = none)
    uint8_t Unknown118[0x122 - 0x118];
    uint8_t Flags122;         // +0x122 bit 3
    uint8_t Unknown123;
    uint32_t Unknown124;      // +0x124 linked car
    uint8_t Unknown128[0x161 - 0x128];
    uint8_t Flags161;         // +0x161 bit 7
    uint8_t Unknown162[0x1bc - 0x162];
    uint16_t Unknown1BC;      // +0x1bc car byte offset from 0x66fef0, or 0xffff
    uint8_t Unknown1BE[0x1d4 - 0x1be];
    uint32_t Unknown1D4;      // +0x1d4 due time (0 = none)
};

static inline Unknown_00430aef_Car* carOf(uint32_t address) {
    return reinterpret_cast<Unknown_00430aef_Car*>(static_cast<uintptr_t>(address));
}

static inline uint32_t addressOf(const uint8_t* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// FUNCTION: GP4 0x00430aef
// Per-car AI step on the car esi. If the due time +0x1d4 has not yet been reached and +0x1bc
// names a car, that car is pushed with key 0x900 (0x0042a0fb). When bit 6 of +0xfe is set, the
// other car +0x114 is released (0x0042e314, +0x114 = 0) unless it is still relevant (linked
// through +0x124 / +0x1bc, or the absolute gap is below [0x603b32] and the lane test fails);
// then, unless bit 7 of +0xce, 0x0042e97e is asked about lane 0 and on success the lane word
// +0xea is set and 0x0042f9f8 runs. Finally 0x0042ff39 runs: on carry bit 7 of +0x161 is
// cleared, otherwise 0x0042f16d and 0x0042f47f run. Registers and flags are those of the
// callees (or of the final AND).
void __cdecl FUN_00430aef(gp4::Regs* r) {
    uint8_t* const cars = GP4_ARRAY(uint8_t, 0x0066fef0);
    Unknown_00430aef_Car* car = carOf(r->esi);

    if (car->Unknown1BC != 0xffffu && car->Unknown1D4 != 0u
        && static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - car->Unknown1D4) < 0) {
        r->ebx = addressOf(cars + car->Unknown1BC);
        r->edx = 0x900u;
        gp4::call_regs(0x0042a0fb, *r);
        car = carOf(r->esi);
    }

    if ((car->FlagsFE & 0x40u) != 0u) {
        bool keep = false;    // 0x430c43: straight to 0x430c73
        r->ebx = car->Unknown114;
        if (r->ebx != 0u) {
            Unknown_00430aef_Car* other = carOf(r->ebx);
            if ((car->Flags122 & 8u) != 0u && r->ebx == car->Unknown124) {
                keep = true;
            } else {
                bool release;
                if ((car->FlagsFE & 0x10u) == 0u) {
                    if ((other->FlagsFE & 0x40u) != 0u) {
                        release = (other->FlagsFE & 0x10u) == 0u;
                    } else {
                        release = other->UnknownEA == 8 || other->UnknownEA == 0x10;
                    }
                } else {
                    if ((other->FlagsFE & 0x40u) != 0u) {
                        release = (other->FlagsFE & 0x10u) != 0u;
                    } else {
                        release = other->UnknownEA == -8 || other->UnknownEA == -0x10;
                    }
                }
                if (!release) {
                    // 0x430bd1
                    const uint16_t link = car->Unknown1BC;
                    r->edx = link;
                    if (link != 0xffffu) {
                        r->edx = addressOf(cars + link);
                        keep = r->edx == r->ebx;
                    }
                    if (!keep) {
                        uint16_t gap;
                        if ((car->Flags9C & 2u) == 0u && (other->Flags9C & 2u) == 0u) {
                            gap = static_cast<uint16_t>(other->UnknownE2 - car->UnknownE2);
                            r->eax = (r->eax & 0xffff0000u) | gap;
                        } else {
                            gp4::call_regs(0x0040dc8a, *r);
                            car = carOf(r->esi);
                            other = carOf(r->ebx);
                            gap = static_cast<uint16_t>(r->eax);
                        }
                        if ((other->Flags122 & 8u) != 0u && r->esi == other->Unknown124) {
                            keep = true;
                        } else {
                            if (static_cast<int16_t>(gap) < 0) {
                                gap = static_cast<uint16_t>(0u - gap);
                                r->eax = (r->eax & 0xffff0000u) | gap;
                            }
                            keep = static_cast<int16_t>(gap) < GP4_GLOBAL(int16_t, 0x00603b32);
                        }
                    }
                }
                if (!keep) {
                    gp4::call_regs(0x0042e314, *r);
                    car = carOf(r->esi);
                    car->Unknown114 = 0u;
                }
            }
        }
        if (!keep && (car->FlagsCE & 0x80u) == 0u) {
            GP4_GLOBAL(int16_t, 0x0060414e) = 0;
            gp4::call_regs(0x0042e97e, *r);
            if ((r->eflags & gp4::flags::CF) == 0u) {
                car = carOf(r->esi);
                car->UnknownEA = GP4_GLOBAL(int16_t, 0x0060414e);
                gp4::call_regs(0x0042f9f8, *r);
            }
        }
    }

    gp4::call_regs(0x0042ff39, *r);
    if ((r->eflags & gp4::flags::CF) != 0u) {
        car = carOf(r->esi);
        car->Flags161 = static_cast<uint8_t>(car->Flags161 & 0x7fu);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(car->Flags161));
    } else {
        gp4::call_regs(0x0042f16d, *r);
        gp4::call_regs(0x0042f47f, *r);
    }
}
GP4_IMPL(0x00430aef, FUN_00430aef, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+state_00426091+gap_0042a533+state_0042da35+state_0042fa52+state_0066d8c0+state_0042ecbf+state_0042efee+state_004059ef+state_0042f3c7+cars_0042f47f+list_0066fe3c+cursor_006043fc; maxinsn=400000; cases=600")
