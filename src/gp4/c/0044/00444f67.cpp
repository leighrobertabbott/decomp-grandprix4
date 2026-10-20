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

// Opcode 0x0c handler of the stream whose read cursor is the pointer 0x0062d1a8: the operand byte after the
// opcode goes to 0x0066d8b6, 0x0095404e is set to 0xff and the cursor advances by 2. ebx keeps the old cursor,
// al the operand; flags are those of the final "add dword [cursor], 2".
// FUNCTION: GP4 0x00444f67
void __cdecl FUN_00444f67(gp4::Regs* r) {
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(before));
    const uint8_t operand = cursor[1];

    GP4_GLOBAL(uint8_t, 0x0066d8b6) = operand;
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = before + 2u;

    r->ebx = before;
    r->eax = (r->eax & 0xffffff00u) | operand;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 2u);
}
GP4_IMPL(0x00444f67, FUN_00444f67, "regs(eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a8")
