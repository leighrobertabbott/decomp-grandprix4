#include <gp4/gp4.h>
#include <gp4/flags.h>

// Selects the current record with 0x00531803 (ebx = its address), then asks 0x005318d1 for the
// clamped value in eax (edx = the unclamped one) and stores it in dword +0x18 of that record.
// ebx stays the record address (0x005318d1 restores the registers it pushes); the flags are those of
// the last compare inside 0x005318d1.
// FUNCTION: GP4 0x00530737
void __cdecl FUN_00530737(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00531803, c);
    gp4::call_regs(0x005318d1, c);
    GP4_FIELD(uint32_t, c.ebx, 0x18) = c.eax;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00530737, FUN_00530737, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00531859+rsrec_007ad638+recip_005318d1")
