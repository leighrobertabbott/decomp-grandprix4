#include <gp4/gp4.h>
#include <gp4/x87.h>

// Same refresh as 0x00433f30 (two float constants from 0x0051faee scaled by the doubles at
// 0x006f2af4 and 0x00601400, rounded to 0x00630224 / 0x00630228 through the scratch dword
// 0x006f2bf4; three table pointers cached; selectors rebuilt by 0x00433faa), but the selector byte
// 0x00852d8c is forced to 0x80 for the 0x00433faa call (a word store, so 0x00852d8d is cleared) and
// then restored. eax leaves with the low byte replaced by the old selector (push/pop eax), edx is
// the second rounded value; the flags are those left by 0x00433faa.
// FUNCTION: GP4 0x00433ea1
void __cdecl FUN_00433ea1(gp4::Regs* r) {
    using namespace gp4::x87;

    gp4::Regs c = *r;
    gp4::call_regs(0x0051faee, c);

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = c.eax;
    const Ext first = ext_f32(GP4_GLOBAL(float, 0x006f2bf4))
        * ext(GP4_GLOBAL(double, 0x006f2af4))
        * ext(GP4_GLOBAL(double, 0x00601400));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(first);
    c.eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    GP4_GLOBAL(uint32_t, 0x00630224) = c.eax;

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = c.edx;
    const Ext second = ext_f32(GP4_GLOBAL(float, 0x006f2bf4))
        * ext(GP4_GLOBAL(double, 0x006f2af4))
        * ext(GP4_GLOBAL(double, 0x00601400));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(second);
    c.edx = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    GP4_GLOBAL(uint32_t, 0x00630228) = c.edx;

    gp4::call_regs(0x00434515, c);
    GP4_GLOBAL(uint32_t, 0x0062ff58) = c.eax;
    gp4::call_regs(0x0043452b, c);
    GP4_GLOBAL(uint32_t, 0x0062ff5c) = c.eax;
    gp4::call_regs(0x00434541, c);
    GP4_GLOBAL(uint32_t, 0x0062ff60) = c.eax;

    const uint32_t savedEax = (c.eax & ~0xffu) | GP4_GLOBAL(uint8_t, 0x00852d8c);
    GP4_GLOBAL(uint16_t, 0x00852d8c) = 0x80;
    gp4::call_regs(0x00433faa, c);
    GP4_GLOBAL(uint8_t, 0x00852d8c) = static_cast<uint8_t>(savedEax);
    c.eax = savedEax;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00433ea1, FUN_00433ea1, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00433faa")
