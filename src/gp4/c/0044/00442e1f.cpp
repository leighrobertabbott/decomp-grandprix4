#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST / OR (CF = OF = AF = 0) with result `value`.
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return byteParity(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// stc / clc keep the other flags of the last flag writer and replace CF.
static inline uint32_t withCarry(uint32_t writerFlags, uint32_t carry) {
    return (writerFlags & ~1u) | carry;
}

static inline gp4::Regs callWith(const gp4::Regs* r, uint32_t addr, uint32_t eax, uint32_t edx, uint32_t ebx, uint32_t esi,
                                 uint32_t flags) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.edx = edx;
    c.ebx = ebx;
    c.esi = esi;
    c.eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
    gp4::call_regs(addr, c);
    return c;
}

// Re-initialises the car whose number the byte [0x0066d897] holds (nothing, CF = 1, when it is zero): the
// number goes to [0x0066d896] and 0x00409b87 selects the car into [0x0066d820]. A missing car, or one with
// bit 5 of +0x7a, only logs the number into the opcode stream (0x00444ee7, when [0x007aafd4] is set) and
// returns CF = 1. Otherwise the car is reset through 0x00410d04 (with [0x00604419] raised to 0xff around it;
// bit 1 of +0x7b is set again when it was set before), +0xf6 receives 0x4000 jittered by 0x0043ce04 from the
// car number byte +0x7c, +0x10e becomes 0x4000, 0x0040f6d0 runs, +0x83 gets bit 5, [0x0095404e] becomes 0xff,
// and the reset is logged with opcode 5 (0x00444d4f, when [0x007aafd4] is set); CF = 0. pushal / popal restore
// every register; the flags are those of the last flag writer with CF replaced.
// FUNCTION: GP4 0x00442e1f
void __cdecl FUN_00442e1f(gp4::Regs* r) {
    const uint8_t requested = GP4_GLOBAL(uint8_t, 0x0066d897);
    if (requested == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | withCarry(logicFlags8(requested), 1u);
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0066d896) = requested;

    callWith(r, 0x00409b87, r->eax, r->edx, r->ebx, r->esi, r->eflags);

    const uint32_t carAddress = GP4_GLOBAL(uint32_t, 0x0066d820);
    uint32_t flags = byteParity(carAddress) | (carAddress == 0u ? 0x40u : 0u) | ((carAddress & 0x80000000u) != 0u ? 0x80u : 0u);

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(carAddress));
    bool usable = false;
    if (carAddress != 0u) {
        const uint32_t retired = car[0x7a] & 0x20u;
        flags = logicFlags8(retired);
        usable = (retired == 0u);
    }

    if (!usable) {
        const uint8_t logging = GP4_GLOBAL(uint8_t, 0x007aafd4);
        flags = logicFlags8(logging);
        if (logging != 0u) {
            gp4::Regs logged = callWith(r, 0x00444ee7, r->eax, r->edx, r->ebx, r->esi, flags);
            flags = logged.eflags & 0x8d5u;
        }
        r->eflags = (r->eflags & ~0x8d5u) | withCarry(flags, 1u);
        return;
    }

    const uint32_t esi = carAddress;
    const uint32_t ebx = 0x007ab548u;
    const uint32_t carByte = car[0x7c];
    const uint32_t edx = (r->edx & 0xffffff00u) | carByte;

    const bool hadBit = (car[0x7b] & 2u) != 0u;
    flags = logicFlags8(car[0x7b] & 2u);
    GP4_GLOBAL(uint8_t, 0x00604419) = 0xff;
    gp4::Regs reset = callWith(r, 0x00410d04, r->eax, r->edx, r->ebx, esi, flags);
    GP4_GLOBAL(uint8_t, 0x00604419) = 0;
    flags = reset.eflags & 0x8d5u;
    if (hadBit) {
        car[0x7b] = static_cast<uint8_t>(car[0x7b] | 2u);
        flags = logicFlags8(car[0x7b]);
    }

    gp4::Regs jitter = callWith(r, 0x0043ce04, 0x4000u, edx, ebx, esi, flags);
    flags = jitter.eflags & 0x8d5u;
    GP4_FIELD(uint16_t, car, 0xf6) = static_cast<uint16_t>(jitter.eax);
    GP4_FIELD(uint16_t, car, 0x10e) = 0x4000u;

    callWith(r, 0x0040f6d0, 0x4000u, edx, ebx, esi, flags);
    car[0x83] = static_cast<uint8_t>(car[0x83] | 0x20u);
    flags = logicFlags8(car[0x83]);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xff;

    const uint8_t logging = GP4_GLOBAL(uint8_t, 0x007aafd4);
    flags = logicFlags8(logging);
    if (logging != 0u) {
        gp4::Regs logged = callWith(r, 0x00444d4f, 0x4000u, edx, ebx, esi, flags);
        flags = logged.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | withCarry(flags, 0u);
}
GP4_IMPL(0x00442e1f, FUN_00442e1f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab590+carst_0040f6d0+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+state_0066d8c0+state_0043b920+carno_0066ff6c")
