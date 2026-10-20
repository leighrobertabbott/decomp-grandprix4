#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Opcode-stream reader: ORs the operand byte after the opcode into the mask byte at 0x007c2ead and advances the
// read cursor (pointer at 0x0062d1a8) by 2. ebx is left holding the old cursor, al the new mask; the flags are
// those of the final "add dword [cursor], 2".
// FUNCTION: GP4 0x00444e1e
void __cdecl FUN_00444e1e(gp4::Regs* r) {
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(before));
    const uint8_t mask = static_cast<uint8_t>(cursor[1] | GP4_GLOBAL(uint8_t, 0x007c2ead));
    GP4_GLOBAL(uint8_t, 0x007c2ead) = mask;
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = before + 2u;

    r->ebx = before;
    r->eax = (r->eax & 0xffffff00u) | mask;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 2u);
}
GP4_IMPL(0x00444e1e, FUN_00444e1e, "regs(eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0+stream_0062d1a8")
