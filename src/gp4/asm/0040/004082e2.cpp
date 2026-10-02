#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004082e2
void __cdecl FUN_004082e2(gp4::Regs* r) {
    const uint32_t index = r->edx;
    const uint8_t* base = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int8_t>(base[index + 0x214u]);
    const uint32_t result = static_cast<uint32_t>(value) << 4;
    // shl eax, 4: CF is the last bit shifted out; OF/AF are undefined for a
    // count above one and follow the pinned oracle model (AF=0, OF=bit31(prev^result)).
    const uint32_t previous = static_cast<uint32_t>(value) << 3;
    const uint32_t flags = (previous >> 31) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004082e2, FUN_004082e2, "regs(edx:i32[0..11], esi:ptr[0x228]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
