#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// st0 -= float [esi]; then the low byte of eax + 1 is the opcode flag byte: bit 2 raises the byte flag
// 0x0095404e, and its upper five bits (al & 0xf8) are OR-ed into the byte 0x007c2ead. eax keeps its upper
// 24 bits (+1 carry included) with al masked; the flags are those of the byte `or`.
// FUNCTION: GP4 0x0044524f
void __cdecl FUN_0044524f(gp4::Regs* r) {
    const float operand = *reinterpret_cast<const float*>(static_cast<uintptr_t>(r->esi));
    const gp4::x87::Ext st0 = gp4::x87::in_st(r, 0);
    gp4::x87::set_out_st(r, 0, st0 - gp4::x87::ext_f32(operand));

    const uint32_t incremented = r->eax + 1u;
    if ((incremented & 4u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    }
    const uint32_t masked = incremented & 0xf8u;
    const uint8_t result = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) | masked);
    GP4_GLOBAL(uint8_t, 0x007c2ead) = result;

    r->eax = (incremented & 0xffffff00u) | masked;
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}
GP4_IMPL(0x0044524f, FUN_0044524f, "regs(eax:u32, esi:ptr[0x40]:f32, st0:f64) -> (eax, st0, cf, pf, af, zf, sf, of); globals=fuzz:i32")
