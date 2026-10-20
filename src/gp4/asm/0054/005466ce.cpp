#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws one scaled GDI line segment: the start is (eax * [0x00b1bf80], ebx * [0x00b1bf40] / 0x50) and the end
// is ((eax + ecx) * [0x00b1bf80], the same y). The y is divided signed by 0x50; all products keep the low 32
// bits. It calls the stdcall MoveToEx wrapper 0x005085e0 (y, x) inside an inner pushal/popal, then the stdcall
// LineTo wrapper 0x00508610 (y, x + width) with the live registers. The outer popal restores every register;
// the flags are those the LineTo wrapper leaves.
// FUNCTION: GP4 0x005466ce
void __cdecl FUN_005466ce(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;

    const uint32_t widthEnd = (ecx + eax) * GP4_GLOBAL(uint32_t, 0x00b1bf80);
    const uint32_t xStart = eax * GP4_GLOBAL(uint32_t, 0x00b1bf80);
    const int32_t yScaled = static_cast<int32_t>(ebx * GP4_GLOBAL(uint32_t, 0x00b1bf40));
    int32_t remainder;
    const int32_t y = gp4::idiv64(static_cast<int64_t>(yScaled), 0x50, &remainder);

    gp4::Regs c = *r;
    c.eax = xStart;
    c.ebx = static_cast<uint32_t>(y);
    c.ecx = widthEnd;
    c.edx = static_cast<uint32_t>(remainder);
    c.esi = 0x50u;
    const uint32_t moveTo[2] = {static_cast<uint32_t>(y), xStart};
    gp4::call_regs_stdcall(0x005085e0, c, moveTo);

    c.eax = xStart;
    c.ebx = static_cast<uint32_t>(y);
    c.ecx = widthEnd;
    c.edx = static_cast<uint32_t>(remainder);
    c.esi = 0x50u;
    const uint32_t lineTo[2] = {static_cast<uint32_t>(y), widthEnd};
    gp4::call_regs_stdcall(0x00508610, c, lineTo);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005466ce, FUN_005466ce, "regs(eax:i32, ebx:i32, ecx:i32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gdi_007ab53c")
