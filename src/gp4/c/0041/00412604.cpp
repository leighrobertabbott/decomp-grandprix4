#include <gp4/gp4.h>

// FUNCTION: GP4 0x00412604
// Copies the rate dword 0x007ad8b8 into 0x00603854 / 0x0060384c and derives the fixed-point
// constants that depend on it (reciprocal word, 8000*S>>16, 0x147ae14/S, (S<<14)/0xa00,
// (0xa00<<14)/S, a clamped table lookup and 0x006247bc*S>>16), then runs the table builders
// 0x004124ea and 0x004125ce. The registers on exit are the ones those two leave behind.
void __cdecl FUN_00412604(gp4::Regs* r) {
    const uint32_t rate = GP4_GLOBAL(uint32_t, 0x007ad8b8);
    GP4_GLOBAL(uint32_t, 0x00603854) = rate;
    GP4_GLOBAL(uint32_t, 0x0060384c) = rate;

    gp4::Regs callee = *r;
    callee.eax = GP4_GLOBAL(uint32_t, 0x007ad8b8);
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x004126bc, callee);
    GP4_GLOBAL(uint16_t, 0x0060386c) = (uint16_t)callee.eax;

    const uint32_t current = GP4_GLOBAL(uint32_t, 0x00603854);
    GP4_GLOBAL(uint32_t, 0x00623df0) = (0x1f40u * current) >> 16;
    GP4_GLOBAL(uint32_t, 0x00623e0c) = 0x147ae14u / current;
    GP4_GLOBAL(uint32_t, 0x00603858) = (current << 14) / 0xa00u;
    GP4_GLOBAL(int32_t, 0x0060385c) = 0x2800000 / (int32_t)current;

    uint32_t slot = current >> 8;
    if ((int32_t)slot > 0x20) {
        slot = 0x20;
    }
    if ((int32_t)slot < 0xa) {
        slot = 0xa;
    }
    GP4_GLOBAL(uint32_t, 0x00603344) = GP4_ARRAY(uint32_t, 0x00603348)[slot];

    const int64_t product = (int64_t)GP4_GLOBAL(int32_t, 0x006247bc) * (int64_t)(int32_t)current;
    GP4_GLOBAL(uint32_t, 0x006247c0) = (uint32_t)(product >> 16);

    gp4::call_regs(0x004124ea, callee);
    gp4::call_regs(0x004125ce, callee);

    r->eax = callee.eax;
    r->ebx = callee.ebx;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->ebp = callee.ebp;
    r->eflags = callee.eflags;
}
GP4_IMPL(0x00412604, FUN_00412604, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004124ea+state_004125ce+rate_007ad8b8")
