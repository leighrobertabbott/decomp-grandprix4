#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that turns three "wanted" bitmap bits (0x00510e04, bitmap at 0x008245a0) into edge
// events. For the key [0x00852dec], then (only while [0x00852d8c] is zero) the keys 0x1b and 0x52, a
// held key sets its mask bit (0x40 / 0x20 / 0x10) in the state byte [0x008914e1] and toggles the same
// bit in the event byte [0x008914e0] when it was not set before; a released key clears the state bit.
// A newly pressed 0x1b sets bit 4 of [0x007c2ead]; when that byte is non-zero afterwards the event byte
// also gets bit 0x20. popal restores every register, so only the flags remain: those of the last
// flag writer (the `test` of [0x852d8c] / [0x7c2ead], or the `or` of the event byte).
static __declspec(noinline) bool KeyHeld(gp4::Regs& c, uint32_t key) {
    c.ecx = (c.ecx & 0xffffff00u) | key;
    gp4::call_regs(0x00510e04, c);
    return (c.eflags & gp4::flags::ZF) != 0u;
}

// FUNCTION: GP4 0x0051f0c7
void __cdecl FUN_0051f0c7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x008914e0) = 0;

    if (KeyHeld(c, GP4_GLOBAL(uint8_t, 0x00852dec))) {
        if ((GP4_GLOBAL(uint8_t, 0x008914e1) & 0x40u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x008914e0) ^= 0x40u;
            GP4_GLOBAL(uint8_t, 0x008914e1) |= 0x40u;
        }
    } else {
        GP4_GLOBAL(uint8_t, 0x008914e1) &= 0xbfu;
    }

    uint32_t flags = gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x00852d8c));
    if (GP4_GLOBAL(uint8_t, 0x00852d8c) == 0u) {
        bool pressed = false;
        if (KeyHeld(c, 0x1bu)) {
            if ((GP4_GLOBAL(uint8_t, 0x008914e1) & 0x20u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x008914e0) ^= 0x20u;
                GP4_GLOBAL(uint8_t, 0x008914e1) |= 0x20u;
                pressed = true;
            }
        } else {
            GP4_GLOBAL(uint8_t, 0x008914e1) &= 0xdfu;
        }
        if (pressed) {
            GP4_GLOBAL(uint8_t, 0x007c2ead) |= 0x10u;
        }

        if (KeyHeld(c, 0x52u)) {
            if ((GP4_GLOBAL(uint8_t, 0x008914e1) & 0x10u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x008914e0) ^= 0x10u;
                GP4_GLOBAL(uint8_t, 0x008914e1) |= 0x10u;
            }
        } else {
            GP4_GLOBAL(uint8_t, 0x008914e1) &= 0xefu;
        }

        flags = gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x007c2ead));
        if (GP4_GLOBAL(uint8_t, 0x007c2ead) != 0u) {
            GP4_GLOBAL(uint8_t, 0x008914e0) |= 0x20u;
            flags = gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x008914e0));
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f0c7, FUN_0051f0c7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0")
