#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0040ad35 {
    uint8_t Unknown00[0x2c];
    uint32_t Flags2C;
};

// FUNCTION: GP4 0x0040ad35
void __cdecl FUN_0040ad35(gp4::Regs* r) {
    const uint32_t mask = GP4_GLOBAL(int8_t, 0x00868582) < 0 ? 0x20u : 0x10u;
    uint32_t record = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x580u;
    for (;;) {
        if ((reinterpret_cast<const Unknown_0040ad35*>(record)->Flags2C & mask) == 0u) {
            break;
        }
        record -= 0x160u;
        if (record == GP4_GLOBAL(uint32_t, 0x00866f00)) {
            break;
        }
    }
    r->eax = record + 0x160u;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(record, 0x160u);
}
GP4_IMPL(0x0040ad35, FUN_0040ad35, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=records_008943a0")
