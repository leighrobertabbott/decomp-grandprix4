#include <gp4/gp4.h>
#include <gp4/flags.h>

// `mov ecx, 1; call 0x00541e4d; jmp [0x008334d7]`: runs the pending-state reset 0x00541e4d with
// ecx = 1, then tail-jumps through the handler pointer stored (unaligned) at 0x008334d7. Registers
// and flags come straight back from the handler.
// FUNCTION: GP4 0x00531fc8
void __cdecl FUN_00531fc8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = 1u;
    gp4::call_regs(0x00541e4d, c);
    gp4::call_regs(GP4_GLOBAL(uint32_t, 0x008334d7), c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00531fc8, FUN_00531fc8, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
