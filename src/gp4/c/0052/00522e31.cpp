#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mirror of 0x00522e0e for the bytes with both bits 7 and 6 set: when exactly one of the 22
// per-driver bytes qualifies (count in edx from 0x00522da0), takes it (0x00522dec) and, if it still
// reads as such, runs 0x00522cda with dl = [0x007a93b2] to insert it into the 22-byte list at
// 0x00675958. Registers and flags are those the last routine left.
// FUNCTION: GP4 0x00522e31
void __cdecl FUN_00522e31(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00522da0, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, 1u));
    if (c.edx == 1u) {
        gp4::call_regs(0x00522dec, c);
        const uint8_t al = static_cast<uint8_t>(c.eax);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(al & 0x40u));
        if ((al & 0x40u) != 0u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(al));
            if ((al & 0x80u) != 0u) {
                c.edx = (c.edx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007a93b2);
                gp4::call_regs(0x00522cda, c);
            }
        }
    }
    *r = c;
}
GP4_IMPL(0x00522e31, FUN_00522e31, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+order_00675958")
