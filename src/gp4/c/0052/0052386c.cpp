#include <gp4/gp4.h>
#include <gp4/flags.h>

// Countdown poll: while the due time [0x00852db4] is still in the future of the clock [0x0066d814] (signed)
// nothing happens and CF is set. Otherwise the due time is cleared and the two countdown bytes are stepped:
// first [0x00852d9e] and then [0x00852d9d] (each only when nonzero). A byte that is decremented but is still
// nonzero ends the poll with CF set; a byte that reaches zero runs 0x005188a6 and continues. Returns CF clear
// when both countdowns are exhausted or idle. eax is the due time loaded for the compare; the flags are those
// of the last flag writer before the stc/clc (CF replaced).
// FUNCTION: GP4 0x0052386c
void __cdecl FUN_0052386c(gp4::Regs* r) {
    const uint32_t due = GP4_GLOBAL(uint32_t, 0x00852db4);
    r->eax = due;
    uint32_t arith = gp4::flags::sub32(due, GP4_GLOBAL(uint32_t, 0x0066d814));
    if (static_cast<int32_t>(due) > GP4_GLOBAL(int32_t, 0x0066d814)) {
        r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00852db4) = 0u;

    const uint32_t outer = GP4_GLOBAL(uint8_t, 0x00852d9e);
    arith = gp4::flags::logic8(outer);
    if (outer != 0u) {
        const uint32_t next = (outer - 1u) & 0xffu;
        GP4_GLOBAL(uint8_t, 0x00852d9e) = static_cast<uint8_t>(next);
        arith = gp4::flags::sub8(outer, 1u) & ~gp4::flags::CF;
        if (next != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
            return;
        }
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x005188a6, c);
        arith = c.eflags;
    }

    const uint32_t inner = GP4_GLOBAL(uint8_t, 0x00852d9d);
    arith = gp4::flags::logic8(inner);
    if (inner != 0u) {
        const uint32_t next = (inner - 1u) & 0xffu;
        GP4_GLOBAL(uint8_t, 0x00852d9d) = static_cast<uint8_t>(next);
        arith = gp4::flags::sub8(inner, 1u) & ~gp4::flags::CF;
        if (next != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
            return;
        }
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x005188a6, c);
        arith = c.eflags;
    }

    r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
}
GP4_IMPL(0x0052386c, FUN_0052386c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0051d607")
