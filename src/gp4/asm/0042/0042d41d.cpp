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
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 16-bit logic result (test ax, 0xffff): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `sub` / `cmp left, right`.
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

// Car body fields read here.
struct Unknown_0042d41d_Car {
    uint8_t Unknown00[0x90];
    int16_t Lateral90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xb0 - 0x9d];
    int16_t LimitB0;
    uint8_t UnknownB2[0xcb - 0xb2];
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xdc - 0xcc];
    int16_t OffsetDC;
    uint8_t UnknownDE[0xe2 - 0xde];
    uint16_t KeyE2;
    uint8_t UnknownE4[0x174 - 0xe4];
    uint16_t Mode174;
    uint8_t Unknown176[0x273 - 0x176];
    uint8_t Flags273;
};

// |lateral| - 0x40 against the limit word +0xb0 (signed): true when the car is clear of the edge.
static inline bool clearOfEdge(gp4::Regs& c, uint32_t& flags, int16_t lateral, int16_t limit) {
    uint16_t ax = static_cast<uint16_t>(lateral);
    flags = logicFlags16(ax);
    if ((ax & 0x8000u) != 0u) {
        ax = static_cast<uint16_t>(0u - ax);
    }
    flags = subFlags16(ax, 0x40u);
    ax = static_cast<uint16_t>(ax - 0x40u);
    flags = subFlags16(ax, static_cast<uint16_t>(limit));
    c.eax = (c.eax & 0xffff0000u) | ax;
    return static_cast<int16_t>(ax) < limit;
}

// Pair test between the car at esi and the one at ebx: stores the gap word of the two (word +0xe2
// difference, or 0x0040dc8a when either car has bit 1 of +0x9c) at 0x00603e70 and, when both cars are
// clear of the edge by 0x40 and the gap lookup 0x0042beea finds a result, records the lateral target
// of esi: the steering word 0x0042b740 produces from [0x00603d9c] is stored at 0x006077c5, 0x004037a9
// updates the car record, and +0xdc = word - [0x00604402] + +0x90, +0x174 = 8 and bits 0x80 / 0x10
// of +0x273 (0x80 of +0xcb) are set. CF = 0 on that path, CF = 1 otherwise; the flags are those of the
// last compare / or.
// FUNCTION: GP4 0x0042d41d
void __cdecl FUN_0042d41d(gp4::Regs* r) {
    Unknown_0042d41d_Car* const self = reinterpret_cast<Unknown_0042d41d_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042d41d_Car* const other = reinterpret_cast<const Unknown_0042d41d_Car*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t flags = 0;

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
    GP4_GLOBAL(uint16_t, 0x00603e70) = gap;

    bool found = false;
    if (clearOfEdge(c, flags, self->Lateral90, self->LimitB0) &&
        clearOfEdge(c, flags, other->Lateral90, other->LimitB0)) {
        c.eflags = (c.eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x0042beea, c);
        flags = c.eflags & 0x8d5u;
        if ((c.eflags & 1u) == 0u) {
            found = true;
        }
    }

    uint32_t carry = 1;
    if (found) {
        c.ecx = (c.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00603d9c);
        gp4::call_regs(0x0042b740, c);
        const uint32_t steer = c.ecx;
        GP4_GLOBAL(uint16_t, 0x006077c5) = static_cast<uint16_t>(steer);

        c.ecx = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(steer)));
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x006077c7)));
        gp4::call_regs(0x004037a9, c);

        const uint16_t cx = static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint16_t>(steer) - GP4_GLOBAL(uint16_t, 0x00604402)) +
                                                  static_cast<uint16_t>(self->Lateral90));
        c.ecx = (steer & 0xffff0000u) | cx;
        self->OffsetDC = static_cast<int16_t>(cx);
        self->Mode174 = 8;
        self->FlagsCB = static_cast<uint8_t>(self->FlagsCB | 0x80u);
        self->Flags273 = static_cast<uint8_t>(self->Flags273 | 0x80u);
        self->Flags273 = static_cast<uint8_t>(self->Flags273 | 0x10u);
        flags = logicFlags8(self->Flags273);
        carry = 0;
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x0042d41d, FUN_0042d41d, "regs(eax:u32,ecx:u32,edx:u32,ebp:u32,ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677) -> (eax,ecx,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e")
