#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Switches the group numbers [n-1 .. 0] off through 0x00441e18, where n is the word at 0x00868556 (the loop always
// runs at least once: eax = n - 1 is decremented after the call and the loop repeats while it stays non-negative).
// Leaves eax = -1 (-2 when n = 0); the flags are those of the final `dec eax`, whose CF is the one
// 0x00441e18 left.
// FUNCTION: GP4 0x00441de4
void __cdecl FUN_00441de4(gp4::Regs* r) {
    uint32_t eax = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868556)) - 1u;
    gp4::Regs call = *r;
    uint32_t before;
    do {
        call.eax = eax;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00441e18, call);
        before = eax;
        eax = eax - 1u;
    } while (static_cast<int32_t>(eax) >= 0);

    const uint32_t flags = (call.eflags & 0x01u) | parityFlag(eax) | ((before & 0x0fu) == 0u ? 0x10u : 0u)
        | (eax == 0u ? 0x40u : 0u) | (eax & 0x80000000u ? 0x80u : 0u) | (before == 0x80000000u ? 0x800u : 0u);
    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441de4, FUN_00441de4, "regs(edx:u32, edi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+groups_00868556")
