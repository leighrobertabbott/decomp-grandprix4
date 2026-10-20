#include <gp4/gp4.h>

// Resets the selected-car sampling state: the four samples 0x0062f958/5c/60/64 and the retire mask 0x0062f8dc
// are cleared, [0x0062f8fe] becomes 0xff, [0x007a8e00] is cleared, then 0x00445e86 refreshes the selected car
// (0x0062f8b0). Registers and flags are whatever 0x00445e86 leaves.
// FUNCTION: GP4 0x00446012
void __cdecl FUN_00446012(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062f964) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f95c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f958) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f960) = 0;
    GP4_GLOBAL(uint8_t, 0x0062f8fe) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x0062f8dc) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8e00) = 0;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00445e86, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00446012, FUN_00446012, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+carbg_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+carno_0066ff6c+flag_007a97a0+state_0042595c+words_0062d210+state_0066d8c0+cars_00444d60+state_00442d1f+clock_0066d814+flag_00604418+state_00446012")
