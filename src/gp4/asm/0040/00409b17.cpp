#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00409b17
// Searches the 22-byte table at 0x007c2a48 for al. Returns the index >> 1 in eax (0 when absent).
// ecx is left as the remaining loop count, edx is preserved, flags are those of `shr edx, 1`.
void __cdecl FUN_00409b17(gp4::Regs* r) {
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x007c2a48);
    const uint8_t needle = static_cast<uint8_t>(r->eax);
    uint32_t index = 0u;
    uint32_t remaining = 0u;
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        if (needle == table[i]) {
            index = i;
            remaining = 0x16u - i;
            break;
        }
    }
    const uint32_t result = index >> 1;
    // shr by 1: CF = bit shifted out, AF = 0, OF = bit31 of the operand (0), SF = 0
    const uint32_t flags = (index & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u);
    r->eax = result;
    r->ecx = remaining;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409b17, FUN_00409b17, "regs(eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
