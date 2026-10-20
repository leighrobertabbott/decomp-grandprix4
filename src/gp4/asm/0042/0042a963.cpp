#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` / `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042a963_Subject {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0xdc - 0x92];
    int16_t UnknownDC;
    uint8_t UnknownDE[0x174 - 0xde];
    int16_t Unknown174;
};

// The dividend 0x580000 is built at run time: as an immediate it would read as a code address.
static __declspec(noinline) int64_t dividend(int32_t half) {
    return static_cast<int64_t>(half) * 2;
}

// pushal ... popal wrapper: only memory [esi+0x174] and the flags of the last flag writer escape.
// When edi is below both [0x00891500] and [0x00866f00], stores into [esi+0x174] the magnitude of
// (0x580000 / ([0x00866f00] - edi)) * ([esi+0xdc] - [esi+0x90]) * [0x00603c28] >> 28, clamped to
// [8, 0x1ff]; otherwise stores 0xff.
// FUNCTION: GP4 0x0042a963
void __cdecl FUN_0042a963(gp4::Regs* r) {
    const uint32_t edi = r->edi;
    Unknown_0042a963_Subject* subject = reinterpret_cast<Unknown_0042a963_Subject*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (edi >= limit) {
        flags = subFlags32(edi, limit);
        subject->Unknown174 = 0xff;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866f00);
    const uint32_t span = end - edi;
    if (end <= edi) {
        flags = subFlags32(end, edi);
        subject->Unknown174 = 0xff;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    int32_t rem;
    const int32_t quotient = gp4::idiv64(dividend(0x2c0000), static_cast<int32_t>(span), &rem);
    const int16_t diff = static_cast<int16_t>(static_cast<uint16_t>(subject->UnknownDC) - static_cast<uint16_t>(subject->Unknown90));
    const int32_t first = static_cast<int32_t>(static_cast<int64_t>(quotient) * static_cast<int32_t>(diff));
    const int64_t second = static_cast<int64_t>(first) * GP4_GLOBAL(int32_t, 0x00603c28);
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 28));
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    if (value > 0x1ff) {
        value = 0x1ff;
    }
    flags = subFlags32(static_cast<uint32_t>(value), 8u);
    if (value < 8) {
        value = 8;
    }
    subject->Unknown174 = static_cast<int16_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042a963, FUN_0042a963, "regs(edi:u32, esi:ptr[0x180]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
