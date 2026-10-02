#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x004243cd
void __cdecl FUN_004243cd(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->eax));
    const uint8_t index = GP4_FIELD(uint8_t, record, 0xdc);
    // 0x78-byte entries of a table based at 0x0063bd7c; fields +0x30/+0x34/+0x38
    const uint32_t entry = 0x0063bd7cu + static_cast<uint32_t>(index) * 0x78u;

    const int32_t first = GP4_GLOBAL(int32_t, entry + 0x30u) >> 5;
    GP4_FIELD(int32_t, record, 0x78) = first;
    const int32_t second = GP4_GLOBAL(int32_t, entry + 0x34u) >> 5;
    GP4_FIELD(int32_t, record, 0x7c) = second;
    const int32_t sourceThird = GP4_GLOBAL(int32_t, entry + 0x38u);
    const int32_t third = sourceThird >> 5;
    GP4_FIELD(int32_t, record, 0x80) = third;

    // Flags are those of the last `sar eax, 5` (count 5: OF/AF are 0 in the oracle model).
    uint32_t flags = parityFlag(static_cast<uint32_t>(third));
    if (((static_cast<uint32_t>(sourceThird) >> 4) & 1u) != 0u) flags |= 0x001u;
    if (third == 0) flags |= 0x040u;
    if (third < 0) flags |= 0x080u;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004243cd, FUN_004243cd, "regs(eax:ptr[0xe8]:bytes) -> (cf, pf, af, zf, sf, of)")
