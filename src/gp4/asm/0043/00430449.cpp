#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Flags of the 16-bit `left - right` (cmp / sub on words).
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
struct Unknown_00430449_Car {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
    uint8_t Unknown6A[0x7a - 0x6a];
    uint8_t Flags7A;
    uint8_t Flags7B;
    uint8_t Unknown7C[0x9c - 0x7c];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xc8 - 0x9d];
    uint16_t UnknownC8;
    uint8_t UnknownCA[0xcb - 0xca];
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xe2 - 0xcc];
    uint16_t UnknownE2;
    uint8_t UnknownE4[0xea - 0xe4];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0x102 - 0xec];
    uint16_t Unknown102;
    uint8_t Unknown104[0x110 - 0x104];
    uint32_t Unknown110;
    uint8_t Unknown114[0x230 - 0x114];
    uint8_t Flags230;
    uint8_t Unknown231[0x25f - 0x231];
    uint8_t Flags25F;
    uint8_t Unknown260[0x26b - 0x260];
    uint8_t Flags26B;
};

// esi = this car. Unless bit 6 of +0x7b or bit 1 of +0x25f is set: with bit 7 of +0x230 set, 0x0042fe2f
// looks for a blocking car ahead (found: bit 1 of +0xcb is set); otherwise 0x0043b974 picks the partner
// car in ebx (a free car whose +0x7c is the paired slot) and, when its +0x26b bit 3 is clear, word +0xea
// is 0x20, the lap-position gap +0x68 (partner - this) is in [0, word 0x00603afc) and its +0x7a bit 5
// is clear, the offset difference (word +0xe2 of the two, or the 0x0040dc8a value when either has
// +0x9c bit 1) goes to word 0x00603e70; when it is at least word 0x00603b28 the partner is stored at
// +0x110 and, with [0x00603e8c] held at 0x4000, 0x0042a6dc, 0x0042a533 and 0x0042f05a update the words
// +0xc8 and +0x102 and the factor (the saved [0x00603e8c] is restored). eax, ebx, ecx, edx and the
// flags are what the last executed step leaves.
// FUNCTION: GP4 0x00430449
void __cdecl FUN_00430449(gp4::Regs* r) {
    Unknown_00430449_Car* self = reinterpret_cast<Unknown_00430449_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t flags = r->eflags;

    flags = logicFlags8(self->Flags7B & 0x40u);
    if ((self->Flags7B & 0x40u) != 0u) {
        goto finish;
    }
    flags = logicFlags8(self->Flags25F & 2u);
    if ((self->Flags25F & 2u) != 0u) {
        goto finish;
    }
    flags = logicFlags8(self->Flags230 & 0x80u);
    if ((self->Flags230 & 0x80u) != 0u) {
        gp4::call_regs(0x0042fe2f, s);
        flags = s.eflags;
        if ((s.eflags & 1u) == 0u) {
            self->FlagsCB = static_cast<uint8_t>(self->FlagsCB | 0x02u);
            flags = logicFlags8(self->FlagsCB);
        }
        goto finish;
    }

    gp4::call_regs(0x0043b974, s);
    flags = s.eflags;
    if ((s.eflags & 1u) == 0u) {
        goto finish;
    }

    {
        Unknown_00430449_Car* other = reinterpret_cast<Unknown_00430449_Car*>(static_cast<uintptr_t>(s.ebx));

        flags = logicFlags8(other->Flags26B & 8u);
        if ((other->Flags26B & 8u) != 0u) {
            goto finish;
        }
        flags = subFlags16(other->UnknownEA, 0x20u);
        if (other->UnknownEA != 0x20u) {
            goto finish;
        }
        const uint16_t gap = static_cast<uint16_t>(other->Unknown68 - self->Unknown68);
        s.eax = (s.eax & 0xffff0000u) | gap;
        flags = subFlags16(static_cast<uint16_t>(other->Unknown68), static_cast<uint16_t>(self->Unknown68));
        if ((gap & 0x8000u) != 0u) {
            goto finish;
        }
        const uint16_t reach = GP4_GLOBAL(uint16_t, 0x00603afc);
        flags = subFlags16(gap, reach);
        if (static_cast<int16_t>(gap) >= static_cast<int16_t>(reach)) {
            goto finish;
        }
        flags = logicFlags8(other->Flags7A & 0x20u);
        if ((other->Flags7A & 0x20u) != 0u) {
            goto finish;
        }

        uint16_t offset;
        if ((self->Flags9C & 2u) == 0u && (other->Flags9C & 2u) == 0u) {
            offset = static_cast<uint16_t>(other->UnknownE2 - self->UnknownE2);
            flags = subFlags16(other->UnknownE2, self->UnknownE2);
            s.eax = (s.eax & 0xffff0000u) | offset;
        } else {
            gp4::call_regs(0x0040dc8a, s);
            offset = static_cast<uint16_t>(s.eax);
            flags = s.eflags;
        }
        GP4_GLOBAL(uint16_t, 0x00603e70) = offset;
        const uint16_t floorValue = GP4_GLOBAL(uint16_t, 0x00603b28);
        flags = subFlags16(offset, floorValue);
        if (static_cast<int16_t>(offset) < static_cast<int16_t>(floorValue)) {
            goto finish;
        }

        self->Unknown110 = s.ebx;
        const uint32_t savedRate = GP4_GLOBAL(uint32_t, 0x00603e8c);
        GP4_GLOBAL(uint32_t, 0x00603e8c) = 0x4000u;
        gp4::call_regs(0x0042a6dc, s);
        self->UnknownC8 = static_cast<uint16_t>(s.ecx);
        gp4::call_regs(0x0042a533, s);
        self->Unknown102 = static_cast<uint16_t>(s.ecx);
        gp4::call_regs(0x0042f05a, s);
        flags = s.eflags;
        GP4_GLOBAL(uint32_t, 0x00603e8c) = savedRate;
    }

finish:
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00430449, FUN_00430449, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); cases=800; globals=fuzz:i32+layout=list_0066fe3c+search_0066d82c+pair_0042e8e6+cars_0066fef0+state_004059ef+carx_0042fe2f+state_00426091+gap_0042a533+state_0042efee+flags_00432dcf+state_0041104f+flags_00430449")
