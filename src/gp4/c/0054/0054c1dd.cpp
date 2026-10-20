#include <gp4/gp4.h>

// Calls the two-argument routine 0x0054c21d (stdcall, `ret 8`) with the constants 0x896 and 200 and
// returns its result.
// FUNCTION: GP4 0x0054c1dd
int __cdecl FUN_0054c1dd() {
    return gp4::stdcall_<0x0054c21d, int>(0x896, 200);
}
GP4_IMPL(0x0054c1dd, FUN_0054c1dd, "cdecl() -> i32; globals=fuzz:i32")
