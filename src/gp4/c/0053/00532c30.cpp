#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-table lookup by row: ecx (clamped to 4) selects the row 0xac0 - 0x44 * ecx of the string table
// relative to the current circuit number [0x007ad894]; 0x0054d1ad turns that index into the table
// dword (a string pointer) and it is returned in eax and esi. `mul ecx` with eax = 0xffffffbc leaves
// edx = ecx - 1 (0 for ecx = 0); ecx ends clamped. The flags are those 0x0054d1ad leaves.
// FUNCTION: GP4 0x00532c30
void __cdecl FUN_00532c30(gp4::Regs* r) {
    uint32_t count = r->ecx;
    if (count > 4u) {
        count = 4u;
    }
    const uint32_t low = 0u - 0x44u * count;
    const uint32_t high = (count == 0u) ? 0u : (count - 1u);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ecx = count;
    c.eax = low;
    c.edx = high;
    c.esi = GP4_GLOBAL(uint32_t, 0x007ad894) + 0xac0u + low;
    gp4::call_regs(0x0054d1ad, c);

    r->eax = c.esi;
    r->ecx = count;
    r->edx = high;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00532c30, FUN_00532c30, "regs(ecx:u32[0..8]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894")
