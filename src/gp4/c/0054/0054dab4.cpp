#include <gp4/gp4.h>

// `push esi ; mov esi, 0xb3 ; call 0x0054d798 ; pop esi ; ret`: expands the text script with id 0xb3
// (string pointer from the table 0x835846, via the interpreter 0x0054d798) at the cursor edi. esi is
// restored by the pop; every other register and the flags come back from the callee.
// FUNCTION: GP4 0x0054dab4
void __cdecl FUN_0054dab4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = 0x000000b3u;
    gp4::call_regs(0x0054d798, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054dab4, FUN_0054dab4, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:ptr[0x100]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0054dab4")
