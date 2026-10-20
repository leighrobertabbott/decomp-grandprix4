#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Runs 0x00405f77 and 0x00434602, then sets bit 0 (when the low two bits of the byte at
// [edi+0x48] equal 1) or bit 1 of the byte at [esi+0x9d]. al = those two bits; the flags are
// those of the final byte `or`.
// FUNCTION: GP4 0x0043462a
void __cdecl FUN_0043462a(gp4::Regs* r) {
    gp4::Regs state = *r;
    gp4::call_regs(0x00405f77, state);
    gp4::call_regs(0x00434602, state);

    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(state.esi));
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(state.edi));
    const uint32_t mode = source[0x48] & 3u;
    const uint32_t result = static_cast<uint32_t>(target[0x9d]) | (mode == 1u ? 1u : 2u);
    target[0x9d] = static_cast<uint8_t>(result);

    state.eax = (state.eax & 0xffffff00u) | mode;
    *r = state;
    r->eflags = (state.eflags & ~0x8d5u) | parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}
GP4_IMPL(0x0043462a, FUN_0043462a, "regs(eax:u32, esi:ptr[0x1a0]:bytes, edi:ptr[0x50]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
