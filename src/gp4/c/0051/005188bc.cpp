#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sound update dispatcher. Nothing happens while the byte 0x0088f6e1 is set. Otherwise, when the byte
// 0x007ad868 is clear it runs the pushal wrapper 0x00518868; when it is set it releases every sound
// channel (0x00518ba7 with eax = -1, inside pushal / push ds,es,fs,gs) and clears the byte 0x00852d97.
// All registers are restored (popal does not restore the flags), so only the flags change: those of
// `test byte [0x88f6e1], 0xff`, of `cmp byte [0x7ad868], 0` or the ones the callee leaves.
// FUNCTION: GP4 0x005188bc
void __cdecl FUN_005188bc(gp4::Regs* r) {
    uint32_t flags = gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x0088f6e1));
    if (GP4_GLOBAL(uint8_t, 0x0088f6e1) == 0u) {
        const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007ad868);
        flags = gp4::flags::sub8(mode, 0u);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        if (mode == 0u) {
            gp4::call_regs(0x00518868, c);
        } else {
            c.eax = 0xffffffffu;
            gp4::call_regs(0x00518ba7, c);
            GP4_GLOBAL(uint8_t, 0x00852d97) = 0u;
        }
        flags = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005188bc, FUN_005188bc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; gfields={0x0088f6e1:u8{=0,1}, 0x007ad868:u8{=0,1}}")
