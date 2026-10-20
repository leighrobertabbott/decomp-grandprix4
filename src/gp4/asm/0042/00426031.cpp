#include <gp4/gp4.h>

// Only the fields this routine reads.
struct Unknown_00426031 {
    uint8_t Unknown00[0xc4];
    int16_t UnknownC4;
    uint8_t UnknownC6[0x10];
    uint16_t UnknownD6;
    uint8_t UnknownD8[8];
    int16_t UnknownE0;
};
static_assert(sizeof(Unknown_00426031) == 0xe2, "layout");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul + "shrd eax, edx, count": bits count..count+31 of the signed 64-bit product.
static inline uint32_t shrdQ(int64_t product, unsigned count) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> count);
}

// eax = a Q-format value (ebx); esi = body. Result eax; ebx, edx and edi are saved and restored.
// FUNCTION: GP4 0x00426031
void __cdecl FUN_00426031(gp4::Regs* r) {
    const Unknown_00426031* body = reinterpret_cast<const Unknown_00426031*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int32_t>(r->eax);

    const int16_t difference = static_cast<int16_t>(body->UnknownC4 - body->UnknownE0);
    const int32_t scaled = static_cast<int32_t>(shrdQ(
        static_cast<int64_t>(difference) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062d28c)), 14));

    const int64_t square = static_cast<int64_t>(value) * static_cast<int64_t>(value);
    const int32_t squareHigh = static_cast<int32_t>(static_cast<uint64_t>(square) >> 32);
    const uint32_t correction = shrdQ(static_cast<int64_t>(scaled) * static_cast<int64_t>(squareHigh), 23);

    int32_t index = value >> 22;
    if (index < 0) index = static_cast<int32_t>(0u - static_cast<uint32_t>(index));
    const int32_t entry = GP4_ARRAY(int32_t, 0x0063a194)[index];
    const uint32_t base = shrdQ(static_cast<int64_t>(entry) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854)), 8);

    const int32_t delta = static_cast<int32_t>(base - correction);
    const int64_t product = static_cast<int64_t>(delta) * static_cast<int64_t>(body->UnknownD6);
    const uint32_t result = shrdQ(product, 14);

    // Flags are those of the final `shrd eax, edx, 14` (pinned oracle model: AF = 0,
    // OF = bit31(previous ^ result) where previous is the product shifted by 13).
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426031, FUN_00426031, "regs(eax:i32, esi:ptr[0xe8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
