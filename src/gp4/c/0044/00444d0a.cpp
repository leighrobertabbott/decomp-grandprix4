#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `and` result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Opcode 4 of the stream dispatcher: applies the next stream record to its car (0x00445130 finds it; CF set =
// no such car, nothing else happens). The car gets bit 2 of +0xe5 set, its +0xe6 flags cleared (0x00410d92),
// is told the difficulty word [0x007ad89e] through 0x0043cdf8 / 0x0043ce04 (dl = car number +0x7c), has its
// +0x6a..+0x6f bytes cleared (0x0040f9e6), the car pointer [0x0066d828] is zeroed, +0x83 bit 5 is set and +0x8d
// bit 5 cleared. The flags are those of the final `and`.
// FUNCTION: GP4 0x00444d0a
void __cdecl FUN_00444d0a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00445130, c);
    if ((c.eflags & 1u) == 0u) {
        uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
        car[0xe5] = static_cast<uint8_t>(car[0xe5] | 4u);
        gp4::call_regs(0x00410d92, c);
        c.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
        gp4::call_regs(0x0043cdf8, c);
        c.edx = (c.edx & 0xffffff00u) | car[0x7c];
        gp4::call_regs(0x0043ce04, c);
        gp4::call_regs(0x0040f9e6, c);
        GP4_GLOBAL(uint32_t, 0x0066d828) = 0u;
        car[0x83] = static_cast<uint8_t>(car[0x83] | 0x20u);
        car[0x8d] = static_cast<uint8_t>(car[0x8d] & 0xdfu);
        c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(car[0x8d]);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00444d0a, FUN_00444d0a, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c+stream_0062d1a8+flag_007a97a0+state_0042595c+words_0062d210+state_0066d8c0+cars_0066fef0")
