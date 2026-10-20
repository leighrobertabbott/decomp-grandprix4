#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the flag byte 0x00828101; when the text cursor [0x008280ed] is nonzero it is decremented and
// 0x00533dc5 deletes the character at the new cursor. The flags are those of `cmp [0x8280ed], 0` when
// the cursor was zero, else those the callee leaves.
// FUNCTION: GP4 0x00533ce0
void __cdecl FUN_00533ce0(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008280ed);
    uint32_t flags = gp4::flags::sub32(cursor, 0u);
    if (cursor != 0u) {
        GP4_GLOBAL(uint32_t, 0x008280ed) = cursor - 1u;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00533dc5, c);
        flags = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00533ce0, FUN_00533ce0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5")
