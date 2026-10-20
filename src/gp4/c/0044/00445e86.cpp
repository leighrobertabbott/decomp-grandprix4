#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic operation (TEST / OR / AND): CF = OF = AF = 0.
static inline uint32_t logic8Flags(uint8_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit ADD.
static inline uint32_t add32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((~(left ^ right)) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00445e86_Car {
    uint8_t Unknown00[0x81];
    uint8_t Flags81;
    uint8_t Unknown82[0x63];
    uint8_t FlagsE5;
};

// Pit-board refresh for the selected car [0x62f8b0] (esi, null = nothing selected). With the
// mode byte 0x7a8dfb == 0xff the car (when bit 4 of +0x81 is clear) restarts: the deadline
// [0x62f8d8] is cleared and bit 2 of +0xe5 set. Otherwise, a car with bit 2 of +0xe5 set runs
// 0x445f46 (message text for the time left) and either arms the deadline at clock + 5000 or,
// once the clock passes it, clears the deadline and the bit and calls 0x40f6d0. The selected car
// being the viewed car [0x679d30] additionally calls 0x51f094 / 0x51f07a. The flags are those of
// the last flag writer on each path.
// FUNCTION: GP4 0x00445e86
void __cdecl FUN_00445e86(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x0062f8b0);

    uint32_t flags = 0;
    if (c.esi == 0) {
        flags = 0x44;
    } else {
        Unknown_00445e86_Car* car = reinterpret_cast<Unknown_00445e86_Car*>(static_cast<uintptr_t>(c.esi));
        if (GP4_GLOBAL(uint8_t, 0x007a8dfb) == 0xff) {
            const uint8_t seen = static_cast<uint8_t>(car->Flags81 & 0x10u);
            flags = logic8Flags(seen);
            if (seen == 0) {
                if (c.esi == GP4_GLOBAL(uint32_t, 0x00679d30)) {
                    gp4::call_regs(0x0051f094, c);
                }
                GP4_GLOBAL(uint32_t, 0x0062f8d8) = 0;
                const uint8_t armed = static_cast<uint8_t>(car->FlagsE5 & 4u);
                flags = logic8Flags(armed);
                if (armed == 0) {
                    car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 | 4u);
                    flags = logic8Flags(car->FlagsE5);
                }
            }
        } else {
            const uint8_t armed = static_cast<uint8_t>(car->FlagsE5 & 4u);
            flags = logic8Flags(armed);
            if (armed != 0) {
                if (c.esi == GP4_GLOBAL(uint32_t, 0x00679d30)) {
                    gp4::call_regs(0x0051f07a, c);
                }
                gp4::call_regs(0x00445f46, c);
                flags = c.eflags & 0x8d5u;
                if (GP4_GLOBAL(uint32_t, 0x0062f8d8) == 0) {
                    GP4_GLOBAL(uint32_t, 0x0062f8d8) = GP4_GLOBAL(uint32_t, 0x0066d814);
                    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062f8d8);
                    GP4_GLOBAL(uint32_t, 0x0062f8d8) = before + 0x1388u;
                    flags = add32Flags(before, 0x1388u);
                } else {
                    const uint32_t negated = 0u - GP4_GLOBAL(uint32_t, 0x0062f8d8);
                    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                    flags = add32Flags(negated, clock);
                    if ((negated + clock) >> 31 == 0u) {
                        GP4_GLOBAL(uint32_t, 0x0062f8d8) = 0;
                        car->FlagsE5 = static_cast<uint8_t>(car->FlagsE5 & 0xfbu);
                        flags = logic8Flags(car->FlagsE5);
                        c.eflags = (c.eflags & ~0x8d5u) | flags;
                        gp4::call_regs(0x0040f6d0, c);
                        flags = c.eflags & 0x8d5u;
                    }
                }
            }
        }
    }
    c.eflags = (c.eflags & ~0x8d5u) | flags;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00445e86, FUN_00445e86, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+carbg_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+carno_0066ff6c+flag_007a97a0+state_0042595c+words_0062d210+state_0066d8c0+cars_00444d60+state_00442d1f+clock_0066d814+flag_00604418+state_00446012")
