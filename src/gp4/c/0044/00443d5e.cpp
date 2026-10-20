#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST byte, 0xff (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint8_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80u) != 0u ? 0x080u : 0u);
}

// When the mode byte [0x007c2ea9] is negative: counts the 22 car records at 0x0066fef0 whose byte +0x7a bit 5 is
// clear or whose byte +0x8d bit 2 is set into the word [0x0066d89c], sets bit 2 of the byte at 0x00675958 + 0x88
// (the esi left after the walk) and runs 0x0043b689. pushal / popal restore every register; the flags are those
// the callee leaves.
// FUNCTION: GP4 0x00443d5e
void __cdecl FUN_00443d5e(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if ((mode & 0x80u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | testFlags8(mode);
        return;
    }

    uint32_t count = 0u;
    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (uint32_t i = 0; i != 0x16u; ++i) {
        if ((GP4_FIELD(uint8_t, car, 0x7a) & 0x20u) == 0u || (GP4_FIELD(uint8_t, car, 0x8d) & 4u) != 0u) {
            count = (count + 1u) & 0xffffu;
        }
        car += 0x41c;
    }
    GP4_GLOBAL(uint16_t, 0x0066d89c) = static_cast<uint16_t>(count);
    GP4_FIELD(uint8_t, car, 0x88) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x88) | 4u);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffff0000u) | count;
    c.ecx = 0u;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    gp4::call_regs(0x0043b689, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00443d5e, FUN_00443d5e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lap_0043b482+state_004059ef+state_0044334d")
