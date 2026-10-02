#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ed48
void __cdecl FUN_0043ed48(gp4::Regs* r) {
    uint8_t last = GP4_GLOBAL(uint8_t, 0x00627b64);
    uint32_t carry = 0u;
    if (last == 0u) {
        last = GP4_GLOBAL(uint8_t, 0x00627b62);
        if (last == 0u) {
            const uint8_t selector = GP4_GLOBAL(uint8_t, 0x00627b63);
            last = GP4_GLOBAL(uint8_t, 0x00627b61);
            carry = ((selector != 0u) != (last != 0u)) ? 1u : 0u;
        }
    }
    uint32_t parity = last;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = carry | ((~parity & 1u) << 2)
        | (last == 0u ? 0x40u : 0u) | (last & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ed48, FUN_0043ed48, "regs() -> (cf,pf,af,zf,sf,of); globals=layout=state_0043ed48")
