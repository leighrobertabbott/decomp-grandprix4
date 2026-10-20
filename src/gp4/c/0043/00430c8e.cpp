#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
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

// Flags of the 16-bit `test value, 0xffff` (CF = OF = AF = 0).
static inline uint32_t testFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

struct Unknown_00430c8e_Car {
    uint8_t Unknown00[0xea];
    int16_t StateEA;
};

// Calls the original routine `addr` with the running register block.
static inline void step(gp4::Regs& s, uint32_t addr) {
    gp4::call_regs(addr, s);
}

// FUNCTION: GP4 0x00430c8e
// Per-car state dispatch on the word +0xea (after the 0x0042f6ee update): 0 -> 0x00430aef;
// +-8 -> 0x004307a9; +-0x10 -> 0x0043052f; 0x18 -> 0x00430339; 0x20 -> 0x00430449; anything
// else leaves with CF set. A routine returning CF set stops the chain (fall-through to the next
// handler only on CF clear); the common tail runs 0x0042f121, 0x0042fb30, 0x0042fd08 and
// 0x0043190d and returns CF clear.
void __cdecl FUN_00430c8e(gp4::Regs* r) {
    const Unknown_00430c8e_Car* car = reinterpret_cast<const Unknown_00430c8e_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    step(s, 0x0042f6ee);

    const int16_t state = car->StateEA;
    int16_t magnitude = state;
    s.ecx = (s.ecx & 0xffff0000u) | static_cast<uint16_t>(state);

    // Handler selection. `next` is the first handler to run: 1 = 0x430339, 2 = 0x430449,
    // 3 = 0x43052f, 4 = 0x4307a9, 5 = 0x430aef.
    int next;
    if (state == 0) {
        s.eflags = (s.eflags & ~0x8d5u) | testFlags16(0);
        next = 5;
    } else {
        if (magnitude < 0) {
            magnitude = static_cast<int16_t>(-magnitude);
            s.ecx = (s.ecx & 0xffff0000u) | static_cast<uint16_t>(magnitude);
        }
        if (magnitude == 8) {
            s.eflags = (s.eflags & ~0x8d5u) | cmpFlags16(static_cast<uint16_t>(magnitude), 8u);
            next = 4;
        } else if (magnitude == 0x10) {
            s.eflags = (s.eflags & ~0x8d5u) | cmpFlags16(static_cast<uint16_t>(magnitude), 0x10u);
            next = 3;
        } else if (state == 0x18) {
            s.eflags = (s.eflags & ~0x8d5u) | cmpFlags16(static_cast<uint16_t>(state), 0x18u);
            next = 1;
        } else if (state == 0x20) {
            s.eflags = (s.eflags & ~0x8d5u) | cmpFlags16(static_cast<uint16_t>(state), 0x20u);
            next = 2;
        } else {
            // cmp word [esi+0xea], 0x20 ; jne -> stc ; ret
            r->eax = s.eax;
            r->ebx = s.ebx;
            r->ecx = s.ecx;
            r->edx = s.edx;
            r->edi = s.edi;
            r->ebp = s.ebp;
            r->eflags = (r->eflags & ~0x8d5u) | cmpFlags16(static_cast<uint16_t>(state), 0x20u) | 1u;
            return;
        }
    }

    bool done = false;
    if (next == 1) {
        step(s, 0x00430339);
        if ((s.eflags & 1u) != 0u) {
            done = true;
        } else {
            next = 2;
        }
    }
    if (!done && next == 2) {
        step(s, 0x00430449);
        done = true;
    }
    if (!done && next == 3) {
        step(s, 0x0043052f);
        if ((s.eflags & 1u) != 0u) {
            done = true;
        } else {
            next = 4;
        }
    }
    if (!done && next == 4) {
        step(s, 0x004307a9);
        if ((s.eflags & 1u) != 0u) {
            done = true;
        } else {
            next = 5;
        }
    }
    if (!done && next == 5) {
        step(s, 0x00430aef);
    }

    step(s, 0x0042f121);
    step(s, 0x0042fb30);
    step(s, 0x0042fd08);
    step(s, 0x0043190d);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->ebp = s.ebp;
    // clc: CF cleared, the other flags are those of the last flag writer (0x0043190d)
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d4u);
}
GP4_IMPL(0x00430c8e, FUN_00430c8e, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+list_0066fe3c+search_0066d82c+state_004059ef+carx_0042fe2f+state_00426091+gap_0042a533+state_0042efee+flags_00432dcf+state_0041104f+state_0042b740+flags_00430449+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+list_0042dfd2+chain_0042dc79+bounds_0042e330+clock_0066d814+link_0042e5bf+walk_0042e06a+walk_0042e191+cursor_006043fc+bias_00430339+state_0042da35+state_0042fa52+state_0066d8c0+state_0042ecbf+state_0042f3c7+cars_0042f47f+state_0042b8bb+cand_0042e783+mode_00430c8e")
// v8: rerun of v7 for diagnostics
