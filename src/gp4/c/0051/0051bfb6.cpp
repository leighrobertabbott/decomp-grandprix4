#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: clears [0x96a370], copies [0x7a8e36] to [0x6975e0], points [0x6975dc] at
// 0x6975ec, then initialises four 0x700-entry tables: two word tables (0x9e1b20, 0x9e2920) to
// 0xffff and two byte tables (0x9e3720, 0x9e3e20) to 0x14. Every GPR is restored; the flags are
// those of the final `cmp ecx, 0x700` (equal).
// FUNCTION: GP4 0x0051bfb6
void __cdecl FUN_0051bfb6(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0096a370) = 0u;
    GP4_GLOBAL(uint32_t, 0x006975e0) = GP4_GLOBAL(uint32_t, 0x007a8e36);
    GP4_GLOBAL(uint32_t, 0x006975dc) = 0x006975ecu;
    uint32_t index = 0u;
    do {
        GP4_ARRAY(uint16_t, 0x009e1b20)[index] = 0xffffu;
        GP4_ARRAY(uint16_t, 0x009e2920)[index] = 0xffffu;
        GP4_ARRAY(uint8_t, 0x009e3720)[index] = 0x14u;
        GP4_ARRAY(uint8_t, 0x009e3e20)[index] = 0x14u;
        ++index;
    } while (index != 0x700u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, 0x700u));
}
GP4_IMPL(0x0051bfb6, FUN_0051bfb6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
