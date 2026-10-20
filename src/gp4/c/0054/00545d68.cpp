#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: formats the race time eax (milliseconds) as text into the static buffer 0x00823100 through
// 0x00545d7f, then draws that text with the string renderer 0x00545f09 (esi = the buffer). Every register is
// kept; the flags are those the renderer leaves.
// FUNCTION: GP4 0x00545d68
void __cdecl FUN_00545d68(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00823100)));
    gp4::call_regs(0x00545d7f, c);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00823100)));
    gp4::call_regs(0x00545f09, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00545d68, FUN_00545d68, "regs(eax:u32{0,1,500,999,1000,1001,59999,60000,60001,3599999,3600000,3600001,3660000,3661000,3661001,0x3e7ffff,0x3e80000,0xffffffff,0x80000000}, ebx:u32, ecx:u32, edx:u32, ebp:u32{0,0,0,1,0xffffffff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0052ad37+cursor_00545f09+surf_0054654a")
