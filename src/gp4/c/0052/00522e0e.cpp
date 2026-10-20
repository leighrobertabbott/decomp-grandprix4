#include <gp4/gp4.h>
#include <gp4/flags.h>

// When exactly one of the 22 per-driver bytes has bit 7 set and bit 6 clear (count in eax from
// 0x00522da0), takes that byte (0x00522dca) and, if it still reads as such, runs 0x00522cda with
// dl = [0x007ab756] to insert it into the 22-byte list at 0x00675958. Registers and flags are those
// the last routine left.
// FUNCTION: GP4 0x00522e0e
void __cdecl FUN_00522e0e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00522da0, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
    if (c.eax == 1u) {
        gp4::call_regs(0x00522dca, c);
        const uint8_t al = static_cast<uint8_t>(c.eax);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(al & 0x40u));
        if ((al & 0x40u) == 0u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(al));
            if ((al & 0x80u) != 0u) {
                c.edx = (c.edx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007ab756);
                gp4::call_regs(0x00522cda, c);
            }
        }
    }
    *r = c;
}
GP4_IMPL(0x00522e0e, FUN_00522e0e, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023+order_00675958")
