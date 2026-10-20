#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the array walker 0x0052ab04, then (unless the byte [0x0081d1e8] is 0xff) copies the first four bytes
// of the 0xf28-byte record [0x007c2f28] of the array at 0x007adb4c (address from 0x0052a254) to 0x0081d1d0
// inside a pushal/popal block, and runs the walker again. The registers come back unchanged and the flags
// are those of the closing 0x0052ab04.
// FUNCTION: GP4 0x0052a030
void __cdecl FUN_0052a030(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0052ab04, c);

    if (GP4_GLOBAL(uint8_t, 0x0081d1e8) != 0xffu) {
        gp4::Regs d = c;
        d.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081d1e8), 0xffu));
        d.eax = GP4_GLOBAL(uint32_t, 0x007c2f28);
        gp4::call_regs(0x0052a254, d);
        const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(d.eax));
        uint8_t* target = GP4_ARRAY(uint8_t, 0x0081d1d0);
        for (uint32_t i = 0; i != 4u; ++i) {
            target[i] = source[i];
        }
    }

    gp4::call_regs(0x0052ab04, c);
    *r = c;
}
GP4_IMPL(0x0052a030, FUN_0052a030, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ent_0052a0ec")
