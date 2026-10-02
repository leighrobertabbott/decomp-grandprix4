#include <gp4/gp4.h>

struct Unknown_00408c07 {
    uint8_t Unknown00[0x7c];
    uint8_t Unknown7C;
};

static inline uint32_t logicalFlags32(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00408c07
void __cdecl FUN_00408c07(gp4::Regs* r) {
    const Unknown_00408c07* body = reinterpret_cast<const Unknown_00408c07*>(r->esi);
    const uint32_t index = static_cast<uint8_t>(body->Unknown7C - 1u) & 0x3fu;

    GP4_ARRAY(uint32_t, 0x0066fb80)[index] = 0x21980000u;
    GP4_ARRAY(uint32_t, 0x0066fbd8)[index] = 0x21980000u;
    GP4_ARRAY(uint32_t, 0x0066fc30)[index] = 0x21980000u;
    GP4_ARRAY(uint32_t, 0x0066fc88)[index] = 0x21980000u;

    // pushal/popal restores every GPR; only the flags of "and edx, 0x3f" survive.
    r->eflags = (r->eflags & ~0x8d5u) | logicalFlags32(index);
}
GP4_IMPL(0x00408c07, FUN_00408c07, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of)")
