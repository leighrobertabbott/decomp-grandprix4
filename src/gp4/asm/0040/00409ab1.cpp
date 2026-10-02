#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00409ab1
void __cdecl FUN_00409ab1(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    // imul eax, eax, 0x60 (cannot overflow: the index is a byte)
    const uint32_t product = static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 0x202)) * 0x60u;
    // lea leaves the flags from the imul: CF=OF=AF=0, the rest from the product
    // (SF/ZF/PF/AF are architecturally undefined after imul: pinned oracle model)
    const uint32_t flags = parityFlag(product)
        | (product == 0u ? 0x40u : 0u)
        | ((product & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eax = product + 0x006020b0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409ab1, FUN_00409ab1, "regs(esi:ptr[0x20a]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
