#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043c75b
// eax = car record. Returns in eax the sum of the three position terms 0x0043c608 + 0x0043c677 +
// 0x0043c6fb computed for that car (esi = eax, ebx accumulates). All other registers are restored by
// the push/pop pairs; the flags are those of the final `add eax, ebx`.
void __cdecl FUN_0043c75b(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.esi = saved.eax;
    call.ebx = 0;

    gp4::call_regs(0x0043c608, call);
    call.ebx += call.eax;
    gp4::call_regs(0x0043c677, call);
    call.ebx += call.eax;
    gp4::call_regs(0x0043c6fb, call);

    saved.eflags = (saved.eflags & ~0x8d5u) | addFlags32(call.eax, call.ebx);
    saved.eax = call.eax + call.ebx;
    *r = saved;
}
GP4_IMPL(0x0043c75b, FUN_0043c75b, "regs(eax:ptr[0x238]:layout=car_0043c677) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00432330+state_0043c677+segw_0043c608")
