#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Entry of the 16-bit table at 0x007c5c5c for the angle `angle` (a 16-bit quantity whose magnitude
// is taken first): table[(|angle| >> 2) & 0xfffe bytes].
static inline int32_t tableWord(uint16_t angle) {
    if ((angle & 0x8000u) != 0u) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
}

// Subject fields used here.
struct Unknown_004049bf {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[0x80];
    int16_t Unknown9A;
};

// FUNCTION: GP4 0x004049bf
// [0x00623e30] = (table(0x4000 - w18) * w9a) << 2, [0x00623e38] = (table(w18) * w9a) << 2 and
// [0x00623e34] = 0, where w18 / w9a are the signed words at esi+0x18 / esi+0x9a. Leaves eax = the
// second value, edx = the high half of its 64-bit product and the flags of the final "shl eax, 2"
// (CF = bit 30 of the product, OF = bit 30 ^ bit 29, AF = 0).
void __cdecl FUN_004049bf(gp4::Regs* r) {
    Unknown_004049bf* subject = reinterpret_cast<Unknown_004049bf*>(static_cast<uintptr_t>(r->esi));
    const int32_t scale = subject->Unknown9A;

    const uint16_t complement = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(subject->Unknown18));
    const int32_t firstProduct = tableWord(complement) * scale;
    GP4_GLOBAL(uint32_t, 0x00623e30) = static_cast<uint32_t>(firstProduct) << 2;

    const int64_t product = static_cast<int64_t>(tableWord(static_cast<uint16_t>(subject->Unknown18))) * scale;
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t result = low << 2;
    GP4_GLOBAL(uint32_t, 0x00623e38) = result;
    GP4_GLOBAL(uint32_t, 0x00623e34) = 0u;

    const uint32_t flags = ((low >> 30) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((low >> 30) ^ (low >> 29)) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004049bf, FUN_004049bf, "regs(esi:ptr[0xa2]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
