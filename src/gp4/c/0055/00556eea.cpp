#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte at 0x0086857c is 1 the flags are those of `cmp byte [0x86857c], 1`; for any
// other mode the routine falls into `add esi, 0` and returns the flags of that add (esi unchanged).
// FUNCTION: GP4 0x00556eea
void __cdecl FUN_00556eea(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0086857c);
    uint32_t flags;
    if (mode == 1u) {
        flags = gp4::flags::sub8(mode, 1u);
    } else {
        flags = gp4::flags::add32(r->esi, 0u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00556eea, FUN_00556eea, "regs(esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
