#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043c31b
void __cdecl FUN_0043c31b(gp4::Regs* r) {
    const int32_t input = static_cast<int32_t>(r->eax);
    const int32_t* tableA = GP4_ARRAY(int32_t, 0x006227c4);
    const int32_t* tableB = GP4_ARRAY(int32_t, 0x0063b844);
    const int32_t i = GP4_GLOBAL(int32_t, 0x006227e8);
    const int32_t j = GP4_GLOBAL(int32_t, 0x006227ec);

    const int32_t a = static_cast<int32_t>(static_cast<uint32_t>(tableA[i]) - static_cast<uint32_t>(tableA[j]));
    const int32_t b = static_cast<int32_t>(static_cast<uint32_t>(tableB[i]) - static_cast<uint32_t>(tableB[j]));
    const int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));

    // imul dword [0x6227a0]; shrd eax, edx, 14; add eax, b
    const int64_t first = static_cast<int64_t>(diff) * GP4_GLOBAL(int32_t, 0x006227a0);
    const uint32_t blended = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14) + static_cast<uint32_t>(b);

    // imul dword [0x62d244]; shrd eax, edx, 12; add eax, [0x6227f0]
    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(blended)) * GP4_GLOBAL(int32_t, 0x0062d244);
    const uint32_t offset = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 12) + GP4_GLOBAL(uint32_t, 0x006227f0);

    // imul by the caller's original eax; edx is restored afterwards
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(offset)) * input;
    const uint32_t low = static_cast<uint32_t>(product);
    const int32_t high = static_cast<int32_t>(static_cast<uint64_t>(product) >> 32);
    const bool overflow = high != (static_cast<int32_t>(low) >> 31);

    uint32_t parity = low & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = (overflow ? 0x801u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (low == 0u ? 0x40u : 0u)
        | ((low & 0x80000000u) != 0u ? 0x80u : 0u);

    r->eax = low;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c31b, FUN_0043c31b, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0043c31b")
