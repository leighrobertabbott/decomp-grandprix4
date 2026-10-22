#include <gp4/gp4.h>
#include <gp4/flags.h>

namespace f = gp4::flags;

static inline uint8_t ReadU8(uint32_t base, uint32_t off) { return GP4_FIELD(uint8_t, base, off); }
static inline uint16_t ReadU16(uint32_t base, uint32_t off) { return GP4_FIELD(uint16_t, base, off); }
static inline uint32_t ReadU32(uint32_t base, uint32_t off) { return GP4_FIELD(uint32_t, base, off); }
static inline uint32_t SetLow16(uint32_t reg, uint32_t value) { return (reg & 0xffff0000u) | (value & 0xffffu); }
static inline uint32_t SetLow8(uint32_t reg, uint32_t value) { return (reg & 0xffffff00u) | (value & 0xffu); }

// esi = this car (0x41c-byte record of 0x0066fef0), edi = its track segment; AI state step of the caller
// 0x00430c8e (the variant of 0x0043052f with the linked car in dword +0x114). With a linked car +0x114
// (-> ebx) whose offset gap (word +0xe2 difference, or 0x0040dc8a when either car has +0x9c bit 1) reaches
// word 0x00603b4a, the pair is pushed with 0x0042a0fb (edx = 0x900) unless one of the segment / gap /
// due-time (+0x1d4 against the clock 0x0066d814) / speed (+0x9a) tests sends it to the release block.
// Otherwise the lane-neighbour car [0x006041a4] is examined: lap-distance gap +0x68 in [0, word 0x00603afc),
// offset gap into word 0x00603e70, then 0x0042ac9c links it, or, when it is the previous car [0x006043e0]
// and passes the state tests (+0x122 bit 1, +0x203 bit 4 or the [0x7c2ea9] sign with the +0x70 order,
// +0xea == 0, the +0x1c pit test, lap gap below word 0x00603b10), it is marked (+0x122 bit 1, +0x124 =
// esi, +0xf0 = clock + 0x40 * the random byte of 0x00401462); then the 0x0042d92e test links it to +0x114.
// The release block tries the two sides (byte 0x0060414c = 0x40 / 0x80 by the sign of +0xea, then the other
// one) through 0x0042e97e; when one returns CF = 0 it stores the lane word 0x0060414e in +0xea and, unless
// +0xfe bit 6 is set, clears +0x114 and sets bit 6 of +0x135 (clearing bit 3 when segment +0x51 bit 0 is
// set), returning CF = 0. Every other way ends with 0x0042ff39 and CF = 1. One register block is threaded
// through the callees.
// FUNCTION: GP4 0x004307a9
void __cdecl FUN_004307a9(gp4::Regs* r) {
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
    uint32_t due = 0;
    uint32_t remaining = 0;
    uint8_t mask = 0;
    uint8_t order = 0;
    uint32_t segment = 0;

    s.ecx = ReadU32(s.esi, 0x114);
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
    gp4::call_regs(0x0042e314, s);
    flags = s.eflags;
    mask = static_cast<uint8_t>(ReadU8(s.edi, 0x10) ^ ReadU8(s.esi, 0xeb));
    s.ecx = SetLow8(s.ecx, mask);
    flags = f::logic8(mask);
    if ((mask & 0x80u) != 0u) {
        goto release;
    }
    s.ecx = GP4_GLOBAL(uint32_t, 0x006041a4);
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
            goto previousCar;
        }
follow:
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x0042ac9c, s);
        flags = s.eflags;
        goto link;
    }

previousCar:
    flags = f::sub32(s.ebx, GP4_GLOBAL(uint32_t, 0x006043e0));
    if (s.ebx != GP4_GLOBAL(uint32_t, 0x006043e0)) {
        goto overtakeChecks;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x122) & 2u);
    if ((ReadU8(s.ebx, 0x122) & 2u) != 0u) {
        goto overtakeChecks;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x203) & 0x10u);
    if ((ReadU8(s.ebx, 0x203) & 0x10u) == 0u) {
        flags = f::logic8(GP4_GLOBAL(uint8_t, 0x007c2ea9));
        if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) == 0u) {
            goto overtakeChecks;
        }
        order = static_cast<uint8_t>(ReadU8(s.esi, 0x70) - ReadU8(s.ebx, 0x70));
        flags = f::sub8(ReadU8(s.esi, 0x70), ReadU8(s.ebx, 0x70));
        s.edx = SetLow8(s.edx, order);
        if (order == 0u) {
            goto overtakeChecks;
        }
        flags = f::logic8(order);
        if ((order & 0x80u) != 0u) {
            goto overtakeChecks;
        }
    }
    flags = f::sub16(ReadU16(s.ebx, 0xea), 0u);
    if (ReadU16(s.ebx, 0xea) != 0u) {
        goto overtakeChecks;
    }
    flags = f::logic8(ReadU8(s.ebx, 0x71) & 0x80u);
    if ((ReadU8(s.ebx, 0x71) & 0x80u) != 0u) {
        segment = ReadU32(s.ebx, 0x1c);
        flags = f::sub32(segment, GP4_GLOBAL(uint32_t, 0x00891500));
        if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
            segment += 0x6e00u;
            flags = f::sub32(segment, GP4_GLOBAL(uint32_t, 0x00866f00));
            if (segment >= GP4_GLOBAL(uint32_t, 0x00866f00)) {
                goto overtakeChecks;
            }
        }
    }
    dx = static_cast<uint16_t>(ReadU16(s.ebx, 0x68) - ReadU16(s.esi, 0x68));
    flags = f::sub16(ReadU16(s.ebx, 0x68), ReadU16(s.esi, 0x68));
    s.edx = SetLow16(s.edx, dx);
    if ((dx & 0x8000u) != 0u) {
        goto overtakeChecks;
    }
    limit = GP4_GLOBAL(uint16_t, 0x00603b10);
    flags = f::sub16(dx, limit);
    if (static_cast<int16_t>(dx) >= static_cast<int16_t>(limit)) {
        goto overtakeChecks;
    }
    {
        const uint8_t marked = static_cast<uint8_t>(ReadU8(s.ebx, 0x122) | 2u);
        GP4_FIELD(uint8_t, s.ebx, 0x122) = marked;
        flags = f::logic8(marked);
        GP4_FIELD(uint32_t, s.ebx, 0x124) = s.esi;
        const uint32_t savedEax = s.eax;
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00401462, s);
        const uint32_t delay = (s.eax & 0xffu) << 6;
        GP4_FIELD(uint32_t, s.ebx, 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, s.ebx, 0xf0) += delay;
        s.eax = savedEax;
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
    GP4_FIELD(uint32_t, s.esi, 0x114) = s.ebx;
    goto tail;

release:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e314, s);
    flags = s.eflags;
    s.edx = SetLow8(s.edx, 0x40u);
    flags = f::logic16(ReadU16(s.esi, 0xea));
    if ((ReadU16(s.esi, 0xea) & 0x8000u) != 0u) {
        s.edx = SetLow8(s.edx, 0x80u);
    }
    GP4_GLOBAL(uint8_t, 0x0060414c) = static_cast<uint8_t>(s.edx);
    GP4_GLOBAL(uint16_t, 0x0060414e) = 0;
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0042e97e, s);
    flags = s.eflags;
    if ((flags & f::CF) == 0u) {
        const uint8_t side = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0060414c) ^ 0xc0u);
        GP4_GLOBAL(uint8_t, 0x0060414c) = side;
        flags = f::logic8(side);
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x0042e97e, s);
        flags = s.eflags;
        if ((flags & f::CF) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0060414c) = 0;
            GP4_FIELD(uint16_t, s.esi, 0xea) = GP4_GLOBAL(uint16_t, 0x0060414e);
            flags = f::logic8(ReadU8(s.esi, 0xfe) & 0x40u);
            if ((ReadU8(s.esi, 0xfe) & 0x40u) == 0u) {
                s.eflags = f::merge(s.eflags, flags);
                gp4::call_regs(0x0042e314, s);
                flags = s.eflags;
                GP4_FIELD(uint32_t, s.esi, 0x114) = 0u;
                const uint8_t state = static_cast<uint8_t>(ReadU8(s.esi, 0x135) | 0x40u);
                GP4_FIELD(uint8_t, s.esi, 0x135) = state;
                flags = f::logic8(state);
                flags = f::logic8(ReadU8(s.edi, 0x51) & 1u);
                if ((ReadU8(s.edi, 0x51) & 1u) != 0u) {
                    const uint8_t bits = static_cast<uint8_t>(ReadU8(s.esi, 0x135) & 0xf7u);
                    GP4_FIELD(uint8_t, s.esi, 0x135) = bits;
                    flags = f::logic8(bits);
                }
            }
            flags &= ~f::CF;   // clc
            goto finish;
        }
    }
    GP4_GLOBAL(uint8_t, 0x0060414c) = 0;

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
// Spec: the composition of the sibling 0x0043052f plus lane_0042e97e (lane table 0x604194.., the neighbour
// [0x6041a4] and the previous car [0x6043e0]). gfields: [0x6043e0] is never null (0x0042e8e6 reads the car
// it names: faults at 0x42e8f0) and shares cars 0..2 with [0x6041a4] so the previous-car block runs; car word
// +0xea is 0 in a third of the draws (0x43099c); segment byte +0x64 bit 7 (0x430a4c); the pit entry [0x866f00]
// is a lap record 4..6 or a record more than 0x50 records ahead (0x4309bf); [0x7c2ea9] / [0x603b10] both ways.
GP4_IMPL(0x004307a9, FUN_004307a9, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+link_0042e5bf+carf_00410d04+lap_0043b482+state_00426091+gap_0042a533+state_0042da35+state_0042fa52+state_0066d8c0+state_0042eee3+state_0042ecbf+state_0042efee+state_004059ef+state_0042f3c7+list_0066fe3c+cursor_006043fc+order_0042e97e+flags_00430449+cand_0042e783+lane_0042e97e; maxinsn=4000000; cases=2500; wall=1800; gfields={0x006041a4:u32{=0,0x66fef0,0x67030c,0x670728}, 0x006043e0:u32{=0x66fef0,0x67030c,0x670728}, 0x007c2ea9:u8{=0x80,0xff,0,0x7f}, 0x0066ffda*22/0x41c:u16{=0,0,0,0x18,0x20,8,0xfff8,0x10,0xfff0}, 0x00894404*26/0x160:u8{=0,0x80}, 0x00866f00:u32{=0x894920,0x894a80,0x894be0,0x8a0000}, 0x00603b10:i16{=0x7fff,0x7fff,0x100,0}}")
