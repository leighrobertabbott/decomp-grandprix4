#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte 0x0086857c is 2, stores ebx at 0x008684f0 (flags of the `cmp byte [..], 2`);
// otherwise only the flags of the dead `add esi, 0` change.
// FUNCTION: GP4 0x00557346
void __cdecl FUN_00557346(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0086857c);
    if (mode == 2u) {
        GP4_GLOBAL(uint32_t, 0x008684f0) = r->ebx;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 2u));
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
}
GP4_IMPL(0x00557346, FUN_00557346, "regs(ebx:u32, esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
