#include <gp4/gp4.h>
#include <gp4/flags.h>

// Block-transfer driver: sets up the 0x300-byte block counter from eax/ecx (0x00541830), then repeats the
// block step 0x00541864 and the check 0x005418b0 for as long as the check returns CF = 1 (data left to
// send). Registers and flags are those the last check leaves (CF = 0).
// FUNCTION: GP4 0x00541816
void __cdecl FUN_00541816(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00541830, c);
    do {
        gp4::call_regs(0x00541864, c);
        gp4::call_regs(0x005418b0, c);
    } while ((c.eflags & 1u) != 0u);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00541816, FUN_00541816, "regs(eax:u32, ebx:u32, ecx:u32{=0,0,1,0x10,0x2ff,0x300,0x301,0x600,0x1000}, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
