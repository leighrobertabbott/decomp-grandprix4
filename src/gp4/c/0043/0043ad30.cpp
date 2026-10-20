#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// The 22 car records at 0x0066fef0 (only the flag byte this routine tests).
struct Unknown_0066fef0 {
    uint8_t Unknown00[0xe5];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x41c - 0xe6];
};
static_assert(sizeof(Unknown_0066fef0) == 0x41c, "car stride");

// One of the two slots: when the object state byte [0x63bcfc + slot] is 2 and its car (index byte
// [0x63bd3c + slot]) has not been reset yet (+0xe5 bit 0 clear), run the car reset 0x0040f6d0 with
// esi = car. Returns the flags the last executed flag writer leaves.
static __forceinline uint32_t resetSlotCarIfNeeded(gp4::Regs* r, uint32_t slot) {
    const uint32_t state = GP4_ARRAY(uint8_t, 0x0063bcfc)[slot];
    uint32_t flags = cmpFlags8(state, 2u);
    if (state == 2u) {
        Unknown_0066fef0* car = GP4_ARRAY(Unknown_0066fef0, 0x0066fef0) + GP4_ARRAY(uint8_t, 0x0063bd3c)[slot];
        const uint32_t reset = car->FlagsE5 & 1u;
        flags = logicFlags8(reset);
        if (reset == 0u) {
            gp4::Regs c = *r;
            c.ebx = slot;
            c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            c.eflags = (c.eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x0040f6d0, c);
            flags = c.eflags & 0x8d5u;
        }
    }
    return flags;
}

// FUNCTION: GP4 0x0043ad30
// For the two object slots [0x006097f0] and [0x006097f4]: if the slot's state byte is 2 and its car
// (0x41c-byte records at 0x0066fef0, index byte at 0x0063bd3c + slot) is not yet reset, runs the car
// reset 0x0040f6d0. ebx/esi are saved; the flags are those of the last cmp / test / callee.
void __cdecl FUN_0043ad30(gp4::Regs* r) {
    resetSlotCarIfNeeded(r, GP4_GLOBAL(uint32_t, 0x006097f0));
    const uint32_t flags = resetSlotCarIfNeeded(r, GP4_GLOBAL(uint32_t, 0x006097f4));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ad30, FUN_0043ad30, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=trackgeo_008943a0+tbl_00868598+bytes_0063bcfc+flags_00408f7b+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+segflag_004381ab+slots_0043ad30+carst_0040f6d0")
