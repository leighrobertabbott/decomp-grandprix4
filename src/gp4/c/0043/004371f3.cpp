#include <gp4/gp4.h>

static inline uint32_t CompareFlags16(uint16_t lhs, uint16_t rhs) {
    const uint16_t result = static_cast<uint16_t>(lhs - rhs);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) >> 8)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x8000u) >> 4);
}

// FUNCTION: GP4 0x004371f3
void __cdecl FUN_004371f3(gp4::Regs* r) {
    uint8_t* body = GP4_GLOBAL(uint8_t*, 0x00629e04);
    GP4_GLOBAL(uint8_t, 0x00629f30) = 0u;
    uint32_t flags = 0u;
    if ((body[0xe5] & 4u) == 0u) {
        const uint16_t selector = GP4_GLOBAL(uint16_t, 0x007ad89e);
        if (selector == 3u) {
            flags = CompareFlags16(selector, 3u);
            GP4_GLOBAL(uint8_t, 0x00629f30) = 0xffu;
        } else {
            flags = CompareFlags16(selector, 4u);
            if (selector == 4u) {
                GP4_GLOBAL(uint8_t, 0x00629f30) = 0xffu;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004371f3, FUN_004371f3, "regs() -> (cf,pf,af,zf,sf,of); globals=layout=state_004371f3")
