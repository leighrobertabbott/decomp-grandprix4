#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script wrapper: runs 0x00545efe with esi = eax + 0xb90 (the text entry number biased by
// 0xb90, so eax = entry - 0xb90) and restores esi afterwards (push/pop). Every other register and
// the flags come straight back from the callee.
// FUNCTION: GP4 0x00545ea5
void __cdecl FUN_00545ea5(gp4::Regs* r) {
    const uint32_t saved_esi = r->esi;
    gp4::Regs c = *r;
    c.esi = r->eax + 0xb90u;
    gp4::call_regs(0x00545efe, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = saved_esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00545ea5, FUN_00545ea5, "regs(eax:u32{=0xfffff470,0xfffff471,0xfffff472,0xfffff473,0xfffff474,0xfffff475,0xfffff476,0xfffff477}, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_00545f09+surf_0054654a+script_00545efe")
