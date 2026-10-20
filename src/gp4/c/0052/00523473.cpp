#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the control-state bytes when the slot search comes back empty: nothing happens (flags of
// `cmp [0x7c2e72], 0xff`) while the marker byte [0x7c2e72] is 0xff. Otherwise, with [0x852d8c] zero, the
// four state bytes 0x852d9d / 0x852d9e / 0x852d9b / 0x852da3 are set to 0x80; the slot search 0x00520c08
// then runs (registers and flags as it leaves them) and, if it reports failure (CF set), the two bytes
// 0x852d9f / 0x852da0 are set to 0x80 as well.
// FUNCTION: GP4 0x00523473
void __cdecl FUN_00523473(gp4::Regs* r) {
    const uint8_t marker = GP4_GLOBAL(uint8_t, 0x007c2e72);
    if (marker == 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(marker, 0xffu));
        return;
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00852d8c);
    if (mode == 0u) {
        GP4_GLOBAL(uint8_t, 0x00852d9d) = 0x80u;
        GP4_GLOBAL(uint8_t, 0x00852d9e) = 0x80u;
        GP4_GLOBAL(uint8_t, 0x00852d9b) = 0x80u;
        GP4_GLOBAL(uint8_t, 0x00852da3) = 0x80u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(mode));
    gp4::call_regs(0x00520c08, c);
    if ((c.eflags & gp4::flags::CF) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00852d9f) = 0x80u;
        GP4_GLOBAL(uint8_t, 0x00852da0) = 0x80u;
    }

    r->eax = c.eax;
    r->ebp = c.ebp;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00523473, FUN_00523473, "regs() -> (eax, ebp, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584+rows_00520c08+scan_00520c08")
