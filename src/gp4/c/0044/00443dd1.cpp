#include <gp4/gp4.h>

static __declspec(noinline) void runDefaultView(gp4::Regs& c) {
    gp4::call_regs(0x00443de9, c);
}

static __declspec(noinline) void runAlternateView(gp4::Regs& c) {
    gp4::call_regs(0x00443e26, c);
}

// Dispatches on the byte flag 0x0062794b: zero runs 0x00443de9, non-zero runs 0x00443e26 (both take the object
// in esi). pushal / popal restore every register; only the flags left by the callee survive.
// FUNCTION: GP4 0x00443dd1
void __cdecl FUN_00443dd1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    if (GP4_GLOBAL(uint8_t, 0x0062794b) == 0u) {
        runDefaultView(c);
    } else {
        runAlternateView(c);
    }
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00443dd1, FUN_00443dd1, "regs(esi:ptr[0x90]:layout=view_00443de9) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0")
