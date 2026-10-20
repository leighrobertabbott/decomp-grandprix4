#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push ebx/ecx/esi/edi/ebp ; ... ; pop ...`: 0x00531847 tests the bitmap bit of the current selector.
// With the bit set 0x00531803 selects the record (ebx) and the fixed-point reciprocal 0x004126e2 of
// its dword +0x18 is taken; otherwise 0x00531859 looks the selector up in the device rows. The value
// is returned in edx and, clamped to [dword 0x007a8dca, dword 0x007a8dce] (lower bound first), in
// eax. The flags are those of the last `cmp eax, [0x7a8dce]`; the pushed registers are restored.
// FUNCTION: GP4 0x005318d1
void __cdecl FUN_005318d1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00531847, c);

    if ((c.eflags & gp4::flags::CF) != 0u) {
        gp4::call_regs(0x00531803, c);
        c.eax = GP4_FIELD(uint32_t, c.ebx, 0x18);
        gp4::call_regs(0x004126e2, c);
    } else {
        gp4::call_regs(0x00531859, c);
    }

    uint32_t value = c.eax;
    const uint32_t edx = value;
    if (value < GP4_GLOBAL(uint32_t, 0x007a8dca)) {
        value = GP4_GLOBAL(uint32_t, 0x007a8dca);
    }
    const uint32_t flags = gp4::flags::sub32(value, GP4_GLOBAL(uint32_t, 0x007a8dce));
    if (value > GP4_GLOBAL(uint32_t, 0x007a8dce)) {
        value = GP4_GLOBAL(uint32_t, 0x007a8dce);
    }

    r->eax = value;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005318d1, FUN_005318d1, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00531859+rsrec_007ad638+recip_005318d1")
