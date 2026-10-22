#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that keeps the menu cursor object [0x0081ce75] in step with the byte [0x0082fd5e].
// Non-zero: 0x00528535 picks the object for the current cursor; when it differs from the cached one it
// is stored in [0x0081ce75] and 0x00528140 is run on it. Zero: 0x005284d8 (eax = 2) clears the matching
// entries of the 8-entry table and the cache is reset to -1. popal restores every register; one Regs
// is threaded through the callees and the flags that remain are those of the last flag writer (the
// callee's, or the `cmp eax, [0x0081ce75]` when the cache already holds the object).
static __declspec(noinline) void pick_object(gp4::Regs& c) {
    gp4::call_regs(0x00528535, c);
}

static __declspec(noinline) void run_object(gp4::Regs& c) {
    gp4::call_regs(0x00528140, c);
}

static __declspec(noinline) void clear_entries(gp4::Regs& c) {
    gp4::call_regs(0x005284d8, c);
}

// FUNCTION: GP4 0x005284fc
void __cdecl FUN_005284fc(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x0082fd5e);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(enabled, 0u));
    if (enabled != 0u) {
        pick_object(c);
        const uint32_t cached = GP4_GLOBAL(uint32_t, 0x0081ce75);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, cached));
        if (c.eax != cached) {
            GP4_GLOBAL(uint32_t, 0x0081ce75) = c.eax;
            run_object(c);
        }
    } else {
        c.eax = 2u;
        clear_entries(c);
        GP4_GLOBAL(uint32_t, 0x0081ce75) = 0xffffffffu;
    }

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005284fc, FUN_005284fc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0052848b+frames_008174f8+mode_007aafd3+flag_007ab020+cell_0053ae38+drv22_0052d402")
