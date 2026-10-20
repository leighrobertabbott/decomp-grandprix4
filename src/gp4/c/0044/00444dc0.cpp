#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Opcode 6 of the opcode-stream reader (dispatched through 0x00601270 by 0x004452f1): the read cursor
// 0x0062d1a8 points at the opcode byte followed by two words. When the car pointer 0x0066d828 is non-null
// the words are stored at +0xf0 / +0xf2 of that car record and three marker bytes are set; the cursor then
// advances by 5. ebx (the cursor before the advance) and esi (the car) are left set; the flags are those of
// the final `add dword [0x62d1a8], 5`.
// FUNCTION: GP4 0x00444dc0
void __cdecl FUN_00444dc0(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d828);
    r->ebx = cursor;
    r->esi = car;
    if (car != 0u) {
        const uint8_t* stream = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car));
        *reinterpret_cast<uint16_t*>(record + 0xf0) = *reinterpret_cast<const uint16_t*>(stream + 1);
        *reinterpret_cast<uint16_t*>(record + 0xf2) = *reinterpret_cast<const uint16_t*>(stream + 3);
        GP4_GLOBAL(uint8_t, 0x006f3b7c) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x006f3b7d) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80u;
    }
    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 5u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursor, 5u);
}
GP4_IMPL(0x00444dc0, FUN_00444dc0, "regs() -> (ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a8")
