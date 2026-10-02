#include <gp4/gp4.h>

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ((~parity & 1u) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00404e9c
void __cdecl FUN_00404e9c(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00603104);
    const uint32_t b = GP4_GLOBAL(uint32_t, 0x00603108);
    const uint32_t c = GP4_GLOBAL(uint32_t, 0x0060310c);
    const uint32_t d = GP4_GLOBAL(uint32_t, 0x00603110);
    const uint32_t subtotal = a + b + c;
    const uint32_t sum = subtotal + d;
    r->ebp = sum;
    uint32_t flags = AddFlags(subtotal, d, sum);
    if ((sum & 0x80000000u) == 0u) {
        const int64_t firstProduct = static_cast<int64_t>(static_cast<int32_t>(incomingEax))
            * GP4_GLOBAL(int32_t, 0x00603130);
        const uint32_t firstTerm = static_cast<uint32_t>(static_cast<uint64_t>(firstProduct) >> 14);
        GP4_GLOBAL(uint32_t, 0x00603134) = firstTerm;
        const int64_t secondProduct = static_cast<int64_t>(static_cast<int32_t>(firstTerm))
            * static_cast<int32_t>(sum);
        const uint32_t secondTerm = static_cast<uint32_t>(static_cast<uint64_t>(secondProduct) >> 14);
        const uint32_t result = secondTerm + incomingEax;
        r->eax = result;
        r->edx = incomingEax;
        flags = AddFlags(secondTerm, incomingEax, result);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00404e9c, FUN_00404e9c, "regs(eax:i32) -> (eax,edx,ebp,cf,pf,af,zf,sf,of); globals=layout=state_00404e9c")
