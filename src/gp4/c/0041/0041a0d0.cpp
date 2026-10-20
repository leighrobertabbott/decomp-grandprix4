#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041a0d0
// Resets the two Q14 factors [0x62f880] / [0x62fb54] to 1.0 (0x4000), then runs the eight
// per-step routines in a fixed order. esi (the car record) is passed through unchanged; the other
// registers flow from one routine to the next, so the caller sees what the last one left.
void __cdecl FUN_0041a0d0(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062f880) = 0x4000u;
    GP4_GLOBAL(uint32_t, 0x0062fb54) = 0x4000u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004168df, c);
    gp4::call_regs(0x00416b4c, c);
    gp4::call_regs(0x00417467, c);
    gp4::call_regs(0x004168b6, c);
    gp4::call_regs(0x00416b52, c);
    gp4::call_regs(0x0041746d, c);
    gp4::call_regs(0x00417066, c);
    gp4::call_regs(0x00416ec9, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0041a0d0, FUN_0041a0d0, "regs(esi:ptr[0x300]:layout=car_00416ec9) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00416dec+state_00416fce")
