#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the byte at 0x007ab0f2 to 2 and increments the (unaligned) dword at 0x00828293. The flags
// are those of the `inc` (CF is kept).
// FUNCTION: GP4 0x00534fd7
void __cdecl FUN_00534fd7(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007ab0f2) = 2u;
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
    GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
}
GP4_IMPL(0x00534fd7, FUN_00534fd7, "regs(cf:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
