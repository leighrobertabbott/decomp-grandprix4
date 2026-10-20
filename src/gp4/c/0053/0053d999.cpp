#include <gp4/gp4.h>

// push ecx / edi = [0x82fd34] / do edi = [edi] while (--ecx) / pop ecx: follows the pointer chain
// starting at [0x82fd34] ecx times (ecx = 1: one dereference of the global's target) and returns
// the last link in edi; ecx is restored by the pop and `loop` leaves the flags alone.
// FUNCTION: GP4 0x0053d999
void __cdecl FUN_0053d999(gp4::Regs* r) {
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t count = r->ecx;
    do {
        edi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(edi));
        count -= 1u;
    } while (count != 0u);
    r->edi = edi;
}
GP4_IMPL(0x0053d999, FUN_0053d999, "regs(ecx:u32[1..4]) -> (edi); globals=fuzz:i32+layout=chain_0082fd34")
