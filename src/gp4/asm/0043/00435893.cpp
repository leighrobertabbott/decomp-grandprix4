#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// (a * b) >> 14 taken from the 64-bit product (imul + shrd eax, edx, 14).
static inline uint32_t scaledProduct(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

struct Unknown_00435893 {
    uint8_t Unknown00[0x78];
    uint8_t Flags78;
    uint8_t Unknown79[0xb4 - 0x79];
    int16_t UnknownB4;
    uint8_t UnknownB6[0xe5 - 0xb6];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x178 - 0xe6];
    uint8_t Unknown178;
};

// Blends eax with the word at +0xb4 using a 14-bit weight: weight = 0x4000 (flag +0xe5 & 4), or
// 0x4000 * byte(+0x178) / 100 (flag +0x78 & 0x20), scaled by DAT_00603554 >> 14; the result is
// eax * (0x4000 - weight) >> 14 plus word(+0xb4) * weight >> 14. Without flag +0xe5 & 1 eax is
// returned unchanged, without flags +0xe5 & 4 and +0x78 & 0x20 it is 0x4000; those early exits leave
// the flags of the failing "test" (ZF and PF). ebp, ebx and edx are saved and restored; the flags
// are otherwise those of the final "add eax, edx".
// FUNCTION: GP4 0x00435893
void __cdecl FUN_00435893(gp4::Regs* r) {
    const Unknown_00435893* state = reinterpret_cast<const Unknown_00435893*>(static_cast<uintptr_t>(r->esi));
    const int32_t input = static_cast<int32_t>(r->eax);

    if ((state->FlagsE5 & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    int32_t weight = 0x4000;
    if ((state->FlagsE5 & 4u) == 0u) {
        if ((state->Flags78 & 0x20u) == 0u) {
            r->eax = 0x4000u;
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        weight = static_cast<int32_t>(0x4000u * static_cast<uint32_t>(state->Unknown178)) / 100;
    }
    const int32_t scaledWeight = static_cast<int32_t>(scaledProduct(weight, GP4_GLOBAL(int32_t, 0x00603554)));
    const uint32_t complement = 0x4000u - static_cast<uint32_t>(scaledWeight);
    const uint32_t base = scaledProduct(static_cast<int32_t>(complement), input);
    const uint32_t blend = scaledProduct(state->UnknownB4, scaledWeight);
    const uint32_t result = blend + base;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(blend, base, result);
}
GP4_IMPL(0x00435893, FUN_00435893, "regs(eax:i32, esi:ptr[0x180]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
