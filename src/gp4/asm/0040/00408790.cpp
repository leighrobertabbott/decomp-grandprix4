#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00408790
void __cdecl FUN_00408790(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    // movzx word -> eax ; mul dword [0x602a4c] ; shrd eax, edx, 6
    const uint64_t product = static_cast<uint64_t>(GP4_FIELD(uint16_t, record, 0x60))
        * static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00602a4c));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(product >> 32);
    const uint32_t result = static_cast<uint32_t>(product >> 6);
    const uint32_t previous = static_cast<uint32_t>(product >> 5);
    // shrd by 6: CF is the last bit shifted out; OF/AF are undefined by the architecture,
    // this follows the pinned oracle model (OF = bit31(previous ^ result), AF = 0)
    const uint32_t flags = ((low >> 5) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
    GP4_FIELD(uint32_t, record, 0x19c) = result;
    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408790, FUN_00408790, "regs(esi:ptr[0x1a4]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
