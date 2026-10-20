#include <gp4/gp4.h>

// Replaces the low word of a register image.
static inline uint32_t withLow16(uint32_t reg, uint16_t value) {
    return (reg & 0xffff0000u) | value;
}

// FUNCTION: GP4 0x0043e26c
// esi = car record, [esi+0x1c] = its track-segment record (0x160-byte ring at 0x008943a0).
// Per-frame car update: resets the per-frame flags, publishes position/heading/speed to the
// 0x00603d28.. scratch block, runs the engine/gearbox chain (0x405893, 0x427b5e, 0x415d1c,
// 0x415ef6; a down-change draws 0x401462 for bit 7 of +0x86), the segment walk 0x427396, then
// derives +0x66 (clamped +-0x6000), the heading rate +0xac and the new position via 0x426ff4 /
// 0x4046d1, finishing with 0x435aa4.
void __cdecl FUN_0043e26c(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.esi));

    s.edi = GP4_FIELD(uint32_t, car, 0x1c);
    uint8_t* const segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.edi));

    GP4_FIELD(uint16_t, car, 0x10c) = GP4_FIELD(uint16_t, car, 0x9c);
    GP4_FIELD(uint16_t, car, 0x9c) = 0;
    GP4_FIELD(uint8_t, car, 0x86) = 0;
    GP4_FIELD(uint8_t, car, 0x76) = 0;
    GP4_FIELD(uint8_t, car, 0x8f) &= 0x7fu;
    GP4_FIELD(uint8_t, car, 0x8f) &= 0xbfu;
    GP4_FIELD(uint8_t, car, 0x8f) &= 0xdfu;
    GP4_FIELD(uint8_t, car, 0x8f) &= 0xefu;

    s.eax = withLow16(s.eax, static_cast<uint16_t>(GP4_FIELD(uint16_t, segment, 0x60) - 0x100u));
    GP4_FIELD(uint16_t, car, 0x11c) = static_cast<uint16_t>(s.eax);
    s.eax = withLow16(s.eax, static_cast<uint16_t>(GP4_FIELD(uint16_t, segment, 0x62) - 0x100u));
    GP4_FIELD(uint16_t, car, 0x11e) = static_cast<uint16_t>(s.eax);
    if (GP4_FIELD(int16_t, car, 0x90) < 0) {
        s.eax = withLow16(s.eax, GP4_FIELD(uint16_t, car, 0x11c));
    }
    GP4_FIELD(uint16_t, car, 0xb0) = static_cast<uint16_t>(s.eax);

    s.eax = GP4_FIELD(uint32_t, car, 0);
    GP4_GLOBAL(uint32_t, 0x00603d28) = s.eax;
    s.eax = GP4_FIELD(uint32_t, car, 4);
    GP4_GLOBAL(uint32_t, 0x00603d2c) = s.eax;
    s.eax = withLow16(s.eax, GP4_FIELD(uint16_t, car, 0x18));
    GP4_GLOBAL(uint16_t, 0x00603d6a) = static_cast<uint16_t>(s.eax);
    s.eax = withLow16(s.eax, static_cast<uint16_t>(static_cast<uint16_t>(s.eax) - GP4_FIELD(uint16_t, car, 0x142)));
    GP4_GLOBAL(uint16_t, 0x00603d6c) = static_cast<uint16_t>(s.eax);
    s.eax = GP4_FIELD(uint16_t, car, 0xee);
    GP4_GLOBAL(uint32_t, 0x00603d3c) = s.eax;
    s.eax = GP4_FIELD(uint32_t, car, 0x98);
    GP4_GLOBAL(uint32_t, 0x00603d38) = s.eax;
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0xda)));
    GP4_GLOBAL(uint32_t, 0x00603d34) = s.eax;
    s.eax = (s.eax & 0xffffff00u) | GP4_FIELD(uint8_t, car, 0x72);
    GP4_FIELD(uint8_t, car, 0x121) = GP4_FIELD(uint8_t, car, 0x72);

    gp4::call_regs(0x00405893, s);
    gp4::call_regs(0x00427b5e, s);

    GP4_FIELD(uint16_t, car, 0xa0) = 0x4000;
    s.eax = GP4_FIELD(uint16_t, car, 0x62);
    GP4_GLOBAL(uint32_t, 0x00603a48) = s.eax;
    gp4::call_regs(0x00415d1c, s);
    gp4::call_regs(0x00415ef6, s);

    if ((s.eflags & gp4::CF) != 0u) {
        // the gear byte +0x72 changed: a down-change draws a random bit for flag 0x80 of +0x86
        const uint8_t previous = GP4_FIELD(uint8_t, car, 0x121);
        s.eax = (s.eax & 0xffffff00u) | previous;
        if (previous > GP4_FIELD(uint8_t, car, 0x72)) {
            gp4::call_regs(0x00401462, s);
            s.eax &= 0xffu;
            s.eax &= 1u;
            if (s.eax == 0u) {
                GP4_FIELD(uint8_t, car, 0x86) |= 0x80u;
            }
        }
        s.eax = GP4_GLOBAL(uint32_t, 0x00602fc0);
        GP4_FIELD(uint16_t, car, 0x1ae) = static_cast<uint16_t>(s.eax);
    }
    if (GP4_FIELD(uint16_t, car, 0x1ae) != 0u) {
        s.edx = GP4_GLOBAL(uint32_t, 0x00603854);
        gp4::call_regs(0x0041ae6c, s);
    }
    gp4::call_regs(0x00426e8f, s);
    gp4::call_regs(0x00427396, s);

    // +0x66 = ((+0x96 - 0x400) << 14) / max(+0x146, 0x10) + 0x4000, magnitude limited to 0x6000
    const int32_t numerator = (static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x96)) - 0x400) * 16384;
    int32_t divisor = static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0x146));
    if (divisor < 0x10) divisor = 0x10;
    s.ecx = static_cast<uint32_t>(divisor);
    s.edx = static_cast<uint32_t>(numerator % divisor);
    uint32_t value = static_cast<uint32_t>(numerator / divisor) + 0x4000u;
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= 0x6000u) magnitude = 0x6000u;
        value = 0u - magnitude;
    } else if (value >= 0x6000u) {
        value = 0x6000u;
    }
    s.eax = value;
    GP4_FIELD(uint16_t, car, 0x66) = static_cast<uint16_t>(value);
    GP4_FIELD(uint32_t, car, 0x1b8) = s.edi;
    gp4::call_regs(0x00409416, s);

    // +0xac = (int16)(+0x416 - old +0x18) << 24 / [0x603854]
    const uint16_t oldHeading = GP4_FIELD(uint16_t, car, 0x18);
    s.edx = withLow16(s.edx, oldHeading);
    const uint16_t heading = GP4_FIELD(uint16_t, car, 0x416);
    GP4_FIELD(uint16_t, car, 0x18) = heading;
    GP4_FIELD(uint16_t, car, 0xe) = heading;
    const int32_t turn = static_cast<int16_t>(static_cast<uint16_t>(heading - oldHeading));
    int32_t remainder = 0;
    s.eax = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(turn) * 16777216, GP4_GLOBAL(int32_t, 0x00603854), &remainder));
    s.edx = static_cast<uint32_t>(remainder);
    GP4_FIELD(uint32_t, car, 0xac) = s.eax;
    gp4::call_regs(0x00426ff4, s);

    GP4_FIELD(uint32_t, car, 0x1a0) = 0x4000;
    const uint16_t across = GP4_FIELD(uint16_t, car, 0x94);
    GP4_FIELD(uint16_t, car, 0x90) = across;
    GP4_GLOBAL(uint32_t, 0x00934df0) = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(across))) << 8;
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x92))) << 8;
    GP4_GLOBAL(uint32_t, 0x00934df4) = s.eax;
    gp4::call_regs(0x004046d1, s);

    s.eax = GP4_GLOBAL(uint32_t, 0x00934df0);
    GP4_FIELD(uint32_t, car, 0) = s.eax;
    s.eax = GP4_GLOBAL(uint32_t, 0x00934df4);
    GP4_FIELD(uint32_t, car, 4) = s.eax;
    gp4::call_regs(0x00435aa4, s);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = s.eflags;
}
GP4_IMPL(0x0043e26c, FUN_0043e26c, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88+rec_00426947) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=engine_00413606+state_0041e500+state_0041242f+state_0066d8c0+ring_008943a0+ring_00405a12+links_00405a12+bank_004279ab+state_00405489+state_00426947+state_00427880+frac_00624790+state_0043e26c")
