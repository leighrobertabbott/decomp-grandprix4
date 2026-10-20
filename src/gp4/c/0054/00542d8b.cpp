#include <gp4/gp4.h>

// Register-preserving stdcall wrapper around 0x00542da8: edx = the group selector argument, and the
// pointer the routine leaves in esi is returned in eax. ebx, ecx, edx, ebp, esi and edi are saved and
// restored around the call.
// FUNCTION: GP4 0x00542d8b
int32_t __stdcall FUN_00542d8b(int32_t group) {
    gp4::Regs c = {};
    c.edx = static_cast<uint32_t>(group);
    gp4::call_regs(0x00542da8, c);
    return static_cast<int32_t>(c.esi);
}
GP4_IMPL(0x00542d8b, FUN_00542d8b, "stdcall(i32[0..20]) -> i32; globals=layout=strtab_00542da8")
