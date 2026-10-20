#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of the 32-bit "cmp lhs, rhs" (lhs - rhs)
static inline uint32_t cmpFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// eax = DAT_00629e08; when it is not below DAT_00629dc0 (signed) runs the record update 0x00416387.
// FUNCTION: GP4 0x00437846
void __cdecl FUN_00437846(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00629e08);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00629dc0);
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(value, limit);
    if (static_cast<int32_t>(value) >= static_cast<int32_t>(limit)) {
        gp4::call_regs(0x00416387, *r);
    }
}
GP4_IMPL(0x00437846, FUN_00437846, "regs(esi:ptr[0x106]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
