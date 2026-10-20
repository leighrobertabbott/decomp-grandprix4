#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the size of the open file handle [0x0081ea28] into [0x0081ea30]: GetFileSize(handle, NULL)
// through the thunk 0x005a48fa (bracketed by push/pop of ebx, ecx, edx). eax is the size, CF is
// cleared by `clc`; the other flags stay as the call left them.
// FUNCTION: GP4 0x0053644d
void __cdecl FUN_0053644d(gp4::Regs* r) {
    const uint32_t size = gp4::stdcall_<0x005a48fa, uint32_t>(GP4_GLOBAL(uint32_t, 0x0081ea28), 0u);
    GP4_GLOBAL(uint32_t, 0x0081ea30) = size;
    r->eax = size;
    r->eflags = r->eflags & ~gp4::CF;
}
GP4_IMPL(0x0053644d, FUN_0053644d, "regs() -> (eax, cf); globals=fuzz:i32")
