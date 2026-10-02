#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00415cac {
    uint8_t Unknown00[0x232];
    uint8_t Flags232;
};

// FUNCTION: GP4 0x00415cac
void __cdecl FUN_00415cac(gp4::Regs* r) {
    Unknown_00415cac* self = reinterpret_cast<Unknown_00415cac*>(r->esi);

    gp4::Regs callee = *r;
    gp4::call_regs(0x00401462, callee);

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>((callee.eax & 0xffu) << 6))
        * GP4_GLOBAL(int32_t, 0x00603aa0);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const int32_t limit = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00603a9c) - shifted);

    int32_t factor = GP4_GLOBAL(int32_t, 0x0063b834) - GP4_GLOBAL(int32_t, 0x0066d814);
    if (factor < 0) {
        factor = 0;
    }

    uint32_t flags = subtractionFlags32(static_cast<uint32_t>(factor), static_cast<uint32_t>(limit));
    if (!(factor > limit)) {
        const uint8_t value = self->Flags232 & 0xbfu;
        self->Flags232 = value;
        flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
    }

    r->eax = static_cast<uint32_t>(limit);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415cac, FUN_00415cac, "regs(esi:ptr[0x23a]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
