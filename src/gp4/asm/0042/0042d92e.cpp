#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / or / xor): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Car body fields read here: the flag byte +0x9c and the word +0xe2 (the signed gap key).
struct Unknown_0042d92e_Car {
    uint8_t Unknown00[0x9c];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    uint16_t KeyE2;
};

// Source record (edi): signed byte +0x10 and 7-bit amount +0x50.
struct Unknown_0042d92e_Source {
    uint8_t Unknown00[0x10];
    uint8_t Byte10;
    uint8_t Unknown11[0x50 - 0x11];
    uint8_t Byte50;
};

// Ends the routine: eax is restored (push / pop), the other registers are whatever the last callee
// left, CF is forced (stc / clc) over the callee's flags.
static inline void finish(gp4::Regs* r, const gp4::Regs& c, uint32_t carry) {
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u) | carry;
}

static inline uint32_t carryOf(const gp4::Regs& c) { return c.eflags & 1u; }

// Pairwise evaluation of the car at esi against the one at ebx for the source record edi: clears the three
// flag bytes 0x00604150..52, stores the gap word of the two cars (word +0xe2 difference, or 0x0040dc8a when
// either is flagged) at 0x00603e70, runs the gap lookup 0x0042b17e with the step 0x160 and then the source
// checks 0x0042d4e6 / 0x0042d782 / 0x0042d7de for the byte +0x10 (bit 0x80 of the flag bytes: passed) and,
// when the amount +0x50 is at least 4, again for the flipped byte (bit 0x40). CF = 1 when the first
// check or the final 0x0042d883 selection fails.
// FUNCTION: GP4 0x0042d92e
void __cdecl FUN_0042d92e(gp4::Regs* r) {
    const Unknown_0042d92e_Car* const self = reinterpret_cast<const Unknown_0042d92e_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042d92e_Car* const other = reinterpret_cast<const Unknown_0042d92e_Car*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00604151) = 0;
    GP4_GLOBAL(uint8_t, 0x00604152) = 0;
    GP4_GLOBAL(uint8_t, 0x00604150) = 0;

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

    c.eax = 0x160u;
    gp4::call_regs(0x0042b17e, c);

    c.ecx = 0x10u;
    gp4::call_regs(0x0042d4e6, c);
    if (carryOf(c) != 0u) {
        finish(r, c, 1u);
        return;
    }

    GP4_GLOBAL(uint8_t, 0x00604150) = 0x80u;
    uint8_t dl = reinterpret_cast<const Unknown_0042d92e_Source*>(static_cast<uintptr_t>(c.edi))->Byte10;
    c.edx = (c.edx & 0xffffff00u) | dl;
    GP4_GLOBAL(uint8_t, 0x00604153) = dl;
    gp4::call_regs(0x0042d782, c);
    if (carryOf(c) == 0u) {
        GP4_GLOBAL(uint8_t, 0x00604152) = 0x80u;
    }
    gp4::call_regs(0x0042d7de, c);
    if (carryOf(c) == 0u) {
        GP4_GLOBAL(uint8_t, 0x00604151) = 0x80u;
    }

    dl = reinterpret_cast<const Unknown_0042d92e_Source*>(static_cast<uintptr_t>(c.edi))->Byte50;
    c.edx = (c.edx & 0xffffff00u) | dl;
    c.eflags = (c.eflags & ~0x8d5u) | cmpFlags8(dl, 4u);
    if (dl >= 4u) {
        c.ecx = 0x50u;
        gp4::call_regs(0x0042d4e6, c);
        if (carryOf(c) == 0u) {
            const uint8_t merged = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00604150) | 0x40u);
            GP4_GLOBAL(uint8_t, 0x00604150) = merged;
            dl = static_cast<uint8_t>(reinterpret_cast<const Unknown_0042d92e_Source*>(static_cast<uintptr_t>(c.edi))->Byte10 ^ 0x80u);
            c.edx = (c.edx & 0xffffff00u) | dl;
            c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(dl);
            GP4_GLOBAL(uint8_t, 0x00604153) = dl;
            gp4::call_regs(0x0042d782, c);
            if (carryOf(c) == 0u) {
                const uint8_t second = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00604152) | 0x40u);
                GP4_GLOBAL(uint8_t, 0x00604152) = second;
                c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(second);
            }
            gp4::call_regs(0x0042d7de, c);
            if (carryOf(c) == 0u) {
                const uint8_t third = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00604151) | 0x40u);
                GP4_GLOBAL(uint8_t, 0x00604151) = third;
                c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(third);
            }
        }
    }

    gp4::call_regs(0x0042d883, c);
    finish(r, c, carryOf(c));
}
GP4_IMPL(0x0042d92e, FUN_0042d92e, "regs(ecx:u32,edx:u32,edi:ptr[0xb1]:bytes,ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677) -> (ebx,ecx,edx,esi,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e")
