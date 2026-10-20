#include <gp4/gp4.h>
#include <gp4/flags.h>

// Translates the character in al through the 256-byte table at 0x00833d18 (al = table[al]); the
// other bits of eax and ebx are preserved. The flags are those of `add ebx, 0x833d18` (ebx = al).
// FUNCTION: GP4 0x0054d747
void __cdecl FUN_0054d747(gp4::Regs* r) {
    const uint32_t index = r->eax & 0xffu;
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x00833d18);
    r->eax = (r->eax & 0xffffff00u) | table[index];
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(index, 0x00833d18u));
}
GP4_IMPL(0x0054d747, FUN_0054d747, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
