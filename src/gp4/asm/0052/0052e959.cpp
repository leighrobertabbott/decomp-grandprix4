#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks al up in the table of inclusive byte ranges {low, high} at 0x00826cdb, which ends at the first
// pair whose low byte is zero (the first pair is tested before that check). Inside pushal/popal, so only
// the flags survive: CF = 0 when al lies in a range (the other flags are those of the `cmp al, high`),
// CF = 1 when the end was reached (the other flags are those of `cmp byte [esi], 0` of a zero byte:
// ZF = PF = 1).
// FUNCTION: GP4 0x0052e959
void __cdecl FUN_0052e959(gp4::Regs* r) {
    const uint8_t* entry = GP4_ARRAY(uint8_t, 0x00826cdb);
    const uint32_t al = r->eax & 0xffu;
    uint32_t arith;

    for (;;) {
        if (al >= entry[0] && al <= entry[1]) {
            arith = gp4::flags::sub8(al, entry[1]) & ~static_cast<uint32_t>(gp4::flags::CF);
            break;
        }
        entry += 2;
        if (entry[0] == 0u) {
            arith = gp4::flags::sub8(entry[0], 0u) | static_cast<uint32_t>(gp4::flags::CF);
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052e959, FUN_0052e959, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
