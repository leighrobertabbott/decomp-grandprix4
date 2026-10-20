#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; eax = [0x007ad894] * 4 + [0x00838e94] + 0x236 (the circuit number scaled to a frame id);
// the R3 frame-cell hand-off 0x00527fd4 runs with that eax and the caller's esi/edi; popal restores
// every register, so only the flags (those the hand-off leaves) come back.
// FUNCTION: GP4 0x0054f555
void __cdecl FUN_0054f555(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (GP4_GLOBAL(uint32_t, 0x007ad894) << 2) + GP4_GLOBAL(uint32_t, 0x00838e94) + 0x236u;
    gp4::call_regs(0x00527fd4, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054f555, FUN_0054f555, "regs(esi:ptr[0x10]:bytes, edi:ptr[8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae+slot8_005388ba+trk_007ad894+frame_0054f555")
