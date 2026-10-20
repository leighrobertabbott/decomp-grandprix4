#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: dispatches on the menu/key-state mode byte 0x00824649 (clamped to 15) through the
// 16-entry handler table at 0x0052d9c5 (eax = the table slot address, as in `call [eax]`), then stores
// 0xff in 0x0082fd5f. popal restores every GPR; the flags are those the handler leaves, and a handler
// that touches none keeps those of the `add eax, 0x52d9c5` that formed the slot address.
// FUNCTION: GP4 0x0052d99f
void __cdecl FUN_0052d99f(gp4::Regs* r) {
    uint32_t mode = GP4_GLOBAL(uint8_t, 0x00824649);
    if (mode > 0xfu) {
        mode = 0xfu;
    }
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = (mode << 2) + 0x0052d9c5u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(mode << 2, 0x0052d9c5u));
    gp4::call_regs(GP4_ARRAY(uint32_t, 0x0052d9c5)[mode], c);
    GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052d99f, FUN_0052d99f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0052dba4+bitmap_0052d6ed")
