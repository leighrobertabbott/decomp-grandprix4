#include <gp4/gp4.h>

// push ecx / esi = [0x82fd34] / do esi = [esi] while (--ecx) / pop ecx: follows the pointer chain
// starting at [0x82fd34] ecx times (ecx = 1: one dereference of the global's target) and returns
// the last link in esi; ecx is restored by the pop.
// FUNCTION: GP4 0x0053d98c
void __cdecl FUN_0053d98c(gp4::Regs* r) {
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t count = r->ecx;
    do {
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        count -= 1u;
    } while (count != 0u);
    r->esi = esi;
}
GP4_IMPL(0x0053d98c, FUN_0053d98c, "regs(ecx:u32[1..4]) -> (esi); globals=fuzz:i32+layout=chain_0082fd34")
