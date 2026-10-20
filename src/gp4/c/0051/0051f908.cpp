#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: al = byte [0x00a69878 + 0x007c2a48] (a car number of the slot table), then
// 0x00408359 searches the 22 car records for it. eax (the record address, or the unchanged eax with the
// new al when none matches) is stored in 0x00a6987c. Every GPR is restored; the flags are those
// 0x00408359 leaves.
// FUNCTION: GP4 0x0051f908
void __cdecl FUN_0051f908(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = index;
    c.eax = (r->eax & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x007c2a48)[index];
    gp4::call_regs(0x00408359, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f908, FUN_0051f908, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+carno_0066ff6c+idx_005162fd")
