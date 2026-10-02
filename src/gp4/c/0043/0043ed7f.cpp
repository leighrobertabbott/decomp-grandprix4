#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ed7f
void __cdecl FUN_0043ed7f(gp4::Regs* r) {
    // Carry set only when the byte at 0x627b64 is zero and the byte at 0x627b62 is not.
    // The other flags are those of the last "cmp byte, 0" executed (value - 0).
    const uint32_t first = GP4_GLOBAL(uint8_t, 0x00627b64);
    uint32_t last = first;
    uint32_t carry = 0u;
    if (first == 0u) {
        last = GP4_GLOBAL(uint8_t, 0x00627b62);
        carry = last != 0u ? 1u : 0u;
    }
    uint32_t parity = last;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = carry | ((~parity & 1u) << 2)
        | (last == 0u ? 0x40u : 0u) | (last & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ed7f, FUN_0043ed7f, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0043ed48")
