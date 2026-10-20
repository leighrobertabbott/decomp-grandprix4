#include <gp4/gp4.h>

struct Unknown_Object {
    uint8_t Unknown00[0x75];
    uint8_t Selector75;
    uint8_t Unknown76[0x0a];
    uint8_t Flags80;
};

static inline uint32_t byteParity(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit OR (CF = OF = AF = 0).
static inline uint32_t orFlags8(uint8_t result) {
    return byteParity(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u);
}

// FUNCTION: GP4 0x00440d37
void __cdecl FUN_00440d37(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));
    const uint8_t selector = object->Selector75;
    uint32_t flags = 0;

    flags = cmpFlags8(selector, 0);
    if (selector == 0) {
        object->Flags80 = static_cast<uint8_t>(object->Flags80 | 0x20);
        flags = orFlags8(object->Flags80);
    }
    flags = cmpFlags8(selector, 1);
    if (selector == 1) {
        object->Flags80 = static_cast<uint8_t>(object->Flags80 | 0x10);
        flags = orFlags8(object->Flags80);
    }
    flags = cmpFlags8(selector, 2);
    if (selector == 2) {
        object->Flags80 = static_cast<uint8_t>(object->Flags80 | 0x08);
        flags = orFlags8(object->Flags80);
    }
    flags = cmpFlags8(selector, 3);
    if (selector == 3) {
        object->Flags80 = static_cast<uint8_t>(object->Flags80 | 0x04);
        flags = orFlags8(object->Flags80);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440d37, FUN_00440d37, "regs(esi:ptr[0x88]:u8[0..3]) -> (cf, pf, af, zf, sf, of)")
