#include <gp4/gp4.h>
#include <gp4/flags.h>

namespace f = gp4::flags;

static inline uint8_t ReadU8(uint32_t base, uint32_t off) { return GP4_FIELD(uint8_t, base, off); }
static inline uint16_t ReadU16(uint32_t base, uint32_t off) { return GP4_FIELD(uint16_t, base, off); }
static inline uint32_t ReadU32(uint32_t base, uint32_t off) { return GP4_FIELD(uint32_t, base, off); }
static inline uint32_t SetLow16(uint32_t reg, uint32_t value) { return (reg & 0xffff0000u) | (value & 0xffffu); }
static inline uint32_t SetLow8(uint32_t reg, uint32_t value) { return (reg & 0xffffff00u) | (value & 0xffu); }

// esi = this car (0x41c-byte record of 0x0066fef0), edi = its track segment. Lane-change state +0x10/-0x10
// of the AI (caller 0x00430c8e). With a linked car +0x118 (-> ebx) whose offset gap (word +0xe2 difference,
// or 0x0040dc8a when either car has +0x9c bit 1) reaches word 0x00603b4a, the pair is pushed with
// 0x0042a0fb (edx = 0x900) unless one of the segment / gap / due-time (+0x1d4 against the clock
// 0x0066d814) / speed (+0x9a) tests sends it to the release block. Otherwise the lane-neighbour car of the
// side given by the sign of +0xea (0x006041ac or 0x0060419c) is examined: lap-distance gap +0x68 in
// [0, word 0x00603afc), offset gap into word 0x00603e70, then 0x0042ac9c or the 0x0042d92e test link it to
// +0x118. The release block steers to lane +-8 (word 0x0060414e) through 0x0042e97e and, when that
// returns CF = 0, stores the lane in +0xea, clears +0x118 (and bit 3 of +0x135 when segment +0x51 bit 0
// is set) and returns CF = 0. Every other way ends with 0x0042ff39 and CF = 1. One register block is
// threaded through the callees.
// FUNCTION: GP4 0x0043052f
void __cdecl FUN_0043052f(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    uint32_t flags = r->eflags;
    uint16_t ax = 0;
    uint16_t dx = 0;
    uint16_t cx = 0;
    uint16_t limit = 0;
    uint16_t lane = 0;
    uint32_t due = 0;
    uint32_t remaining = 0;
    uint8_t mask = 0;

    s.ecx = ReadU32(s.esi, 0x118);
    flags = f::logic32(s.ecx);
    if (s.ecx == 0u) {
        goto neighbour;
    }
    s.ebx = s.ecx;

    flags = f::logic8(ReadU8(s.esi, 0x9c) & 2u);
    if ((ReadU8(s.esi, 0x9c) & 2u) != 0u) {
        goto linkedHelper;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x9c) & 2u);
    if ((ReadU8(s.ebx, 0x9c) & 2u) != 0u) {
        goto linkedHelper;
    }
    flags = f::sub16(ReadU16(s.ebx, 0xe2), ReadU16(s.esi, 0xe2));
    s.eax = SetLow16(s.eax, ReadU16(s.ebx, 0xe2) - ReadU16(s.esi, 0xe2));
    goto linkedGap;
linkedHelper:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0040dc8a, s);
    flags = s.eflags;
linkedGap:
    ax = static_cast<uint16_t>(s.eax);
    limit = GP4_GLOBAL(uint16_t, 0x00603b4a);
    flags = f::sub16(ax, limit);
    if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
        goto neighbour;
    }
    flags = f::logic8(ReadU8(s.esi, 0xcc) & 0x10u);
    if ((ReadU8(s.esi, 0xcc) & 0x10u) != 0u) {
        goto release;
    }
    flags = f::logic8(ReadU8(s.ebx, 0xcc) & 0x10u);
    if ((ReadU8(s.ebx, 0xcc) & 0x10u) != 0u) {
        goto pushPair;
    }
    s.edi = ReadU32(s.esi, 0x1c);
    flags = f::logic8(ReadU8(s.edi, 0x12) & 4u);
    if ((ReadU8(s.edi, 0x12) & 4u) != 0u) {
        flags = f::logic8(ReadU8(s.ebx, 0x122) & 0x10u);
        if ((ReadU8(s.ebx, 0x122) & 0x10u) != 0u) {
            flags = f::logic8(ReadU8(s.edi, 0x51) & 0x80u);
            if ((ReadU8(s.edi, 0x51) & 0x80u) == 0u) {
                goto dueCheck;
            }
            limit = GP4_GLOBAL(uint16_t, 0x00603b5c);
        } else {
            limit = GP4_GLOBAL(uint16_t, 0x00603b5a);
        }
        flags = f::sub16(ax, limit);
        if (static_cast<int16_t>(ax) >= static_cast<int16_t>(limit)) {
            goto release;
        }
        limit = GP4_GLOBAL(uint16_t, 0x00603b5e);
        flags = f::sub16(ax, limit);
        if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
            goto release;
        }
    }
