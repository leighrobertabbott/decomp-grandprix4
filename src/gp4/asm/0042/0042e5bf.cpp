#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test byte, imm): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 16-bit logic result (test ax, ax): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
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

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042e5bf
// Picks the car that esi is overtaking (or being overtaken by) into the pointer 0x00604280 and asks
// 0x0042e4f8 whether it still holds. Either the stored target (+0x1bc, a byte offset into the car array
// 0x0066fef0, valid until the due time +0x1d4 passes the clock 0x0066d814) is re-checked with the side taken
// from +0x21d, or, with no target, the car linked at +0x114 (state 8) / +0x118 (state 0x10) of the state
// word +0xea becomes the target when it points back at esi (its +0x124) with bit 4 of its +0x122 set.
// 0x00604280 is cleared when no target survives; the flags are those of the last compare.
void __cdecl FUN_0042e5bf(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t eax = GP4_FIELD(uint16_t, car, 0x1bc);
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;
    uint32_t flags = 0;
    bool call = false;

    if (eax != 0xffffu) {
        const uint32_t due = GP4_FIELD(uint32_t, car, 0x1d4);
        bool pending = false;
        if (due != 0u) {
            const uint32_t negDue = 0u - due;
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            flags = addFlags32(negDue, clock);
            pending = static_cast<int32_t>(negDue + clock) < 0;
        } else {
            flags = logicFlags32(0u);
        }
        if (pending) {
            ebx = eax + 0x0066fef0u;
            GP4_GLOBAL(uint32_t, 0x00604280) = ebx;
            edx = 0;
            flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x21d) & 8u);
            if ((GP4_FIELD(uint8_t, car, 0x21d) & 8u) != 0u) {
                edx = 0xffffffffu;
                flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x21d) & 0x10u);
                if ((GP4_FIELD(uint8_t, car, 0x21d) & 0x10u) == 0u) {
                    edx = 1;
                }
            }
            call = true;
        } else {
            GP4_FIELD(uint16_t, car, 0x1bc) = 0xffff;
            GP4_GLOBAL(uint32_t, 0x00604280) = 0;
        }
    } else {
        uint16_t state = GP4_FIELD(uint16_t, car, 0xea);
        flags = logicFlags16(state);
        if ((state & 0x8000u) != 0u) {
            flags = cmpFlags16(0u, state);      // neg ax
            state = static_cast<uint16_t>(0u - state);
        }
        eax = state;
        flags = cmpFlags16(state, 8u);
        bool linked = false;
        if (state == 8u) {
            ebx = GP4_FIELD(uint32_t, car, 0x114);
            flags = logicFlags32(ebx);
            linked = ebx != 0u;
        } else {
            flags = cmpFlags16(state, 0x10u);
            if (state == 0x10u) {
                ebx = GP4_FIELD(uint32_t, car, 0x118);
                flags = logicFlags32(ebx);
                linked = ebx != 0u;
            }
        }
        if (linked) {
            uint8_t* const other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(ebx));
            flags = logicFlags8(GP4_FIELD(uint8_t, other, 0x122) & 0x10u);
            if ((GP4_FIELD(uint8_t, other, 0x122) & 0x10u) != 0u) {
                flags = cmpFlags32(r->esi, GP4_FIELD(uint32_t, other, 0x124));
                if (r->esi == GP4_FIELD(uint32_t, other, 0x124)) {
                    GP4_GLOBAL(uint32_t, 0x00604280) = ebx;
                    edx = 0;
                    call = true;
                }
            }
        }
        if (!call) {
            GP4_GLOBAL(uint32_t, 0x00604280) = 0;
        }
    }

    uint32_t ecx = r->ecx;
    uint32_t edi = r->edi;
    uint32_t ebp = r->ebp;
    if (call) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = eax;
        c.ebx = ebx;
        c.edx = edx;
        c.eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
        gp4::call_regs(0x0042e4f8, c);
        eax = c.eax;
        ebx = c.ebx;
        ecx = c.ecx;
        edx = c.edx;
        edi = c.edi;
        ebp = c.ebp;
        flags = c.eflags;
        if ((c.eflags & 1u) != 0u) {
            GP4_GLOBAL(uint32_t, 0x00604280) = 0;
        }
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->edi = edi;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0042e5bf, FUN_0042e5bf, "regs(ecx:u32,edi:u32,ebp:u32,esi:rec[0x66fef0,0x41c,0..21]) -> (eax,ebx,ecx,edx,edi,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+link_0042e5bf; maxinsn=400000")
