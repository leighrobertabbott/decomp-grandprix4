#include <gp4/gp4.h>

// Reads the performance counter into the qword at 0x00838c5c (thunk 0x005a48be:
// QueryPerformanceCounter) and returns it in edx:eax. esi, edi, ebx and ecx are saved and restored;
// no flag is written by the routine itself.
// FUNCTION: GP4 0x0054e733
void __cdecl FUN_0054e733(gp4::Regs* r) {
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c5c));
    r->eax = GP4_GLOBAL(uint32_t, 0x00838c5c);
    r->edx = GP4_GLOBAL(uint32_t, 0x00838c60);
}
GP4_IMPL(0x0054e733, FUN_0054e733, "regs() -> (eax, edx); globals=fuzz:i32+layout=qpc_0054e733")
