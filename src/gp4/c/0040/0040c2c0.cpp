#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c2c0
// pushal ... popal: when the track is not being warmed up ([0x0062ac99] != 0xff) and the surface
// target level [0x00622584], clamped to 0..11, differs from the current one [0x00622588], stores it
// there, resets the 10-byte entries of every record of the 0x160-byte ring at 0x008943a0 (lap ring
// up to [0x00866ecc], then the pit ring [0x00891500]..[0x00866ef8]) with 0x40b629 + 0x51f2e9, ends
// with 0x40c0ae and sets the surface kind [0x0062258c] to 5. If the target is nonzero it then runs
// up to 0x40 rounds of 0x40bd5f over both rings (+ 0x40c0ae) until the high nibble of the byte
// [0x00622529] equals the target. popal restores every register; every exit leaves the flags of an
// equal compare / "or ecx, ecx" with ecx == 0, i.e. ZF = PF = 1 and the rest clear.
void __cdecl FUN_0040c2c0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if (GP4_GLOBAL(uint8_t, 0x0062ac99) != 0xff) {
        int32_t target = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00622584));
        if (target > 0xb) {
            target = 0xb;
        }
        if (target < 0) {
            target = 0;
        }
        c.eax = static_cast<uint32_t>(target);
        if (c.eax != GP4_GLOBAL(uint32_t, 0x00622588)) {
            GP4_GLOBAL(uint32_t, 0x00622588) = c.eax;

            c.edi = 0x008943a0u;
            do {
                gp4::call_regs(0x0040b629, c);
                gp4::call_regs(0x0051f2e9, c);
                c.edi += 0x160u;
            } while (c.edi < GP4_GLOBAL(uint32_t, 0x00866ecc));
            c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
            do {
                gp4::call_regs(0x0040b629, c);
                gp4::call_regs(0x0051f2e9, c);
                c.edi += 0x160u;
            } while (c.edi < GP4_GLOBAL(uint32_t, 0x00866ef8));
            gp4::call_regs(0x0040c0ae, c);
            GP4_GLOBAL(uint32_t, 0x0062258c) = 5u;

            if (GP4_GLOBAL(uint32_t, 0x00622584) != 0u) {
                uint32_t rounds = 0x40u;
                for (;;) {
                    c.ecx = rounds;
                    c.edi = 0x008943a0u;
                    do {
                        gp4::call_regs(0x0040bd5f, c);
                        c.edi += 0x160u;
                    } while (c.edi < GP4_GLOBAL(uint32_t, 0x00866ecc));
                    c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
                    do {
                        gp4::call_regs(0x0040bd5f, c);
                        c.edi += 0x160u;
                    } while (c.edi < GP4_GLOBAL(uint32_t, 0x00866ef8));
                    gp4::call_regs(0x0040c0ae, c);
                    rounds -= 1u;
                    c.ecx = rounds;
                    c.eax = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x00622529)) >> 4;
                    if (c.eax == GP4_GLOBAL(uint32_t, 0x00622584) || rounds == 0u) {
                        break;
                    }
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040c2c0, FUN_0040c2c0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+map_0040c43b+ring_0040c0ae+state_0040bec7+state_0040bd5f+state_0040bce6")
