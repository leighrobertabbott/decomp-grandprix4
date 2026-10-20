#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x0054cce5. Stores eax at 0x00833cd7 - unless the byte [0x00ae6740] is zero, in which
// case it first repacks eax into a 5-bit low field plus the bits 5..14 doubled
// (eax = (eax & 0x1f) | ((eax & 0x7fe0) << 1)) and also loads ebx with the doubled high part.
// Then, after the empty routine 0x00531b02, stores ebx, ecx, edx and their doubles into six
// consecutive unaligned dwords 0x00833cdb... pushal/popal keeps every register; the flags are
// those of the last `add edx, edx`.
// FUNCTION: GP4 0x0054cc96
void __cdecl FUN_0054cc96(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    if (GP4_GLOBAL(uint8_t, 0x00ae6740) == 0) {
        ebx = (eax & 0x7fe0u) << 1;
        eax = (eax & 0x1fu) | ebx;
    }
    GP4_GLOBAL(uint32_t, 0x00833cd7) = eax;

    gp4::Regs c = *r;
    c.eax = eax;
    c.ebx = ebx;
    gp4::call_regs(0x00531b02, c);

    const uint32_t b = c.ebx;
    const uint32_t cx = c.ecx;
    const uint32_t d = c.edx;
    GP4_GLOBAL(uint32_t, 0x00833cdb) = b;
    GP4_GLOBAL(uint32_t, 0x00833cdf) = cx;
    GP4_GLOBAL(uint32_t, 0x00833ce3) = d;

    const uint32_t flags = gp4::flags::add32(d, d);
    GP4_GLOBAL(uint32_t, 0x00833ce7) = b + b;
    GP4_GLOBAL(uint32_t, 0x00833ceb) = cx + cx;
    GP4_GLOBAL(uint32_t, 0x00833cef) = d + d;

    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x0054cc96, FUN_0054cc96, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
