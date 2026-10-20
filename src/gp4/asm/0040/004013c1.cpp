#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// 16-bit "test ax, ax / jns / neg ax": magnitude of a signed word (0x8000 stays 0x8000).
static inline uint16_t Abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

// (angle >> 2) & 0xfffe is a byte offset into the int16 table at 0x007c5c5c.
static inline uint32_t TableOffset(uint16_t angle) {
    return static_cast<uint32_t>(static_cast<uint16_t>(angle >> 2)) & 0xfffeu;
}

// FUNCTION: GP4 0x004013c1
// eax = (table[|0x4000 - ax|] << 14) / table[|ax|] for the 16-bit angle in ax: the cotangent in
// 14-bit fixed point from the word table at 0x007c5c5c. ecx and edx are preserved.
void __cdecl FUN_004013c1(gp4::Regs* r) {
    const uint16_t angle = static_cast<uint16_t>(r->eax);
    uint8_t* table = GP4_ARRAY(uint8_t, 0x007c5c5c);

    const uint16_t complement = Abs16(static_cast<uint16_t>(0x4000u - angle));
    const int32_t numerator = static_cast<int32_t>(
        static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, table, TableOffset(complement)))) << 14);

    const uint32_t divisorOffset = TableOffset(Abs16(angle));
    const int32_t divisor = GP4_FIELD(int16_t, table, divisorOffset);

    r->eax = static_cast<uint32_t>(numerator / divisor);   // cdq / idiv

    // idiv leaves the flags of the preceding "and ecx, 0xfffe": CF = OF = AF = 0
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag8(divisorOffset) | (divisorOffset == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x004013c1, FUN_004013c1, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
