#include <gp4/gp4.h>

struct Unknown_0043883d {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;
    uint8_t Unknown20[0x70];
    int16_t Unknown90;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right (32-bit), without CF
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of test word, 0xffff (CF = OF = 0; AF as the oracle model leaves it, 0)
static inline uint32_t testFlags16(uint16_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0043883d
// Returns CF = 1 when the value at +0x1c lies in [DAT_00866f08, DAT_00866f0c) and the
// sign of the word at +0x90 agrees with the sign of the byte at DAT_00868582.
void __cdecl FUN_0043883d(gp4::Regs* r) {
    const Unknown_0043883d* item = reinterpret_cast<const Unknown_0043883d*>(r->esi);
    const uint32_t value = item->Unknown1C;
    const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866f08);
    const uint32_t high = GP4_GLOBAL(uint32_t, 0x00866f0c);
    uint32_t flags;
    bool carry;

    if (value < low) {
        flags = compareFlags32(value, low);
        carry = false;
    } else if (value >= high) {
        flags = compareFlags32(value, high);
        carry = false;
    } else {
        const int8_t selector = GP4_GLOBAL(int8_t, 0x00868582);
        const int16_t word = item->Unknown90;
        flags = testFlags16(static_cast<uint16_t>(word));
        carry = (selector < 0) ? (word < 0) : (word >= 0);
    }
    flags |= carry ? 1u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043883d, FUN_0043883d, "regs(esi:ptr[0x98]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
