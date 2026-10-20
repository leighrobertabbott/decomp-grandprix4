#include <gp4/gp4.h>

struct Unknown_00429100_Owner {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// eax += (the value 0x403800 derives from edi, differenced against the owner's 16-bit field + 0x50):
// the sign of byte DAT_00868582 selects Unknown60 + 0x50 - offset or offset - (Unknown62 + 0x50).
// ecx and edx are preserved; the flags are those of the final "add eax, ecx".
// FUNCTION: GP4 0x00429100
void __cdecl FUN_00429100(gp4::Regs* r) {
    const uint32_t addend = r->eax;
    const Unknown_00429100_Owner* owner = reinterpret_cast<const Unknown_00429100_Owner*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs callee = *r;
    gp4::call_regs(0x00403800, callee);
    const uint32_t offset = callee.eax;

    uint32_t partial;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        partial = (static_cast<uint32_t>(owner->Unknown60) + 0x50u) - offset;
    } else {
        partial = offset - (static_cast<uint32_t>(owner->Unknown62) + 0x50u);
    }
    const uint32_t result = partial + addend;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(partial, addend, result);
}
GP4_IMPL(0x00429100, FUN_00429100, "regs(eax:u32, edi:ptr[0x70]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
