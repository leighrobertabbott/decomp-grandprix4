#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic result (test / and / or): CF = OF = AF = 0.
static inline uint32_t logic8Flags(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

static inline uint8_t& byteAt(uint8_t* p, uint32_t offset) { return GP4_FIELD(uint8_t, p, offset); }
static inline int16_t wordAt(uint8_t* p, uint32_t offset) { return GP4_FIELD(int16_t, p, offset); }

static inline void setAx(gp4::Regs& s, uint32_t value) {
    s.eax = (s.eax & 0xffff0000u) | (value & 0xffffu);
}

// 0x434d09: or byte +0xcc, 0x10.
static uint32_t markCc10(uint8_t* car) {
    byteAt(car, 0xcc) = static_cast<uint8_t>(byteAt(car, 0xcc) | 0x10u);
    return logic8Flags(byteAt(car, 0xcc));
}

// 0x434cf7: clear bit 4 of +0xcc unless bit 6 is set.
static uint32_t clearCc10(uint8_t* car) {
    if ((byteAt(car, 0xcc) & 0x40u) != 0u) {
        return markCc10(car);
    }
    byteAt(car, 0xcc) = static_cast<uint8_t>(byteAt(car, 0xcc) & 0xefu);
    return logic8Flags(byteAt(car, 0xcc));
}

// 0x434cd1
static uint32_t checkLateral(gp4::Regs& s, uint8_t* car) {
    if ((byteAt(car, 0x9c) & 2u) != 0u) {
        return clearCc10(car);
    }
    uint16_t value = static_cast<uint16_t>(wordAt(car, 0x90));
    if (static_cast<int16_t>(value) < 0) {
        value = static_cast<uint16_t>(0u - value);
    }
    value = static_cast<uint16_t>(value - 0x19cu);
    setAx(s, value);
    if (static_cast<int16_t>(value) >= wordAt(car, 0xb0)) {
        return markCc10(car);
    }
    return clearCc10(car);
}

// 0x434cab
static uint32_t checkFlags(gp4::Regs& s, uint8_t* car) {
    if ((byteAt(car, 0xcb) & 8u) != 0u && wordAt(car, 0x9a) >= 0x1d5) {
        return clearCc10(car);
    }
    if ((byteAt(car, 0x231) & 0x40u) == 0u || (byteAt(car, 0x203) & 2u) != 0u) {
        return markCc10(car);
    }
    return checkLateral(s, car);
}

// 0x434c6d
static uint32_t checkSpeed(gp4::Regs& s, uint8_t* car, uint8_t* segment) {
    if ((byteAt(car, 0x9c) & 2u) != 0u) {
        if (wordAt(car, 0x9a) >= 0x755) {
            return clearCc10(car);
        }
        if (static_cast<int8_t>(byteAt(car, 0x235)) >= 2) {
            return checkFlags(s, car);
        }
        if (wordAt(car, 0x9a) >= 0x11a) {
            return checkLateral(s, car);
        }
        return checkFlags(s, car);
    }
    const uint16_t quarter = static_cast<uint16_t>(GP4_FIELD(uint16_t, segment, 0xca) >> 2);
    setAx(s, quarter);
    if (static_cast<int16_t>(quarter) < wordAt(car, 0x9a)) {
        return checkLateral(s, car);
    }
    return checkFlags(s, car);
}

// One `push eax; push ecx; mov cx, steps; mov ax, kind; call 0x441af6; pop ecx; pop eax`.
static void startEffect(gp4::Regs& s, uint16_t steps, uint16_t kind) {
    gp4::Regs c = s;
    c.ecx = (c.ecx & 0xffff0000u) | steps;
    c.eax = (c.eax & 0xffff0000u) | kind;
    gp4::call_regs(0x00441af6, c);
    c.eax = s.eax;
    c.ecx = s.ecx;
    s = c;
}

// 0x434be0
static uint32_t engage(gp4::Regs& s, uint8_t* car) {
    byteAt(car, 0xe7) = static_cast<uint8_t>(byteAt(car, 0xe7) & 0xfdu);
    if ((byteAt(car, 0xcc) & 0x40u) == 0u) {
        byteAt(car, 0xcc) = static_cast<uint8_t>(byteAt(car, 0xcc) | 0x40u);
        gp4::call_regs(0x0043488b, s);
        GP4_GLOBAL(uint32_t, 0x0062fc10) = 0u;
        startEffect(s, 0xffc0u, 0u);
        GP4_GLOBAL(uint32_t, 0x0062fc10) = 1u;
        startEffect(s, 0xffe0u, 0u);
        GP4_GLOBAL(uint32_t, 0x0062fc10) = 2u;
        startEffect(s, 0xfff6u, 0u);
        GP4_GLOBAL(uint32_t, 0x0062fc10) = 3u;
        startEffect(s, 0x0020u, 1u);
    }
    return markCc10(car);
}

// 0x434b87
static uint32_t release(gp4::Regs& s, uint8_t* car, uint8_t* segment) {
    if ((byteAt(car, 0x7a) & 0x20u) != 0u) {
        return engage(s, car);
    }
    byteAt(car, 0xcc) = static_cast<uint8_t>(byteAt(car, 0xcc) & 0xbfu);
    if ((byteAt(car, 0x25f) & 0x80u) != 0u) {
        s.eax = 1u;
        gp4::call_regs(0x00434044, s);
        byteAt(car, 0x25f) = static_cast<uint8_t>(byteAt(car, 0x25f) & 0x7fu);
    }
    return checkSpeed(s, car, segment);
}

// 0x434b80
static uint32_t cancel(gp4::Regs& s, uint8_t* car, uint8_t* segment) {
    byteAt(car, 0xe7) = static_cast<uint8_t>(byteAt(car, 0xe7) & 0xfdu);
    return release(s, car, segment);
}

static inline bool absBelow80000(uint32_t value) {
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
    }
    return static_cast<int32_t>(value) < 0x80000;
}

