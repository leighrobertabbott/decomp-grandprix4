#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Debris slot eax (flag byte 0x0063bcfc[eax], owning car number 0x0063bd3c[eax]): CF is set when the slot is in
// state 1 and its car number equals the car byte +0x202 of the record in esi. The other flags are those of the
// last cmp; edx is saved and restored.
// FUNCTION: GP4 0x004440a0
void __cdecl FUN_004440a0(gp4::Regs* r) {
    const uint32_t slot = r->eax;
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t state = GP4_ARRAY(uint8_t, 0x0063bcfc)[slot];

    uint32_t flags = cmpFlags8(state, 1u);
    uint32_t carry = 0u;
    if (state == 1u) {
        const uint8_t carSlot = car[0x202];
        const uint8_t owner = GP4_ARRAY(uint8_t, 0x0063bd3c)[slot];
        flags = cmpFlags8(carSlot, owner);
        if (carSlot == owner) {
            carry = 1u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x004440a0, FUN_004440a0, "regs(eax:u32[0..63], esi:ptr[0x20a]:layout=car_00424cf5) -> (cf, pf, af, zf, sf, of); globals=layout=slots_0043ad30")
