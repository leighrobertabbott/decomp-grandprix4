#include <gp4/gp4.h>

// pushal wrapper: refreshes the dword at +0x19c of the record in esi (0x00408790), then derives the
// +0xb2 / +0xd6 words from it (0x004087aa). popal restores every register but not the flags, which
// are those 0x004087aa leaves.
// FUNCTION: GP4 0x00408783
void __cdecl FUN_00408783(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x00408790, call);
    gp4::call_regs(0x004087aa, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00408783, FUN_00408783, "regs(esi:ptr[0x1a4]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
