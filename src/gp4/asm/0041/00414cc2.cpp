#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic operation (test / and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit "cmp left, right".
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~0x8d5u) | flags;
}

static inline uint32_t withLow8(uint32_t reg, uint8_t value) {
    return (reg & 0xffffff00u) | value;
}

static inline uint32_t withLow16(uint32_t reg, uint16_t value) {
    return (reg & 0xffff0000u) | value;
}

// edx = 0x19, al = side, 0x00434090; eax = 0xb, 0x00434044.
static inline void notify(gp4::Regs& s, uint8_t side) {
    s.edx = 0x19u;
    s.eax = withLow8(s.eax, side);
    gp4::call_regs(0x00434090, s);
    s.eax = 0xbu;
    gp4::call_regs(0x00434044, s);
}

// FUNCTION: GP4 0x00414cc2
// esi = car record, edi = its track-segment record. State machine on the flag byte +0x7a
// (bits 0x40 / 1 / 0x10 / 8 / 4 / 2) with the clock 0x0066d814 timers at +0xf0.
void __cdecl FUN_00414cc2(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.esi));
    uint8_t* const segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.edi));

    GP4_FIELD(uint8_t, car, 0x6f) |= 2u;

    const uint8_t mode = GP4_FIELD(uint8_t, car, 0x7a);
    if ((mode & 0x40u) != 0u) {
        setFlags(s, logicFlags8(mode & 0x40u));
        goto done;
    }
    if ((GP4_FIELD(uint8_t, car, 0x1ce) & 2u) != 0u) {
        setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x1ce) & 2u));
        goto done;
    }

    if ((mode & 1u) != 0u) {
        const uint32_t due = GP4_FIELD(uint32_t, car, 0xf0);
        if (due != 0u) {
            const uint32_t negated = 0u - due;
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            setFlags(s, addFlags32(negated, clock));
            if (static_cast<int32_t>(negated + clock) >= 0) {
                gp4::call_regs(0x00408708, s);
                notify(s, 0xffu);
                goto done;
            }
        }
        gp4::call_regs(0x00414aba, s);
        goto done;
    }

    if ((mode & 0x10u) == 0u) {
        if ((GP4_FIELD(uint8_t, car, 0xcc) & 0x80u) == 0u && (GP4_FIELD(uint8_t, car, 0x26b) & 8u) == 0u) {
            gp4::call_regs(0x00414aba, s);
            GP4_FIELD(uint8_t, car, 0x7b) |= 0x80u;
            setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x7b)));
            gp4::call_regs(0x004086f9, s);
            goto done;
        }

        GP4_FIELD(uint8_t, car, 0x7b) |= 0x80u;
        setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x7b)));
        gp4::call_regs(0x004086f9, s);
        GP4_FIELD(uint8_t, car, 0x7a) |= 0x10u;
        GP4_FIELD(uint8_t, car, 0xfe) &= 0xfbu;

        bool arm = false;
        if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0u && GP4_FIELD(uint32_t, car, 0x40) == 0u
            && static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) < 0) {
            GP4_FIELD(uint8_t, car, 0x71) &= 0x7fu;
            uint16_t ax = GP4_FIELD(uint16_t, car, 0x90);
            if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
            ax = static_cast<uint16_t>(ax - GP4_FIELD(uint16_t, car, 0xb0));
            ax = static_cast<uint16_t>(ax - 0x100u);
            s.eax = withLow16(s.eax, ax);
            if (static_cast<int16_t>(ax) >= 0x500) {
                const uint8_t lane = static_cast<uint8_t>(GP4_FIELD(uint8_t, segment, 0x48) & 3u);
                s.eax = withLow8(s.eax, lane);
                if (lane == 0u) {
                    ax = GP4_FIELD(uint16_t, segment, 0x14);
                    s.eax = withLow16(s.eax, ax);
                    if (ax == 0u) {
                        arm = true;
                    } else {
                        const uint16_t dx = GP4_FIELD(uint16_t, car, 0x90);
                        s.edx = withLow16(s.edx, dx);
                        ax = static_cast<uint16_t>(ax ^ dx);
                        s.eax = withLow16(s.eax, ax);
                        arm = static_cast<int16_t>(ax) >= 0;
                    }
                }
            }
        }

        if (arm) {
            GP4_FIELD(uint8_t, car, 0x7a) |= 1u;
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_FIELD(uint32_t, car, 0xf0) = clock + 6000u;
            setFlags(s, addFlags32(clock, 6000u));
            goto done;
        }
        if ((GP4_FIELD(uint8_t, car, 0x26b) & 8u) == 0u) {
            notify(s, 0u);
            goto done;
        }
        notify(s, 0xffu);
        GP4_FIELD(uint8_t, car, 0x1ce) |= 2u;
        setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x1ce)));
        goto done;
    }

    if ((mode & 8u) == 0u) {
        gp4::call_regs(0x00414aba, s);
        s.eax = 8u;
        gp4::call_regs(0x00433ff8, s);
        if ((s.eflags & gp4::CF) == 0u) goto done;
        GP4_FIELD(uint8_t, car, 0x7a) |= 8u;
        GP4_FIELD(uint8_t, car, 0x7b) &= 0x7fu;
        setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x7b)));
        GP4_FIELD(uint32_t, car, 0xa4) = 0;
        GP4_FIELD(uint32_t, car, 0xa8) = 0;
        GP4_GLOBAL(uint32_t, 0x0063803c) = 0;
        goto done;
    }

    GP4_FIELD(uint32_t, car, 0xa4) = 0;
    GP4_FIELD(uint32_t, car, 0xa8) = 0;
    GP4_GLOBAL(uint32_t, 0x0063803c) = 0;

    if ((mode & 4u) == 0u) {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623ddc))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
        const uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        s.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        GP4_FIELD(uint16_t, car, 0x282) = static_cast<uint16_t>(value);
        GP4_FIELD(uint16_t, car, 0x27c) = static_cast<uint16_t>(value);
        GP4_FIELD(uint16_t, car, 0x280) = 0xe00;
        GP4_FIELD(uint16_t, car, 0x27a) = 0xe00;
        const uint16_t level = GP4_FIELD(uint16_t, car, 0x27e);
        s.eax = withLow16(value, level);
        setFlags(s, subFlags16(level, 0xdc0u));
        if (static_cast<int16_t>(level) < 0xdc0) goto done;
        GP4_FIELD(uint8_t, car, 0x7a) |= 4u;
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, car, 0xf0) = clock + 500u;
        setFlags(s, addFlags32(clock, 500u));
        goto done;
    }

    if ((mode & 2u) == 0u) {
        const uint32_t due = GP4_FIELD(uint32_t, car, 0xf0);
        if (due == 0u) {
            setFlags(s, logicFlags8(0u));  // test dword, -1 on zero: ZF = PF = 1
            goto done;
        }
        const uint32_t negated = 0u - due;
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        setFlags(s, addFlags32(negated, clock));
        if (static_cast<int32_t>(negated + clock) < 0) goto done;
        GP4_FIELD(uint8_t, car, 0x7a) |= 2u;
        setFlags(s, logicFlags8(GP4_FIELD(uint8_t, car, 0x7a)));
        goto done;
    }

    gp4::call_regs(0x00414c1a, s);

done:
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = s.eflags;
}
GP4_IMPL(0x00414cc2, FUN_00414cc2, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x2c4]:layout=car_00411109+rec_00414562+car_00414cc2, edi:ptr[0x300]:layout=rec_004132e5+body_0043ebc2+edi_00414cc2) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00434044+state_00433ff8+flags_0066d909+state_00414cc2")
