#include <gp4/gp4.h>
#include <gp4/flags.h>

// The 64-bit difference of the fresh counter qword at 0x00838c5c and the reference qword at 0x00838c54
// (`mov eax, [c]; mov edx, [c+4]; sub eax, [r]; sbb edx, [r+4]`). Only the low half is used by the
// caller, but the original loads and subtracts both halves.
static __declspec(noinline) uint64_t counterDelta() {
    return GP4_GLOBAL(uint64_t, 0x00838c5c) - GP4_GLOBAL(uint64_t, 0x00838c54);
}

// Elapsed time since the reference counter 0x00838c54, in ticks of a 1193180 / 10 Hz clock (0x1d216 = 119318):
// reads the performance counter into the qword 0x00838c5c (thunk 0x005a48be: QueryPerformanceCounter),
// takes the difference to the reference, multiplies the LOW dword of it by 100000 (`mul ebx` uses eax only,
// the high dword of the difference is overwritten) and divides the 64-bit product by 119318; the quotient
// is returned in eax. ebx, ecx and edx are saved and restored. The flags are those of the `mul` (DIV leaves
// them): CF = OF = high half nonzero, ZF/SF/PF from the low half, AF = 0.
// FUNCTION: GP4 0x0054e6f7
void __cdecl FUN_0054e6f7(gp4::Regs* r) {
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c5c));
    const uint32_t low = static_cast<uint32_t>(counterDelta());

    const uint64_t product = static_cast<uint64_t>(low) * 100000u;
    uint32_t flags = gp4::flags::szp(static_cast<uint32_t>(product), 32);
    if ((product >> 32) != 0u) {
        flags |= gp4::flags::CF | gp4::flags::OF;
    }
    uint32_t remainder;
    r->eax = gp4::div64(product, 0x1d216u, &remainder);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e6f7, FUN_0054e6f7, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=qpc_0054e733")
