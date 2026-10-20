#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the dword at +0x14 of the record at 0x00829868 with 0x0082fcac. When they differ it runs
// the empty routine 0x0054e232 (esi = 0x00538799) between a pushal/popal pair, so every register
// is unchanged; eax is saved and restored too. The flags are those of the `cmp`.
// FUNCTION: GP4 0x00538786
void __cdecl FUN_00538786(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x00829868);
    const uint32_t first = GP4_FIELD(uint32_t, record, 0x14);
    const uint32_t second = GP4_GLOBAL(uint32_t, 0x0082fcac);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(first, second));
    if (first != second) {
        gp4::Regs c = *r;
        c.esi = 0x00538799u;
        gp4::call_regs(0x0054e232, c);
    }
}
GP4_IMPL(0x00538786, FUN_00538786, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=eq_00538786")
