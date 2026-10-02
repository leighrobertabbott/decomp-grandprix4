#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00419891
void __cdecl FUN_00419891(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t amount = *reinterpret_cast<const uint32_t*>(record + 0x3c);

    // add [0x603560], eax: only its CF survives the following inc
    const uint32_t total = GP4_GLOBAL(uint32_t, 0x00603560);
    const uint32_t newTotal = total + amount;
    GP4_GLOBAL(uint32_t, 0x00603560) = newTotal;
    const uint32_t carry = newTotal < total ? 1u : 0u;

    // inc [0x603564]: every flag but CF
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00603564);
    const uint32_t newCount = count + 1u;
    GP4_GLOBAL(uint32_t, 0x00603564) = newCount;
    const uint32_t flags = carry | parityFlag(newCount)
        | (((count ^ 1u ^ newCount) & 0x10u) != 0u ? 0x10u : 0u)
        | (newCount == 0u ? 0x40u : 0u)
        | ((newCount & 0x80000000u) != 0u ? 0x80u : 0u)
        | (count == 0x7fffffffu ? 0x800u : 0u);

    r->eax = amount;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419891, FUN_00419891, "regs(esi:ptr[0x44]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
