#include <gp4/gp4.h>
#include <gp4/flags.h>

// Replaces the first character of the text buffer 0x0082811f: the flag byte [0x828101] is cleared,
// 0x00533dc5 deletes the character at the cursor, the cursor [0x8280ed] is reset to 0 and 0x00533d9e
// inserts the byte [0x828161] (al) there. eax keeps its upper bytes (both callees restore all
// registers); the flags are those 0x00533d9e leaves.
// FUNCTION: GP4 0x00533c98
void __cdecl FUN_00533c98(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00828101) = 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00533dc5, c);

    GP4_GLOBAL(uint32_t, 0x008280ed) = 0u;
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
    gp4::call_regs(0x00533d9e, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00533c98, FUN_00533c98, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5")
