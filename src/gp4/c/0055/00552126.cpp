#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-segment draw setup: walks the 0x160-byte ring records from [0x009631fc] for [0x009631f4] = 0..
// [0x009631f8] (signed, inclusive). For each record 0x00551155 loads the segment values and 0x005520a0
// fills the ten slots of the block esi = 0x38 * index. After each step the record pointer advances by
// 0x160 and wraps: at or past the lap end [0x00866ecc] it goes back by [0x00866ec4] while still below the
// pit start [0x00891500], or is clamped to the last pit record [0x00866ef8] - 0x160 once at or past the
// pit end. Leaves edi (record), esi (0x38 * last index), eax (counter) and the flags of the final compare.
// FUNCTION: GP4 0x00552126
void __cdecl FUN_00552126(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t record = GP4_GLOBAL(uint32_t, 0x009631fc);
    GP4_GLOBAL(uint32_t, 0x009631f4) = 0u;
    uint32_t counter;
    do {
        c.edi = record;
        gp4::call_regs(0x00551155, c);
        c.esi = GP4_GLOBAL(uint32_t, 0x009631f4) * 0x38u;
        gp4::call_regs(0x005520a0, c);
        GP4_GLOBAL(uint32_t, 0x009631f4) = GP4_GLOBAL(uint32_t, 0x009631f4) + 1u;
        record += 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        counter = GP4_GLOBAL(uint32_t, 0x009631f4);
    } while (static_cast<int32_t>(counter) <= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x009631f8)));

    c.edi = record;
    c.eax = counter;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(counter, GP4_GLOBAL(uint32_t, 0x009631f8)));
    *r = c;
}
GP4_IMPL(0x00552126, FUN_00552126, "regs() -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227+mask_005520a0+ring_00555b28+state_00552126")