dueCheck:
    due = ReadU32(s.esi, 0x1d4);
    flags = f::logic32(due);
    if (due != 0u) {
        remaining = (0u - due) + GP4_GLOBAL(uint32_t, 0x0066d814);
        flags = f::add32(0u - due, GP4_GLOBAL(uint32_t, 0x0066d814));
        if ((remaining & 0x80000000u) != 0u) {
            goto edgeCheck;
        }
    }
    limit = GP4_GLOBAL(uint16_t, 0x00603b4c);
    flags = f::sub16(ax, limit);
    if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
        goto edgeCheck;
    }
    dx = static_cast<uint16_t>(ReadU16(s.esi, 0x9a) - ReadU16(s.ebx, 0x9a));
    flags = f::sub16(ReadU16(s.esi, 0x9a), ReadU16(s.ebx, 0x9a));
    s.edx = SetLow16(s.edx, dx);
    if ((dx & 0x8000u) == 0u) {
        goto edgeCheck;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x122) & 0x10u);
    if ((ReadU8(s.ebx, 0x122) & 0x10u) == 0u) {
        goto release;
    }
    limit = GP4_GLOBAL(uint16_t, 0x00603ba6);
    flags = f::sub16(dx, limit);
    if (static_cast<int16_t>(dx) < static_cast<int16_t>(limit)) {
        goto release;
    }
edgeCheck:
    flags = f::logic16(ReadU16(s.edi, 0x1a) & 0x1000u);
    if ((ReadU16(s.edi, 0x1a) & 0x1000u) != 0u) {
        goto release;
    }
pushPair:
    s.edx = 0x900u;
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042a0fb, s);
    flags = s.eflags;
    goto tail;

neighbour:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e322, s);
    flags = s.eflags;
    mask = static_cast<uint8_t>(ReadU8(s.edi, 0x10) ^ ReadU8(s.esi, 0xeb));
    s.ecx = SetLow8(s.ecx, mask);
    flags = f::logic8(mask);
    if ((mask & 0x80u) != 0u) {
        goto release;
    }
    cx = ReadU16(s.esi, 0xea);
    s.ecx = SetLow16(s.ecx, cx);
    flags = f::logic16(cx);
    if ((cx & 0x8000u) != 0u) {
        s.ecx = GP4_GLOBAL(uint32_t, 0x0060419c);
    } else {
        s.ecx = GP4_GLOBAL(uint32_t, 0x006041ac);
    }
    flags = f::logic32(s.ecx);
    if (s.ecx == 0u) {
        goto release;
    }
    s.ebx = s.ecx;
    ax = static_cast<uint16_t>(ReadU16(s.ebx, 0x68) - ReadU16(s.esi, 0x68));
    flags = f::sub16(ReadU16(s.ebx, 0x68), ReadU16(s.esi, 0x68));
    s.eax = SetLow16(s.eax, ax);
    if ((ax & 0x8000u) != 0u) {
        goto release;
    }
    limit = GP4_GLOBAL(uint16_t, 0x00603afc);
    flags = f::sub16(ax, limit);
    if (static_cast<int16_t>(ax) >= static_cast<int16_t>(limit)) {
        goto release;
    }

    flags = f::logic8(ReadU8(s.esi, 0x9c) & 2u);
    if ((ReadU8(s.esi, 0x9c) & 2u) != 0u) {
        goto neighbourHelper;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x9c) & 2u);
    if ((ReadU8(s.ebx, 0x9c) & 2u) != 0u) {
        goto neighbourHelper;
    }
    flags = f::sub16(ReadU16(s.ebx, 0xe2), ReadU16(s.esi, 0xe2));
    s.eax = SetLow16(s.eax, ReadU16(s.ebx, 0xe2) - ReadU16(s.esi, 0xe2));
    goto neighbourGap;
neighbourHelper:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0040dc8a, s);
    flags = s.eflags;
