#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item event handler keyed by edx. 2: pushal wrapper that runs the item activation 0x0052d5f8, takes the
// letter of the (index 0x824712)-th set bit of the drive mask (0x0052f408) and, when it differs from the first
// byte of the current path buffer 0x81e7f2, remembers it at 0x826e3f, stores the old path under its drive
// (0x0052f44d), loads the path of the new drive (0x0052f4b8) and sets the byte 0x82fd5f to 0xff. popal brings
// every register back except edx = 5; the flags stay those of the last flag writer (no popal restore):
// `cmp [0x81e7f2], al` when the letter is unchanged, else the flags 0x0052f4b8 leaves.
// 5 or 6: clears the dword 0x82fd88. Every other value does nothing.
// FUNCTION: GP4 0x0052f310
void __cdecl FUN_0052f310(gp4::Regs* r) {
    const uint32_t event = r->edx;
    uint32_t flags;
    if (event == 2u) {
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
        gp4::call_regs(0x0052d5f8, c);
        c.eax = GP4_GLOBAL(uint32_t, 0x00824712);
        gp4::call_regs(0x0052f408, c);
        const uint32_t letter = c.eax & 0xffu;
        const uint32_t current = GP4_GLOBAL(uint8_t, 0x0081e7f2);
        flags = gp4::flags::sub8(current, letter);
        if (current != letter) {
            GP4_GLOBAL(uint8_t, 0x00826e3f) = (uint8_t)letter;
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0081e7f2);
            c.eflags = gp4::flags::merge(c.eflags, flags);
            gp4::call_regs(0x0052f44d, c);
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00826e3f);
            gp4::call_regs(0x0052f4b8, c);
            GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
            flags = c.eflags;
        }
        r->edx = 5u;
    } else if (event == 5u) {
        flags = gp4::flags::sub32(event, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        flags = gp4::flags::sub32(event, 6u);
        if (event == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f310, FUN_0052f310, "regs(edx:u32{=2,5,6,0,1,3,7}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8+drives_0082474b+kind_0052f44d+letters_0052f429")
