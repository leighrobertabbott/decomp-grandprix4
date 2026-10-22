#include <gp4/gp4.h>
#include <gp4/flags.h>

// Screen runner: pushes the record kind 0 on the 0x28-byte record stack (0x0054faff, al = 0), runs the front-end
// screen loop 0x0053d350, pops the record again (0x0054fb87) and returns the selected entry, the byte
// [0x0081e4f8], zero-extended in eax. Registers and flags thread through the callees.
// FUNCTION: GP4 0x0053c8af
void __cdecl FUN_0053c8af(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = c.eax & 0xffffff00u;
    gp4::call_regs(0x0054faff, c);
    gp4::call_regs(0x0053d350, c);
    gp4::call_regs(0x0054fb87, c);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081e4f8);
    *r = c;
}
GP4_IMPL(0x0053c8af, FUN_0053c8af, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32{=0x0054fd8c,0x0054fd98,0x0054e232,0x00527d03,0x00531741}, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; emu=skip:win32 PeekMessageA")
