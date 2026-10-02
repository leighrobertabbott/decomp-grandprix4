#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004082f7
void __cdecl FUN_004082f7(gp4::Regs* r) {
    // esi = record, edx = index; byte at +0x2a4 is a level (0 = none)
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t value = GP4_FIELD(uint8_t, record, 0x2a4 + r->edx);
    if (value != 0u) value += 1u;
    const uint32_t result = value << 6;
    // shl eax, 6 sets the flags
    const uint32_t previous = value << 5;
    const uint32_t flags = ((value >> 26) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) << 11);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004082f7, FUN_004082f7, "regs(esi:ptr[0x2b8]:bytes, edx:u32[0..7]) -> (eax, cf, pf, af, zf, sf, of)")
