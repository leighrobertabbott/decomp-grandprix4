#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Word-sized twin of 0x00441de4: ax = n - 1 (n is the word at 0x00868556) is passed to 0x00441e0b, decremented and
// the loop repeats while ax stays non-negative (at least once). Only ax changes (the high half of eax is kept); the
// flags are those of the final 16-bit `dec ax`, whose CF is the one 0x00441e0b left.
// FUNCTION: GP4 0x00441df7
void __cdecl FUN_00441df7(gp4::Regs* r) {
    uint32_t ax = (static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868556)) - 1u) & 0xffffu;
    gp4::Regs call = *r;
    uint32_t before;
    do {
        call.eax = (r->eax & 0xffff0000u) | ax;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00441e0b, call);
        before = ax;
        ax = (ax - 1u) & 0xffffu;
    } while ((ax & 0x8000u) == 0u);

    const uint32_t flags = (call.eflags & 0x01u) | parityFlag(ax) | ((before & 0x0fu) == 0u ? 0x10u : 0u)
        | (ax == 0u ? 0x40u : 0u) | (ax & 0x8000u ? 0x80u : 0u) | (before == 0x8000u ? 0x800u : 0u);
    r->eax = (r->eax & 0xffff0000u) | ax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441df7, FUN_00441df7, "regs(eax:u32, edx:u32, edi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+groups_00868556")
