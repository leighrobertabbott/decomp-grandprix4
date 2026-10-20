#include <gp4/gp4.h>

// pushal / popal around a call of the empty stdcall stub 0x005cad90 (`ret 12`) with the arguments
// (0, 0x100, 0x007a9480): the table at 0x007a9480 is never touched, so nothing changes (registers
// and flags stay as they were).
// FUNCTION: GP4 0x00541e39
void __cdecl FUN_00541e39(gp4::Regs* r) {
    (void)r;
    gp4::stdcall_<0x005cad90, void>(0u, 0x100u, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x007a9480))));
}
GP4_IMPL(0x00541e39, FUN_00541e39, "regs() -> (); globals=fuzz:i32")
