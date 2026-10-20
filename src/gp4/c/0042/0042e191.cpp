#include <gp4/gp4.h>

// A car record (array at 0x66fef0, 0x41c bytes); only the fields this routine reads or writes.
struct Unknown_0042e191_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x68 - 0x20];
    int16_t Unknown68;
    uint8_t Unknown6A[0x90 - 0x6a];
    int16_t Unknown90;
    uint8_t Unknown92[2];
    int16_t Unknown94;
    uint8_t Unknown96[0x9c - 0x96];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xb0 - 0x9d];
    int16_t UnknownB0;
    uint8_t UnknownB2[0xca - 0xb2];
    uint8_t FlagsCA;
    uint8_t UnknownCB;
    uint8_t FlagsCC;
    uint8_t UnknownCD;
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xe2 - 0xcf];
    int16_t UnknownE2;
    uint8_t UnknownE4[0xea - 0xe4];
    int16_t UnknownEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
};

static inline uint32_t withLow16(uint32_t entry, uint32_t low) {
    return (entry & 0xffff0000u) | (low & 0xffffu);
}

static inline uint16_t abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

// One entry of the car-pointer table 0x00607968 whose lap distance (+0x68) does not exceed bp.
static void considerCandidate(const gp4::Regs* entry, uint32_t index, uint32_t& eax, uint32_t ebx, uint32_t& edi) {
    const int16_t limit = static_cast<int16_t>(entry->ebp);
    Unknown_0042e191_Car* self = reinterpret_cast<Unknown_0042e191_Car*>(static_cast<uintptr_t>(entry->esi));
    Unknown_0042e191_Car* car = reinterpret_cast<Unknown_0042e191_Car*>(static_cast<uintptr_t>(ebx));

    if ((car->FlagsCC & 0x40u) != 0u) {
        uint16_t ax = abs16(static_cast<uint16_t>(car->Unknown90));
        ax = static_cast<uint16_t>(ax - 0x280u);
        eax = withLow16(eax, ax);
        bool mark = static_cast<int16_t>(ax) < car->UnknownB0;
        if (!mark) {
            ax = abs16(static_cast<uint16_t>(car->Unknown90 - car->Unknown94));
            eax = withLow16(eax, ax);
            mark = static_cast<int16_t>(ax) < 0x280;
        }
        if (mark) {
            self->FlagsCA |= 0x40u;
        }
    }

    gp4::Regs distance = *entry;
    distance.st_in = 0;
    distance.st_out = 0;
    distance.eax = eax;
    distance.ebx = ebx;
    distance.edx = index;
    distance.edi = self->Segment1C;
    gp4::call_regs(0x0042dd4b, distance);
    eax = distance.eax;
    if ((distance.eflags & 1u) != 0u) {
        return;
    }
    if (static_cast<int32_t>(eax) < 0) {
        eax = 0u - eax;
    }

    uint32_t edx = index;
    bool adjusted = false;
    if ((self->FlagsCE & 0x10u) != 0u && self->UnknownEA == 0 && (self->FlagsFE & 0x40u) == 0u) {
        uint16_t word = static_cast<uint16_t>(self->Unknown90 - self->Unknown94);
        word ^= GP4_GLOBAL(uint16_t, 0x00604404);
        edx = withLow16(index, word);
        if (static_cast<int16_t>(word) >= 0) {
            word = abs16(static_cast<uint16_t>(self->Unknown90 - self->Unknown94));
            word = static_cast<uint16_t>(word + GP4_GLOBAL(uint16_t, 0x00603ae8));
            edx = withLow16(index, word);
            if (static_cast<int16_t>(eax) >= static_cast<int16_t>(word)) {
                return;
            }
            adjusted = true;
        }
    }
    if (!adjusted && static_cast<int16_t>(eax) >= static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603ae8))) {
        return;
    }

    gp4::Regs held = *entry;
    held.st_in = 0;
    held.st_out = 0;
    held.eax = eax;
    held.ebx = ebx;
    held.edx = edx;
    gp4::call_regs(0x0042dee4, held);

    if (ebx == 0u || ebx == GP4_GLOBAL(uint32_t, 0x00604280) || limit < car->Unknown68) {
        return;
    }

    int32_t scan = static_cast<int32_t>(edi) - 1;
    eax = static_cast<uint32_t>(scan);
    while (scan >= 0) {
        if (ebx == GP4_ARRAY(uint32_t, 0x00604294)[scan]) {
            return;
        }
        --scan;
        eax = static_cast<uint32_t>(scan);
    }

    if ((self->Flags9C & 2u) == 0u && (car->Flags9C & 2u) == 0u) {
        eax = withLow16(eax, static_cast<uint16_t>(car->UnknownE2 - self->UnknownE2));
    } else {
        gp4::Regs lane = *entry;
        lane.st_in = 0;
        lane.st_out = 0;
        lane.eax = eax;
        lane.ebx = ebx;
        lane.edx = edx;
        gp4::call_regs(0x0040dc8a, lane);
        eax = lane.eax;
    }
    if (static_cast<int16_t>(eax) < static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603b28))) {
        return;
    }
    GP4_ARRAY(uint32_t, 0x00604294)[edi] = ebx;
    ++edi;
}

// FUNCTION: GP4 0x0042e191
// esi = the reference car, edi = the number of entries already in the pointer list 0x00604294,
// bp = a 16-bit lap-distance limit. Walks the car-pointer table 0x00607968 from index [0x00607960] up to
// [0x00607964] (exclusive): a car whose +0x68 is above bp just advances [0x00607960]; otherwise, after the
// 0x0042dd4b distance test (CF = rejected) and the 0x0042dee4 release of [0x00604280], the car is appended
// to the list unless it is already there or its lane gap is below the word 0x00603b28. Leaves eax / ebx =
// the last values, edx = [0x00607964], edi = the new list length, and the flags of the final
// `cmp edx, [0x607964]` (equal: ZF | PF).
void __cdecl FUN_0042e191(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edi = r->edi;
    uint32_t index = GP4_GLOBAL(uint32_t, 0x00607960);

    if (index != GP4_GLOBAL(uint32_t, 0x00607964)) {
        do {
            ebx = GP4_ARRAY(uint32_t, 0x00607968)[index];
            const Unknown_0042e191_Car* car = reinterpret_cast<const Unknown_0042e191_Car*>(static_cast<uintptr_t>(ebx));
            if (static_cast<int16_t>(r->ebp) < car->Unknown68) {
                GP4_GLOBAL(uint32_t, 0x00607960) = GP4_GLOBAL(uint32_t, 0x00607960) + 1u;
            } else {
                considerCandidate(r, index, eax, ebx, edi);
            }
            ++index;
        } while (index != GP4_GLOBAL(uint32_t, 0x00607964));
    }

    r->eax = eax;
    r->ebx = ebx;
    r->edx = index;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042e191, FUN_0042e191, "regs(eax:u32, ebx:u32, ecx:u32, esi:rec[0x66fef0,0x41c,0..21], edi:i32[0..8], ebp:i32[-4096..4096]) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+walk_0042e06a+walk_0042e191; maxinsn=400000; cases=800")
