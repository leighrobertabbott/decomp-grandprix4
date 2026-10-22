#include <gp4/gp4.h>

// Forwarder: loads ecx = 0x00a64f38 (a record whose +0x18 head of a heap-block list is freed by
// 0x004ac390) and tail-jumps into that method. The method always leaves eax = 0.
// FUNCTION: GP4 0x004ac370
uint32_t __cdecl FUN_004ac370() {
    return gp4::thiscall_<0x004ac390, uint32_t>(reinterpret_cast<void*>(0x00a64f38));
}
GP4_IMPL(0x004ac370, FUN_004ac370, "cdecl() -> u32")
