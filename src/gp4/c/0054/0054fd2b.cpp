#include <gp4/gp4.h>

// pushal wrapper: jumps over an embedded string (0x0054fd2d, "Bad menu type init"), then calls the empty
// stub 0x0054e232 with esi pointing at that string (a debug-message call whose output was compiled out).
// popal restores every GPR and no flag is written.
// FUNCTION: GP4 0x0054fd2b
void __cdecl FUN_0054fd2b() {
    gp4::Regs c = {};
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>("Bad menu type init"));
    gp4::call_regs(0x0054e232, c);
}
GP4_IMPL(0x0054fd2b, FUN_0054fd2b, "cdecl() -> void")
