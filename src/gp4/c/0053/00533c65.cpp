#include <gp4/gp4.h>
#include <gp4/flags.h>

// Types the pending character into the text buffer 0x0082811f (the typing counterpart of the delete
// 0x00533ce0): the flag byte [0x828101] is cleared and, if the cursor [0x8280ed] + 1 + the window
// offset [0x8280f9] is still below the length [0x8280f5], 0x00533dc5 deletes the character at the cursor,
// the cursor advances and 0x00533d9e inserts the character [0x828161] there (al). eax ends as the
// cursor sum (its low byte replaced by the character when typing); the flags are those of the
// `cmp eax, [0x8280f5]` when the buffer is full, else those 0x00533d9e leaves.
// FUNCTION: GP4 0x00533c65
void __cdecl FUN_00533c65(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008280ed);
    uint32_t sum = cursor + 1u;
    uint32_t arith = gp4::flags::add32(sum, GP4_GLOBAL(uint32_t, 0x008280f9));
    sum += GP4_GLOBAL(uint32_t, 0x008280f9);
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x008280f5);
    arith = gp4::flags::sub32(sum, length);
    if (sum >= length) {
        r->eax = sum;
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = sum;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x00533dc5, c);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x008280ed);
    GP4_GLOBAL(uint32_t, 0x008280ed) = before + 1u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(before, c.eflags));
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
    gp4::call_regs(0x00533d9e, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00533c65, FUN_00533c65, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5")
