#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic operation (`or`, `and`, `test`): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

// FUNCTION: GP4 0x0041464c
// Overtaking / yielding decision for the car in esi against the heading word [edi]. Stores the heading
// error ax = word +0xe - [edi] in [0x00934df0], its magnitude class in [0x00934df8] (0x8000 when
// |ax| >= 0x4000, else 0) and the magnitude folded about 0x4000 in [0x00934df4]. Byte +0xff bit 3 hands
// over to 0x414146; byte +0x71 bit 0 to 0x413fb0; byte +0xe7 bit 0 to the |+0x9a| < [0x006013e2] test;
// otherwise a small error (class 0) goes to 0x413f28, a large one to 0x413ea0 (ebp = 8) with the
// follow-up flag edits in +0x6f / +0x203 / +0xe5 / +0x232 / +0x230 and 0x4142bb / 0x4143b9. The result
// is CF = 1 (stc) or 0 (clc) on top of the flags of the last flag writer.
void __cdecl FUN_0041464c(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint16_t* heading = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;

    uint32_t flags;      // arithmetic flags of the last flag writer
    uint32_t carry;      // the clc / stc that follows

    setLow16(w.eax, static_cast<uint32_t>(GP4_FIELD(uint16_t, car, 0xe)) - *heading);
    {
        const uint16_t error = static_cast<uint16_t>(w.eax);
        GP4_GLOBAL(uint16_t, 0x00934df0) = error;
        uint16_t magnitude = error;
        if ((magnitude & 0x8000u) != 0u) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        GP4_GLOBAL(uint16_t, 0x00934df8) = magnitude >= 0x4000u ? 0x8000u : 0u;

        magnitude = error;
        if ((magnitude & 0x8000u) != 0u) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        if (magnitude >= 0x4000u) {
            magnitude = static_cast<uint16_t>(0x8000u - magnitude);
        }
        GP4_GLOBAL(uint16_t, 0x00934df4) = magnitude;
    }

    if ((GP4_FIELD(uint8_t, car, 0xff) & 8u) != 0u) {
        goto call_414146;
    }
    if ((GP4_FIELD(uint8_t, car, 0x71) & 1u) != 0u) {
        goto call_413fb0;
    }
    if ((GP4_FIELD(uint8_t, car, 0xe7) & 1u) != 0u) {
        goto speed_check;
    }
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 0x10u) != 0u) {
        goto call_4142bb;
    }
    if (GP4_GLOBAL(uint16_t, 0x00934df8) == 0u) {
        // small heading error: 0x413f28, CF = 1 -> clc, CF = 0 -> stc
        gp4::call_regs(0x00413f28, w);
        flags = w.eflags & 0x8d5u;
        carry = (w.eflags & 1u) != 0u ? 0u : 1u;
        goto done;
    }
    setLow16(w.edx, GP4_FIELD(uint16_t, car, 0x9a));
    if (static_cast<int16_t>(w.edx) >= GP4_GLOBAL(int16_t, 0x006013e0)) {
        GP4_FIELD(uint8_t, car, 0x6f) |= 2u;
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x6f));
        carry = 0u;
        goto done;
    }
    w.ebp = 8u;
    gp4::call_regs(0x00413ea0, w);
    if ((w.eflags & 1u) != 0u) {
        GP4_FIELD(uint8_t, car, 0x203) &= 0xfdu;
        GP4_FIELD(uint8_t, car, 0xe5) |= 0x10u;
        GP4_FIELD(uint8_t, car, 0x232) |= 8u;
        GP4_FIELD(uint8_t, car, 0x6f) |= 1u;
        GP4_FIELD(uint8_t, car, 0x6f) &= 0xfdu;
        GP4_FIELD(uint8_t, car, 0x72) = 1u;
        GP4_FIELD(uint8_t, car, 0x230) |= 2u;
        goto call_4142bb;
    }
    if ((GP4_FIELD(uint8_t, car, 0x203) & 2u) == 0u) {
        GP4_FIELD(uint8_t, car, 0x203) |= 2u;
        GP4_FIELD(uint8_t, car, 0x1f6) = 5u;
    }
    GP4_FIELD(uint8_t, car, 0x6f) &= 0xfeu;
    GP4_FIELD(uint8_t, car, 0x6f) |= 2u;
    flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x6f));
    carry = 0u;
    goto done;

speed_check:
    {
        uint32_t speed = GP4_FIELD(uint16_t, car, 0x9a);
        if ((speed & 0x8000u) != 0u) {
            speed = (0u - speed) & 0xffffu;
        }
        setLow16(w.edx, speed);
    }
    if (static_cast<int16_t>(w.edx) < GP4_GLOBAL(int16_t, 0x006013e2)) {
        GP4_FIELD(uint8_t, car, 0x71) |= 1u;
        GP4_FIELD(uint8_t, car, 0xe5) &= 0xefu;
        GP4_FIELD(uint8_t, car, 0x72) = 0xffu;
        goto call_413fb0;
    }
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 0x10u) != 0u) {
        goto call_4143b9;
    }
    flags = logicFlags8(0u);   // test byte [esi+0xe5], 0x10 with the bit clear
    carry = 1u;
    goto done;

call_413fb0:
    gp4::call_regs(0x00413fb0, w);
    flags = w.eflags & 0x8d5u;
    carry = (w.eflags & 1u) != 0u ? 1u : 0u;
    goto done;

call_4142bb:
    gp4::call_regs(0x004142bb, w);
    if ((w.eflags & 1u) != 0u) {
        flags = w.eflags & 0x8d5u;
        carry = 1u;
        goto done;
    }

call_4143b9:
    gp4::call_regs(0x004143b9, w);
    flags = w.eflags & 0x8d5u;
    carry = 0u;
    goto done;

call_414146:
    gp4::call_regs(0x00414146, w);
    flags = w.eflags & 0x8d5u;
    carry = (w.eflags & 1u) != 0u ? 1u : 0u;

done:
    r->eax = w.eax;
    r->ebx = w.ebx;
    r->ecx = w.ecx;
    r->edx = w.edx;
    r->ebp = w.ebp;
    r->eflags = (w.eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0041464c, FUN_0041464c, "regs(eax:u32, edx:u32, ebp:u32, esi:ptr[0x240]:layout=car_0041416e+ptr_00414439+car_00413fb0+car_0041464c, edi:rec[0x8943a0,0x160,0..29]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+state_0041416e+list_0066fe3c+state_00413f28+state_0041104f+hdg_008943a0+flags_00413ea0")
