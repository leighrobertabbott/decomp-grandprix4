#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00527095
// Takes the ring record pointer edi = [0x00679b78]. At or above the pit start [0x00891500]
// (unsigned) it is stored in [0x00679d28] and [0x00679d20] gets the record's cursor from
// 0x00405da5; below it the pointer goes to [0x00679d20] and [0x00679d28] gets the result of
// 0x00405d74. [0x00679d24] then copies [0x00679d20] (also left in eax). Hand-written register
// code: eax = [0x00679d20], edi = the record pointer; the flags are those of `cmp edi, [0x891500]`
// on the 0x00405da5 path (that routine keeps them) and the flags 0x00405d74 leaves on the other.
void __cdecl FUN_00527095(gp4::Regs* r) {
    const uint32_t edi = GP4_GLOBAL(uint32_t, 0x00679b78);
    gp4::Regs c = {};
    c.edi = edi;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x00891500)));

    if (edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        GP4_GLOBAL(uint32_t, 0x00679d28) = edi;
        gp4::call_regs(0x00405da5, c);
        GP4_GLOBAL(uint32_t, 0x00679d20) = c.eax;
    } else {
        GP4_GLOBAL(uint32_t, 0x00679d20) = edi;
        gp4::call_regs(0x00405d74, c);
        GP4_GLOBAL(uint32_t, 0x00679d28) = c.eax;
    }
    const uint32_t eax = GP4_GLOBAL(uint32_t, 0x00679d20);
    GP4_GLOBAL(uint32_t, 0x00679d24) = eax;

    r->eax = eax;
    r->edi = edi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00527095, FUN_00527095, "regs() -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+lanes_0040da62+ptr_00679b78")