neighbourGap:
    GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(s.eax);

    flags = f::logic8(ReadU8(s.ebx, 0xcc) & 0x10u);
    if ((ReadU8(s.ebx, 0xcc) & 0x10u) != 0u) {
        ax = GP4_GLOBAL(uint16_t, 0x00603e70);
        s.eax = SetLow16(s.eax, ax);
        limit = GP4_GLOBAL(uint16_t, 0x00603b54);
        flags = f::sub16(ax, limit);
        if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
            goto follow;
        }
        dx = static_cast<uint16_t>(ReadU16(s.esi, 0x9a) - ReadU16(s.ebx, 0x9a));
        cx = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603ba0));
        s.ecx = SetLow16(s.ecx, cx);
        dx = static_cast<uint16_t>(static_cast<int16_t>(dx) >> 4);
        s.edx = SetLow16(s.edx, dx);
        flags = f::sub16(dx, cx);
        if (static_cast<int16_t>(dx) < static_cast<int16_t>(cx)) {
            goto overtakeChecks;
        }
follow:
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x0042ac9c, s);
        flags = s.eflags;
        goto link;
    }
overtakeChecks:
    flags = f::logic8(ReadU8(s.esi, 0x122) & 0x10u);
    if ((ReadU8(s.esi, 0x122) & 0x10u) != 0u) {
        goto release;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x203) & 1u);
    if ((ReadU8(s.ebx, 0x203) & 1u) == 0u) {
        flags = f::logic8(ReadU8(s.edi, 0x51) & 0x20u);
        if ((ReadU8(s.edi, 0x51) & 0x20u) != 0u) {
            s.eflags = f::merge(s.eflags, flags);
            gp4::call_regs(0x0042e968, s);
            flags = s.eflags;
            if ((flags & f::CF) == 0u) {
                goto release;
            }
        }
    }
    flags = f::logic8(ReadU8(s.esi, 0xcb) & 4u);
    if ((ReadU8(s.esi, 0xcb) & 4u) != 0u) {
        goto release;
    }
    flags = f::sub16(ReadU16(s.esi, 0xea), 0x18u);
    if (ReadU16(s.esi, 0xea) == 0x18u) {
        goto release;
    }
    flags = f::logic16(ReadU16(s.edi, 0x1a) & 0x1000u);
    if ((ReadU16(s.edi, 0x1a) & 0x1000u) != 0u) {
        goto release;
    }
    flags = f::logic8(ReadU8(s.esi, 0x71) & 0x80u);
    if ((ReadU8(s.esi, 0x71) & 0x80u) != 0u) {
        flags = f::logic8(ReadU8(s.edi, 0x64) & 0x80u);
        if ((ReadU8(s.edi, 0x64) & 0x80u) != 0u) {
            goto release;
        }
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042d92e, s);
    flags = s.eflags;
    if ((flags & f::CF) != 0u) {
        goto release;
    }
link:
    GP4_FIELD(uint32_t, s.esi, 0x118) = s.ebx;
    goto tail;

release:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e322, s);
    flags = s.eflags;
    GP4_GLOBAL(uint16_t, 0x0060414e) = 8u;
    flags = f::logic16(ReadU16(s.esi, 0xea));
    if ((ReadU16(s.esi, 0xea) & 0x8000u) != 0u) {
        lane = GP4_GLOBAL(uint16_t, 0x0060414e);
        flags = f::neg16(lane);
        GP4_GLOBAL(uint16_t, 0x0060414e) = static_cast<uint16_t>(0u - lane);
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e97e, s);
    flags = s.eflags;
    if ((flags & f::CF) != 0u) {
        goto tail;
    }
    GP4_FIELD(uint16_t, s.esi, 0xea) = GP4_GLOBAL(uint16_t, 0x0060414e);
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e322, s);
    flags = s.eflags;
    GP4_FIELD(uint32_t, s.esi, 0x118) = 0u;
    flags = f::logic8(ReadU8(s.edi, 0x51) & 1u);
    if ((ReadU8(s.edi, 0x51) & 1u) != 0u) {
        const uint8_t bits = static_cast<uint8_t>(ReadU8(s.esi, 0x135) & 0xf7u);
        GP4_FIELD(uint8_t, s.esi, 0x135) = bits;
        flags = f::logic8(bits);
    }
    flags &= ~f::CF;   // clc
    goto finish;

tail:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042ff39, s);
    flags = s.eflags | f::CF;   // stc

finish:
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = f::merge(r->eflags, flags);
}
GP4_IMPL(0x0043052f, FUN_0043052f, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+link_0042e5bf+carf_00410d04+lap_0043b482+state_00426091+gap_0042a533+state_0042da35+state_0042fa52+state_0066d8c0+state_0042eee3+state_0042ecbf+state_0042efee+state_004059ef+state_0042f3c7+list_0066fe3c+cursor_006043fc+order_0042e97e+flags_00430449+cand_0042e783; maxinsn=4000000; cases=2500; wall=1800")
