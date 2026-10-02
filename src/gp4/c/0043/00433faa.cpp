#include <gp4/gp4.h>

// FUNCTION: GP4 0x00433faa
void __cdecl FUN_00433faa(gp4::Regs* r) {
    const uint8_t selector = GP4_GLOBAL(uint8_t, 0x00852d8c);
    uint32_t second;
    if (selector != 0u) {
        GP4_GLOBAL(uint32_t, 0x0062ff64) = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(GP4_ARRAY(uint32_t, 0x0063b968)));
        second = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint32_t, 0x0063b98c)));
    } else {
        GP4_GLOBAL(uint32_t, 0x0062ff64) = GP4_GLOBAL(uint32_t, 0x0062ff5c);
        second = GP4_GLOBAL(uint32_t, 0x0062ff60);
    }
    GP4_GLOBAL(uint32_t, 0x0062ff68) = second;
    uint32_t parity = selector;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((~parity & 1u) << 2)
        | (selector == 0u ? 0x40u : 0u) | (selector & 0x80u);
    r->eax = second;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00433faa, FUN_00433faa, "regs() -> (eax,cf,pf,af,zf,sf,of); globals=layout=state_00433faa")
