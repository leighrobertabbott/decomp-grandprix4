#include <gp4/gp4.h>
#include <gp4/flags.h>

// Moves the cursor of the text buffer 0x0082811f to the end of the text (before the reserved suffix
// [0x8280f9]): the flag byte [0x828101] is cleared, 0x00533dc5 deletes the byte at the cursor
// [0x8280ed], the cursor becomes [0x8280f5] - [0x8280f9] (the length read after the deletion) and
// 0x00533d9e inserts the byte [0x828161] there. eax = cursor with its low byte replaced by the
// character; the other registers are kept (pushal/popal) and the flags are those 0x00533d9e leaves.
// FUNCTION: GP4 0x00533cb9
void __cdecl FUN_00533cb9(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00533dc5, c);

    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008280f5) - GP4_GLOBAL(uint32_t, 0x008280f9);
    GP4_GLOBAL(uint32_t, 0x008280ed) = cursor;
    c.eax = (cursor & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
    gp4::call_regs(0x00533d9e, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00533cb9, FUN_00533cb9, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5+font_005339ed")
