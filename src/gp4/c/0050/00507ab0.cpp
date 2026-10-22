#include <gp4/gp4.h>

typedef int32_t (__stdcall *GlobalFreeFn)(int32_t mem);

// Releases the global memory block whose handle sits at 0x00b1bfac (GlobalFree) and clears the
// slot; returns the GlobalFree result, or the (zero) handle when there was nothing to free.
// FUNCTION: GP4 0x00507ab0
int32_t __cdecl FUN_00507ab0() {
    int32_t mem = GP4_GLOBAL(int32_t, 0x00b1bfac);
    if (mem != 0) {
        mem = GP4_IAT(GlobalFreeFn, 0x005f0244)(mem);
        GP4_GLOBAL(int32_t, 0x00b1bfac) = 0;
    }
    return mem;
}
GP4_IMPL(0x00507ab0, FUN_00507ab0, "cdecl() -> i32; globals=fuzz:i32")
