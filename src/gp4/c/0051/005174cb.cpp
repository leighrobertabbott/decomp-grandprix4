#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005174cb
// Raises one request flag according to the mode byte [0x7c2ea9]: bit 7 sets [0xa4b368]; mode 2 and bit 0x40
// first claim bit 6 of the bitmap through 0x0051d187 (eax = 6) and, when that succeeds (CF = 0), set [0xa4b374]
// (mode 2) or [0xa4b378] (bit 0x40, after testing the gate byte [0x66d8a1]); every other mode sets [0xa4b36c]
// when [0x7c2e75] is 0xff, else [0xa4b370]. eax is only changed by the claim call (it becomes 6); the flags are
// those of the last flag writer on the taken path.
void __cdecl FUN_005174cb(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t arith = gp4::flags::logic8(mode);
    if ((mode & 0x80u) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00a4b368) = 1u;
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }
    bool gated = false;
    arith = gp4::flags::logic8(mode & 0x40u);
    if ((mode & 0x40u) == 0u) {
        arith = gp4::flags::sub8(mode, 2u);
        if (mode != 2u) {
            const uint8_t skip = GP4_GLOBAL(uint8_t, 0x007c2e75);
            arith = gp4::flags::sub8(skip, 0xffu);
            if (skip == 0xffu) {
                GP4_GLOBAL(uint32_t, 0x00a4b36c) = 1u;
            } else {
                GP4_GLOBAL(uint32_t, 0x00a4b370) = 1u;
            }
            r->eflags = gp4::flags::merge(r->eflags, arith);
            return;
        }
    } else {
        gated = true;
    }

    gp4::Regs c = *r;
    c.eax = 6u;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0051d187, c);
    r->eax = c.eax;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        r->eflags = c.eflags;
        return;
    }
    if (gated) {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x0066d8a1)));
        GP4_GLOBAL(uint32_t, 0x00a4b378) = 1u;
    } else {
        GP4_GLOBAL(uint32_t, 0x00a4b374) = 1u;
    }
    r->eflags = c.eflags;
}
GP4_IMPL(0x005174cb, FUN_005174cb, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_005174cb")
