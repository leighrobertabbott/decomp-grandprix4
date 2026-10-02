#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0042b0d5
void __cdecl FUN_0042b0d5(gp4::Regs* r) {
    void* other = reinterpret_cast<void*>(static_cast<uintptr_t>(r->ebx));
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    // 16-bit angle difference, folded to |diff|, then 0x4000 - |diff|, folded again
    const uint16_t diff = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x0e) - GP4_FIELD(uint16_t, other, 0x0e));
    uint16_t value = diff;
    if ((diff & 0x8000u) != 0u) {
        value = static_cast<uint16_t>(0u - diff);
    }
    value = static_cast<uint16_t>(0u - value);
    value = static_cast<uint16_t>(value + 0x4000u);
    if ((value & 0x8000u) != 0u) {
        value = static_cast<uint16_t>(0u - value);
    }
    const uint32_t offset = (static_cast<uint32_t>(value) >> 2) & 0xfffeu;

    // word table at 0x007c5c5c, scaled down by 64 (sar ax, 6)
    const int16_t entry = *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset));
    const int16_t scaled = static_cast<int16_t>(entry >> 6);
    GP4_GLOBAL(int16_t, 0x00603e5e) = scaled;

    // flags of the final 16-bit `sar ax, 6` (OF/AF are 0 in the oracle model)
    uint32_t flags = parityFlag(static_cast<uint16_t>(scaled));
    if (((static_cast<uint32_t>(static_cast<uint16_t>(entry)) >> 5) & 1u) != 0u) flags |= 0x001u;
    if (scaled == 0) flags |= 0x040u;
    if (scaled < 0) flags |= 0x080u;

    r->eax = static_cast<uint16_t>(scaled);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b0d5, FUN_0042b0d5, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
