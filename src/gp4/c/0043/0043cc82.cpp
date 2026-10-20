#include <gp4/gp4.h>

// Classifies the next byte of the shift-register generator at 0x00401462: below 0x80 gives 0,
// below 0xc0 gives -1, otherwise 1.
// FUNCTION: GP4 0x0043cc82
int __cdecl FUN_0043cc82() {
    gp4::Regs call{};
    gp4::call_regs(0x00401462, call);
    const int32_t value = static_cast<int32_t>(call.eax & 0xffu);
    if (value < 0x80) {
        return 0;
    }
    if (value < 0xc0) {
        return -1;
    }
    return 1;
}
GP4_IMPL(0x0043cc82, FUN_0043cc82, "cdecl() -> i32; globals=layout=state_0066d8c0")
