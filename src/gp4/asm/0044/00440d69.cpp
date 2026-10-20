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

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit OR (CF = OF = AF = 0).
static inline uint32_t orFlags8(uint8_t result) {
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_Object {
    uint8_t Unknown00[0x84];
    uint8_t Mode84;
    uint8_t Unknown85[2];
    uint8_t Flags87;
};

// For an object in mode 3 (byte +0x84), a random byte from the generator 0x00401462 below 0x5a sets bit 2 of
// byte +0x87. eax is the random byte (or unchanged when the mode differs); flags are those of the last
// cmp / or.
// FUNCTION: GP4 0x00440d69
void __cdecl FUN_00440d69(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = cmpFlags8(object->Mode84, 3u);

    if (object->Mode84 == 3u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00401462, call);

        const uint32_t draw = call.eax & 0xffu;
        r->eax = draw;
        flags = cmpFlags32(draw, 0x5au);
        if (static_cast<int32_t>(draw) < 0x5a) {
            object->Flags87 = static_cast<uint8_t>(object->Flags87 | 4u);
            flags = orFlags8(object->Flags87);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440d69, FUN_00440d69, "regs(eax:u32, esi:ptr[0x8f]:layout=car_00411109) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
