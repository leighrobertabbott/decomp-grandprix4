#include <gp4/gp4.h>

struct Unknown_00426a66 {
    uint8_t Unknown00[0x98];
    int32_t Unknown98;
    uint8_t Unknown9C[0x28];
    int16_t UnknownC4;
    uint8_t UnknownC6[0x10];
    uint16_t UnknownD6;
    uint8_t UnknownD8[0x8];
    int16_t UnknownE0;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00426a66
void __cdecl FUN_00426a66(gp4::Regs* r) {
    const Unknown_00426a66* body = reinterpret_cast<const Unknown_00426a66*>(static_cast<uintptr_t>(r->esi));

    // movsx eax, (word C4 - word E0) times the unsigned word D6, then >> 14 (shrd on edx:eax)
    const int32_t delta = static_cast<int16_t>(static_cast<uint16_t>(body->UnknownC4) - static_cast<uint16_t>(body->UnknownE0));
    const int64_t product1 = static_cast<int64_t>(delta) * static_cast<int64_t>(body->UnknownD6);
    const uint32_t scaled1 = static_cast<uint32_t>(static_cast<uint64_t>(product1) >> 14);

    // times DAT_0062d28c, then >> 14 again
    const int64_t product2 = static_cast<int64_t>(static_cast<int32_t>(scaled1)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062d28c));
    const uint32_t scaled2 = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14);

    // high half of the square of the dword at +0x98
    const int64_t square = static_cast<int64_t>(body->Unknown98) * static_cast<int64_t>(body->Unknown98);
    const uint32_t squareHigh = static_cast<uint32_t>(static_cast<uint64_t>(square) >> 32);

    const int64_t product3 = static_cast<int64_t>(static_cast<int32_t>(scaled2)) * static_cast<int64_t>(static_cast<int32_t>(squareHigh));
    const uint32_t low = static_cast<uint32_t>(static_cast<uint64_t>(product3));
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product3) >> 32);

    // shld edx, eax, 0x11
    const uint32_t result = (high << 17) | (low >> 15);
    GP4_GLOBAL(uint32_t, 0x006037fc) = result;

    // shld flags under the oracle model: CF = bit 15 of the old edx, OF = bit 31 of
    // ((edx << 16) ^ result), AF = 0
    const uint32_t flags = (((high >> 15) & 1u) != 0u ? 1u : 0u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((high << 16) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = low;
    r->edx = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426a66, FUN_00426a66, "regs(esi:ptr[0xe8]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
