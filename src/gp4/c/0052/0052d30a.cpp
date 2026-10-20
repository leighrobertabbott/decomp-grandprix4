#include <gp4/gp4.h>

// `push eax; push ebx; push edi; pop edi; pop ebx; pop eax; ret`: every register and flag comes
// back unchanged, so the body is empty.
// FUNCTION: GP4 0x0052d30a
void __cdecl FUN_0052d30a(gp4::Regs* /*r*/) {}
GP4_IMPL(0x0052d30a, FUN_0052d30a, "regs() -> ()")
