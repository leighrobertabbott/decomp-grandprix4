#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds a text line at the buffer 0x00823100: expands the text script 0x901 there (0x0054d798), appends
// a token copied from the string [0x0081d410] (0x0052b35a), expands the script text at 0x0081d5fc after
// it (0x0054d7c0), appends a token from the string [0x0081d414] and finally hands the buffer start to the
// empty routine 0x0054e232. esi / edi are saved and restored; the other registers and the flags are those
// the last callee leaves.
// FUNCTION: GP4 0x0052b31c
void __cdecl FUN_0052b31c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.edi = 0x00823100u;
    c.esi = 0x901u;
    gp4::call_regs(0x0054d798, c);

    c.esi = GP4_GLOBAL(uint32_t, 0x0081d410);
    gp4::call_regs(0x0052b35a, c);

    c.esi = 0x0081d5fcu;
    gp4::call_regs(0x0054d7c0, c);

    c.esi = GP4_GLOBAL(uint32_t, 0x0081d414);
    gp4::call_regs(0x0052b35a, c);

    c.esi = 0x00823100u;
    gp4::call_regs(0x0054e232, c);

    c.esi = r->esi;
    c.edi = r->edi;
    *r = c;
}
GP4_IMPL(0x0052b31c, FUN_0052b31c, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=script_all_00835846; gfields={0x0081d410:ptr[0x40]:u8{=0x41,0x5a,0x61,0x30,0x39,0x20,0x3a,0x3d,0}, 0x0081d414:ptr[0x40]:u8{=0x41,0x5a,0x61,0x30,0x39,0x20,0x3a,0x3d,0}, 0x0081d5fc*64/1:u8{=0,1,2,3,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x20,0x41,0x7f,0x80,0xff}}")
