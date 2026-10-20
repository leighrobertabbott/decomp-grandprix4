#include <gp4/gp4.h>

// Runs the mass-property model 0x00444517 over all 23 parts (mask 0x7fffff) and keeps a copy of its results
// (centroid 0x0062a064..6c, inertias 0x0062a070..78, mass 0x0062a07c) as the full-car reference at
// 0x0062a084..0x0062a09c. pushal / popal restore every register; the flags are those the callee left.
// FUNCTION: GP4 0x004446c2
void __cdecl FUN_004446c2(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.eax = 0x007fffffu;
    gp4::call_regs(0x00444517, call);

    GP4_GLOBAL(uint32_t, 0x0062a084) = GP4_GLOBAL(uint32_t, 0x0062a064);
    GP4_GLOBAL(uint32_t, 0x0062a088) = GP4_GLOBAL(uint32_t, 0x0062a068);
    GP4_GLOBAL(uint32_t, 0x0062a08c) = GP4_GLOBAL(uint32_t, 0x0062a06c);
    GP4_GLOBAL(uint32_t, 0x0062a090) = GP4_GLOBAL(uint32_t, 0x0062a070);
    GP4_GLOBAL(uint32_t, 0x0062a094) = GP4_GLOBAL(uint32_t, 0x0062a074);
    GP4_GLOBAL(uint32_t, 0x0062a098) = GP4_GLOBAL(uint32_t, 0x0062a078);
    GP4_GLOBAL(uint32_t, 0x0062a09c) = GP4_GLOBAL(uint32_t, 0x0062a07c);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004446c2, FUN_004446c2, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_00444517")
