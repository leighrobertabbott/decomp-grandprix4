#include <gp4/gp4.h>
#include <gp4/flags.h>

// Backspace in the text buffer 0x0082811f (the mirror of the typing routine 0x00533c65): the flag byte
// [0x828101] is cleared and, when the cursor [0x8280ed] is above zero, 0x00533dc5 deletes the byte at the
// cursor, the cursor moves back one and 0x00533d9e inserts the character [0x828161] there (al). With the
// cursor at or below zero nothing else happens and the flags are those of `cmp [0x8280ed], 0`; otherwise
// they are those 0x00533d9e leaves and eax has its low byte replaced by the character.
// FUNCTION: GP4 0x00533c3f
void __cdecl FUN_00533c3f(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
    const int32_t cursor = GP4_GLOBAL(int32_t, 0x008280ed);
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(cursor), 0u);
    if (cursor <= 0) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x00533dc5, c);

    GP4_GLOBAL(uint32_t, 0x008280ed) = GP4_GLOBAL(uint32_t, 0x008280ed) - 1u;
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
    gp4::call_regs(0x00533d9e, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00533c3f, FUN_00533c3f, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5")
