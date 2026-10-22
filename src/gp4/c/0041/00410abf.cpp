#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test byte, 0xff`: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of `cmp byte value, 0xff`.
static inline uint32_t cmpFlags8Ff(uint32_t value) {
    value &= 0xffu;
    const uint32_t result = (value - 0xffu) & 0xffu;
    return (value < 0xffu ? 1u : 0u) | parityFlag(result) | ((value ^ 0xffu ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | (((value ^ 0xffu) & (value ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of `or reg, reg` on a 32-bit value: CF = OF = AF = 0.
static inline uint32_t orFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// With byte [0x7aafd4] set (two cars in play): runs 0x00445328 first and only continues with the first
// car when byte [0x7a90fc] is 0xff; then runs 0x00410a65 for the car [0x66d820] (if any) and, with
// [0x7aafd4] set and byte [0x8914e3] == 0xff, for the car [0x66d828]. pushal / popal restores every
// register; the flags are those of the last flag writer (the callee after a call).
// FUNCTION: GP4 0x00410abf
void __cdecl FUN_00410abf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t flags = testFlags8(GP4_GLOBAL(uint8_t, 0x007aafd4));
    bool firstCar = true;
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        gp4::call_regs(0x00445328, c);
        flags = c.eflags;
        const uint8_t marker = GP4_GLOBAL(uint8_t, 0x007a90fc);
        flags = cmpFlags8Ff(marker);
        firstCar = marker == 0xffu;
    }
    if (firstCar) {
        const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d820);
        c.esi = car;
        flags = orFlags32(car);
        if (car != 0u) {
            gp4::call_regs(0x00410a65, c);
            flags = c.eflags;
        }
    }

    const uint8_t twoCars = GP4_GLOBAL(uint8_t, 0x007aafd4);
    flags = testFlags8(twoCars);
    if (twoCars != 0u) {
        const uint8_t marker = GP4_GLOBAL(uint8_t, 0x008914e3);
        flags = cmpFlags8Ff(marker);
        if (marker == 0xffu) {
            const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d828);
            c.esi = car;
            flags = orFlags32(car);
            if (car != 0u) {
                gp4::call_regs(0x00410a65, c);
                flags = c.eflags;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00410abf, FUN_00410abf, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=setup_007adb54+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+wheel_00410a1c+state_0040fbf3+state_00410a65+state_0040fa01+send_004453d1; gfields={0x007a90fc:u8{=0xff,0xff,0,1}, 0x008914e3:u8{=0xff,0xff,0,1}}")
