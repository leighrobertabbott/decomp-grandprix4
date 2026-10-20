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

struct Unknown_Object {
    uint8_t Unknown00[0x75];
    uint8_t Selector75;
    uint8_t Unknown76[0x0e];
    uint8_t Mode84;
    uint8_t Unknown85[2];
    uint8_t Flags87;
};

// For an object in mode 2 (byte +0x84): picks a random selector 0..3 (generator 0x00401462) into byte +0x75,
// sets bit 7 of byte +0x87 and lets 0x00440d37 act on the selector. eax is the selector (unchanged in other
// modes); flags are those of the cmp or of 0x00440d37.
// FUNCTION: GP4 0x00440cc8
void __cdecl FUN_00440cc8(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = cmpFlags8(object->Mode84, 2u);

    if (object->Mode84 == 2u) {
        gp4::Regs draw = *r;
        draw.st_in = 0;
        draw.st_out = 0;
        gp4::call_regs(0x00401462, draw);

        const uint32_t selector = draw.eax & 3u;
        r->eax = selector;
        object->Selector75 = static_cast<uint8_t>(selector);
        object->Flags87 = static_cast<uint8_t>(object->Flags87 | 0x80u);

        gp4::Regs apply = *r;
        apply.st_in = 0;
        apply.st_out = 0;
        gp4::call_regs(0x00440d37, apply);
        flags = apply.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440cc8, FUN_00440cc8, "regs(eax:u32, esi:ptr[0x8f]:layout=rec_0041b950) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
