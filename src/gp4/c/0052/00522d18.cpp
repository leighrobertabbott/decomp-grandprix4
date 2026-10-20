#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over the driver-slot counts of 0x00522da0: when the byte [0x007c2e74] is nonzero
// and both counts (eax: bit 7 set / bit 6 clear, edx: both bits set) are at most 1 it returns with
// CF set (stc), otherwise with CF clear (clc). popal restores every register; the other flags are
// those of the last `test`/`cmp`.
// FUNCTION: GP4 0x00522d18
void __cdecl FUN_00522d18(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007c2e74);
    uint32_t flags = gp4::flags::logic8(enabled);
    uint32_t carry = 0u;
    if (enabled != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00522da0, c);
        flags = gp4::flags::sub32(c.eax, 1u);
        if (c.eax <= 1u) {
            flags = gp4::flags::sub32(c.edx, 1u);
            if (c.edx <= 1u) {
                carry = 1u;
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x00522d18, FUN_00522d18, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023")
