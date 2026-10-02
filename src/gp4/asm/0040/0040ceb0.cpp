#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0040ceb0
void __cdecl FUN_0040ceb0(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi));
    const int32_t scale = GP4_FIELD(int32_t, record, 0x150);
    const uint32_t base = GP4_FIELD(uint16_t, record, 0x62);
    const uint32_t addend = GP4_FIELD(uint32_t, record, 0xf4);

    const int32_t offset = static_cast<int32_t>(0u - ((base + 0x50u) << 8));
    const int32_t scaled = gp4::x87::fistp32(
        static_cast<double>(scale) * GP4_GLOBAL(double, 0x006f2a04) * static_cast<double>(offset));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;

    // eax = scaled + [edi+0xf4]; the add leaves the arithmetic flags
    const uint32_t left = static_cast<uint32_t>(scaled);
    const uint32_t result = left + addend;
    uint32_t flags = parityFlag(result);
    if (result < left) flags |= 0x001u;
    if (((left ^ addend ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ result) & (addend ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ceb0, FUN_0040ceb0, "regs(edi:ptr[0x158]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:f64")
