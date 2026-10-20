#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-entry cursor step (the forward counterpart of 0x00533ce0): clears the flag byte 0x00828101 and
// computes eax = cursor [0x8280ed] + 1 + suffix [0x8280f9]. When that is below the length [0x8280f5]
// the cursor is advanced, 0x00533dc5 deletes the character there, and the cursor is moved back. eax
// keeps the cursor sum (pushal/popal inside the callee restore it); the flags are those of
// `cmp eax, [0x8280f5]` when nothing ran, else those of the final `dec [0x8280ed]` (CF kept from the
// callee).
// FUNCTION: GP4 0x00533cfc
void __cdecl FUN_00533cfc(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008280ed);
    uint32_t sum = cursor + 1u;
    sum += GP4_GLOBAL(uint32_t, 0x008280f9);
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x008280f5);
    const uint32_t compare = gp4::flags::sub32(sum, length);
    r->eax = sum;
    if (sum >= length) {
        r->eflags = gp4::flags::merge(r->eflags, compare);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x008280ed) = cursor + 1u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(cursor, compare));
    gp4::call_regs(0x00533dc5, c);

    const uint32_t current = GP4_GLOBAL(uint32_t, 0x008280ed);
    GP4_GLOBAL(uint32_t, 0x008280ed) = current - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(current, c.eflags));
}
GP4_IMPL(0x00533cfc, FUN_00533cfc, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5+font_005339ed")
