#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the three row-bound slots 0, 1 and 3 through 0x00520584 (which fills each slot's rows
// and sets its cursor / marker), then clears the surrounding bookkeeping: two scalars, three
// (-1, -1, 0) triples at 0x7a9128.., 0x7a914c, and the 99-entry table at 0x7a9154 set to -1.
// pushal/popal restore every GPR; the flags left behind are those of the last `inc eax` of the
// 99-entry loop (eax 98 -> 99, CF kept from the last callee's compare).
// FUNCTION: GP4 0x005205cf
void __cdecl FUN_005205cf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0u;
    gp4::call_regs(0x00520584, c);
    c.eax = 1u;
    gp4::call_regs(0x00520584, c);
    c.eax = 3u;
    gp4::call_regs(0x00520584, c);

    GP4_GLOBAL(uint32_t, 0x0066d834) = 0u;
    GP4_GLOBAL(uint32_t, 0x006975bc) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9128) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a912c) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9130) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9134) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9138) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a913c) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9140) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9144) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9148) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a914c) = 0u;

    uint32_t* table = GP4_ARRAY(uint32_t, 0x007a9154);
    for (uint32_t i = 0; i < 99u; i++) {
        table[i] = 0xffffffffu;
    }
    GP4_GLOBAL(uint32_t, 0x007a9124) = 0u;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(98u, c.eflags));
}
GP4_IMPL(0x005205cf, FUN_005205cf, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584")
