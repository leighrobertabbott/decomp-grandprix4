#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the readout writer from the elapsed time in eax (signed compare with 600000 ms = 10 min):
// below it 0x0051435d (BCD digits of the time), otherwise 0x0051446b (the "-:--.---" placeholder).
// Everything the callee leaves (registers and flags) is the result.
static __declspec(noinline) void time_readout(gp4::Regs& c) {
    gp4::call_regs(0x0051435d, c);
}

static __declspec(noinline) void dash_readout(gp4::Regs& c) {
    gp4::call_regs(0x0051446b, c);
}

// FUNCTION: GP4 0x005142c3
void __cdecl FUN_005142c3(gp4::Regs* r) {
    gp4::Regs c = *r;
    if (static_cast<int32_t>(r->eax) < 0x927c0) {
        time_readout(c);
    } else {
        dash_readout(c);
    }
    *r = c;
}
GP4_IMPL(0x005142c3, FUN_005142c3, "regs(eax:i32[-100..1200000], edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
