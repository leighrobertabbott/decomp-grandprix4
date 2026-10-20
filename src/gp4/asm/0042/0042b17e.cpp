#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Car record fields this routine tests (all bit tests of single bytes).
struct Unknown_0042b17e_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x8d - 0x20];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x9c - 0x8e];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe5 - 0x9d];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x1ce - 0xe6];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x26b - 0x1cf];
    uint8_t Flags26B;
};

// `test byte, mask`: sets PF/ZF/SF from the result and clears CF/OF/AF; true when any tested bit is set.
static inline bool testBits(uint32_t& flags, uint8_t value, uint8_t mask) {
    const uint32_t result = static_cast<uint32_t>(value & mask);
    flags = parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
    return result != 0u;
}

// Where the checks of the two car records send the common tail.
enum Route {
    kRouteSetCarry = 0,   // 0x0042b279: stc
    kRouteSegments = 1,   // 0x0042b260: segment lookup (0x0042b563) unless bit 1 of [esi+0x9c] is set
    kRouteCompare = 2     // 0x0042b270: pairwise position comparison (0x0042b4b3)
};

// One car blocks the other (`first` has the +0x9c bit 0x10 / `second` the bit 2) or, when neither does and
// `second` is linked (+0xe5 bit 0 and +0x1ce bit 0), the two segment records are tested for overlap by
// 0x0042b10d (CF = overlapping).
static int checkPair(gp4::Regs& c, uint32_t& flags, const Unknown_0042b17e_Car* first,
                     const Unknown_0042b17e_Car* second) {
    if (testBits(flags, first->Flags9C, 0x10)) return kRouteSegments;
    if (testBits(flags, second->Flags9C, 2)) return kRouteSegments;
    if (!testBits(flags, second->FlagsE5, 1)) return kRouteSetCarry;
    if (!testBits(flags, second->Flags1CE, 1)) return kRouteSetCarry;
    c.eax = second->Segment1C;
    c.edx = first->Segment1C;
    c.eflags = (c.eflags & ~0x8d5u) | flags;
    gp4::call_regs(0x0042b10d, c);
    flags = c.eflags & 0x8d5u;
    return (c.eflags & 1u) != 0u ? kRouteSegments : kRouteSetCarry;
}

// Entry of the car-pair gap evaluation: eax = record step (0, 0x160, ...) stored to 0x00603e9c, esi / ebx
// = the two car records. Bit 1 of +0x8d selects, per car, the "behind" role; the byte at 0x00603e6e records
// the case (0xc0, 0xa0, 0x60 or 0). CF = 1 means "no result" (stc), CF = 0 that one of the lookups
// 0x0042b27b / 0x0042b318 / 0x0042b563 / 0x0042b4b3 filled the gap words; the other registers and flags are
// those the last callee leaves.
// FUNCTION: GP4 0x0042b17e
void __cdecl FUN_0042b17e(gp4::Regs* r) {
    const Unknown_0042b17e_Car* const self = reinterpret_cast<const Unknown_0042b17e_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042b17e_Car* const other = reinterpret_cast<const Unknown_0042b17e_Car*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t flags = 0;
    uint32_t carry = 1;

    GP4_GLOBAL(uint32_t, 0x00603e9c) = r->eax;
    GP4_GLOBAL(uint8_t, 0x00603e6e) = 0;

    int route = kRouteSetCarry;
    bool done = false;
    if (!testBits(flags, self->Flags8D, 2)) {
        if (!testBits(flags, other->Flags8D, 2)) {
            if (testBits(flags, self->Flags9C, 2)) {
                route = kRouteCompare;
            } else {
                c.eflags = (r->eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x0042b27b, c);
                flags = c.eflags & 0x8d5u;
                carry = 0;
                done = true;
            }
        } else {
            GP4_GLOBAL(uint8_t, 0x00603e6e) = 0xa0;
            route = checkPair(c, flags, other, self);
        }
    } else if (!testBits(flags, other->Flags8D, 2)) {
        GP4_GLOBAL(uint8_t, 0x00603e6e) = 0xc0;
        route = checkPair(c, flags, self, other);
    } else {
        if (!testBits(flags, self->Flags26B, 8) && !testBits(flags, other->Flags26B, 8)) {
            GP4_GLOBAL(uint8_t, 0x00603e6e) = 0x60;
            c.eflags = (r->eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x0042b318, c);
            flags = c.eflags & 0x8d5u;
            carry = 0;
        }
        done = true;
    }

    if (!done) {
        if (route == kRouteSegments) {
            if (testBits(flags, self->Flags9C, 2)) {
                route = kRouteCompare;
            } else {
                c.eflags = (r->eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x0042b563, c);
                flags = c.eflags & 0x8d5u;
                carry = 0;
                done = true;
            }
        }
        if (!done && route == kRouteCompare) {
            c.eflags = (r->eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x0042b4b3, c);
            flags = c.eflags & 0x8d5u;
            carry = (c.eflags & 1u) != 0u ? 0u : 1u;
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x0042b17e, FUN_0042b17e,"regs(eax:rec[0,0x160,0..3],ecx:u32,edx:u32,ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677) -> (eax,ebx,ecx,edx,esi,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e")
