#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` / `sub left, right` (CF PF AF ZF SF OF).
// `neg x` is `sub 0, x`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0040496c_Subject {
    uint8_t Unknown00[0x98];
    int32_t Unknown98;
    uint8_t Unknown9C[0xda - 0x9c];
    int16_t UnknownDA;
};

// Refreshes the vector length at +0x98 (0x00404961), then adds the change of that length (16.16 fixed
// point, truncated to integer) to the 16-bit value at +0xda, limiting its magnitude to
// ([0x00629dbc] * [0x00603854]) >> 16. ebp keeps that limit, edx the old +0xda, eax the new +0xda.
// FUNCTION: GP4 0x0040496c
void __cdecl FUN_0040496c(gp4::Regs* r) {
    Unknown_0040496c_Subject* subject = reinterpret_cast<Unknown_0040496c_Subject*>(static_cast<uintptr_t>(r->esi));

    const int32_t oldLength = subject->Unknown98;
    gp4::call_regs(0x00404961, *r);

    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629dbc)) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t limit = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);

    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(subject->Unknown98) - static_cast<uint32_t>(oldLength)) >> 16;
    const int32_t current = subject->UnknownDA;
    uint32_t value = static_cast<uint32_t>(delta) + static_cast<uint32_t>(current);
    uint32_t flags;
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
        if (value >= limit) {
            value = limit;
        }
        flags = subFlags32(0u, value);
        value = 0u - value;
    } else {
        flags = subFlags32(value, limit);
        if (value >= limit) {
            value = limit;
        }
    }
    subject->UnknownDA = static_cast<int16_t>(value);

    r->eax = value;
    r->edx = static_cast<uint32_t>(current);
    r->ebp = limit;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040496c, FUN_0040496c, "regs(esi:ptr[0xe8]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
