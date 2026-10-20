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

// Flags of an 8-bit TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t testFlags8(uint8_t result) {
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x6d];
    uint8_t Value6d;
    uint8_t Unknown6e;
    uint8_t Flags6f;
    uint8_t Unknown70[0x76];
    uint8_t FlagsE6;
};

// Carry set when the car record in esi has the flag at +0x6f bit 4 (flag at +0xe6 bit 4 clear) or the signed
// byte +0x6d is >= 0x40 (flag at +0xe6 bit 4 set); clear otherwise. The other flags are those of the last
// test / cmp. eax is saved and restored.
// FUNCTION: GP4 0x00443629
void __cdecl FUN_00443629(gp4::Regs* r) {
    const Unknown_Car* car = reinterpret_cast<const Unknown_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    uint32_t carry;

    if ((car->FlagsE6 & 0x10u) != 0u) {
        const uint8_t value = car->Value6d;
        flags = cmpFlags8(value, 0x40u);
        carry = static_cast<int8_t>(value) < 0x40 ? 0u : 1u;
    } else {
        flags = testFlags8(static_cast<uint8_t>(car->Flags6f & 0x10u));
        carry = (car->Flags6f & 0x10u) != 0u ? 1u : 0u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x00443629, FUN_00443629, "regs(esi:ptr[0xee]:bytes) -> (cf, pf, af, zf, sf, of)")
