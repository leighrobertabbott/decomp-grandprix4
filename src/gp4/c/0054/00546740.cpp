#include <gp4/gp4.h>
#include <gp4/flags.h>

// Prints the unsigned decimal number eax at the cursor edi (0x0054642c, bh forced to 0: no sign, no padding),
// then expands the text script with id eax + 0x45 at the new cursor through the interpreter 0x0054d798. eax and
// esi are saved and restored around the pair; bh stays 0 and the other registers and the flags are those the
// interpreter leaves.
// FUNCTION: GP4 0x00546740
void __cdecl FUN_00546740(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = c.ebx & 0xffff00ffu;
    gp4::call_regs(0x0054642c, c);
    c.esi = c.eax + 0x45u;
    gp4::call_regs(0x0054d798, c);
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00546740, FUN_00546740, "regs(eax:u32{=0xffffffbb,0xffffffbc,0xffffffbd,0xffffffbe,0xffffffbf,0xffffffc0,0xffffffc1,0xffffffc2,0xb61,0xb68,0xb6c,0xb76}, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:ptr[0x100]:bytes, ebp:u32) -> (ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
