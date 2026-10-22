#include <gp4/gp4.h>

// Builds the text line at the string buffer 0x00823100: the text script 0x902 (0x0054d798), then the
// token copied from the string pointed to by [0x0081d410] (0x0052b35a: stops at the first character
// <= 0x20, ':' or '='), then the control-coded script at 0x0081d5f9 (0x0054d7c0). The empty stub
// 0x0054e232 is called with esi = 0x00823100. Finally the byte [0x0081d420] is set to 0xff. One
// register block is threaded through every call; the flags are those the script expansion leaves.
// FUNCTION: GP4 0x0052b28a
void __cdecl FUN_0052b28a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.edi = 0x00823100u;
    c.esi = 0x902u;
    gp4::call_regs(0x0054d798, c);
    c.esi = GP4_GLOBAL(uint32_t, 0x0081d410);
    gp4::call_regs(0x0052b35a, c);
    c.esi = 0x0081d5f9u;
    gp4::call_regs(0x0054d7c0, c);
    c.esi = 0x00823100u;
    gp4::cdecl_<0x0054e232, void>();
    GP4_GLOBAL(uint8_t, 0x0081d420) = 0xffu;
    *r = c;
}
GP4_IMPL(0x0052b28a, FUN_0052b28a, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=script_all_00835846; gfields={0x0081d410:ptr[0x40]:u8{=0x41,0x5a,0x61,0x30,0x39,0x20,0x3a,0x3d,0}, 0x0081d5f9*64/1:u8{=0,1,2,3,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x20,0x41,0x7f,0x80,0xff}}")
