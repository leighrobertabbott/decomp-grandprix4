#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
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

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Car body fields read here.
struct Unknown_0042e4f8_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x9a - 0x20];
    uint16_t Speed9A;
    uint8_t Flags9C;
    uint8_t Unknown9D[0xac - 0x9d];
    int32_t OffsetAC;
    uint8_t UnknownB0[0xe2 - 0xb0];
    uint16_t KeyE2;
    uint8_t UnknownE4[0x284 - 0xe4];
    int32_t Limit284;
};

// Overtake check of the car at esi against the one at ebx for the side edx (-1, 1 or 0 = let
// 0x0042e478 pick): stores edx at 0x00603dc0 and, when the gap word (+0xe2 difference, or 0x0040dc8a when
// either car has bit 1 of +0x9c) is at least the word 0x00603dba, esi is faster (signed +0x9a) and the pair
// lookup 0x0042dd4b succeeds, runs 0x0042e494 and 0x0042e330 (their eax / edx go to 0x00603dc4 /
// 0x00603dc8), picks the side and compares |+0xac + that value| with the limit +0x284: CF = 1 when it is
// larger, 0 for every other exit. edi is saved around 0x0042dd4b; the flags are those of the last compare.
// FUNCTION: GP4 0x0042e4f8
void __cdecl FUN_0042e4f8(gp4::Regs* r) {
    const Unknown_0042e4f8_Car* const self = reinterpret_cast<const Unknown_0042e4f8_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042e4f8_Car* const other = reinterpret_cast<const Unknown_0042e4f8_Car*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t flags = 0;
    uint32_t carry = 0;

    GP4_GLOBAL(uint32_t, 0x00603dc0) = r->edx;

    uint16_t gap;
    if ((self->Flags9C & 2u) == 0u && (other->Flags9C & 2u) == 0u) {
        gap = static_cast<uint16_t>(other->KeyE2 - self->KeyE2);
        c.eax = (c.eax & 0xffff0000u) | gap;
    } else {
        const uint32_t test = ((self->Flags9C & 2u) != 0u) ? (self->Flags9C & 2u) : (other->Flags9C & 2u);
        c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(test);
        gp4::call_regs(0x0040dc8a, c);
        gap = static_cast<uint16_t>(c.eax);
    }

    const int16_t threshold = GP4_GLOBAL(int16_t, 0x00603dba);
    flags = cmpFlags16(gap, static_cast<uint16_t>(threshold));
    if (!(static_cast<int16_t>(gap) < threshold)) {
        const uint16_t selfSpeed = self->Speed9A;
        c.edx = (c.edx & 0xffff0000u) | selfSpeed;
        flags = cmpFlags16(selfSpeed, other->Speed9A);
        if (static_cast<int16_t>(selfSpeed) > static_cast<int16_t>(other->Speed9A)) {
            c.edi = self->Segment1C;
            c.eflags = (c.eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x0042dd4b, c);
            c.edi = r->edi;
            flags = c.eflags & 0x8d5u;
            if ((c.eflags & 1u) == 0u) {
                gp4::call_regs(0x0042e494, c);
                gp4::call_regs(0x0042e330, c);
                GP4_GLOBAL(uint32_t, 0x00603dc4) = c.eax;
                GP4_GLOBAL(uint32_t, 0x00603dc8) = c.edx;
                if (GP4_GLOBAL(uint32_t, 0x00603dc0) == 0u) {
                    gp4::call_regs(0x0042e478, c);
                    GP4_GLOBAL(uint32_t, 0x00603dc0) = c.eax;
                }

                const uint32_t picked = (GP4_GLOBAL(int32_t, 0x00603dc0) == -1) ? GP4_GLOBAL(uint32_t, 0x00603dc4)
                                                                                : GP4_GLOBAL(uint32_t, 0x00603dc8);
                int32_t offset = static_cast<int32_t>(static_cast<uint32_t>(self->OffsetAC) + picked);
                if (offset < 0) {
                    offset = static_cast<int32_t>(0u - static_cast<uint32_t>(offset));
                }
                c.eax = static_cast<uint32_t>(offset);
                flags = cmpFlags32(static_cast<uint32_t>(offset), static_cast<uint32_t>(self->Limit284));
                if (offset > self->Limit284) {
                    carry = 1;
                }
            }
        }
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x0042e4f8, FUN_0042e4f8, "regs(eax:u32,ecx:u32,edx:i32[-1..1],edi:u32,ebp:u32,ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677) -> (eax,ecx,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+bounds_0042e330")
