#include <gp4/gp4.h>
#include <gp4/flags.h>

// Closes the open file handle: when the byte [0x0081ea2c] is set, CloseHandle([0x0081ea28]) is called
// (through the import thunk 0x005a48ee, inside a pushal/popal pair) and the byte is cleared. Every GPR
// is restored; the flags are those of `cmp byte [0x0081ea2c], 0`.
// FUNCTION: GP4 0x0053646a
void __cdecl FUN_0053646a(gp4::Regs* r) {
    const uint32_t open = GP4_GLOBAL(uint8_t, 0x0081ea2c);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(open, 0u));
    if (open == 0u) {
        return;
    }
    gp4::stdcall_<0x005a48ee, uint32_t>(GP4_GLOBAL(uint32_t, 0x0081ea28));
    GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0u;
}
GP4_IMPL(0x0053646a, FUN_0053646a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
