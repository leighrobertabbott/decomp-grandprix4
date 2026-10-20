#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x005339ed ; call 0x00533ae7 ; ret`: the text-field click handler runs first, then the
// text-field key handler. The registers each call leaves carry into the next; registers and flags
// come back from the second call.
// FUNCTION: GP4 0x005339d2
void __cdecl FUN_005339d2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005339ed, c);
    gp4::call_regs(0x00533ae7, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005339d2, FUN_005339d2, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6+span_00533dc5+font_005339ed+chars_00828108+keys_00533ae7")
