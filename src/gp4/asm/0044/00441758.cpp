#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00441758
// eax = the average of the four bytes of the dword +0x240 of the record at esi, capped at 0xb0. ecx and edx
// are saved and restored; the flags are those of `cmp average, 0xb0` (before the cap).
void __cdecl FUN_00441758(gp4::Regs* r) {
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t packed = *reinterpret_cast<const uint32_t*>(record + 0x240);

    uint32_t average = ((packed & 0xffu) + ((packed >> 8) & 0xffu) + ((packed >> 16) & 0xffu) + (packed >> 24)) >> 2;
    const uint32_t flags = cmpFlags32(average, 0xb0u);
    if (static_cast<int32_t>(average) > 0xb0) {
        average = 0xb0;
    }
    r->eax = average;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441758, FUN_00441758, "regs(esi:ptr[0x248]:bytes) -> (eax,cf,pf,af,zf,sf,of)")
