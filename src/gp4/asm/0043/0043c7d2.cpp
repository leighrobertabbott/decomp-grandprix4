#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic operation (`test`): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit logic operation (`test`): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `cmp a, b`.
static inline uint32_t cmpFlags32(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit `cmp a, b`.
static inline uint32_t cmpFlags8(uint32_t a8, uint32_t b8) {
    const uint32_t a = a8 & 0xffu;
    const uint32_t b = b8 & 0xffu;
    const uint32_t result = (a - b) & 0xffu;
    return (a < b ? 1u : 0u)
        | parityFlag(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043c7d2
// Gear-change request for the car in esi (pushal / popal: every register is preserved, the flags are
// those of the last flag writer). Nothing happens when byte +0xfe bit 2 or byte +0x231 bit 5 is set,
// when (byte +0x202 + [0x0063b830]) & 0x1f is nonzero, or when the segment pointer +0x1c is not below
// the pit start [0x00891500] and inside [[0x00866f00] - 0xf0a0, [0x00866f00] - 0x44c0). The partner car
// (0x43b974, ebx) with byte +0x71 bit 7 set and close in segment position (or, past the pit start,
// without byte +0x230 bit 7) is skipped when its 0x43c75b value differs from ours by less than 0x4e20.
// The wanted gear index 0x43ca8b is compared with byte +0x7f; 0x43c54b / 0x43c475 / 0x43c28a / 0x43c36e
// accept it, and the car's byte +0x160 = gear, +0x230 |= 4 (then bit 7 cleared), +0xfe |= 4, +0x71 |= 0x80
// are set; 0x418fd0 follows unless [0x007c2e75] is nonzero. [0x006227a0], [0x006227e8], [0x006227ec] are stored.
void __cdecl FUN_0043c7d2(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;

    uint32_t flags;

    if ((GP4_FIELD(uint8_t, car, 0xfe) & 4u) != 0u) {
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0xfe) & 4u);
        goto done;
    }
    if ((GP4_FIELD(uint8_t, car, 0x231) & 0x20u) != 0u) {
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x231) & 0x20u);
        goto done;
    }
    {
        const uint32_t sum = GP4_FIELD(uint8_t, car, 0x202) + GP4_GLOBAL(uint32_t, 0x0063b830);
        if ((sum & 0x1fu) != 0u) {
            flags = logicFlags32(sum & 0x1fu);
            goto done;
        }
    }
    {
        const uint32_t position = GP4_FIELD(uint32_t, car, 0x1c);
        const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
        if (position >= pitStart) {
            flags = cmpFlags32(position, pitStart);
            goto done;
        }
        uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866f00) - 0x44c0u;
        if (position >= limit) {
            flags = cmpFlags32(position, limit);
            goto done;
        }
        limit -= 0xabe0u;
        if (position < limit) {
            flags = cmpFlags32(position, limit);
            goto done;
        }
    }

    gp4::call_regs(0x0043b974, w);
    if ((w.eflags & 1u) != 0u) {
        uint8_t* partner = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(w.ebx));
        if ((GP4_FIELD(uint8_t, partner, 0x71) & 0x80u) != 0u) {
            const uint32_t partnerPosition = GP4_FIELD(uint32_t, partner, 0x1c);
            bool compare;
            if (partnerPosition < GP4_GLOBAL(uint32_t, 0x00891500)) {
                int32_t distance = static_cast<int32_t>(partnerPosition - GP4_FIELD(uint32_t, car, 0x1c));
                if (distance < 0) {
                    distance = static_cast<int32_t>(0u - static_cast<uint32_t>(distance));
                }
                compare = distance < 0x157c0;
            } else {
                compare = (GP4_FIELD(uint8_t, partner, 0x230) & 0x80u) == 0u;
            }
            if (compare) {
                w.eax = w.ebx;
                gp4::call_regs(0x0043c75b, w);
                w.edx = w.eax;
                w.eax = w.esi;
                gp4::call_regs(0x0043c75b, w);
                int32_t difference = static_cast<int32_t>(w.eax - w.edx);
                if (difference < 0) {
                    difference = static_cast<int32_t>(0u - static_cast<uint32_t>(difference));
                }
                if (difference < 0x4e20) {
                    flags = cmpFlags32(static_cast<uint32_t>(difference), 0x4e20u);
                    goto done;
                }
            }
        }
    }

    {
        int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu) - 1;
        if (index > 0x15) {
            index = 0x15;
        }
        w.eax = static_cast<uint32_t>(index);
    }
    gp4::call_regs(0x0043ca66, w);
    GP4_GLOBAL(uint32_t, 0x006227a0) = w.eax;
    gp4::call_regs(0x0043ca8b, w);
    {
        const uint32_t wanted = w.eax & 0xffu;
        const uint32_t current = GP4_FIELD(uint8_t, car, 0x7f);
        if (wanted == current) {
            flags = cmpFlags8(wanted, current);
            goto done;
        }
        if (static_cast<int8_t>(wanted) < 2 && static_cast<int8_t>(current) < 2) {
            flags = cmpFlags8(current, 2u);
            goto done;
        }
    }

    gp4::call_regs(0x0043c54b, w);
    if ((w.eflags & 1u) == 0u) {
        flags = w.eflags & 0x8d5u;
        goto done;
    }
    w.eax &= 0xffu;
    if (static_cast<int32_t>(w.eax) < 2) {
        gp4::call_regs(0x0043c475, w);
    }
    w.ebx = w.eax;
    w.ecx = GP4_FIELD(uint8_t, car, 0x7f);
    GP4_GLOBAL(uint32_t, 0x006227e8) = w.ebx;
    GP4_GLOBAL(uint32_t, 0x006227ec) = w.ecx;
    w.eax = GP4_ARRAY(uint32_t, 0x0063b844)[w.ebx] - GP4_ARRAY(uint32_t, 0x0063b844)[w.ecx];
    if ((w.eax & 0x80000000u) != 0u) {
        if (static_cast<int32_t>(w.ecx) >= 2) {
            flags = cmpFlags32(w.ecx, 2u);
            goto done;
        }
        gp4::call_regs(0x0043c28a, w);
        if ((w.eflags & 1u) == 0u) {
            flags = w.eflags & 0x8d5u;
            goto done;
        }
    }
    gp4::call_regs(0x0043c36e, w);
    if ((w.eflags & 1u) == 0u) {
        flags = w.eflags & 0x8d5u;
        goto done;
    }
    GP4_FIELD(uint8_t, car, 0x160) = static_cast<uint8_t>(w.ebx);
    GP4_FIELD(uint8_t, car, 0x230) |= 4u;
    GP4_FIELD(uint8_t, car, 0xfe) |= 4u;
    GP4_FIELD(uint8_t, car, 0x71) |= 0x80u;
    GP4_FIELD(uint8_t, car, 0x230) &= 0x7fu;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) {
        gp4::Regs call = w;
        call.ecx = 1u;
        call.eax = 0u;
        call.edx = 0xbb8u;
        gp4::call_regs(0x00418fd0, call);
        flags = call.eflags & 0x8d5u;
    } else {
        flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x007c2e75), 0u);
    }

done:
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c7d2, FUN_0043c7d2, "regs(esi:ptr[0x2a0]:layout=car_0043c677+car_0043c7d2) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=search_0066d82c+list_0066fe3c+state_00432330+state_0043c677+segw_0043c608+state_0043c1ba+state_0043c31b+state_0043c28a+trk_007ad894+circ_007adb50+tbl_0066d94c+state_0066d8c0+gate_0043c7d2; cases=800")
