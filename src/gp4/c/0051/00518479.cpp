#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x0051887c only when both gate bytes are zero: [0x0088f6e1] (`test byte, 0xff`) and then
// [0x007ad868] (`cmp byte, 0`). Registers and flags are those of the last compare, or of the callee
// when it runs.
// FUNCTION: GP4 0x00518479
void __cdecl FUN_00518479(gp4::Regs* r) {
    const uint32_t first = GP4_GLOBAL(uint8_t, 0x0088f6e1);
    uint32_t arith = gp4::flags::logic8(first);
    if (first == 0u) {
        const uint32_t second = GP4_GLOBAL(uint8_t, 0x007ad868);
        arith = gp4::flags::sub8(second, 0u);
        if (second == 0u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eflags = gp4::flags::merge(c.eflags, arith);
            gp4::call_regs(0x0051887c, c);
            *r = c;
            return;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00518479, FUN_00518479, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
