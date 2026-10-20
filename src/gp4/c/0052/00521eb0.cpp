#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: clears the hold byte 0x0066d8e9 and the due time 0x0066d840, runs the per-index
// clamp/step routine 0x00521b93 (esi = the record), and clears both again. Every GPR is restored; the
// flags are those 0x00521b93 leaves.
// FUNCTION: GP4 0x00521eb0
void __cdecl FUN_00521eb0(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0066d8e9) = 0u;
    GP4_GLOBAL(uint32_t, 0x0066d840) = 0u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00521b93, c);
    GP4_GLOBAL(uint8_t, 0x0066d8e9) = 0u;
    GP4_GLOBAL(uint32_t, 0x0066d840) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00521eb0, FUN_00521eb0, "regs(esi:ptr[0x94]:u8{0,1,2,4,8,0x10,0x20,0x40,0x80,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=clock_0066d814+carbg_0066fef0+state_004059ef+player_0040f8d9+car_0052197e+links_00521d6f+grid_00521eb0")
