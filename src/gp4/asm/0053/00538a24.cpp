#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the running total dword 0x0082fcb0 by eax. When the addition carries out of 32 bits the
// table reset routine 0x0053872f runs instead (after the empty 0x0054e232, called inside a
// pushal/popal) and the total is left to it; CF is returned set. Otherwise the total is stored and
// CF returned clear, the other flags being those of
// `cmp total, [0x829868+0x14] + [0x829868+0x10]`. ebx and edx are saved and restored; eax is
// unchanged.
// FUNCTION: GP4 0x00538a24
void __cdecl FUN_00538a24(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t previous = GP4_GLOBAL(uint32_t, 0x0082fcb0);
    const uint32_t total = previous + eax;
    const uint32_t addFlags = gp4::flags::add32(previous, eax);

    if (total < previous) {
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(r->eflags, addFlags);
        gp4::call_regs(0x0054e232, c);

        gp4::Regs d = *r;
        d.eflags = gp4::flags::merge(r->eflags, addFlags);
        gp4::call_regs(0x0053872f, d);
        r->eflags = gp4::flags::merge(r->eflags, (d.eflags & gp4::flags::ARITH) | gp4::flags::CF);
    } else {
        uint8_t* record = GP4_ARRAY(uint8_t, 0x00829868);
        uint32_t limit = GP4_FIELD(uint32_t, record, 0x14);
        limit += GP4_FIELD(uint32_t, record, 0x10);
        const uint32_t cmpFlags = gp4::flags::sub32(total, limit);
        GP4_GLOBAL(uint32_t, 0x0082fcb0) = total;
        r->eflags = gp4::flags::merge(r->eflags, cmpFlags & ~gp4::flags::CF);
    }
}
GP4_IMPL(0x00538a24, FUN_00538a24, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
