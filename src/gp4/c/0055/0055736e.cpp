#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 2 only the flags of the dead
// `add esi, 0` change. In mode 2 it stores ebx into 0x00866ed4 when cl == 0x20, else into 0x00866ed0;
// the flags are those of `cmp cl, 0x20`.
// FUNCTION: GP4 0x0055736e
void __cdecl FUN_0055736e(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
        return;
    }
    const uint8_t sel = static_cast<uint8_t>(r->ecx);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(sel, 0x20u));
    if (sel == 0x20u) {
        GP4_GLOBAL(uint32_t, 0x00866ed4) = r->ebx;
    } else {
        GP4_GLOBAL(uint32_t, 0x00866ed0) = r->ebx;
    }
}
GP4_IMPL(0x0055736e, FUN_0055736e, "regs(ebx:u32, ecx:u32{0x20,0x20,0,0x120,0xffffff20,5}, esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
