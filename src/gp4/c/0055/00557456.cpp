#include <gp4/gp4.h>
#include <gp4/flags.h>

// Empty case of the mode byte 0x0086857c: nothing is stored whatever the mode is (the sibling 0x00557292
// files a record when the mode is 2). The flags are those of `cmp byte [0x86857c], 2` when the mode is 2,
// otherwise those of the dead `add esi, 0`. No register is changed.
// FUNCTION: GP4 0x00557456
void __cdecl FUN_00557456(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0086857c);
    if (mode == 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 2u));
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
}
GP4_IMPL(0x00557456, FUN_00557456, "regs(esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
