#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a logic result (test / or / and): CF = OF = AF = 0. `signBit` is the sign bit of the operand width.
static inline uint32_t logicFlags(uint32_t result, uint32_t mask, uint32_t signBit) {
    return parityFlag8(result)
        | ((result & mask) == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u);
}
static inline uint32_t logicFlags8(uint32_t result) { return logicFlags(result, 0xffu, 0x80u); }
static inline uint32_t logicFlags16(uint32_t result) { return logicFlags(result, 0xffffu, 0x8000u); }
static inline uint32_t logicFlags32(uint32_t result) { return logicFlags(result, 0xffffffffu, 0x80000000u); }

// Flags of the 16-bit `left - right` (cmp / sub / neg on words).
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Fields of the 0x41c-byte car records at 0x0066fef0 that this routine reads / writes.
struct Unknown_00430339_Car {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
    uint8_t Unknown6A[0x7a - 0x6a];
    uint8_t Flags7A;
    uint8_t Unknown7B[0x90 - 0x7b];
    int16_t Unknown90;
    uint8_t Unknown92[0x94 - 0x92];
    uint16_t Unknown94;
    uint8_t Unknown96[0x9a - 0x96];
    int16_t Unknown9A;
    uint8_t Flags9C;
    uint8_t Unknown9D[0xcb - 0x9d];
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xea - 0xcc];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0x114 - 0xec];
    uint32_t Unknown114;
    uint8_t Unknown118[0x162 - 0x118];
    uint8_t Flags162;
    uint8_t Unknown163[0x203 - 0x163];
    uint8_t Flags203;
    uint8_t Unknown204[0x230 - 0x204];
    uint8_t Flags230;
    uint8_t Unknown231[0x26b - 0x231];
    uint8_t Flags26B;
};

