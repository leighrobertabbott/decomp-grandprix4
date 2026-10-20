#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: orders the two corner pairs so that ebp >= ebx (signed; otherwise ebp/ebx and ecx/eax
// are exchanged), then calls the line routine 0x004fe1b0 with (ebx, eax, ebp, ecx, [0x00827559]) as
// five stack arguments, inside a second pushal/popal pair. popal restores every register; the flags
// are those the callee leaves.
// FUNCTION: GP4 0x00531b0a
void __cdecl FUN_00531b0a(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t ebp = r->ebp;
    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(ebp, ebx));
    if (static_cast<int32_t>(ebp) < static_cast<int32_t>(ebx)) {
        const uint32_t swapBp = ebp;
        ebp = ebx;
        ebx = swapBp;
        const uint32_t swapAx = ecx;
        ecx = eax;
        eax = swapAx;
    }
    c.eax = eax;
    c.ebx = ebx;
    c.ecx = ecx;
    c.ebp = ebp;
    const uint32_t args[5] = {ebx, eax, ebp, ecx, GP4_GLOBAL(uint32_t, 0x00827559)};
    gp4::call_regs_stdcall(0x004fe1b0, c, args);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00531b0a, FUN_00531b0a, "regs(eax:i32, ebx:i32, ecx:i32, ebp:i32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
