#include <gp4/gp4.h>

static inline uint32_t logicFlags8(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_004171b6 {
    uint8_t Unknown00[0xe5];
    uint8_t FlagsE5;
};

// FUNCTION: GP4 0x004171b6
void __cdecl FUN_004171b6(gp4::Regs* r) {
    Unknown_004171b6* self = reinterpret_cast<Unknown_004171b6*>(r->esi);
    const uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    const uint8_t selector = base[0x2e] | base[0x32e];
    uint8_t value;
    if ((selector & 0x80u) != 0u) {
        value = self->FlagsE5 | 0x80u | 0x40u;
    } else {
        value = self->FlagsE5 & 0xbfu;
        if ((value & 0x20u) != 0u) {
            value = value | 0x80u;
        } else {
            value = value & 0x7fu;
        }
    }
    self->FlagsE5 = value;

    r->ebx = 0x006380d8;
    r->eax = (r->eax & 0xffffff00u) | selector;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(value);
}
GP4_IMPL(0x004171b6, FUN_004171b6, "regs(esi:ptr[0xed]:bytes, eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=bits_004171b6")