static uint32_t evaluate(gp4::Regs& s, uint8_t* car, uint8_t* segment) {
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u && (byteAt(car, 0xff) & 1u) == 0u) {
        return 0x44u;                                   // test result zero
    }
    if ((byteAt(car, 0x7b) & 0x40u) != 0u || (byteAt(car, 0x25f) & 2u) != 0u
        || (byteAt(car, 0x25f) & 1u) != 0u || (byteAt(car, 0x26b) & 8u) != 0u) {
        return cancel(s, car, segment);
    }
    if (byteAt(car, 0x237) != 0u) {
        return logic8Flags(byteAt(car, 0x237));
    }
    if (GP4_GLOBAL(uint8_t, 0x0066d88c) != 0u) {
        return logic8Flags(GP4_GLOBAL(uint8_t, 0x0066d88c));
    }
    gp4::call_regs(0x00434930, s);
    if ((s.eflags & 1u) != 0u) {
        return s.eflags & 0x8d5u;
    }

    if ((byteAt(car, 0xe5) & 4u) != 0u
        && ((GP4_FIELD(uint32_t, car, 0x40) & 0x15280u) != 0u || (byteAt(car, 0x7b) & 2u) == 0u)) {
        gp4::call_regs(0x00434d11, s);
        if (wordAt(car, 0x9a) < 0x1d5) {
            s.eax = GP4_FIELD(uint32_t, car, 0xac);
            if ((static_cast<int32_t>(s.eax)) < 0) {
                s.eax = 0u - s.eax;
            }
            if (static_cast<int32_t>(s.eax) < 0x80000) {
                return engage(s, car);
            }
        }
        return checkSpeed(s, car, segment);
    }

    if ((byteAt(car, 0x78) & 4u) != 0u && (byteAt(car, 0xe5) & 4u) == 0u) {
        gp4::call_regs(0x00434700, s);
    }
    setAx(s, static_cast<uint16_t>(wordAt(car, 0x9a)));
    if (wordAt(car, 0x9a) >= GP4_GLOBAL(int16_t, 0x006013e2)) {
        return cancel(s, car, segment);
    }
    s.eax = GP4_FIELD(uint32_t, car, 0xac);
    if (static_cast<int32_t>(s.eax) < 0) {
        s.eax = 0u - s.eax;
    }
    if (static_cast<int32_t>(s.eax) >= 0x80000) {
        return cancel(s, car, segment);
    }

    if ((byteAt(car, 0xe5) & 4u) != 0u) {
        gp4::call_regs(0x00434875, s);
        if ((s.eflags & 1u) != 0u) {
            return engage(s, car);
        }
        uint16_t heading = static_cast<uint16_t>(static_cast<uint16_t>(wordAt(car, 0xe))
            - GP4_FIELD(uint16_t, segment, 0) - GP4_FIELD(uint16_t, segment, 0x18));
        if (static_cast<int16_t>(heading) < 0) {
            heading = static_cast<uint16_t>(0u - heading);
        }
        setAx(s, heading);
        if (static_cast<int16_t>(heading) >= 0x2aaa || (byteAt(car, 0x231) & 8u) != 0u) {
            return engage(s, car);
        }
        return cancel(s, car, segment);
    }

    if ((byteAt(car, 0xcc) & 0x40u) != 0u) {
        return engage(s, car);
    }
    if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) == 0u && (byteAt(car, 0x8d) & 2u) != 0u
        && (byteAt(car, 0xff) & 1u) == 0u) {
        return release(s, car, segment);
    }
    if ((byteAt(car, 0xe7) & 2u) == 0u) {
        s.eax = GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u ? 0x1388u : 0x4e20u;
        gp4::call_regs(0x004110dd, s);
        if ((s.eflags & 1u) != 0u) {
            s.eax = GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u ? 0x7d0u : 0x2710u;
        }
        GP4_FIELD(uint32_t, car, 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, car, 0xf0) += s.eax;
        byteAt(car, 0xe7) = static_cast<uint8_t>(byteAt(car, 0xe7) | 2u);
        return release(s, car, segment);
    }
    const uint32_t deadline = GP4_FIELD(uint32_t, car, 0xf0);
    if (deadline != 0u && static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - deadline) >= 0) {
        return engage(s, car);
    }
    return release(s, car, segment);
}

// FUNCTION: GP4 0x004349dc
// Per-car flag update on the car esi against the reference record edi: decides between the
// "engaged" state (bit 6 of +0xcc set, 0x0043488b then four 0x00441af6 effects with kinds in
// [0x0062fc10]) and its release (bits of +0xe7/+0xcc/+0x25f cleared), gated by flags, the speed
// word +0x9a, |dword +0xac|, the timers 0x00434930 / 0x004110dd (+0xf0 against the clock
// 0x0066d814) and finally sets or clears bit 4 of +0xcc. The flags are the last flag writer's.
void __cdecl FUN_004349dc(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    const uint32_t flags = evaluate(s, car, segment);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->edi = s.edi;
    r->eflags = (s.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004349dc, FUN_004349dc, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x420]:layout=car_00434700+rec_00413dd5+car_004349dc+car_0043c677, edi:ptr[0xd0]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); cases=800; globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+state_00441c0c+seg_00441af6+lim_00434700+rows_00434044+state_0041e500+state_0040ee7b+gate_004349dc")
