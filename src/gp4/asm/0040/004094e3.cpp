#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// esi = car record, edi = the track-segment record it is placed on; ax/cx/dx are the entry
// values the callers pass (stored as +0x90 / +0x92 and added to the segment's word +0).
// Stores the segment pointer (+0x1c, +0x1b8), derives the lane words +0x11c/+0x11e/+0xb0, the
// slope terms +0x96/+0x146/+0x66 (an idiv clamped to +-0x6000), the interpolated +0x94, runs the
// original placement callees and copies the 0x60135c/0x601370 dwords into the record. The
// register file is threaded through every callee; the flags are those of the final `shr`.
// FUNCTION: GP4 0x004094e3
void __cdecl FUN_004094e3(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    GP4_FIELD(uint32_t, car, 0x1c) = r->edi;
    GP4_FIELD(uint32_t, car, 0x1b8) = r->edi;
    GP4_FIELD(uint16_t, car, 0x90) = static_cast<uint16_t>(r->eax);
    GP4_FIELD(uint16_t, car, 0x92) = static_cast<uint16_t>(r->ecx);
    const uint16_t height = static_cast<uint16_t>(r->edx + GP4_FIELD(uint16_t, seg, 0x0));
    GP4_FIELD(uint16_t, car, 0xe) = height;
    GP4_FIELD(uint16_t, car, 0x18) = height;
    GP4_FIELD(uint16_t, car, 0x11c) = static_cast<uint16_t>(GP4_FIELD(uint16_t, seg, 0x60) - 0x100u);
    uint16_t lane = static_cast<uint16_t>(GP4_FIELD(uint16_t, seg, 0x62) - 0x100u);
    GP4_FIELD(uint16_t, car, 0x11e) = lane;
    if (GP4_FIELD(int16_t, car, 0x90) < 0) {
        lane = GP4_FIELD(uint16_t, car, 0x11c);
    }
    GP4_FIELD(uint16_t, car, 0xb0) = lane;

    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u && static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) >= 0) {
        gp4::call_regs(0x004322a6, s);
    }

    const int32_t slope = GP4_FIELD(int16_t, seg, 0x14);
    uint16_t term = 0u;
    if (slope != 0) {
        term = static_cast<uint16_t>(static_cast<uint32_t>(slope * GP4_FIELD(int16_t, car, 0x90)) >> 15);
    }
    GP4_FIELD(uint16_t, car, 0x96) = static_cast<uint16_t>(term + GP4_FIELD(uint16_t, car, 0x92));
    uint16_t term2 = 0u;
    if (slope != 0) {
        term2 = static_cast<uint16_t>(static_cast<uint32_t>(slope * GP4_FIELD(int16_t, car, 0x90)) >> 14);
    }
    int16_t width = static_cast<int16_t>(static_cast<uint16_t>(0x400u - term2));
    if (width < 0x10) {
        width = 0x10;
    }
    GP4_FIELD(int16_t, car, 0x146) = width;

    const int32_t dividend = static_cast<int32_t>(
        static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x96)) - 0x400) << 14);
    int32_t divisor = GP4_FIELD(uint16_t, car, 0x146);
    if (divisor < 0x10) {
        divisor = 0x10;
    }
    int32_t ratio = dividend / divisor + 0x4000;
    if (ratio < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(ratio);
        if (magnitude >= 0x6000u) {
            magnitude = 0x6000u;
        }
        ratio = static_cast<int32_t>(0u - magnitude);
    } else if (static_cast<uint32_t>(ratio) >= 0x6000u) {
        ratio = 0x6000;
    }
    GP4_FIELD(uint16_t, car, 0x66) = static_cast<uint16_t>(ratio);

    uint16_t blend;
    if ((car[0x9c] & 0x10u) != 0u) {
        blend = 0u;
    } else {
        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, car, 0x1b8)));
        const int32_t here = GP4_FIELD(int16_t, rec, 0x16);
        const int32_t next = GP4_FIELD(int16_t, rec, 0x176);
        const uint32_t product = static_cast<uint32_t>(next - here) * static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x66)));
        blend = static_cast<uint16_t>((product >> 14) + static_cast<uint32_t>(here));
    }
    GP4_FIELD(uint16_t, car, 0x94) = blend;

    gp4::call_regs(0x00402db6, s);
    GP4_FIELD(uint32_t, car, 0x8) = s.edx;
    GP4_FIELD(uint16_t, car, 0x12) = static_cast<uint16_t>(s.eax);
    GP4_FIELD(uint16_t, car, 0x16) = static_cast<uint16_t>(s.ebp);

    // mov cx,[edi+0x1a]; and cx,0x7ff; mov al,[esi+0x67]; cbw; xchg cx,ax
    uint16_t pos = static_cast<uint16_t>(GP4_FIELD(uint16_t, seg, 0x1a) & 0x7ffu);
    const uint16_t offset = static_cast<uint16_t>(static_cast<int16_t>(static_cast<int8_t>(car[0x67])));
    s.eax = (s.eax & 0xffff0000u) | pos;
    s.ecx = (s.ecx & 0xffff0000u) | offset;

    car[0x21d] = static_cast<uint8_t>(car[0x21d] & 0xfbu);
    if ((car[0x8d] & 2u) != 0u) {
        car[0x21d] = static_cast<uint8_t>(car[0x21d] | 4u);
        gp4::call_regs(0x00405985, s);
        pos = static_cast<uint16_t>(s.eax);
        if (static_cast<int16_t>(pos) >= GP4_GLOBAL(int16_t, 0x00868558)) {
            pos = static_cast<uint16_t>(pos - GP4_GLOBAL(uint16_t, 0x00868558));
        }
    }
    pos = static_cast<uint16_t>(pos - GP4_GLOBAL(uint16_t, 0x0066d88a));
    if (static_cast<int16_t>(pos) < 0) {
        pos = static_cast<uint16_t>(pos + GP4_GLOBAL(uint16_t, 0x00868558));
    }
    if (static_cast<int16_t>(pos) >= GP4_GLOBAL(int16_t, 0x00868558)) {
        pos = static_cast<uint16_t>(pos - GP4_GLOBAL(uint16_t, 0x00868558));
    }
    GP4_FIELD(uint16_t, car, 0x68) = pos;
    const uint16_t cell = static_cast<uint16_t>((pos << 6) + static_cast<uint16_t>(s.ecx));
    GP4_FIELD(uint16_t, car, 0xe2) = cell;
    s.eax = (s.eax & 0xffff0000u) | cell;

    gp4::call_regs(0x0040db6c, s);
    gp4::call_regs(0x004036ca, s);
    car[0x1ce] = static_cast<uint8_t>(car[0x1ce] | 0x40u);

    int32_t angle = GP4_FIELD(int32_t, car, 0x1a0);
    if (angle < -0x4000) {
        angle = -0x4000;
    }
    angle >>= 1;
    if ((car[0x71] & 8u) != 0u) {
        angle -= 0x2000;
    } else {
        angle = 0x2000 - angle;
    }
    GP4_FIELD(uint16_t, car, 0x13e) = static_cast<uint16_t>(angle);
    s.eax = static_cast<uint32_t>(angle);

    gp4::call_regs(0x004044f2, s);
    gp4::call_regs(0x00426fa5, s);

    GP4_FIELD(uint16_t, car, 0x58) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x0060135c));
    GP4_FIELD(uint16_t, car, 0x5a) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00601360));
    GP4_FIELD(uint16_t, car, 0x5c) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00601364));
    GP4_FIELD(uint16_t, car, 0x204) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00601370) >> 8);
    GP4_FIELD(uint16_t, car, 0x206) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00601374) >> 8);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00601378);
    const uint32_t shifted = last >> 8;
    GP4_FIELD(uint16_t, car, 0x208) = static_cast<uint16_t>(shifted);
    GP4_FIELD(uint32_t, car, 0x284) = 0u;
    s.eax = shifted;

    // shr eax, 8: CF = bit 7, AF = OF = SF = 0 (pinned oracle model), PF/ZF from the result.
    const uint32_t flags = ((last >> 7) & 1u) | parityFlag8(shifted) | (shifted == 0u ? 0x40u : 0u);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x004094e3, FUN_004094e3, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..11], esi:ptr[0x420]:bytes) -> (eax, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3")
