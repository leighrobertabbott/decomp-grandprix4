#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "test byte, 0xff": CF = OF = AF = 0, PF/ZF/SF from the byte
static inline uint32_t testFlags8(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// flags of the 8-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags8(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = (lhs - rhs) & 0xffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// When either of the bytes 0x00639652 / 0x00639650 is set and DAT_00601ba5 is not 1, sets
// DAT_00601ba5 to 7. AL carries the value read from DAT_00601ba5 (then 7) on those paths;
// the flags are those of the last test or compare.
// FUNCTION: GP4 0x0041368e
void __cdecl FUN_0041368e(gp4::Regs* r) {
    const uint32_t first = GP4_GLOBAL(uint8_t, 0x00639652);
    uint32_t flags = testFlags8(first);
    if (first == 0u) {
        const uint32_t second = GP4_GLOBAL(uint8_t, 0x00639650);
        flags = testFlags8(second);
        if (second == 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
    }
    uint32_t state = GP4_GLOBAL(uint8_t, 0x00601ba5);
    flags = cmpFlags8(state, 1u);
    if (state != 1u) {
        state = 7u;
        GP4_GLOBAL(uint8_t, 0x00601ba5) = static_cast<uint8_t>(state);
    }
    r->eax = (r->eax & 0xffffff00u) | state;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041368e, FUN_0041368e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
