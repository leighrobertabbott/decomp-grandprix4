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

// Opcode 2 stream handler: the read cursor (pointer at 0x0062d1a8) points at the opcode followed by two bytes that
// are OR-ed into the flag bytes 0x008914e0 and 0x007c2ead; the cursor then advances by 3. ebx is left holding the
// old cursor, al the second OR result; the flags are those of the final "add dword [cursor], 3".
// FUNCTION: GP4 0x00444c5f
void __cdecl FUN_00444c5f(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t* stream = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));

    const uint8_t first = static_cast<uint8_t>(stream[1] | GP4_GLOBAL(uint8_t, 0x008914e0));
    GP4_GLOBAL(uint8_t, 0x008914e0) = first;
    const uint8_t second = static_cast<uint8_t>(stream[2] | GP4_GLOBAL(uint8_t, 0x007c2ead));
    GP4_GLOBAL(uint8_t, 0x007c2ead) = second;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = before + 3u;

    r->eax = (r->eax & 0xffffff00u) | second;
    r->ebx = cursor;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 3u);
}
GP4_IMPL(0x00444c5f, FUN_00444c5f, "regs(eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a8")
