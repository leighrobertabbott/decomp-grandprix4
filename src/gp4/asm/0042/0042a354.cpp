#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* carAt(uint32_t reg) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(reg));
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline uint16_t abs16(uint16_t value) {
    return (value & 0x8000u) != 0u ? static_cast<uint16_t>(0u - value) : value;
}

static inline bool carrySet(const gp4::Regs& c) {
    return (c.eflags & 1u) != 0u;
}

// 0x0042a50d (stc) / 0x0042a520 (clc): byte 0x604157 = 0 / 0xff, the candidate 0x60427c cleared. CF is
// replaced, the other flags are those of the last flag writer.
static void finish(gp4::Regs* r, const gp4::Regs& c, uint32_t flags, bool accepted) {
    GP4_GLOBAL(uint8_t, 0x00604157) = accepted ? 0xffu : 0u;
    GP4_GLOBAL(uint32_t, 0x0060427c) = 0u;
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~gp4::flags::CF) | (accepted ? 0u : gp4::flags::CF));
}

enum Route { kCheck, kAppend, kRetry, kCommit };

// esi = car, ebx = the car it is considering (published in 0x60427c while this runs), edi = the car's
// segment. CF = 0 accepts ebx as the car to follow / pass (0x604157 = 0xff), CF = 1 rejects it.
//  - +0x9c bit 1 (already committed): ebx must have +0xcc bit 4, and when +0x203 bit 3 is set it must be the
//    car held at +0x114, at +0x130 (+0x134 bit 6) or at +0x12c (+0x134 bit 7); then +0x203 |= 8,
//    +0x114 = ebx, 0x0042a0fb (key 0x900) and 0x0042e6a5, and +0x21d bit 3 is cleared.
//  - otherwise +0x8d bit 1 rejects, as does |+0xea| == 8 with another car than +0x114 and +0xce bit 4.
//    When ebx has +0x122 bit 4 and names this car at +0x124 with |segment word +0x18| below [0x603ec8],
//    the side 0x60414c/0x604154 (0x80/0x80, or 0x40/0 with ebx +0xfe bit 4) is tried with 0x0042e97e;
//    else 0x0042a120 (CF: append with key 0x2000 and accept) and 0x0042e97e, retried once at lane 0 when
//    it chose a nonzero lane 0x60414e. A clear 0x0042e97e then needs 0x0042a1ff: 0x0042e6a5, +0x21d bit 3
//    cleared, +0xea = lane, 0x0042a0fb (0x900), accept. A failed one appends ebx (key 0x4000, or the
//    caller's edx when ebx +0x203 bit 0 short-cuts it) when ebx has +0xcc bit 4, and rejects.
// FUNCTION: GP4 0x0042a354
void __cdecl FUN_0042a354(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t* car = carAt(c.esi);
    uint8_t* other = carAt(c.ebx);
    GP4_GLOBAL(uint32_t, 0x0060427c) = c.ebx;

    if ((car[0x9c] & 2u) != 0u) {
        if ((other[0xcc] & 0x10u) == 0u) {
            finish(r, c, gp4::flags::logic8(0u), false);
            return;
        }
        if ((car[0x203] & 8u) != 0u && c.ebx != GP4_FIELD(uint32_t, car, 0x114)) {
            const bool second = (car[0x134] & 0x40u) != 0u && c.ebx == GP4_FIELD(uint32_t, car, 0x130);
            if (!second) {
                if ((car[0x134] & 0x80u) == 0u) {
                    finish(r, c, gp4::flags::logic8(0u), false);
                    return;
                }
                const uint32_t first = GP4_FIELD(uint32_t, car, 0x12c);
                if (c.ebx != first) {
                    finish(r, c, gp4::flags::sub32(c.ebx, first), false);
                    return;
                }
            }
        }
        car[0x203] = static_cast<uint8_t>(car[0x203] | 8u);
        GP4_FIELD(uint32_t, car, 0x114) = c.ebx;
        c.edx = 0x900u;
        gp4::call_regs(0x0042a0fb, c);
        gp4::call_regs(0x0042e6a5, c);
        car = carAt(c.esi);
        car[0x21d] = static_cast<uint8_t>(car[0x21d] & 0xf7u);
        finish(r, c, gp4::flags::logic8(car[0x21d]), true);
        return;
    }

    if ((car[0x8d] & 2u) != 0u) {
        finish(r, c, gp4::flags::logic8(2u), false);
        return;
    }
    const uint16_t lane = abs16(GP4_FIELD(uint16_t, car, 0xea));
    setLow16(c.eax, lane);
    if (lane == 8u && c.ebx != GP4_FIELD(uint32_t, car, 0x114) && (car[0xce] & 0x10u) != 0u) {
        finish(r, c, gp4::flags::logic8(0x10u), false);
        return;
    }
    setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603e70));

    Route route = kCheck;
    if ((other[0x122] & 0x10u) != 0u) {
        if (c.esi == GP4_FIELD(uint32_t, other, 0x124)) {
            const uint16_t sway = abs16(GP4_FIELD(uint16_t, carAt(c.edi), 0x18));
            setLow16(c.edx, sway);
            if (static_cast<int16_t>(sway) < GP4_GLOBAL(int16_t, 0x00603ec8)) {
                GP4_GLOBAL(uint8_t, 0x0060414c) = 0x80u;
                GP4_GLOBAL(uint8_t, 0x00604154) = 0x80u;
                if ((other[0xfe] & 0x10u) != 0u) {
                    GP4_GLOBAL(uint8_t, 0x0060414c) = 0x40u;
                    GP4_GLOBAL(uint8_t, 0x00604154) = 0u;
                }
                GP4_GLOBAL(uint16_t, 0x0060414e) = 0u;
                gp4::call_regs(0x0042e97e, c);
                route = carrySet(c) ? kRetry : kCommit;
            }
        }
    } else if ((other[0x203] & 1u) != 0u) {
        route = kAppend;
    }

    if (route == kCheck) {
        gp4::call_regs(0x0042a120, c);
        if (carrySet(c)) {
            c.edx = 0x2000u;
            gp4::call_regs(0x0042a0fb, c);
            finish(r, c, c.eflags, true);
            return;
        }
        gp4::call_regs(0x0042e97e, c);
        if (!carrySet(c)) {
            route = kCommit;
        } else if (GP4_GLOBAL(uint16_t, 0x0060414e) == 0u || GP4_GLOBAL(uint8_t, 0x0060414c) == 0u) {
            route = kRetry;
        } else {
            GP4_GLOBAL(uint16_t, 0x0060414e) = 0u;
            gp4::call_regs(0x0042e97e, c);
            route = carrySet(c) ? kRetry : kCommit;
        }
    }

    if (route == kRetry) {
        // 0x0042a4b1
        if ((carAt(c.ebx)[0xcc] & 0x10u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0060414c) = 0u;
            finish(r, c, gp4::flags::logic8(0u), false);
            return;
        }
        c.edx = 0x4000u;
        route = kAppend;
    }

    if (route == kAppend) {
        // 0x0042a501
        gp4::call_regs(0x0042a0fb, c);
        GP4_GLOBAL(uint8_t, 0x0060414c) = 0u;
        finish(r, c, c.eflags, false);
        return;
    }

    // 0x0042a4c1
    gp4::call_regs(0x0042a1ff, c);
    if (carrySet(c)) {
        finish(r, c, c.eflags, false);
        return;
    }
    gp4::call_regs(0x0042e6a5, c);
    car = carAt(c.esi);
    car[0x21d] = static_cast<uint8_t>(car[0x21d] & 0xf7u);
    GP4_FIELD(uint16_t, car, 0xea) = GP4_GLOBAL(uint16_t, 0x0060414e);
    c.edx = 0x900u;
    gp4::call_regs(0x0042a0fb, c);
    GP4_GLOBAL(uint8_t, 0x0060414c) = 0u;
    finish(r, c, c.eflags, true);
}
GP4_IMPL(0x0042a354, FUN_0042a354, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, ebx:rec[0x66fef0,0x41c,0..5], esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+list_0066fe3c+search_0066d82c+state_004059ef+carx_0042fe2f+state_00426091+gap_0042a533+state_0042efee+flags_00432dcf+state_0041104f+state_0042b740+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+list_0042dfd2+chain_0042dc79+bounds_0042e330+clock_0066d814+link_0042e5bf+walk_0042e06a+walk_0042e191+cursor_006043fc+link_0042a120")
