#include <gp4/gp4.h>

static inline uint32_t subtractionFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Source {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
};

struct Unknown_Dest {
    uint8_t Unknown00[0x2a];
    uint8_t Unknown2A;
    uint8_t Unknown2B[0x79];
    int16_t UnknownA4;
    int16_t UnknownA6;
    uint8_t UnknownA8[0xb4];
    int16_t Unknown15C;
};

// FUNCTION: GP4 0x0043d76b
void __cdecl FUN_0043d76b(gp4::Regs* r) {
    const Unknown_Source* source = reinterpret_cast<const Unknown_Source*>(static_cast<uintptr_t>(r->esi));
    Unknown_Dest* dest = reinterpret_cast<Unknown_Dest*>(static_cast<uintptr_t>(r->edi));
    const uint32_t upper = r->eax & 0xffff0000u;
    uint32_t low16;
    uint16_t left;
    uint16_t right;
    uint32_t carry;
    if (GP4_GLOBAL(uint8_t, 0x00622426) == 0xff) {
        const int16_t value = source->Unknown9A;
        left = static_cast<uint16_t>(value);
        right = static_cast<uint16_t>(dest->UnknownA4);
        low16 = left;
        if (value >= dest->UnknownA4) {
            carry = 1;
        } else {
            dest->UnknownA4 = value;
            carry = 0;
        }
    } else {
        const int16_t value = dest->UnknownA6;
        left = static_cast<uint16_t>(value);
        right = static_cast<uint16_t>(source->Unknown9A);
        low16 = left;
        if (value <= source->Unknown9A) {
            carry = 1;
        } else {
            const int16_t replacement = source->Unknown9A;
            dest->UnknownA6 = replacement;
            dest->Unknown15C = replacement;
            const uint8_t flagsByte = GP4_GLOBAL(uint8_t, 0x0062698a);
            dest->Unknown2A = flagsByte;
            low16 = (static_cast<uint16_t>(replacement) & 0xff00u) | flagsByte;
            carry = 0;
        }
    }
    r->eax = upper | low16;
    r->eflags = (r->eflags & ~0x8d5u) | ((subtractionFlags16(left, right) & ~1u) | carry);
}
GP4_IMPL(0x0043d76b, FUN_0043d76b, "regs(eax:u32, esi:ptr[0xa4]:bytes, edi:ptr[0x16c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
