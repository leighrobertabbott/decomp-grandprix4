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

// Appends the five-byte record {6, word [esi + 0xf0], word [esi + 0xf2]} to the opcode stream whose write cursor is
// the pointer at 0x0062d1a0 and advances the cursor by 5. pushal / popal restore every register; only the flags of
// the final "add dword [cursor], 5" survive.
// FUNCTION: GP4 0x00444d97
void __cdecl FUN_00444d97(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));
    cursor[0] = 6u;
    *reinterpret_cast<uint16_t*>(cursor + 1) = *reinterpret_cast<const uint16_t*>(car + 0xf0);
    *reinterpret_cast<uint16_t*>(cursor + 3) = *reinterpret_cast<const uint16_t*>(car + 0xf2);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 5u;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 5u);
}
GP4_IMPL(0x00444d97, FUN_00444d97, "regs(esi:ptr[0xfa]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0")
