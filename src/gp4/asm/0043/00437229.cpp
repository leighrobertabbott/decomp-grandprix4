#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00437229_Body {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
};

// Turns |word at +0x9a| of the body in esi / ebx into a clamped response value:
// 4 * x below 0x200, x + 0x600 from there, limited to 0x4000.
static inline int32_t scaleMagnitude(int32_t x, uint32_t* flags) {
    if (x < 0) {
        x = static_cast<int32_t>(0u - static_cast<uint32_t>(x));
    }
    if (x < 0x200) {
        x = static_cast<int32_t>(static_cast<uint32_t>(x) << 2);
    } else {
        x = (x - 0x200) + 0x800;
    }
    *flags = subFlags32(static_cast<uint32_t>(x), 0x4000u);
    if (x > 0x4000) {
        x = 0x4000;
    }
    return x;
}

// Stores the responses of the two bodies in [0x00629e28] (esi) and [0x00629e50] (ebx);
// eax keeps the second one and the flags those of its final `cmp eax, 0x4000`.
// FUNCTION: GP4 0x00437229
void __cdecl FUN_00437229(gp4::Regs* r) {
    const Unknown_00437229_Body* first = reinterpret_cast<const Unknown_00437229_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00437229_Body* second = reinterpret_cast<const Unknown_00437229_Body*>(static_cast<uintptr_t>(r->ebx));

    uint32_t flags;
    int32_t value = scaleMagnitude(first->Unknown9A, &flags);
    GP4_GLOBAL(int32_t, 0x00629e28) = value;

    value = scaleMagnitude(second->Unknown9A, &flags);
    GP4_GLOBAL(int32_t, 0x00629e50) = value;

    r->eax = static_cast<uint32_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00437229, FUN_00437229, "regs(esi:ptr[0xa8]:bytes, ebx:ptr[0xa8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
