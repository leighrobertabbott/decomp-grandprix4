#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004108ac
void __cdecl FUN_004108ac(gp4::Regs* r) {
    const uint32_t index = r->ecx;
    uint8_t* base = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int16_t value = GP4_ARRAY(int16_t, 0x0062ee28)[index];
    const int16_t result = static_cast<int16_t>(value >> 4);
    GP4_FIELD(uint8_t, base, index + 0x214u) = static_cast<uint8_t>(result);

    // sar ax, 4: CF is the last bit shifted out; AF/OF follow the pinned oracle model (0).
    const uint32_t flags = (static_cast<uint32_t>(value >> 3) & 1u)
        | parityFlag(static_cast<uint32_t>(result))
        | (result == 0 ? 0x40u : 0u)
        | ((static_cast<uint32_t>(result) >> 15 & 1u) << 7);
    r->eax = (r->eax & 0xffff0000u) | static_cast<uint16_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004108ac, FUN_004108ac, "regs(eax:u32, ecx:i32[0..11], esi:ptr[0x228]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
