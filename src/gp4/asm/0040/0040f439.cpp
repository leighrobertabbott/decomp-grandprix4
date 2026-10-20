#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x0040f439
void __cdecl FUN_0040f439(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    // or [esi+0x82], 8 ; or [esi+0x82], 0x10 ; or [esi+0x88], 0x10 ; or [esi+0x82], 1
    rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x08u);
    rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x10u);
    rec[0x88] = static_cast<uint8_t>(rec[0x88] | 0x10u);
    rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x01u);

    // popal does not restore flags: they come from the last `or`, CF = OF = AF = 0.
    const uint32_t result = rec[0x82];
    const uint32_t flags = parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f439, FUN_0040f439, "regs(esi:ptr[0x90]:bytes) -> (cf, pf, af, zf, sf, of)")