// esi = this car. Two ways in: with bit 1 of +0x9c set (and bit 3 of +0x203) the car's linked partner
// +0x114 (-> ebx) is tracked: when its offset difference (0x0040dc8a) reaches word 0x00603b4a the pair
// (ebx, 0x900) is pushed with 0x0042a0fb, otherwise the link and the +0x203 bit 3 are cleared. With the bit
// clear a car with a speed word +0x94 whose |+0x90| (sign flipped by byte 0x00868582 bit 7) reaches
// word 0x00603b1a, bit 3 of +0x162 set and bit 7 of +0x230 clear returns CF = 0 at once; else, with +0x230
// bit 7 and +0x26b bit 7 clear/set respectively, the partner 0x0043b974 finds may get bit 1 of +0xcb
// set. Everything else ends in 0x0042ff39 and CF = 1. (The original re-tests bit 1 of +0x9c in the
// linked branch, which is always set there, so its direct word difference is never taken.)
// FUNCTION: GP4 0x00430339
void __cdecl FUN_00430339(gp4::Regs* r) {
    Unknown_00430339_Car* self = reinterpret_cast<Unknown_00430339_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t flags = r->eflags;

    flags = logicFlags8(self->Flags9C & 2u);
    if ((self->Flags9C & 2u) != 0u) {
        flags = logicFlags8(self->Flags203 & 8u);
        if ((self->Flags203 & 8u) == 0u) {
            goto callTail;
        }
        s.ecx = self->Unknown114;
        flags = logicFlags32(s.ecx);
        if (s.ecx != 0u) {
            s.ebx = s.ecx;
            flags = logicFlags8(self->Flags9C & 2u);
            s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d5u);
            gp4::call_regs(0x0040dc8a, s);
            {
                const uint16_t difference = static_cast<uint16_t>(s.eax);
                const uint16_t limit = GP4_GLOBAL(uint16_t, 0x00603b4a);
                flags = subFlags16(difference, limit);
                if (static_cast<int16_t>(difference) >= static_cast<int16_t>(limit)) {
                    s.edx = 0x900u;
                    s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d5u);
                    gp4::call_regs(0x0042a0fb, s);
                    flags = s.eflags;
                    goto callTail;
                }
            }
        }
        self->Flags203 = static_cast<uint8_t>(self->Flags203 & 0xf7u);
        flags = logicFlags8(self->Flags203);
        self->Unknown114 = 0u;
        goto callTail;
    }

    flags = logicFlags16(self->Unknown94);
    if (self->Unknown94 != 0u) {
        uint16_t speed = static_cast<uint16_t>(self->Unknown90);
        s.eax = (s.eax & 0xffff0000u) | speed;
        flags = logicFlags8(GP4_GLOBAL(uint8_t, 0x00868582));
        if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
            flags = subFlags16(0u, speed);
            speed = static_cast<uint16_t>(0u - speed);
            s.eax = (s.eax & 0xffff0000u) | speed;
        }
        const uint16_t threshold = GP4_GLOBAL(uint16_t, 0x00603b1a);
        flags = subFlags16(speed, threshold);
        if (static_cast<int16_t>(speed) >= static_cast<int16_t>(threshold)) {
            flags = logicFlags8(self->Flags162 & 8u);
            if ((self->Flags162 & 8u) != 0u) {
                flags = logicFlags8(self->Flags230 & 0x80u);
                if ((self->Flags230 & 0x80u) != 0u) {
                    goto callTail;
                }
                // clc; ret
                s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d4u);
                goto finish;
            }
        }
    }

    flags = logicFlags8(self->Flags230 & 0x80u);
    if ((self->Flags230 & 0x80u) != 0u) {
        goto callTail;
    }
    flags = logicFlags8(self->Flags26B & 0x80u);
    if ((self->Flags26B & 0x80u) == 0u) {
        goto callTail;
    }

    s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d5u);
    gp4::call_regs(0x0043b974, s);
    flags = s.eflags;
    if ((s.eflags & 1u) == 0u) {
        goto callTail;
    }

    {
        Unknown_00430339_Car* other = reinterpret_cast<Unknown_00430339_Car*>(static_cast<uintptr_t>(s.ebx));

        flags = logicFlags8(other->Flags26B & 8u);
        if ((other->Flags26B & 8u) != 0u) {
            goto callTail;
        }
        flags = subFlags16(other->UnknownEA, 0x20u);
        if (other->UnknownEA != 0x20u) {
            goto callTail;
        }
        const uint16_t gap = static_cast<uint16_t>(other->Unknown68 - self->Unknown68);
        s.eax = (s.eax & 0xffff0000u) | gap;
        flags = subFlags16(static_cast<uint16_t>(other->Unknown68), static_cast<uint16_t>(self->Unknown68));
        if ((gap & 0x8000u) != 0u) {
            goto callTail;
        }
        const uint16_t reach = GP4_GLOBAL(uint16_t, 0x00603afc);
        flags = subFlags16(gap, reach);
        if (static_cast<int16_t>(gap) >= static_cast<int16_t>(reach)) {
            goto callTail;
        }
        flags = logicFlags8(other->Flags7A & 0x20u);
        if ((other->Flags7A & 0x20u) != 0u) {
            goto callTail;
        }
        flags = subFlags16(static_cast<uint16_t>(self->Unknown9A), 0x0b00u);
        if (self->Unknown9A < 0x0b00) {
            goto callTail;
        }
        self->FlagsCB = static_cast<uint8_t>(self->FlagsCB | 0x02u);
        flags = logicFlags8(self->FlagsCB);
    }

callTail:
    s.eflags = (s.eflags & ~0x8d5u) | (flags & 0x8d5u);
    gp4::call_regs(0x0042ff39, s);
    s.eflags |= 1u;   // stc

finish:
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->ebp = s.ebp;  // preserved
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00430339, FUN_00430339, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); cases=800; globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+list_0066fe3c+search_0066d82c+state_004059ef+carx_0042fe2f+state_00426091+gap_0042a533+state_0042efee+flags_00432dcf+state_0041104f+state_0042b740+flags_00430449+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+list_0042dfd2+chain_0042dc79+bounds_0042e330+clock_0066d814+link_0042e5bf+walk_0042e06a+walk_0042e191+cursor_006043fc+bias_00430339")
