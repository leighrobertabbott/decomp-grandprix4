#include <gp4/gp4.h>

// Passes its argument in eax to the block-pool allocator 0x005389b9 and returns its eax.
// FUNCTION: GP4 0x00507a00
int __cdecl FUN_00507a00(int size) {
    gp4::Regs c = {};
    c.eax = static_cast<uint32_t>(size);
    gp4::call_regs(0x005389b9, c);
    return static_cast<int>(c.eax);
}
GP4_IMPL(0x00507a00, FUN_00507a00, "cdecl(u32{0,1,0x10,0x20,0x3f,0x40,0x41,0x80,0xc0,0x100,0x140,0x180,0x200,0x280,0x300,0xa06100,0xa06200,0xa06301,0xffffffff,0x1000000}) -> i32; globals=layout=pool_005389b9")
