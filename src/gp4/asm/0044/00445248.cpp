#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Applies the flag byte in al of an opcode record: bit 1 sets 0x40 in the byte 0x0066d87c, bit 2 raises the
// byte flag 0x0095404e, and the upper five bits (al & 0xf8) are OR-ed into the byte 0x007c2ead. al ends masked;
// the flags are those of the byte `or`.
// FUNCTION: GP4 0x00445248
void __cdecl FUN_00445248(gp4::Regs* r) {
    const uint8_t flagsByte = static_cast<uint8_t>(r->eax);

    if ((flagsByte & 2u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x0066d87c) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0066d87c) | 0x40u);
    }
    if ((flagsByte & 4u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    }

    const uint8_t masked = static_cast<uint8_t>(flagsByte & 0xf8u);
    const uint8_t result = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) | masked);
    GP4_GLOBAL(uint8_t, 0x007c2ead) = result;

    r->eax = (r->eax & 0xffffff00u) | masked;
    r->eflags = (r->eflags & ~0x8d5u)
        | byteParity(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}
GP4_IMPL(0x00445248, FUN_00445248, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
