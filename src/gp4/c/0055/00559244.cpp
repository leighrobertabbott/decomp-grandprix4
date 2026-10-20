#include <gp4/gp4.h>
#include <gp4/flags.h>

// `test byte [0x874b4f], 0x10 ; jne skip ; call 0x00559253 ; ret`: when bit 4 of the byte at
// 0x00874b4f is clear, 0x00559253 masks the flag bytes of the record ebx with dh and sets the flags;
// otherwise nothing happens and the flags are those of the `test`.
// FUNCTION: GP4 0x00559244
void __cdecl FUN_00559244(gp4::Regs* r) {
    const uint32_t masked = GP4_GLOBAL(uint8_t, 0x00874b4f) & 0x10u;
    if (masked != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(masked));
        return;
    }
    gp4::call_regs(0x00559253, *r);
}
GP4_IMPL(0x00559244, FUN_00559244, "regs(ebx:ptr[0x44]:bytes, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=flags_00874b3e")
