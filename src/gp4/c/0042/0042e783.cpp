#include <gp4/gp4.h>

// A car record (array at 0x66fef0); only the fields used here are named.
struct Unknown_0042e783_Car {
    uint8_t Unknown00[0x68];
    int16_t LapDistance68;
    uint8_t Unknown6A[0x9c - 0x6a];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xce - 0x9d];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xe2 - 0xcf];
    int16_t TimeE2;
    uint8_t UnknownE4[0xea - 0xe4];
    int16_t StateEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t SideFE;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// cmp left, right (32-bit) arithmetic flags.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t setLow16(uint32_t full, uint32_t low) {
    return (full & 0xffff0000u) | (low & 0xffffu);
}

static inline const Unknown_0042e783_Car* carAt(uint32_t address) {
    return reinterpret_cast<const Unknown_0042e783_Car*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0042e783
// Collects the cars that esi has to watch into the pointer list 0x00604294 (count [0x00604290]) and
// picks the best one. 0x42e5bf refreshes the target [0x00604280]; the lap-distance limit bp is
// word [esi+0x68] + word 0x00603afc. The car chosen through the side bits of byte [esi+0xfe] (bit 0x40:
// 0x6043dc / 0x6043d8 before the chain walk 0x42dc79, 0x604278 / 0x604274 after it; clear: the
// car-pointer tables 0x006041a4 / 0x00604224 at the state word [esi+0xea]) is appended when it is not
// [0x00604280], not listed yet, within bp laps and at least word 0x00603b28 ahead (word [+0xe2]
// difference, or 0x40dc8a when either car has bit 1 of +0x9c). 0x42dfd2 and 0x42e191 (and 0x42e06a
// unless bit 4 of [esi+0xce] is set, with bp = [esi+0x68] + word 0x00603b00) add more. Result: ebx
// = edx = the best listed car, CF clear; stc when the list is empty. With several cars each is scored
// by 0x42a792 (cx; capped at word 0x00623e04) and the lowest wins, ties going to the one 0x42e6fc
// does not reject. edi is restored; ebx/ecx/edx/eax/ebp leak out of the callees and the flags are
// those of the last compare with CF replaced.
void __cdecl FUN_0042e783(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0042e5bf, c);
    const uint32_t savedEdi = c.edi;

    const Unknown_0042e783_Car* subject = carAt(c.esi);
    const uint16_t limit = static_cast<uint16_t>(subject->LapDistance68 + GP4_GLOBAL(uint16_t, 0x00603afc));
    c.ebp = setLow16(c.ebp, limit);
    c.edi = 0;
    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(subject->StateEA));

    if ((subject->SideFE & 0x40u) == 0u) {
        c.ebx = GP4_GLOBAL(uint32_t, 0x006041a4u + c.ecx);
    } else if ((subject->SideFE & 0x10u) == 0u) {
        c.ebx = GP4_GLOBAL(uint32_t, 0x006043dc);
    } else {
        c.ebx = GP4_GLOBAL(uint32_t, 0x006043d8);
    }
    gp4::call_regs(0x0042dc79, c);

    if ((subject->SideFE & 0x40u) == 0u) {
        c.ebx = GP4_GLOBAL(uint32_t, 0x00604224u + c.ecx);
    } else if ((subject->SideFE & 0x10u) == 0u) {
        c.ebx = GP4_GLOBAL(uint32_t, 0x00604278);
    } else {
        c.ebx = GP4_GLOBAL(uint32_t, 0x00604274);
    }

    if (c.ebx != 0u && c.ebx != GP4_GLOBAL(uint32_t, 0x00604280)
        && static_cast<int16_t>(c.ebp) >= carAt(c.ebx)->LapDistance68) {
        bool listed = false;
        for (int32_t index = static_cast<int32_t>(c.edi) - 1; index >= 0; --index) {
            if (c.ebx == GP4_ARRAY(uint32_t, 0x00604294)[index]) {
                listed = true;
                break;
            }
        }
        if (!listed) {
            c.eax = 0xffffffffu;
            if ((subject->Flags9C & 2u) != 0u || (carAt(c.ebx)->Flags9C & 2u) != 0u) {
                gp4::call_regs(0x0040dc8a, c);
            } else {
                c.eax = setLow16(c.eax, static_cast<uint16_t>(carAt(c.ebx)->TimeE2 - subject->TimeE2));
            }
            if (static_cast<int16_t>(c.eax) >= GP4_GLOBAL(int16_t, 0x00603b28)) {
                GP4_ARRAY(uint32_t, 0x00604294)[c.edi] = c.ebx;
                c.edi += 1u;
            }
        }
    }

    gp4::call_regs(0x0042dfd2, c);
    if ((subject->FlagsCE & 0x10u) != 0u) {
        gp4::call_regs(0x0042e191, c);
    } else {
        const uint32_t savedEbp = c.ebp;
        c.ebp = setLow16(c.ebp, static_cast<uint16_t>(subject->LapDistance68 + GP4_GLOBAL(uint16_t, 0x00603b00)));
        gp4::call_regs(0x0042e191, c);
        c.ebp = savedEbp;
        gp4::call_regs(0x0042e06a, c);
    }
    GP4_GLOBAL(uint32_t, 0x00604290) = c.edi;
    c.edi = savedEdi;
    c.edx = GP4_GLOBAL(uint32_t, 0x00604294);

    const int32_t count = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00604290));
    if (count < 1) {
        // stc after "cmp [0x604290], 1".
        r->eax = c.eax;
        r->ebx = c.ebx;
        r->ecx = c.ecx;
        r->edx = c.edx;
        r->edi = c.edi;
        r->ebp = c.ebp;
        r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(static_cast<uint32_t>(count), 1u) | 1u;
        return;
    }
    if (count != 1) {
        const uint32_t outerEdi = c.edi;
        c.edi = 0;
        c.ebp = setLow16(c.ebp, 0x7fffu);
        do {
            c.ebx = GP4_ARRAY(uint32_t, 0x00604294)[c.edi];
            const uint32_t savedEbpLoop = c.ebp;
            const uint32_t savedEdx = c.edx;
            GP4_GLOBAL(uint16_t, 0x00623e08) = GP4_GLOBAL(uint16_t, 0x00623e04);
            gp4::call_regs(0x0042a792, c);
            c.edx = savedEdx;
            c.ebp = savedEbpLoop;

            const int16_t score = static_cast<int16_t>(c.ecx);
            const int16_t best = static_cast<int16_t>(c.ebp);
            bool take = false;
            if (score <= best) {
                if (score < best) {
                    take = true;
                } else {
                    gp4::call_regs(0x0042e6fc, c);
                    take = (c.eflags & 1u) != 0u;
                }
            }
            if (take) {
                c.ebp = setLow16(c.ebp, c.ecx);
                c.edx = c.ebx;
            }
            c.edi += 1u;
        } while (c.edi != GP4_GLOBAL(uint32_t, 0x00604290));
        c.edi = outerEdi;
    }

    c.ebx = c.edx;
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    // The last compare was equal (ZF | PF); clc leaves CF clear.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042e783, FUN_0042e783, "regs(ecx:u32, edi:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+link_0042e5bf+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+state_00426091+gap_0042a533+cand_0042e783; maxinsn=400000; cases=300")
