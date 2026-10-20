#include <gp4/gp4.h>

// pushal / push ebx,ecx,edx / popal wrapper around kernel32!QueryPerformanceCounter (thunk
// 0x005a48be: jmp [0x5f0254]) storing the 64-bit counter at 0x00838c54 (the start stamp that
// 0x0054e665/0x0054e6af subtract). Every register is preserved.
// FUNCTION: GP4 0x0054e615
void __cdecl FUN_0054e615(gp4::Regs* r) {
    (void)r;
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c54));
}
GP4_IMPL(0x0054e615, FUN_0054e615, "regs() -> (); globals=fuzz:i32")
