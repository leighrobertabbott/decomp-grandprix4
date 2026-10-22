#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands the text pieces of the entry with key al in the table at 0x0081d891 (0x0052c689: eax/ebx/ecx
// become its three signed bytes, eax = -1 when there is no such entry, which ends the routine). A hit
// writes into the buffer at edi: the fixed string at 0x0081d956 (0x0054d7c0), then the script text with
// id 0xa12 (ebx != 0) or 0xa02 (ebx == 0) plus ecx (0x0054d798), then the fixed string at 0x0081d959.
// ebx and ecx are saved and restored; eax, edx, esi, edi, ebp and the flags are left as the last callee
// leaves them (the flags of the `cmp eax, -1` on a miss).
// FUNCTION: GP4 0x0052c9e1
void __cdecl FUN_0052c9e1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0052c689, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0xffffffffu));
    if (c.eax != 0xffffffffu) {
        c.esi = 0x0081d956u;
        gp4::call_regs(0x0054d7c0, c);

        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebx));
        c.esi = (c.ebx != 0u) ? 0x00000a12u : 0x00000a02u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.esi, c.ecx));
        c.esi += c.ecx;
        gp4::call_regs(0x0054d798, c);

        c.esi = 0x0081d959u;
        gp4::call_regs(0x0054d7c0, c);
    }

    r->eax = c.eax;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052c9e1, FUN_0052c9e1, "regs(eax:u32[0..255], esi:u32, edi:ptr[0x400]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846")
